#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef union stack_value
{
    uint32_t number;
    char string[4];
} stack_value;

typedef struct stack_node
{
    struct stack_node *next;
    stack_value value;
}stack_node;

typedef struct func_node
{
    struct func_node *prev;
    fpos_t back_pos;
}func_node;

stack_node *new_node(stack_value value)
{
    stack_node *node = malloc(sizeof(stack_node));
    node->next = NULL;
    node->value = value;
    return node;
}

func_node *new_func_node(fpos_t pos)
{
    func_node *node = malloc(sizeof(func_node));
    node->prev = NULL;
    node->back_pos = pos;
    return node;
}

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Usage: orange file.ore\n");
        return 0;
    }

    FILE *file = fopen(argv[1], "r");
    if(!file)
    {
        printf("Couldn't open file: %s\n", argv[1]);
        return 0;
    }

    stack_node *current_node = NULL;
    func_node *current_func_node = NULL;

    fpos_t funcpos;
    bool func_active = false;

    fpos_t looppos;
    bool loop_active = true;

    stack_value value;
    value.number = 0;
    while(1)
    {
        int command = fgetc(file);
        if(feof(file)) break;
        switch (command)
        {
            case '+': value.number++; break;
            case '-': value.number--; break;
            case '>':
            {
                stack_node *new_one = new_node(value);
                new_one->next = current_node;
                current_node = new_one;
            }
            break;
            case '<':
            {
                if(!current_node)
                {
                    printf("Couldn't pop stack because there's no any values!\n");
                    return 0;
                }

                value = current_node->value;
                stack_node *next = current_node->next;
                free(current_node);
                current_node = next;
            }
            break;
            case '!':
            {
                char printable[5] = {0};
                memcpy(printable, value.string, 4);
                printf("%s", printable);
            }
            break;
            case '?': 
            {
                fread(&value, 4, 1, stdin);
            }
            break;
            case '{':
            {
                func_active = true;
                fgetpos(file, &funcpos);
                // skip to }
                while(fgetc(file) != '}' && !feof(file))
                {
                }
            }
            break;
            case '}':
            {
                if(!current_func_node)
                {
                    printf("Couldn't back to the last pos!\n");
                    return 0;
                }
                fsetpos(file, &current_func_node->back_pos);
                
                func_node *prev = current_func_node->prev;
                free(current_func_node);
                current_func_node = prev;
            }
            break;
            case '=':
            {
                if(!func_active)
                {
                    printf("No function has been defined!\n");
                    return 0;
                }

                fpos_t current_pos;
                fgetpos(file, &current_pos);
                func_node *new_one = new_func_node(current_pos);
                new_one->prev = current_func_node;
                current_func_node = new_one;
                fsetpos(file, &funcpos);
            }
            break;
            case '0':
            {
                value.number = 0U;
            }
            break;
            case '*':
            {
                if(!current_node)
                {
                    printf("No content in the stack!\n");
                    return 0;
                }
                value.number *= current_node->value.number;
            }
            break;
            case '/':
            {
                if(!current_node)
                {
                    printf("No content in the stack!\n");
                    return 0;
                }
                if (current_node->value.number == 0)
                {
                    printf("Division by zero!\n");
                    return 0;
                }
                value.number /= current_node->value.number;
            }
            break;
            case '^':
            {
                value.number *= value.number;
            }
            break;
            case '@':
            {
                int next_command = fgetc(file);
                if(feof(file)) break;
                switch (next_command)
                {
                    case '+':
                    {
                        if(!current_node)
                        {
                            printf("No content in the stack!\n");
                            return 0;
                        }
                        value.number += current_node->value.number;
                    }
                    break;
                    case '-':
                    {
                        if(!current_node)
                        {
                            printf("No content in the stack!\n");
                            return 0;
                        }
                        value.number -= current_node->value.number;
                    }
                    break;
                    case '*':
                    {
                        if(!current_node)
                        {
                            printf("No content in the stack!\n");
                            return 0;
                        }
                        value.number *= current_node->value.number;
                    }
                    break;
                    case '/':
                    {
                        if(!current_node)
                        {
                            printf("No content in the stack!\n");
                            return 0;
                        }
                        if (current_node->value.number == 0)
                        {
                            printf("Division by zero!\n");
                            return 0;
                        }
                        value.number /= current_node->value.number;
                    }
                    break;
                    default:
                    {
                        printf("Unknown @ command: '%c'\n", next_command);
                        return 0;
                    }
               }
            }
            break;
            case '[':
            {
                if(!current_node)
                {
                    printf("No content in the stack!\n");
                    return 0;
                }
                if(current_node->value.number == 0)
                {
                    // skip to ]
                    while(fgetc(file) != ']' && !feof(file))
                    {
                    }
                    loop_active = false;
                    break;
                }
                loop_active = true;
                fgetpos(file, &looppos);
            }
            break;
            case ']':
            {
                if(!loop_active)
                {
                    printf("No loop has been defined!\n");
                    return 0;
                }
                fsetpos(file, &looppos);
            }
            break;
        }
    }
    putchar('\n');

    printf("stack content:\n");
    uint32_t count = 0;
    char printable[5] = {0};
    while(current_node)
    {
        stack_node *next = current_node->next;
        memcpy(printable, current_node->value.string, 4);
        
        if(current_node->value.number > 32U)
            printf("%u: %u %s\n", count, current_node->value.number, printable);
        else
            printf("%u: %u\n", count, current_node->value.number);
        free(current_node);
        current_node = next;
        count++;
    }

    fclose(file);
    return 0;
}
