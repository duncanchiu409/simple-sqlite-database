#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}Input_Buffer;

Input_Buffer* new_Input_Buffer(){
    Input_Buffer* input_buffer = (Input_Buffer*)malloc(sizeof(Input_Buffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void close_Input_Buffer(Input_Buffer* input_buffer){
    free(input_buffer->buffer);
    free(input_buffer);
}

void print_prompt(){
    printf("sqlite > ");
}

void read_input(Input_Buffer* input_buffer){
    size_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if(bytes_read <= 0){
        printf("Error reading input\n");
        close_Input_Buffer(input_buffer);
        exit(EXIT_FAILURE);
    }

    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = '\0'; // replace newling char to end of line char
}

int main(int argc, char* argv[]){
    Input_Buffer* input_buffer = new_Input_Buffer();
    while(1){
        print_prompt();
        read_input(input_buffer);

        if(strcmp(input_buffer->buffer, ".exit") == 0){
            printf("Hello World, bye bye\n");
            close_Input_Buffer(input_buffer);
            exit(EXIT_SUCCESS);
        }else{
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
    }
    return 0;
}