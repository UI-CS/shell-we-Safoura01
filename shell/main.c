#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 80
#define MAX_ARGS (MAX_LINE / 2 + 1)

char history[MAX_LINE];

int parse_input(char *input, char **args, int *is_background, int *pipe_pos)
{
    int i = 0;
    *is_background = 0;
    *pipe_pos = -1;

    char *token = strtok(input, " \n");
    while (token != NULL && i < MAX_ARGS)
    {
        if (strcmp(token, "&") == 0)
        {
            *is_background = 1;
            break;
        }
        else if (strcmp(token, "|") == 0)
        {
            *pipe_pos = i;
            args[i] = NULL;
        }
        else
        {
            args[i] = token;
        }
        i++;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
    return i;
}

int handle_builtins(char **args)
{
    if (args[0] == NULL)
        return 0;
    if (strcmp(args[0], "exit") == 0)
        exit(0);
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL || chdir(args[1]) != 0)
            perror("cd failed");
        return 1;
    }
    if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        return 1;
    }
    if (strcmp(args[0], "help") == 0)
    {
        printf("uinxsh - Custom Unix Shell\nCommands: exit, cd, pwd, help, history (!!), pipes (|), background (&)\n");
        return 1;
    }
    return 0;
}

int main(void)
{
    char *args[MAX_ARGS];
    char input[MAX_LINE];

    while (1)
    {
        printf("uinxsh> ");
        fflush(stdout);
        if (fgets(input, MAX_LINE, stdin) == NULL)
            break;

        if (strcmp(input, "!!\n") == 0)
        {
            if (strlen(history) == 0)
            {
                printf("No commands in history.\n");
                continue;
            }
            printf("%s", history);
            strcpy(input, history);
        }
        else
        {
            strcpy(history, input);
        }

        int is_background, pipe_pos;
        int num_args = parse_input(input, args, &is_background, &pipe_pos);
        if (num_args == 0)
            continue;
        if (handle_builtins(args))
            continue;

        if (pipe_pos != -1)
        {
            int fd[2];
            if (pipe(fd) == -1)
            {
                perror("Pipe failed");
                continue;
            }
            if (fork() == 0)
            {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(args[0], args);
                exit(1);
            }
            if (fork() == 0)
            {
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(args[pipe_pos + 1], &args[pipe_pos + 1]);
                exit(1);
            }
            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            wait(NULL);
        }
        else
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                if (execvp(args[0], args) == -1)
                {
                    printf("Command not found: %s\n", args[0]);
                    exit(1);
                }
            }
            else if (pid > 0)
            {
                if (!is_background)
                    waitpid(pid, NULL, 0);
                else
                    printf("[Running in background]\n");
            }
        }
        while (waitpid(-1, NULL, WNOHANG) > 0)
            ;
    }
    return 0;
}