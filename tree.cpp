#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tree.h"
#include "difftor.h"

node_t* node_ctor(enum node_type type, const char* data)
{
    node_t* new_node = (node_t*)calloc(1, sizeof(node_t));
    if (!new_node) return NULL;

    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;
    new_node->type = type;

    switch (type)
    {
        case NUMBER:
            new_node->data.number = atof(data);
            break;

        case OPERATOR:
            new_node->data.oper = get_func(data);
            break;

        case VARIABLE:
            new_node->data.variable = strdup(data);
            break;
    }

    return new_node;
}

enum funcs get_func(const char* data)
{
    if (!strcmp(data, "cos")) return COS;
    if (!strcmp(data, "sin")) return SIN;
    if (!strcmp(data, "ln"))  return LN;
    if (!strcmp(data, "exp")) return EXP;
    if (!strcmp(data, "-"))   return SUB;
    if (!strcmp(data, "*"))   return MUL;
    if (!strcmp(data, "/"))   return DIV;
    if (!strcmp(data, "^"))   return POW;
    if (!strcmp(data, "+"))   return ADD;
    return INC_FUNC;
}

void tree_dtor(node_t* node)
{
    if (node == NULL)
        return;

    tree_dtor(node->left);
    tree_dtor(node->right);

    if (node->type == VARIABLE && node->data.variable)
        free(node->data.variable);

    free(node);
}

char* cp1251_to_utf8(const char* cp1251_str)
{
    assert (cp1251_str);

    size_t input_len = strlen(cp1251_str);
    char* utf8_str = (char*)calloc(input_len * 2 + 1, sizeof(char));
    if (!utf8_str)
        return NULL;

    size_t output_pos = 0;

    for (size_t i = 0; i < input_len; i++)
    {
        unsigned char c = (unsigned char)cp1251_str[i];

        if (c < 0x80)
            utf8_str[output_pos++] = c;

        else if (c >= 0xC0 && c <= 0xDF)
        {
            utf8_str[output_pos++] = (char)0xD0;
            utf8_str[output_pos++] = (char)(0x90 + (c - 0xC0));
        }

        else if (c >= 0xE0 && c <= 0xEF)
        {
            utf8_str[output_pos++] = (char)0xD0;
            utf8_str[output_pos++] = (char)(0xB0 + (c - 0xE0));
        }

        else if (c >= 0xF0 && c < 0xFF)
        {
            utf8_str[output_pos++] = (char)0xD1;
            utf8_str[output_pos++] = (char)(0x80 + (c - 0xF0));
        }

        else
            utf8_str[output_pos++] = c;
    }

    utf8_str[output_pos] = '\0';

    return utf8_str;
}
