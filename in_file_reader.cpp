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

node_t* read_file(const char* filename)
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

    char* cur_pos = buffer;

    node_t* head = get_g(&cur_pos);

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

node_t* get_g(char** cur_pos)
{
    skip_spaces(cur_pos);
    char* buffer = *cur_pos;

    node_t* tree = get_e(cur_pos);
    skip_spaces(cur_pos);

    if(**cur_pos != ';')
    {
        printf("syntax error in position %ld\n \"%c\"\n", *cur_pos - buffer, **cur_pos);
    }

    return tree;
}

node_t* get_n(char** cur_pos)
{
    double val = 0;

    while ('0' <= **cur_pos && **cur_pos <= '9')
    {
        val = **cur_pos - '0' + val * 10;
        (*cur_pos)++;
    }

    if(**cur_pos == '.')
    {
        char* start = *cur_pos;
        (*cur_pos)++;

        int after_dot = 0;
        while ('0' <= **cur_pos && **cur_pos <= '9')
        {
            after_dot = **cur_pos - '0' + after_dot * 10;
            (*cur_pos)++;
        }

        int num_len = start - *cur_pos + 1;

        val = val + after_dot * pow(10, num_len);
    }

    return create_number_node(val);
}

node_t* get_e(char** cur_pos)
{
    node_t* tree = get_t(cur_pos);

    while(**cur_pos == '+' || **cur_pos == '-')
    {
        char op = **cur_pos;

        (*cur_pos)++;
        skip_spaces(cur_pos);

        node_t* subtree = get_t(cur_pos);

        if (op == '+')
            tree = ADD_(tree, subtree);

        else
            tree = SUB_(tree, subtree);
    }

    return tree;
}

node_t* get_t(char** cur_pos)
{
    node_t* tree = get_pow(cur_pos);

    while(**cur_pos == '*' || **cur_pos == '/')
    {
        char op = **cur_pos;

        (*cur_pos)++;
        skip_spaces(cur_pos);

        node_t* subtree = get_pow(cur_pos);

        if (op == '*')
            tree =  MUL_(tree, subtree);

        else
            tree = DIV_(tree, subtree);
    }

    return tree;
}

node_t* get_p(char** cur_pos)
{
    node_t* tree = NULL;

    if (**cur_pos == '(')
    {
        (*cur_pos)++;
        skip_spaces(cur_pos);

        tree = get_e(cur_pos);

        if(**cur_pos == ')')
        {
            (*cur_pos)++;
            skip_spaces(cur_pos);
        }

        else
            printf("Syntax Error: expected )");
    }

    else if (isdigit(**cur_pos))
    {

        tree = get_n(cur_pos);
        skip_spaces(cur_pos);

        if (tree == NULL)
            printf("Syntax Error\n");
    }

    else
    {
        tree = get_w(cur_pos);
    }

    return tree;
}

node_t* get_pow(char** cur_pos)
{
    node_t* tree = get_p(cur_pos);

    while(**cur_pos == '^')
    {
        char op = **cur_pos;

        (*cur_pos)++;
        skip_spaces(cur_pos);

        node_t* subtree = get_p(cur_pos);


        tree =  POW_(tree, subtree);
    }

    return tree;
}

node_t* get_w(char** cur_pos)
{
    node_t* tree = NULL;

    char word[STR_MAX_LEN] = "";
    char* start = *cur_pos;

    while(isalpha(**cur_pos))
    {
        (*cur_pos)++;
    }

    if(start == *cur_pos)
        printf("Syntax error\n\"%c\"", *start);

    else
    {
        int word_len = (int)(*cur_pos - start);
        strncpy(word, start, word_len);
        word[word_len + 1] = '\0';

        tree = set_word_node(cur_pos, word);
    }

    return tree;
}

node_t* set_word_node(char** cur_pos, const char* word)
{
    enum funcs func = get_func(word);

    if (func != INC_FUNC)
    {
        node_t* subtree = get_p(cur_pos);

        return create_operator_node(word, NULL, subtree);
    }

    else
        return node_ctor(VARIABLE, word);
}

void skip_spaces(char** curr_pos)
{
    while (**curr_pos == ' ' || **curr_pos == '\t' || **curr_pos == '\n')
        (*curr_pos)++;
}
