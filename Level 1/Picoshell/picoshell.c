#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
** execute_child:
** Configura stdin/stdout del child ed esegue il comando.
*/
void	execute_child(char **cmd, int prev, int fd[2], int has_next_cmd)
{
	/* 1. Gestione INPUT (dal comando precedente) */
	if (prev != -1)
	{
		if (dup2(prev, STDIN_FILENO) == -1)
			exit(1);
		close(prev);
	}

	/* 2. Gestione OUTPUT (verso il prossimo comando) */
	if (has_next_cmd)
	{
		if (dup2(fd[1], STDOUT_FILENO) == -1)
			exit(1);
		close(fd[0]);
		close(fd[1]);
	}

	execvp(cmd[0], cmd);
	exit(1);
}

/*
** fork_execute:
** Crea il child ed aggiorna i fd nel parent.
*/
int	fork_execute(char **cmd, int *prev, int fd[2], int has_next_cmd)
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		if (has_next_cmd)
		{
			close(fd[0]);
			close(fd[1]);
		}
		if (*prev != -1)
			close(*prev);
		return (1);
	}

	if (pid == 0)
		execute_child(cmd, *prev, fd, has_next_cmd);

	/* parent */
	if (*prev != -1)
		close(*prev);

	if (has_next_cmd)
	{
		close(fd[1]);
		*prev = fd[0];
	}
	else
		*prev = -1;

	return (0);
}

/*
** Esegue una pipeline di comandi.
*/
int	picoshell(char **cmds[])
{
	int	fd[2];
	int	prev;
	int	i;

	if (!cmds || !cmds[0])
		return (1);

	prev = -1;
	i = 0;
	while (cmds[i])
	{
		if (cmds[i + 1] != NULL)
		{
			if (pipe(fd) == -1)
			{
				if (prev != -1)
					close(prev);
				return (1);
			}
		}

		if (fork_execute(cmds[i], &prev, fd, cmds[i + 1] != NULL))
			return (1);

		i++;
	}

	while (wait(NULL) > 0)
		;

	return (0);
}

/*
#include <stdio.h>

int	main(void)
{
	char	*cmd1[] = {"ls", "-la", NULL};
	char	*cmd2[] = {"grep", ".c", NULL};
	char	*cmd3[] = {"wc", "-l", NULL};
	char	*cmd4[] = {"echo", "Hello World!", NULL};
	char	*cmd5[] = {"nonexisting", NULL};

	char	**pipe1[] = {cmd1, cmd2, cmd3, NULL};
	char	**pipe2[] = {cmd4, NULL};
	char	**pipe3[] = {cmd5, NULL};
	char	**pipe4[] = {cmd1, NULL};

	printf("Running: ls -la | grep .c | wc -l\n");
	printf("Return: %d\n\n", picoshell(pipe1));

	printf("Running: echo Hello World!\n");
	printf("Return: %d\n\n", picoshell(pipe2));

	printf("Running: nonexisting\n");
	printf("Return: %d\n\n", picoshell(pipe3));

	printf("Running: ls -la\n");
	printf("Return: %d\n", picoshell(pipe4));

	return (0);
}
*/
