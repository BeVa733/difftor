#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <assert.h>

#include "tree.h"
#include "difftor.h"
#include "difftor_dump.h"

node_t* read_file(const char* filename, var_t** variables, int* var_count)
{
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        printf("file opening error\n");
        return NULL;
    }

    long int file_size = check_file_size(file);
    if (file_size <= 0)
    {
        fclose(file);
        return NULL;
    }

    char* buffer = (char*)calloc(file_size + 1, sizeof(char));
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(buffer, sizeof(char), file_size, file);
    buffer[read_size] = '\0';
    fclose(file);

    char* curr_pos = buffer;

    read_info read_data = {};

    read_data.cur_pos = &curr_pos;
    read_data.buffer = buffer;
    read_data.variables = variables;
    read_data.var_count = var_count;

    node_t* head = get_g(&read_data);

    free(buffer);
    return head;
}

long int check_file_size(FILE* file)
{
    struct stat file_info = {};
    int fd = fileno(file);
    if (fstat(fd, &file_info)== -1)
    {
        printf("ERROR: check file size is incorrect\n");
        return -1;
    }

    return file_info.st_size;
}

node_t* get_g(read_info* read_data)
{
    skip_spaces(read_data->cur_pos);

    node_t* tree = get_e(read_data);
    skip_spaces(read_data->cur_pos);

    if(**(read_data->cur_pos) != ';')
    {
        printf("syntax error in position %ld\n \"%c\"\n", *(read_data->cur_pos) - read_data->buffer, **(read_data->cur_pos));
    }

    return tree;
}

node_t* get_n(read_info* read_data)
{
    skip_spaces((read_data->cur_pos));
    double val = 0;

    while ('0' <= **(read_data->cur_pos) && **(read_data->cur_pos) <= '9')
    {
        val = **(read_data->cur_pos) - '0' + val * 10;
        (*(read_data->cur_pos))++;
    }

    if(**(read_data->cur_pos) == '.')
    {
        char* start = *(read_data->cur_pos);
        (*(read_data->cur_pos))++;

        int after_dot = 0;
        while ('0' <= **(read_data->cur_pos) && **(read_data->cur_pos) <= '9')
        {
            after_dot = **(read_data->cur_pos) - '0' + after_dot * 10;
            (*(read_data->cur_pos))++;
        }

        int num_len = start - *(read_data->cur_pos) + 1;

        val = val + after_dot * pow(10, num_len);
    }

    return create_number_node(val);
}

node_t* get_e(read_info* read_data)
{
    skip_spaces((read_data->cur_pos));
    node_t* tree = get_t(read_data);

    while(**(read_data->cur_pos) == '+' || **(read_data->cur_pos) == '-')
    {
        char op = **(read_data->cur_pos);
        skip_spaces((read_data->cur_pos));

        (*(read_data->cur_pos))++;
        skip_spaces((read_data->cur_pos));

        node_t* subtree = get_t(read_data);

        if (op == '+')
            tree = ADD_(tree, subtree);

        else
            tree = SUB_(tree, subtree);
    }

    return tree;
}

node_t* get_t(read_info* read_data)
{
    skip_spaces((read_data->cur_pos));
    node_t* tree = get_pow(read_data);

    while(**(read_data->cur_pos) == '*' || **(read_data->cur_pos) == '/')
    {
        char op = **(read_data->cur_pos);

        (*(read_data->cur_pos))++;
        skip_spaces((read_data->cur_pos));

        node_t* subtree = get_pow(read_data);

        if (op == '*')
            tree =  MUL_(tree, subtree);

        else
            tree = DIV_(tree, subtree);
    }

    return tree;
}

node_t* get_p(read_info* read_data)
{
    skip_spaces((read_data->cur_pos));
    node_t* tree = NULL;

    if (**(read_data->cur_pos) == '(')
    {
        (*(read_data->cur_pos))++;
        skip_spaces((read_data->cur_pos));

        tree = get_e(read_data);
        skip_spaces((read_data->cur_pos));

        if(**(read_data->cur_pos) == ')')
        {
            (*(read_data->cur_pos))++;
            skip_spaces((read_data->cur_pos));
        }

        else
            printf("Syntax Error: expected ) in position %ld", *(read_data->cur_pos) - read_data->buffer);
    }

    else if (isdigit(**(read_data->cur_pos)))
    {

        tree = get_n(read_data);
        skip_spaces((read_data->cur_pos));

        if (tree == NULL)
            printf("Syntax Error in position %ld\n", *(read_data->cur_pos) - read_data->buffer);
    }

    else
    {
        tree = get_w(read_data);
        skip_spaces((read_data->cur_pos));
    }

    return tree;
}

node_t* get_pow(read_info* read_data)
{
    skip_spaces((read_data->cur_pos));
    node_t* tree = get_p(read_data);

    while(**(read_data->cur_pos) == '^')
    {
        char op = **(read_data->cur_pos);

        (*(read_data->cur_pos))++;
        skip_spaces((read_data->cur_pos));

        node_t* subtree = get_p(read_data);

        tree =  POW_(tree, subtree);
    }

    return tree;
}

node_t* get_w(read_info* read_data)
{
    skip_spaces(read_data->cur_pos);
    node_t* tree = NULL;

    char word[STR_MAX_LEN] = "";
    char* start = *(read_data->cur_pos);

    while(isalpha(**(read_data->cur_pos)))
    {
        (*(read_data->cur_pos))++;
    }

    if(start == *(read_data->cur_pos))
        printf("Syntax error in position %ld\n\"%c\"", *(read_data->cur_pos) - read_data->buffer, *start);

    else
    {
        int word_len = (int)(*(read_data->cur_pos) - start);
        strncpy(word, start, word_len);
        word[word_len] = '\0';

        tree = set_word_node(read_data, word);
    }

    return tree;
}

node_t* set_word_node(read_info* read_data, const char* word)
{
    enum funcs func = get_func(word);

    if (func != INC_FUNC)
    {
        skip_spaces((read_data->cur_pos));
        node_t* subtree = get_p(read_data);
        skip_spaces((read_data->cur_pos));

        return create_operator_node(word, NULL, subtree);
    }

    else
    {
        if (is_variable_unique(read_data, word))
        {
            var_t* new_var = (var_t*)calloc(1, sizeof(var_t));
            if (!new_var)
            {
                printf("Memory allocation error for variable\n");
                return NULL;
            }

            new_var->name = strdup(word);
            new_var->value = NAN;

            read_data->variables[(*read_data->var_count)] = new_var;
            (*read_data->var_count)++;
        }

        return node_ctor(VARIABLE, word);
    }
}

bool is_variable_unique(read_info* read_data, const char* var_name)
{
    for (int i = 0; i < *read_data->var_count; i++)
    {
        if (strcmp(read_data->variables[i]->name, var_name) == 0)
            return false;
    }
    return true;
}

void skip_spaces(char** curr_pos)
{
    while (**curr_pos == ' ' || **curr_pos == '\t' || **curr_pos == '\n')
        (*curr_pos)++;
}
