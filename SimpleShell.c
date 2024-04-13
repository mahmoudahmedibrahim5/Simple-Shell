#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_INPUT_LINE 1000
#define MAX_ENV_VAR 500
#define MAX_NO_OF_ARGUMENTS 10
#define MAX_ARGUMENT 100

void handle_sigint(int sig);
char **readInput(void);
char **evaluateExpressions(char **inp_list, int n);
void setup_environment();
void shell();
void executeShellBuiltin(char **list);
void executeCommand(char **list);

int main()
{
    signal(SIGCHLD, handle_sigint);
    setup_environment();
    shell();

    return 0;
}

void handle_sigint(int sig)
{
    // Wait for child to terminate
    wait(NULL);
    FILE *ptr;
    ptr = fopen("/home/mahmoud/log.txt", "a");
    // Append new sentence to log.txt
    fputs("Child process was terminated\n", ptr);
    fclose(ptr);
}

void setup_environment()
{
    /* cd to the current working directory */
    chdir(getenv("PATH"));
}

char **readInput(void)
{
    /* Declare Some Variables */
    // i is counter for loops
    // n is the number of strings in the input line (command + arguments)
    // ind is index for copying charachters
    int i = 0, j = 0, n = 0, ind = 0;

    /* Allocate Space */
    // Allocate space for input line
    // Allocate space for array to parse input line
    char *input = (char *)malloc(sizeof(char) * MAX_INPUT_LINE);
    char **arr = (char **)malloc(sizeof(char *) * MAX_NO_OF_ARGUMENTS);
    for (i = 0; i < 10; i++)
        arr[i] = (char *)malloc(sizeof(char) * MAX_ARGUMENT);

    /* Read the input */
    fgets(input, MAX_INPUT_LINE, stdin);

    /* Slicing The input Line */
    for (i = 0; i < 100; i++)
    {
        if (input[i] == '\n'){
            arr[n][ind] = '\0';
            n++;
            break;
        }
        else if (input[i] == ' '){
            arr[n][ind] = '\0';
            ind = 0;
            n++;
        }
        else{
            arr[n][ind] = input[i];
            ind++;
        }
    }
    /* Make the actual list */
    // Allocate space for the actual length of strings in the input line
    // Then, copy the parsed input to it
    char **list = (char **)malloc(sizeof(char *) * (n + 1));
    for (i = 0; i < n; i++){
        list[i] = (char *)malloc(sizeof(char) * strlen(arr[i]));
        strcpy(list[i], arr[i]);
    }
    list[n] = NULL;

    /* free allocated space */
    free(input);
    for (i = 0; i < 10; i++)
        free(arr[i]);
    free(arr);

    /* Evaluate Expressions */
    list = evaluateExpressions(list, n);
    return list;
}

char **evaluateExpressions(char **inp_list, int n)
{
    char **list = inp_list;
    int i = 0, j = 0, k = 1;
    // Loop on elements of i searchin for any $ sign
    while (list[i] != NULL)
    {
        k=1;
        /* we have two cases
         * First: the string start with $ sign for example (ls $x or echo "hello $x")
         * Second: the string start with " followed by $ sign for example(echo "$x")
         * 
         * I divide them to 2 cases because the second one will occur in echo lines only,
         * while the first could occur in any line
         */

        /* First Case */
        if (list[i][0] == '$')
        {
            /* Here, we have two cases
             * First: string ends with " for example (echo "hello $x")
             * Second: string doesn't end with " for example (ls $x)
             */
            char c; // we will set the case whether it's 1 or 2
            // envName will store enviromental variable name
            char *envName = (char *)malloc(sizeof(char) * MAX_ENV_VAR);
            // Copying the enviromental variable name (after $) to env
            for (j = 1; j < strlen(list[i]); j++)
                envName[j - 1] = list[i][j];
            // Check whether the case is 1 or 2
            if (envName[j - 2] == '"'){ // for case 1
                envName[j - 2] = '\0';  // we want to remove " and replace it with \0
                c = 1;
            }
            else{                      // for case 2
                envName[j - 1] = '\0'; // we want to just add \0
                c = 2;
            }
            // delete the string
            free(list[i]);

            char *envValue = (char *)malloc(sizeof(char) * MAX_ENV_VAR);
            // replace enviromental variable name with its value
            strcpy(envValue, getenv(envName));
            int cntr = 0;
            // we check if the enviromental variable value has ("") we will remove it
            if (envValue[0] == '"')
            {
                for (j = 0; j < strlen(envValue) - 2; j++)
                {
                    if (envValue[j] == ' ')
                        cntr++;
                    envValue[j] = envValue[j + 1];
                }
                envValue[j] = '\0';
                /* if we don't use echo we want to parse the enviromental variable value */
                if (strcmp(list[0], "echo") != 0)
                {
                    int parse_length = 0;
                    char *buffer = (char *)malloc(sizeof(char) * 10);
                    int new_n = n + cntr;
                    char **envParsed = (char **)malloc(sizeof(char *) * (cntr));
                    int l = 0, m = 0, k = 0;
                    for (k = 0; k < cntr + 1; k++)
                    {
                        m = 0, parse_length = 0;
                        while (envValue[l] != ' ' && envValue[l] != '\0')
                        {
                            buffer[m] = envValue[l];
                            parse_length++;
                            l++;
                            m++;
                        }
                        buffer[m] = '\0';
                        l++;
                        envParsed[k] = (char *)malloc(sizeof(char) * parse_length);
                        strcpy(envParsed[k], buffer);
                    }
                    free(buffer);
                    char **new_list = (char **)malloc(sizeof(char *) * (new_n + 1));
                    for (j = 0; j < new_n; j++)
                    {
                        if (j < i){
                            new_list[j] = (char *)malloc(sizeof(char) * (strlen(list[j]) + 1));
                            strcpy(new_list[j], list[j]);
                            new_list[j][strlen(list[j])] = '\0';
                        }
                        else if (j < (new_n - i + 1)){
                            new_list[j] = (char *)malloc(sizeof(char) * (strlen(envParsed[j - i])));
                            strcpy(new_list[j], envParsed[j - i]);
                            new_list[j][strlen(envParsed[j - i])] = '\0';
                        }
                        else{
                            strcpy(new_list[j], list[j - cntr + 1]);
                        }
                    }
                    new_list[new_n] = NULL;
                    return new_list;
                }
            }
            if (c == 1){
                list[i] = (char *)malloc(sizeof(char) * strlen(envValue) + 2);
                strncpy(list[i], envValue, strlen(envValue));
                list[i][strlen(envValue)] = '"';
                list[i][strlen(envValue) + 1] = '\0';
            }
            else{
                list[i] = (char *)malloc(sizeof(char) * strlen(envValue));
                strcpy(list[i], envValue);
            }
            free(envName);
            free(envValue);
        }
        
        /* Second Case */
        while (list[i][k] != '\0')
        {// We loop on the whole string checking whether there is $ sign
            if (list[i][k] == '$')
            { // if $ sign Found
                /* Here, we have two cases
                 * First: string ends with " for example (echo "$x")
                 * Second: string doesn't end with " for example (ls "$x hello")
                 */
                char c; // we will set the case whether it's 1 or 2
                // envName will store enviromental variable name
                char *envName = (char *)malloc(sizeof(char) * MAX_ENV_VAR);
                for (j = k + 1; j < strlen(list[i]); j++)
                    envName[j - (k + 1)] = list[i][j];
                if (envName[j - (k + 2)] == '"')
                {                                // for case 1
                    envName[j - (k + 2)] = '\0'; // we want to remove " and replace it with \0
                    c = 1;
                }
                else
                {                                // for case 2
                    envName[j - (k + 1)] = '\0'; // we want to just add \0
                    c = 2;
                }
                
                char *old = (char *)malloc(sizeof(char) * strlen(list[i]));
                strcpy(old, list[i]);
                // free the old string
                free(list[i]);

                int cntr = 0; // count the number of strings in envValue
                char *envValue = (char *)malloc(sizeof(char) * MAX_ENV_VAR);
                strcpy(envValue, getenv(envName));
                // if the enviromental variable has "" remove them
                if (envValue[0] == '"')
                {
                    for (j = 0; j < strlen(envValue) - 2; j++)
                    {
                        if (envValue[j] == ' ')
                            cntr++;
                        envValue[j] = envValue[j + 1];
                    }
                    envValue[j] = '\0';
                }

                if (c == 1)
                {// if we are in case 1
                    list[i] = (char *)malloc(sizeof(char) * strlen(envValue) + k + 2);
                    // copy the first part of hte string (before $)
                    for (int m = 0; m < k; m++)
                        list[i][m] = old[m];
                    // copy the value replace the $
                    for (j = k; j < strlen(envValue) + k+2; j++)
                        list[i][j] = envValue[j - k];
                    // Add " and \0 to terminate the string
                    list[i][strlen(envValue) + k] = '"';
                    list[i][strlen(envValue) + k+1] = '\0';
                }
                else
                {// if we are in case 2
                    list[i] = (char *)malloc(sizeof(char) * strlen(envValue) + k);
                    // copy the first part of hte string (before $)
                    for (int m = 0; m < k; m++)
                        list[i][m] = old[m];
                    // copy the value replace the $
                    for (j = k; j < strlen(envValue) + k+1; j++)
                        list[i][j] = envValue[j - k];
                }
                free(envName);
                free(envValue);
            }
            k++;
        }
        i++;
    }
    return list;
}

void shell()
{
    char **list;

    /* Run pwd and read it */
    FILE *pipe;
    char buffer[1024];

    // Run the pwd command and open a pipe to read its output
    pipe = popen("pwd", "r");
    
    // Read the output of the pwd command
    if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        // Remove the trailing newline character, if any
        if (buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }
    }
    
    // Close the pipe
    pclose(pipe);

    while (1)
    {
        printf("\033[0;31m"); // Make The print color red
        printf("Mahmoud-Ahmed simple-shell:");

        printf("\033[0;33m"); // Make The print color yellow
        printf("%s", buffer);

        printf("$ ");
        printf("\033[0m"); // Make The print color white (Default)

        list = readInput();

        if ((int)strcmp(list[0], "exit") == 0)
            exit(0);

        if (strcmp(list[0], "cd") == 0 || strcmp(list[0], "echo") == 0 || strcmp(list[0], "export") == 0)
            executeShellBuiltin(list);
        else
            executeCommand(list);

        // If user make cd, edit the printed path
        if(!strcmp(list[0], "cd"))
        {
            // Run the pwd command and open a pipe to read its output
            pipe = popen("pwd", "r");
            
            // Read the output of the pwd command
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                // Remove the trailing newline character, if any
                if (buffer[strlen(buffer) - 1] == '\n') {
                    buffer[strlen(buffer) - 1] = '\0';
                }
            }
            
            // Close the pipe
            pclose(pipe);
        }
    }
}

void executeCommand(char **list)
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        execvp(list[0], list);
        if((int)strlen(list[0]) != 0)
            printf("Error\n");
        exit(0);
    }
    else if (pid == -1)
    {
        printf("error creating child");
        return;
    }
    else
    {
        if ((list[1] == NULL) || (strcmp(list[1], "&") != 0))
        {
            waitpid(pid, NULL, 0);
        }
    }
}

void executeShellBuiltin(char **list)
{
    /* We have 3 builtin functions (cd, echo, export)
     * i, j ,k are three counters  
     */
    int i, j, k;
    if (!strcmp(list[0], "cd"))
    {
        if ((list[1] == NULL) || (strcmp(list[1], "~") == 0))
            chdir(getenv("HOME"));
        else
            chdir(list[1]);
    }
    else if (!strcmp(list[0], "echo"))
    {
        j = 1;
        while (list[j][strlen(list[j]) - 1] != '"')
        {
            for (i = 0; i < strlen(list[j]); i++)
            {
                if (i == 0 && j == 1)
                    continue;
                printf("%c", list[j][i]);
            }
            printf(" ");
            j++;
        }
        for (i = 0; i < strlen(list[j]) - 1; i++)
        {
            if (i == 0 && j == 1)
                continue;
            printf("%c", list[j][i]);
        }
        printf("\n");
    }
    else if (!strcmp(list[0], "export"))
    {
        char *envName = (char *)malloc(sizeof(char) * MAX_ENV_VAR);
        char *envValue = (char *)malloc(sizeof(char) * MAX_ENV_VAR);
        i = 0;
        while (list[1][i] != '=')
        {
            envName[i] = list[1][i];
            i++;
        }
        envName[i] = '\0';
        i++;
        j = 0;
        while (list[1][i] != '\0')
        {
            envValue[j] = list[1][i];
            i++;
            j++;
        }
        k = 2;
        while (list[k] != NULL)
        {
            envValue[j] = ' ';
            j++;
            i = 0;
            while (list[k][i] != '\0')
            {
                envValue[j] = list[k][i];
                i++;
                j++;
            }
            k++;
        }
        envValue[j] = '\0';
        setenv(envName, envValue, 1);

        free(envName);
        free(envValue);
    }
}
