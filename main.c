#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

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

int main(void)
{
    char buffer[256];

    while (
        printf("eduos> "),
        fflush(stdout),
        fgets(buffer, sizeof(buffer), stdin)
    )
    {
        buffer[strcspn(buffer, "\n")] = 0;

        char* argv[64];
        tokenize(buffer, argv);

        if (argv[0] == NULL) continue;

        const pid_t pid = fork();

        if (pid == -1)
        {
            perror("Fork Failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        {
            execvp(argv[0], argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        waitpid(pid,NULL, 0);
    }

    return 0;
}
