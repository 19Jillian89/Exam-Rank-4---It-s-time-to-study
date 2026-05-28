#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char	*s;

void	error(char c)
{
	if (c) printf("Unexpected token '%c'\n", c);
	else printf("Unexpected end of input\n");
	exit(1);
}

int expr();
int term();
int factor();

int	factor()
{
	if (isdigit(*s)) return (*s++ - '0');
	if (*s == '(') {
		s++;
		int val = expr();
		if (*s != ')') error(*s);
		s++;
		return (val);
	}
	else error(*s);
	return (0);
}

int	term()
{
	int val = factor();
	while (*s == '*') {
		s++;
		val *= factor();
	}
	return (val);
}

int	expr()
{
	int val = term();
	while (*s == '+') {
		s++;
		val += term();
	}
	return (val);
}

int	main(int argc, char **argv)
{
	if (argc != 2) return (1);
	s = argv[1];
	int result = expr();
	if (*s) error(*s);
	printf("%d\n", result);
	return (0);
}

