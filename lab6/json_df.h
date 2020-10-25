typedef struct Json {
    struct Value *value;
} Json;

typedef struct Value {
    union {
        struct Object *object;
        struct Array *array;
        char *string;
        double number;
        enum {TRUE, FALSE} boolean;
        enum {VNULL} vnull;
    };
} Value;

typedef struct Object {
    struct Member **member;
} Object;

typedef struct Member {
    char *key;
    struct Value *value;
} Member;

typedef struct Array {
    struct Value **base;
} Array;
