#include "vbc.h"

char* input_char;

void error(char non_char)
{
    if (unexpected_char)
        printf("Unexpected token '%c'\n", unexpected_char);
    else
        printf("Unexpected end of input\n");
    exit(1);
}

int expr();

int factor()
{
    int value;

    if (*input_char == '\0')
    error('\0');
    
    if (isdigit(*input_char))
        return *input_char++ - '0';
    if (*input_char == '(') {
        input_char++;
        value = expr();
        if (*input_char != ')')
            error(*input_char);
        input_char++;
        return value;
    }
    error(*input_char);
    return 0;
}

int term()
{
    int value = factor();

    while (*input_char == '*') {
        input_char++;
        value *= factor();
    }
    return value;
}

int expr()
{
    int value = term();

    while (*input_char == '+') {
        input_char++;
        value += term();
    }
    return value;
}

int main(int ac, char** av)
{
    int result;

    if (ac != 2)
        return 1;

    input_char = av[1];
    result = expr();

    if (*input_char)
        error(*input_char);

    printf("%d\n", result);
    return 0;
}
