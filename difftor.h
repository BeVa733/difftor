#ifndef DIFFTOR_H
#define DIFFTOR_H

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

node_t* read_file       (const char* filename, var_t** variables, int* var_count);
long int check_file_size(FILE* file);
void skip_spaces        (char** curr_pos);
bool is_variable_unique (read_info* read_data, const char* var_name);
node_t* set_word_node   (read_info* read_data, const char* word);

node_t* get_g  (read_info* read_data);
node_t* get_n  (read_info* read_data);
node_t* get_e  (read_info* read_data);
node_t* get_t  (read_info* read_data);
node_t* get_p  (read_info* read_data);
node_t* get_w  (read_info* read_data);
node_t* get_pow(read_info* read_data);


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
