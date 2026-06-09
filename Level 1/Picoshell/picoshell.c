#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
** execute_child:
** Configura stdin/stdout ed esegue il comando chiudendo TUTTI i fd superflui.
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
		close(fd[0]); /* Chiude il lato di lettura della nuova pipe nel child */
		close(fd[1]); /* Chiude il lato di scrittura originale dopo il dup2 */
	}

	execvp(cmd[0], cmd);
	exit(1);
}

/*
** fork_execute:
** Crea il child ed aggiorna i fd nel parent in modo sicuro.
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
			close(*prev); /* Risolto: chiude prev in caso di fallimento */
		return (1);
	}

	if (pid == 0)
		execute_child(cmd, *prev, fd, has_next_cmd);

	/* PARENT PROCESS */
	if (*prev != -1)
		close(*prev);

	if (has_next_cmd)
	{
		close(fd[1]); /* Il parent non scrive nella pipe, chiude il lato di scrittura */
		*prev = fd[0]; /* Salva il lato di lettura per il prossimo comando */
	}
	else
		*prev = -1;

	return (0);
}

/*
** picoshell:
** Firma della funzione corretta secondo il subject.
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

	/* Aspetta tutti i processi figli */
	while (wait(NULL) > 0)
		;

	return (0);
}

/*
// NUOVA PARTE CON MACRO: controlla lo stato di uscita di ogni processo figlio

	int status;
	int result = 0;
	
	while (wait(&status) != -1)
	{
		if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			result = 1;
		}
	}

	return (result);
}*/

/*
#include <stdio.h>

// Prototipo necessario per compilare senza warning
int	picoshell(char **cmds[]);

int	main(void)
{
	// Usiamo i percorsi assoluti per essere conformi agli esempi del subject
	char	*cmd1[] = {"/bin/ls", "-la", NULL};
	char	*cmd2[] = {"/usr/bin/grep", ".c", NULL};
	char	*cmd3[] = {"/usr/bin/wc", "-l", NULL};
	char	*cmd4[] = {"/bin/echo", "Hello World!", NULL};
	char	*cmd5[] = {"/bin/nonexisting", NULL};

	char	**pipe1[] = {cmd1, cmd2, cmd3, NULL};
	char	**pipe2[] = {cmd4, NULL};
	char	**pipe3[] = {cmd5, NULL};
	char	**pipe4[] = {cmd1, NULL};

	printf("--- TEST 1: Pipeline complessa ---\n");
	printf("Running: /bin/ls -la | /usr/bin/grep .c | /usr/bin/wc -l\n");
	printf("Return atteso: 0\n");
	printf("Return effettivo: %d\n\n", picoshell(pipe1));

	printf("--- TEST 2: Comando singolo ---\n");
	printf("Running: /bin/echo Hello World!\n");
	printf("Return atteso: 0\n");
	printf("Return effettivo: %d\n\n", picoshell(pipe2));

	printf("--- TEST 3: Comando inesistente ---\n");
	printf("Running: /bin/nonexisting\n");
	printf("Nota: execvp fallirà internamente nel child, ma picoshell deve comunque rendere 0.\n");
	printf("Return atteso: 0\n");
	printf("Return effettivo: %d\n\n", picoshell(pipe3));

	printf("--- TEST 4: Comando singolo senza pipe ---\n");
	printf("Running: /bin/ls -la\n");
	printf("Return atteso: 0\n");
	printf("Return effettivo: %d\n", picoshell(pipe4));

	return (0);
}*/
