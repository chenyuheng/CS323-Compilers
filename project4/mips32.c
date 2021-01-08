#include "mips32.h"

/* the output file descriptor, may not be explicitly used */
FILE *fd;
Register current_reg;
int var_offset;

#define _tac_kind(tac) (((tac)->code).kind)
#define _tac_quadruple(tac) (((tac)->code).tac)
#define _reg_name(reg) regs[reg].name


void _mips_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
    fputs("\n", fd);
}

void _mips_iprintf(const char *fmt, ...){
    va_list args;
    fputs("  ", fd); // `iprintf` stands for indented printf
    va_start(args, fmt);
    vfprintf(fd, fmt, args);
    va_end(args);
    fputs("\n", fd);
}

// RETURN NULL if not found
struct VarDesc* search_VarDesc(char* var_name) {
    //printf("search: %s\n", var_name);
    struct VarDesc* temp = vars;
    while (temp != NULL) {
        if (!strcmp(var_name, temp->var)) {
            //printf("found: %s\n", temp->var);
            return temp;}
        temp = temp->next;
    }
    //printf("not found\n");

    return NULL;
}

struct VarDesc* add_to_vars(char* var_name) {
    // create new VarDesc
    struct VarDesc* new_VarDesc = (struct VarDesc*) malloc(sizeof(struct VarDesc));
    strcpy(new_VarDesc->var, var_name);
    new_VarDesc->offset = var_offset + 4;
    var_offset += 4;
    _mips_iprintf("addi %s, %s, %d", _reg_name(sp), _reg_name(sp), -4);
    // add to the end of vars
    struct VarDesc* temp = vars;
    while (temp->next != NULL) temp = temp->next;
    temp->next = new_VarDesc;
    return new_VarDesc;
}

Register get_register(tac_opd *opd){
    assert(opd->kind == OP_VARIABLE);
    char *var = opd->char_val;
    /* COMPLETE the register allocation */
    struct VarDesc* varDesc = search_VarDesc(var);
    assert(varDesc != NULL);
    if (varDesc->reg == zero) {
        _mips_iprintf("lw %s, %d(%s)", _reg_name(current_reg), -varDesc->offset, _reg_name(fp));
        strcpy(regs[current_reg].var, var);
        varDesc->reg = current_reg++;
    }
    return varDesc->reg;
}

Register get_register_w(tac_opd *opd){
    assert(opd->kind == OP_VARIABLE);
    char *var = opd->char_val;
    /* COMPLETE the register allocation (for write) */
    struct VarDesc* varDesc = search_VarDesc(var);
    if (varDesc != NULL) {        
        if (varDesc->reg == zero) {
            strcpy(regs[current_reg].var, var);
            varDesc->reg = current_reg++;
        } else {
            __assert_fail;
        }
    } else {
        strcpy(regs[current_reg].var, var);
        //printf("test %d, %s\n", var_offset, var);
        struct VarDesc* new_VarDesc = add_to_vars(var);
        new_VarDesc->reg = current_reg;
        //_mips_iprintf("lw %s, %d(%s)//get register w, varDesc == NULL", regs[current_reg].name, new_VarDesc->offset, regs[sp].name);
        return current_reg++;
    }
    return varDesc->reg;
}

void spill_register(Register reg){
    /* COMPLETE the register spilling */
            //printf("====%s\n", regs[reg].var);
    struct VarDesc* varDesc = search_VarDesc(regs[reg].var);
    _mips_iprintf("sw %s, %d(%s)", _reg_name(reg), -varDesc->offset, _reg_name(fp));
}

void spill_registers() {
    Register reg = t0;
    while (strcmp(regs[reg].var, "")) {
        search_VarDesc(regs[reg].var)->reg = zero;
        spill_register(reg);
        strcpy(regs[reg].var, "");
        reg++;
    }
    current_reg = t0;
}





/* PARAM: a pointer to `struct tac_node` instance
   RETURN: the next instruction to be translated */
tac *emit_label(tac *label){
    assert(_tac_kind(label) == LABEL);
    _mips_printf("label%d:", _tac_quadruple(label).labelno->int_val);
    return label->next;
}

tac *emit_function(tac *function){
    _mips_printf("%s:", _tac_quadruple(function).funcname);
    _mips_iprintf("subu %s, %s, %d", _reg_name(sp), _reg_name(sp), 8);
    _mips_iprintf("sw %s, %d(%s)", _reg_name(ra), 4, _reg_name(sp));
    _mips_iprintf("sw %s, %d(%s)", _reg_name(fp), 0, _reg_name(sp));
    _mips_iprintf("move $fp, $sp");
    var_offset = 4;
    return function->next;
}

tac *emit_assign(tac *assign){
    Register x, y;

    x = get_register_w(_tac_quadruple(assign).left);
    if(_tac_quadruple(assign).right->kind == OP_CONSTANT){
        _mips_iprintf("li %s, %d", _reg_name(x),
                                   _tac_quadruple(assign).right->int_val);
    }
    else{
        y = get_register(_tac_quadruple(assign).right);
        _mips_iprintf("move %s, %s", _reg_name(x), _reg_name(y));
    }
    spill_registers();
    return assign->next;
}

tac *emit_add(tac *add){
    Register x, y, z;

    x = get_register_w(_tac_quadruple(add).left);
    if(_tac_quadruple(add).r1->kind == OP_CONSTANT){
        y = get_register(_tac_quadruple(add).r2);
        _mips_iprintf("addi %s, %s, %d", _reg_name(x),
                                         _reg_name(y),
                                         _tac_quadruple(add).r1->int_val);
    }
    else if(_tac_quadruple(add).r2->kind == OP_CONSTANT){
        y = get_register(_tac_quadruple(add).r1);
        _mips_iprintf("addi %s, %s, %d", _reg_name(x),
                                         _reg_name(y),
                                         _tac_quadruple(add).r2->int_val);
    }
    else{
        y = get_register(_tac_quadruple(add).r1);
        z = get_register(_tac_quadruple(add).r2);
        _mips_iprintf("add %s, %s, %s", _reg_name(x),
                                        _reg_name(y),
                                        _reg_name(z));
    }
    spill_registers();
    return add->next;
}

tac *emit_sub(tac *sub){
    Register x, y, z;

    x = get_register_w(_tac_quadruple(sub).left);
    if(_tac_quadruple(sub).r1->kind == OP_CONSTANT){
        y = get_register(_tac_quadruple(sub).r2);
        _mips_iprintf("neg %s, %s", _reg_name(y), _reg_name(y));
        _mips_iprintf("addi %s, %s, %d", _reg_name(x),
                                         _reg_name(y),
                                         _tac_quadruple(sub).r1->int_val);
    }
    else if(_tac_quadruple(sub).r2->kind == OP_CONSTANT){
        y = get_register(_tac_quadruple(sub).r1);
        _mips_iprintf("addi %s, %s, -%d", _reg_name(x),
                                          _reg_name(y),
                                          _tac_quadruple(sub).r2->int_val);
    }
    else{
        y = get_register(_tac_quadruple(sub).r1);
        z = get_register(_tac_quadruple(sub).r2);
        _mips_iprintf("sub %s, %s, %s", _reg_name(x),
                                        _reg_name(y),
                                        _reg_name(z));
    }
    spill_registers();
    return sub->next;
}

tac *emit_mul(tac *mul){
    Register x, y, z;

    x = get_register_w(_tac_quadruple(mul).left);
    if(_tac_quadruple(mul).r1->kind == OP_CONSTANT){
        y = get_register_w(_tac_quadruple(mul).r1);
        z = get_register(_tac_quadruple(mul).r2);
        _mips_iprintf("lw %s, %d", _reg_name(y),
                                   _tac_quadruple(mul).r1->int_val);
    }
    else if(_tac_quadruple(mul).r2->kind == OP_CONSTANT){
        y = get_register(_tac_quadruple(mul).r1);
        z = get_register_w(_tac_quadruple(mul).r2);
        _mips_iprintf("lw %s, %d", _reg_name(z),
                                   _tac_quadruple(mul).r2->int_val);
    }
    else{
        y = get_register(_tac_quadruple(mul).r1);
        z = get_register(_tac_quadruple(mul).r2);
    }
    _mips_iprintf("mul %s, %s, %s", _reg_name(x),
                                    _reg_name(y),
                                    _reg_name(z));
    spill_registers();
    return mul->next;
}

tac *emit_div(tac *div){
    Register x, y, z;

    x = get_register_w(_tac_quadruple(div).left);
    if(_tac_quadruple(div).r1->kind == OP_CONSTANT){
        y = get_register_w(_tac_quadruple(div).r1);
        z = get_register(_tac_quadruple(div).r2);
        _mips_iprintf("lw %s, %d", _reg_name(y),
                                   _tac_quadruple(div).r1->int_val);
    }
    else if(_tac_quadruple(div).r2->kind == OP_CONSTANT){
        y = get_register(_tac_quadruple(div).r1);
        z = get_register_w(_tac_quadruple(div).r2);
        _mips_iprintf("lw %s, %d", _reg_name(z),
                                   _tac_quadruple(div).r2->int_val);
    }
    else{
        y = get_register(_tac_quadruple(div).r1);
        z = get_register(_tac_quadruple(div).r2);
    }
    _mips_iprintf("div %s, %s", _reg_name(y), _reg_name(z));
    _mips_iprintf("mflo %s", _reg_name(x));
    spill_registers();
    return div->next;
}

tac *emit_addr(tac *addr){
    Register x, y;

    x = get_register_w(_tac_quadruple(addr).left);
    y = get_register(_tac_quadruple(addr).right);
    _mips_iprintf("move %s, %s", _reg_name(x), _reg_name(y));
    return addr->next;
}

tac *emit_fetch(tac *fetch){
    Register x, y;

    x = get_register_w(_tac_quadruple(fetch).left);
    y = get_register(_tac_quadruple(fetch).raddr);
    _mips_iprintf("lw %s, 0(%s)", _reg_name(x), _reg_name(y));
    return fetch->next;
}

tac *emit_deref(tac *deref){
    Register x, y;

    x = get_register(_tac_quadruple(deref).laddr);
    y = get_register(_tac_quadruple(deref).right);
    _mips_iprintf("sw %s, 0(%s)", _reg_name(y), _reg_name(x));
    return deref->next;
}

tac *emit_goto(tac *goto_){
    _mips_iprintf("j label%d", _tac_quadruple(goto_).labelno->int_val);
    return goto_->next;
}

tac *emit_iflt(tac *iflt){
    /* COMPLETE emit function */
    return iflt->next;
}

tac *emit_ifle(tac *ifle){
    /* COMPLETE emit function */
    return ifle->next;
}

tac *emit_ifgt(tac *ifgt){
    /* COMPLETE emit function */
    return ifgt->next;
}

tac *emit_ifge(tac *ifge){
    /* COMPLETE emit function */
    return ifge->next;
}

tac *emit_ifne(tac *ifne){
    /* COMPLETE emit function */
    return ifne->next;
}

tac *emit_ifeq(tac *ifeq){
    /* COMPLETE emit function */
    return ifeq->next;
}

tac *emit_return(tac *return_){
    /* COMPLETE emit function */
    _mips_iprintf("jr %s", _reg_name(ra));
    return return_->next;
}

tac *emit_dec(tac *dec){
    /* NO NEED TO IMPLEMENT */
    return dec->next;
}

tac *emit_arg(tac *arg){
    /* COMPLETE emit function */
    return arg->next;
}

tac *emit_call(tac *call){
    /* COMPLETE emit function */
    return call->next;
}

tac *emit_param(tac *param){
    /* COMPLETE emit function */
    return param->next;
}

tac *emit_read(tac *read){
    Register x = get_register_w(_tac_quadruple(read).p);

    _mips_iprintf("addi $sp, $sp, -4");
    _mips_iprintf("sw $ra, 0($sp)");
    _mips_iprintf("jal read");
    _mips_iprintf("lw $ra, 0($sp)");
    _mips_iprintf("addi $sp, $sp, 4");
    _mips_iprintf("move %s, $v0", _reg_name(x));
    return read->next;
}

tac *emit_write(tac *write){
    Register x = get_register(_tac_quadruple(write).p);

    _mips_iprintf("move $a0, %s", _reg_name(x));
    _mips_iprintf("addi $sp, $sp, -4");
    _mips_iprintf("sw $ra, 0($sp)");
    _mips_iprintf("jal write");
    _mips_iprintf("lw $ra, 0($sp)");
    _mips_iprintf("addi $sp, $sp, 4");
    return write->next;
}

void emit_preamble(){
    _mips_printf("# SPL compiler generated assembly");
    _mips_printf(".data");
    _mips_printf("_prmpt: .asciiz \"Enter an integer: \"");
    _mips_printf("_eol: .asciiz \"\\n\"");
    _mips_printf(".globl main");
    _mips_printf(".text");
}

void emit_read_function(){
    _mips_printf("read:");
    _mips_iprintf("li $v0, 4");
    _mips_iprintf("la $a0, _prmpt");
    _mips_iprintf("syscall");
    _mips_iprintf("li $v0, 5");
    _mips_iprintf("syscall");
    _mips_iprintf("jr $ra");
}

void emit_write_function(){
    _mips_printf("write:");
    _mips_iprintf("li $v0, 1");
    _mips_iprintf("syscall");
    _mips_iprintf("li $v0, 4");
    _mips_iprintf("la $a0, _eol");
    _mips_iprintf("syscall");
    _mips_iprintf("move $v0, $0");
    _mips_iprintf("jr $ra");
}

static tac* (*emitter[])(tac*) = {
    emit_label, emit_function, emit_assign,
    emit_add, emit_sub, emit_mul, emit_div,
    emit_addr, emit_fetch, emit_deref, emit_goto,
    emit_iflt, emit_ifle, emit_ifgt, emit_ifge, emit_ifne, emit_ifeq,
    emit_return, emit_dec, emit_arg, emit_call, emit_param,
    emit_read, emit_write
};

tac *emit_code(tac *head){
    tac *(*tac_emitter)(tac*);
    tac *tac_code = head;
    emit_preamble();
    emit_read_function();
    emit_write_function();
    while(tac_code != NULL){
        if(_tac_kind(tac_code) != NONE){
            tac_emitter = emitter[_tac_kind(tac_code)];
            tac_code = tac_emitter(tac_code);
        }
        else{
            tac_code = tac_code->next;
        }
    }
}

/* translate a TAC list into mips32 assembly
   output the textual assembly code to _fd */
void mips32_gen(tac *head, FILE *_fd){
    regs[zero].name = "$zero";
    regs[at].name = "$at";
    regs[v0].name = "$v0"; regs[v1].name = "$v1";
    regs[a0].name = "$a0"; regs[a1].name = "$a1";
    regs[a2].name = "$a2"; regs[a3].name = "$a3";
    regs[t0].name = "$t0"; regs[t1].name = "$t1";
    regs[t2].name = "$t2"; regs[t3].name = "$t3";
    regs[t4].name = "$t4"; regs[t5].name = "$t5";
    regs[t6].name = "$t6"; regs[t7].name = "$t7";
    regs[s0].name = "$s0"; regs[s1].name = "$s1";
    regs[s2].name = "$s2"; regs[s3].name = "$s3";
    regs[s4].name = "$s4"; regs[s5].name = "$s5";
    regs[s6].name = "$s6"; regs[s7].name = "$s7";
    regs[t8].name = "$t8"; regs[t9].name = "$t9";
    regs[k0].name = "$k0"; regs[k1].name = "$k1";
    regs[gp].name = "$gp";
    regs[sp].name = "$sp"; regs[fp].name = "$fp";
    regs[ra].name = "$ra";
    vars = (struct VarDesc*)malloc(sizeof(struct VarDesc));
    vars->next = NULL;
    fd = _fd;
    current_reg = t0;
    var_offset = 4;
    emit_code(head);
}
