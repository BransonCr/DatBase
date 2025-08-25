#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/*** Structs ***/
typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer *new_input_buffer(){
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}
/***  Printing ***/
void print_prompt() {printf("db> "); }


/*** Reading ***/
/*lineptr:a pointer to the variable we use to point to the buffer containing the readline.
If se to NULL it is mallocatted by getline and thus be freed by the user, even if the command fails
n:a pointer to the variable we use to save the size of allocated buffer.
stream:The input stream to read from, We'll be reading from standard input.*/

/*So far tell getline to store the read line in input_buffer->buffer and the size of allocated buffer
in input_buffer -> input_length.*/

//buffer starts as null, so getline allocated enough mem to hold the line of input and makes buffer point to in
ssize_t getline(char**lineptr, size_t *n, FILE *stream);
void read_input(InputBuffer *input_buffer){
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

    if(bytes_read <= 0){
        printf("Error reading input \n");
        exit(EXIT_FAILURE);
    }

    // Ignore trailing newline
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = 0;
}
void close_input_buffer(InputBuffer* input_buffer){
    free(input_buffer -> buffer);
    free(input_buffer);
}

/*Making a Simple REPL*/
int main(int argc, char* argv[]) {
    InputBuffer* input_buffer = new_input_buffer();
    while (true) {
        print_prompt();
        read_input(input_buffer);
        //exit
        if (strcmp(input_buffer->buffer, ".exit") == 0) {
            close_input_buffer(input_buffer);
            exit(EXIT_SUCCESS);
        } else {
            printf("Unrecognized command '%s'.\n", input_buffer->buffer);
        }
    }
}