#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void setup_enviroment();
void shell();
void read_input(char string[], int size);
char** parse_input(char input[]);
int check_command(char* command);
void execute_shell_bultin(char **command);
void execute_command(char **command);
int check_background(char **command);

char* variables[100];
char* values[100];

int main() {
    setup_enviroment();
    shell();
    return 0;
}

void setup_enviroment() {
    chdir("/home/saifullah");
}

void shell() {
    char** parsed_input;
    do {
        char input_command[50];
        printf(">> ");
        read_input(input_command, sizeof (input_command));
        parsed_input = parse_input(input_command);
//        printf("%s", *parsed_input);
        switch (check_command(*parsed_input)) {
            case 1:
                execute_shell_bultin(parsed_input);
                break;
            case 2:
                execute_command(parsed_input);
                break;
            default:
                printf("Error1");
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

int check_command(char* command) {
    if (strcmp(command, "cd") == 0 || strcmp(command, "echo") == 0 || strcmp(command, "export") == 0) return 1;
    else return 2;
}

void execute_shell_bultin(char **command) {
    if (strcmp(*command, "cd") == 0) {

    } else if (strcmp(*command, "export") == 0) {

    } else if(strcmp(*command, "echo") == 0) {

    }
}

void execute_command(char** command) {
    int child_id = fork();
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
    int found = 0; // Flag to indicate if "&" is found

    while (*ptr != NULL) {

        if (strcmp(*ptr, "&") == 0) {
            found = 1; // Set the flag if "&" is found
        } else if (found) {
            // If "&" was found previously, shift subsequent elements back
            *(ptr - 1) = *ptr;
        }
        ptr++;
    }

    if (found)
        *(ptr - 1) = NULL; // Null-terminate the modified command array if "&" was found

    return found; // Return the flag indicating if "&" was found
}
// Problems : exit -
// TODO: ZOMBIE - BUILTIN