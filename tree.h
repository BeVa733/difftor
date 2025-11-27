#ifndef TREE_H
#define TREE_H

const int MAX_VARS = 100;
const int STR_MAX_LEN = 1000;
const double LOW_NUMBER = 1e-5;
const int N_FUNCS = 9;

struct var_t
{
    char* name;
    double value;
};

struct read_info
{
    char* buffer;
    char** cur_pos;
    var_t** variables;
    int* var_count;
};

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

struct node_t
{
    union data_member data;
    node_t* left;
    node_t* right;
    node_t* parent;
    enum node_type type;
};

struct funcs_info
{
    enum funcs enum_name;
    char str_name[STR_MAX_LEN];
    void (*tex_func)(FILE* tex_file, node_t* node);
    double (*calc_func)(node_t* node, var_t** variables, int var_count, bool need_var_value);
    node_t* (*diff_func)(node_t* node, const char* var);
    bool (*simplify_func)(node_t* node);
};

node_t* node_ctor(enum node_type type, const char* data);
void tree_dtor(node_t* node);
enum funcs get_func(const char* data);

void print_add_latex(FILE* tex_file, node_t* node);
void print_sub_latex(FILE* tex_file, node_t* node);
void print_mul_latex(FILE* tex_file, node_t* node);
void print_div_latex(FILE* tex_file, node_t* node);
void print_pow_latex(FILE* tex_file, node_t* node);
void print_sin_latex(FILE* tex_file, node_t* node);
void print_cos_latex(FILE* tex_file, node_t* node);
void print_ln_latex (FILE* tex_file, node_t* node);
void print_exp_latex(FILE* tex_file, node_t* node);

double calc_add(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_sub(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_mul(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_div(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_pow(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_sin(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_cos(node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_ln (node_t* node, var_t** variables, int var_count, bool need_var_value);
double calc_exp(node_t* node, var_t** variables, int var_count, bool need_var_value);

node_t* diff_add(node_t* node, const char* var);
node_t* diff_sub(node_t* node, const char* var);
node_t* diff_mul(node_t* node, const char* var);
node_t* diff_div(node_t* node, const char* var);
node_t* diff_pow(node_t* node, const char* var);
node_t* diff_sin(node_t* node, const char* var);
node_t* diff_cos(node_t* node, const char* var);
node_t* diff_ln (node_t* node, const char* var);
node_t* diff_exp(node_t* node, const char* var);

bool simplify_add(node_t* node);
bool simplify_sub(node_t* node);
bool simplify_mul(node_t* node);
bool simplify_div(node_t* node);
bool simplify_pow(node_t* node);
bool simplify_sin(node_t* node);
bool simplify_cos(node_t* node);
bool simplify_ln (node_t* node);
bool simplify_exp(node_t* node);

#ifdef DIFFTOR_CPP
funcs_info funcs[N_FUNCS] =
{
    {ADD, "+"  , print_add_latex, calc_add, diff_add, simplify_add},
    {SUB, "-"  , print_sub_latex, calc_sub, diff_sub, simplify_sub},
    {MUL, "*"  , print_mul_latex, calc_mul, diff_mul, simplify_mul},
    {DIV, "/"  , print_div_latex, calc_div, diff_div, simplify_div},
    {POW, "^"  , print_pow_latex, calc_pow, diff_pow, simplify_pow},
    {SIN, "sin", print_sin_latex, calc_sin, diff_sin, simplify_sin},
    {COS, "cos", print_cos_latex, calc_cos, diff_cos, simplify_cos},
    {LN,  "ln" , print_ln_latex , calc_ln , diff_ln , simplify_ln },
    {EXP, "exp", print_exp_latex, calc_exp, diff_exp, simplify_exp}
};
#else
extern funcs_info funcs[N_FUNCS];
#endif

#endif
