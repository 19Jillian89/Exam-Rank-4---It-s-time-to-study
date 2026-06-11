#ifndef VBC_H
# define VBC_H
 
# include <stdio.h>
# include <stdlib.h>
# include <ctype.h>
 
extern char	*s;
 
void	error(char c);
int		expr(void);
int		term(void);
int		factor(void);
 
#endif
