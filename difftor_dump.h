#ifndef DIFFTOR_DUMP_H
#define DIFFTOR_DUMP_H

void tree_graph_to_file(node_t* node, const char* filename_prefix);
void tree_dot(FILE* dot_file, node_t* node);

FILE* tex_dump_init(const char* filename_prefix);
void tex_dump_append_tree(node_t* node, FILE* tex_file);
void tree_tex(FILE* tex_file, node_t* node);
void make_tex_preamble(FILE* tex_file);
void tex_dump_compile(FILE* tex_file, const char* filename_prefix);


#endif
