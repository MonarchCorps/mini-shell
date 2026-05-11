#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

void handle_sigint(const int sig)
{
    (void)sig; // suppress unused parameter warning
    write(STDOUT_FILENO, "\n", 1); // newline after ^C
    // reprint the prompt
}

void tokenize(char* line, char** argv)
{
    int index = 0;

    char* token = strtok(line, " \n");
    while (token != NULL)
    {
        argv[index] = token;
        index++;
        token = strtok(NULL, " \n");
    }
    argv[index] = NULL;
}

char* parser_redirect(char** argv)
{
    for (int i = 0; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            char* filename = argv[i + 1];
            argv[i] = NULL;
            argv[i + 1] = NULL;
            return filename;
        }
    }

    return NULL;
}

char** parse_pipe(char** argv)
{
    for (int i = 0; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], "|") == 0)
        {
            argv[i] = NULL;
            return &argv[i + 1];
        }
    }

    return NULL;
}

int main(void)
{
    signal(SIGINT, handle_sigint);

    char buffer[256];

    while (
        printf("eduos> "),
        fflush(stdout),
        fgets(buffer, sizeof(buffer), stdin) != NULL
        || !feof(stdin) // ← if interrupted by signal, not real EOF, keep going
    )
    {
        if (ferror(stdin)) // signal interrupted fgets
        {
            clearerr(stdin);
            continue; // reprint prompt, keep looping
        }

        buffer[strcspn(buffer, "\n")] = 0;

        char* argv[64];
        tokenize(buffer, argv);

        if (argv[0] == NULL) continue;

        if (strcmp(argv[0], "exit") == 0)
        {
            exit(EXIT_SUCCESS);
        }

        if (strcmp(argv[0], "cd") == 0)
        {
            if (argv[1] == NULL)
            {
                printf("cd: missing argument\n");
                continue;
            }

            const int code = chdir(argv[1]);
            if (code == -1)
            {
                printf("xsh: could not checkout to directory\n");
            }

            continue;
        }

        char** right = parse_pipe(argv);
        if (right != NULL)
        {
            int fds[2];
            pipe(fds);

            // child 1 — runs left side (ls)
            pid_t left_pid = fork();
            if (left_pid == 0)
            {
                dup2(fds[1], 1);
                close(fds[0]);
                close(fds[1]);
                execvp(argv[0], argv);
                printf("xsh: %s: command not found\n", argv[0]);
                exit(EXIT_FAILURE);
            }

            // child 2 — runs right side (grep)
            pid_t right_pid = fork();
            if (right_pid == 0)
            {
                dup2(fds[0], 0);
                close(fds[0]);
                close(fds[1]);
                execvp(right[0], right);
                printf("xsh: %s: command not found\n", right[0]);
                exit(EXIT_FAILURE);
            }

            // parent — close both ends, wait for both children
            close(fds[0]);
            close(fds[1]);
            waitpid(left_pid, NULL, 0);
            waitpid(right_pid, NULL, 0);

            continue;
        }

        const pid_t pid = fork();

        if (pid == -1)
        {
            printf("xsh: fork failed\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            signal(SIGINT, SIG_DFL);

            const char* outfile = parser_redirect(argv);
            if (outfile != NULL)
            {
                const int oldFd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (oldFd == -1)
                {
                    printf("xsh: failed to open\n");
                    exit(EXIT_FAILURE);
                }

                dup2(oldFd, 1);
                close(oldFd);
            }

            execvp(argv[0], argv);
            printf("xsh: %s: command not found\n", argv[0]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }

        waitpid(pid,NULL, 0);
        fflush(stdout);
    }

    return 0;
}
