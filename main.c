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
    fptr = fopen("log_file.txt", "a");
    fprintf(fptr, "Child process was terminated\n");
    fclose(fptr);
}

void reap_child_zombie(int pid) {
//    sleep(1);
    kill(pid, SIGTERM);
}

void setup_environment() {
    chdir("/home/saifullah");
}

void shell() {
    char** parsed_input;
    do {
        char input_command[100];
        printf(">> ");
        read_input(input_command, sizeof (input_command));
        parsed_input = parse_input(input_command);
        evaluate_expression(parsed_input);
        print_arr(parsed_input);
//        printf("%s", *parsed_input);
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
    while (strncmp(*parsed_input, "exit", 4) != 0);
    exit(0);
}

void read_input(char string[], int size) {
    fgets(string, size, stdin);
}

char** parse_input(char input[]) {
    size_t length = strlen(input);
    if (length > 0 && input[length - 1] =='\n') input[length - 1] = '\0';
    char** tokens = (char**)malloc(sizeof(char*)); // Allocate memory for an array of pointers
    if (tokens == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    char* token = strtok(input, " "); // Split input string by space
    int i = 0;
    while (token != NULL) {
        remove_char(token, '\"');
        tokens[i] = token; // Assign token to the array of pointers
        i++;
        tokens = (char**)realloc(tokens, (i + 1) * sizeof(char*)); // Resize array of pointers
        if (tokens == NULL) {
            printf("Memory reallocation failed\n");
            exit(1);
        }
        token = strtok(NULL, " ");
    }
    tokens[i] = NULL; // Set the last element to NULL to indicate the end of tokens
    return tokens;
}

void evaluate_expression(char** command) {
    char** ptr = command;
    while(*ptr != NULL) {
//        printf("## %s\n", *ptr);
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
//        remove_char(init, '\"');
        variables[len] = strdup(strtok(init, "="));  // Tokenize and copy the variable part
        values[len] = strdup(strtok(NULL, "\0"));    // Tokenize and copy the value part
        len++;
//        printf("%s %s\n", variables[len - 1], values[len - 1]);
    } else if(strcmp(*command, "echo") == 0) {
        char* var = command[1];
        remove_char(var, '\"');
//        printf("asdsf %s %c\n", var, var[0]);
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
    child_id = fork();
    int status;
    int is_background = check_background(command);
    if (child_id == 0) {
        print_arr(command);
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
    int found = 0; // Flag to indicate if "&" is found
    while (*ptr != NULL) {
        if (strcmp(*ptr, "&") == 0)
            found = 1; // Set the flag if "&" is found
        else if (found)
            *(ptr - 1) = *ptr; // If "&" was found previously, shift subsequent elements back
        ptr++;
    }
    if (found)
        *(ptr - 1) = NULL; // Null-terminate the modified command array if "&" was found
    return found; // Return the flag indicating if "&" was found
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
// TODO: ZOMBIE - BUILTIN