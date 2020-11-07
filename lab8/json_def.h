typedef struct JsonObject {
    enum {OBJ, ARR, STR, NUM, BOO, VNU} category;
    union {
        struct ObjectMember *members;
        struct ArrayValue *values;
        char *string;
        double number;
        int boolean;
    };
} JsonObject;

typedef struct ObjectMember {
    char *key;
    struct JsonObject *value;
    struct ObjectMember *next;
} ObjectMember;

typedef struct ArrayValue {
    struct JsonObject *value;
    struct ArrayValue *next;                                       
} ArrayValue;

