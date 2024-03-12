#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

void setup_environment();
void shell();
void read_input(char string[], int size);
char** parse_input(char input[]);
int check_command(char* command);
void execute_shell_builtin(char **command);
void execute_command(char **command);
int check_background(char **command);
void evaluate_expression(char** command);
void remove_char(char* s, char c);
void print_arr(char** arr);
void register_child_signal();
void on_child_exit();
void reap_child_zombie(int pid);

int len = 0;
char* variables[100];
char* values[100];
char cwd[200];
int child_id;

int main() {
    len = 0;
    register_child_signal();
    setup_environment();
    shell();
    return 0;
}

void register_child_signal() {
    signal(SIGCHLD, on_child_exit);
}

void on_child_exit() {
    reap_child_zombie(child_id);
    FILE *fptr;
    fptr = fopen("/home/saifullah/log_file.txt", "a");
    fprintf(fptr, "Child process %d was terminated\n", child_id);
    fclose(fptr);
}

void reap_child_zombie(int pid) {
    int status;
    if (waitpid(pid, &status, WNOHANG) > 0) {
        kill(pid, SIGTERM);
    }
}

void setup_environment() {
    chdir("/home/saifullah");
}

void shell() {
    char input_command[100];
    char** parsed_input;
    do {
        getcwd(cwd, sizeof (cwd));
        printf("%s >> ", cwd);
        read_input(input_command, sizeof (input_command));
        if(strlen(input_command) == 1 && input_command[0] == '\n') continue;
        parsed_input = parse_input(input_command);
        evaluate_expression(parsed_input);
        switch (check_command(*parsed_input)) {
            case 1:
                execute_shell_builtin(parsed_input);
                break;
            case 2:
                execute_command(parsed_input);
                break;
            default:
                break;
        }
    }
    while (strncmp(input_command, "exit", 4) != 0);
    exit(0);
}

void read_input(char string[], int size) {
    fgets(string, size, stdin);
}

char** parse_input(char input[]) {
    size_t length = strlen(input);
    if (length > 0 && input[length - 1] =='\n') input[length - 1] = '\0';
    char** tokens = (char**)malloc(sizeof(char*));
    if (tokens == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    char* token = strtok(input, " "); 
    int i = 0;
    while (token != NULL) {
        remove_char(token, '\"');
        tokens[i] = token; 
        i++;
        tokens = (char**)realloc(tokens, (i + 1) * sizeof(char*)); 
        if (tokens == NULL) {
            printf("Memory reallocation failed\n");
            exit(1);
        }
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL; 
    return tokens;
}

void evaluate_expression(char** command) {
    char** ptr = command;
    while(*ptr != NULL) {
        if (*ptr[0] == '$') {
            for (int i = 0; i < len; ++i) {
                if(strcmp(variables[i], *ptr) == 0) {
                    *ptr = values[i];
                    return;
                }
            }
        }
        ptr++;
    }
}

int check_command(char* command) {
    if (strcmp(command, "cd") == 0 || strcmp(command, "echo") == 0 || strcmp(command, "export") == 0) return 1;
    else if(strcmp(command, "exit") == 0) return -1;
    else return 2;
}

void execute_shell_builtin(char **command) {
    if (strcmp(*command, "cd") == 0) {
        if  (command[1] == NULL) return;
        if (strcmp(command[1], "~") == 0) {
            chdir("/home");
            return;
        }
        int d = chdir(*(command + 1));
        if(d == -1)
            printf("No such file or directory\n");
    } else if (strcmp(*command, "export") == 0) {
        char init[100] = "$";
        char** ptr = command;
        ptr++;
        while (*ptr != NULL) {
            strcat(init, *ptr);
            if(*(ptr+1) != NULL) strcat(init, " ");
            ptr++;
        }
        variables[len] = strdup(strtok(init, "="));  
        values[len] = strdup(strtok(NULL, "\0"));    
        len++;
    } else if(strcmp(*command, "echo") == 0) {
        char* var = command[1];
        remove_char(var, '\"');
        if(var[0] != '$'){
            char** ptr = command;
            ptr++;
            while (*ptr != NULL) {
                printf("%s ", *ptr);
                ptr++;
            }
            printf("\n");
        } else
            printf("Error\n");
    }
}

void execute_command(char** command) {
    if(command[1] != NULL && command[2] == NULL) {
        char* param = command[1];
        char** arg = parse_input(param);
        int i = 1;
        while (*arg != NULL) {
            command[i++] = *arg;
            arg++;
        }
        command[i] = NULL;
    }

    child_id = fork();
    int status;
    int is_background = check_background(command);
    if (child_id == 0) {
        execvp(*command, command);
        printf("Error\n");
        exit(0);
    }  else if (!is_background){
        waitpid(child_id, &status, 0);
    }
}

int check_background(char** command) {
    if (command == NULL)
        return 0;
    char** ptr = command;
    int found = 0; 
    while (*ptr != NULL) {
        if (strcmp(*ptr, "&") == 0)
            found = 1; 
        else if (found)
            *(ptr - 1) = *ptr; 
        ptr++;
    }
    if (found)
        *(ptr - 1) = NULL; 
    return found; 
}

void remove_char(char* s, char c)
{
    int j, n = strlen(s);
    for (int i = j = 0; i < n; i++)
        if (s[i] != c)
            s[j++] = s[i];

    s[j] = '\0';
}

void print_arr(char** arr) {
    char** ptr = arr;
    printf("input >> [");
    while(*ptr != NULL) {
        printf("%s ", *ptr);
        ++ptr;
    }
    printf("]\n");
}
