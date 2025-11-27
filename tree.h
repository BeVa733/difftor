#ifndef TREE_H
#define TREE_H

enum funcs
{
    INC_FUNC = -1,
    ADD      = 0,
    SUB      = 1,
    MUL      = 2,
    DIV      = 3,
    POW      = 4,
    SIN      = 5,
    COS      = 6,
    LN       = 7,
    EXP      = 8
};

union data_member
{
    double number;
    enum funcs oper;
    char* variable;
};

enum node_type
{
    NUMBER   = 0,
    OPERATOR = 1,
    VARIABLE = 2
};

// struct fncs_info
// {
//     enum funcs enum_name;
//     char str[STR_MAX_LEN];
//     bool left_son;
//     bool right_son
// };

struct node_t
{
    union data_member data;
    node_t* left;
    node_t* right;
    node_t* parent;
    enum node_type type;
};

node_t* node_ctor(enum node_type type, const char* data);
void tree_dtor(node_t* node);
enum funcs get_func(const char* data);

#endif
