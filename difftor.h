#ifndef DIFFTOR_H
#define DIFFTOR_H

const int MAX_VARS = 100;
const int STR_MAX_LEN = 1000;
const double LOW_NUMBER = 1e-5;

struct var_t
{
    char* name;
    double value;
};

#define ADD_(left_son, right_son) \
    create_operator_node("+", left_son, right_son)

#define SUB_(left_son, right_son) \
    create_operator_node("-", left_son, right_son)

#define MUL_(left_son, right_son) \
    create_operator_node("*", left_son, right_son)

#define DIV_(left_son, right_son) \
    create_operator_node("/", left_son, right_son)

#define POW_(left_son, right_son) \
    create_operator_node("^", left_son, right_son)

node_t* read_file(const char* filename);
long int check_file_size(FILE* file);
void skip_spaces(char** curr_pos);

node_t* get_g  (char** cur_pos);
node_t* get_n  (char** cur_pos);
node_t* get_e  (char** cur_pos);
node_t* get_t  (char** cur_pos);
node_t* get_p  (char** cur_pos);
node_t* get_w  (char** cur_pos);
node_t* get_pow(char** cur_pos);

node_t* set_word_node(char** cur_pos, const char* word);

// char* read_name(char** curr_pos);
// enum node_type get_type(const char* data);

double tree_sum(node_t* node, var_t** variables, int var_count, bool need_var_value = false);
void request_variable_values(var_t** variables, int var_count);
var_t* var_ctor(char* name, double value);
double get_variable_value(const char* var_name, var_t** variables, int var_count);
void var_dtor(var_t** var, int var_count);

node_t* tree_diff(node_t* node, const char* var);
node_t* tree_copy(node_t* node);
node_t* make_new_node_with_sons(enum node_type type, const char* data, node_t* left_son, node_t* right_son);
node_t* create_operator_node(const char* oper, node_t* left, node_t* right);
node_t* create_number_node(double value);

bool is_variable_in_tree (node_t* node, const char* var = "all");

bool simplify_tree(node_t* node);
bool constant_folding (node_t* node);
bool delete_extra_nodes(node_t* node);
bool is_del_extra(node_t* node);
bool is_tree_const(node_t* node);
void node_to_number(node_t* node, double value);
void replace_with_child(node_t* node, node_t* child);

#endif
