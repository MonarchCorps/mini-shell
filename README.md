# Mini Shell

A Unix shell written in C from scratch. Built as part of learning systems programming — processes, file descriptors, and how the OS actually works underneath every terminal you've ever used.

## What it does

- Runs any command on your system (`ls`, `pwd`, `echo`, `git`, anything)
- Forks a child process per command, execs the binary, waits for it to finish
- Tokenizes raw input into an argument vector
- Handles blank input cleanly
- Reports errors when a command isn't found or can't be executed

## What's coming

- Built-in commands: `cd`, `exit`
- I/O redirection: `echo hello > file.txt`, `cat < file.txt`
- Pipes: `ls | grep .c`
- Signal handling: Ctrl+C kills the running command, not the shell

## How to build

```bash
git clone https://github.com/yourusername/mini-shell
cd mini-shell
mkdir build && cd build
cmake ..
make
./mini_shell
```

Or with gcc directly:

```bash
gcc -std=c99 -Wall -Wextra -o xsh main.c
./xsh
```

## How it works

Every command goes through the same path:

```
read input
    ↓
tokenize into argv[]
    ↓
fork() — creates child process
    ↓
child: execvp(argv[0], argv) — child becomes the command
parent: waitpid() — shell waits, then reprints prompt
```

The shell is the parent process. It never becomes the command — it forks a child, the child transforms into the command via `execvp`, runs, and exits. The parent collects the exit status and loops.

## What I learned building this

- The difference between a terminal and a shell
- How `fork()` clones a process and copy-on-write defers the actual memory duplication
- Why `execvp` never returns on success — it replaces the process entirely
- Why `cd` and `exit` must be built-ins, not forked children
- How file descriptors are inherited across `fork()` — the mechanism that makes pipes possible
- What a zombie process is and why `waitpid()` prevents it

## Environment

- Language: C99
- Platform: macOS (Apple Silicon)
- Built with CLion + CMake
- Memory checked with AddressSanitizer

## Part of

Building toward **EduOS** — a privacy-first, AI-native Linux-based OS for African schools.