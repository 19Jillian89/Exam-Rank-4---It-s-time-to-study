## English

# ft_popen

A custom implementation of the standard `popen()` function in C.

## Prototype

```c
int ft_popen(const char *file, char *const argv[], char type);
```

## Overview

`ft_popen` creates a pipe between a parent and child process, executes a command via `execvp`, and returns a file descriptor that allows the parent to either read the command's output (`'r'`) or write to its input (`'w'`).

## How It Works

### Pipe basics

```c
int fd[2];
pipe(fd);
// fd[0] → read end
// fd[1] → write end
```

### Mode `'r'` — parent reads command output

```
child stdout ──→ pipe ──→ parent
```

The child redirects its `stdout` into the pipe via `dup2(fd[1], STDOUT_FILENO)`. The parent receives `fd[0]` and can call `read()` on it.

```c
int fd = ft_popen("ls", (char *const[]){"ls", NULL}, 'r');
// parent reads ls output from fd
```

### Mode `'w'` — parent writes to command input

```
parent ──→ pipe ──→ child stdin
```

The child redirects its `stdin` from the pipe via `dup2(fd[0], STDIN_FILENO)`. The parent receives `fd[1]` and can call `write()` on it.

```c
int fd = ft_popen("cat", (char *const[]){"cat", NULL}, 'w');
write(fd, "hello\n", 6);
// cat prints: hello
```

## Implementation

```c
int	ft_popen(const char *file, char *const argv[], char type)
{
	int		fd[2];
	pid_t	pid;

	if (!file || !argv || !argv[0]
		|| (type != 'r' && type != 'w'))
		return (-1);

	if (pipe(fd) == -1)
		return (-1);

	pid = fork();

	if (pid == -1)
	{
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}

	if (pid == 0)
	{
		if (type == 'r')
		{
			close(fd[0]);
			if (dup2(fd[1], STDOUT_FILENO) == -1)
				exit(1);
			close(fd[1]);
		}
		else
		{
			close(fd[1]);
			if (dup2(fd[0], STDIN_FILENO) == -1)
				exit(1);
			close(fd[0]);
		}
		execvp(file, argv);
		exit(1);
	}

	if (type == 'r')
	{
		close(fd[1]);
		return (fd[0]);
	}

	close(fd[0]);
	return (fd[1]);
}
```

## Key Concepts

| Concept | Role |
|---|---|
| `pipe()` | Creates the communication channel between processes |
| `fork()` | Spawns the child process |
| `dup2()` | Redirects stdin/stdout to the pipe — does **not** copy data, it changes where the stream points |
| `execvp()` | Replaces the child process image with the target command |
| `close()` | Closes unused file descriptors to avoid leaks and ensure proper EOF |

## Why Close Unused File Descriptors?

Each process must close the end of the pipe it doesn't use. Failing to do so causes:

- **File descriptor leaks**
- **Missing EOF signals** — a process waiting for EOF on a pipe will block if any writer fd is still open somewhere
- **Processes hanging indefinitely**

## Notes

- `wait()` / `waitpid()` are intentionally **not used** — this implements only the "open" side, mirroring how the real `popen()` works. Child cleanup is handled separately (e.g. by a `ft_pclose()`).
- Returns `-1` on any error (invalid args, `pipe()` failure, `fork()` failure).

----------------------------------------------------------------------------------------------------------

## Italiano 🇮🇹

# ft_popen

Implementazione personalizzata della funzione standard `popen()` in C.

## Prototipo

```c
int ft_popen(const char *file, char *const argv[], char type);
```

## Panoramica

`ft_popen` crea una pipe tra un processo padre e uno figlio, esegue un comando tramite `execvp`, e restituisce un file descriptor che permette al padre di leggere l'output del comando (`'r'`) oppure scrivere nel suo input (`'w'`).

## Come funziona

### Le basi della pipe

```c
int fd[2];
pipe(fd);
// fd[0] → lato lettura
// fd[1] → lato scrittura
```

### Modalità `'r'` — il padre legge l'output del comando

```
stdout del figlio ──→ pipe ──→ padre
```

Il figlio redirige il suo `stdout` nella pipe tramite `dup2(fd[1], STDOUT_FILENO)`. Il padre riceve `fd[0]` e può chiamarci `read()`.

```c
int fd = ft_popen("ls", (char *const[]){"ls", NULL}, 'r');
// il padre legge l'output di ls da fd
```

### Modalità `'w'` — il padre scrive nell'input del comando

```
padre ──→ pipe ──→ stdin del figlio
```

Il figlio redirige il suo `stdin` dalla pipe tramite `dup2(fd[0], STDIN_FILENO)`. Il padre riceve `fd[1]` e può chiamarci `write()`.

```c
int fd = ft_popen("cat", (char *const[]){"cat", NULL}, 'w');
write(fd, "hello\n", 6);
// cat stampa: hello
```

## Implementazione

```c
int	ft_popen(const char *file, char *const argv[], char type)
{
	int		fd[2];
	pid_t	pid;

	if (!file || !argv || !argv[0]
		|| (type != 'r' && type != 'w'))
		return (-1);

	if (pipe(fd) == -1)
		return (-1);

	pid = fork();

	if (pid == -1)
	{
		close(fd[0]);
		close(fd[1]);
		return (-1);
	}

	if (pid == 0)
	{
		if (type == 'r')
		{
			close(fd[0]);
			if (dup2(fd[1], STDOUT_FILENO) == -1)
				exit(1);
			close(fd[1]);
		}
		else
		{
			close(fd[1]);
			if (dup2(fd[0], STDIN_FILENO) == -1)
				exit(1);
			close(fd[0]);
		}
		execvp(file, argv);
		exit(1);
	}

	if (type == 'r')
	{
		close(fd[1]);
		return (fd[0]);
	}

	close(fd[0]);
	return (fd[1]);
}
```

## Concetti chiave

| Concetto | Ruolo |
|---|---|
| `pipe()` | Crea il canale di comunicazione tra i processi |
| `fork()` | Genera il processo figlio |
| `dup2()` | Redirige stdin/stdout verso la pipe — **non** copia dati, cambia dove punta lo stream |
| `execvp()` | Sostituisce l'immagine del processo figlio con il comando target |
| `close()` | Chiude i file descriptor inutilizzati per evitare leak e garantire la corretta ricezione dell'EOF |

## Perché chiudere i file descriptor inutilizzati?

Ogni processo deve chiudere il lato della pipe che non usa. Non farlo causa:

- **Leak di file descriptor**
- **Mancata ricezione dell'EOF** — un processo in attesa di EOF su una pipe resta bloccato finché qualche fd di scrittura rimane aperto da qualche parte
- **Processi bloccati a tempo indeterminato**

## Note

- `wait()` / `waitpid()` sono intenzionalmente **non utilizzati** — questa funzione implementa solo la parte "open", specchiando il comportamento del vero `popen()`. La gestione del processo figlio avviene separatamente (es. tramite una `ft_pclose()`).
- Restituisce `-1` in caso di errore (argomenti non validi, fallimento di `pipe()` o `fork()`).
