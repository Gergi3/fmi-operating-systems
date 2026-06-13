#include <err.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

// CONSTANTS FOR ERRORS
#define FILE_OPEN_ERR 100
#define FILE_OPEN_MSG "Couldn't open file"
#define FILE_READ_ERR 101
#define FILE_READ_MSG "Coudln't read from file"
#define FILE_WRITE_ERR 102
#define FILE_WRITE_MSG "Couldn't write to file"

// LOGIC CONSTANTS
#define BUFFER_SIZE 4096

// GLOBALS FOR LINE NUMBERS
static bool print_lines = false;
static int curr_line = 1;
static bool is_line_start = true;

// ERROR HANDLING
static void sys_err(int status, int err_code, const char* message) {
    if (status == -1) {
        err(err_code, message);
    }
}

// GENERIC FILE FUNCS
static int open_file(const char* filename, int flags, mode_t mode) {
    if (strcmp(filename, "-") == 0) {
        return STDIN_FILENO;
    }

    int fd = open(filename, flags, mode);
    sys_err(fd, FILE_OPEN_ERR, FILE_OPEN_MSG);
    return fd;
}

// SPECIFIC FILE FUNCS
static void print_line_num(void) {
    if (!print_lines) {
        return;
    }

    char line_buf[32];
    int written_bytes = snprintf(line_buf, sizeof(line_buf), "%d", curr_line);
    line_buf[written_bytes] = ' ';

    sys_err(write(STDOUT_FILENO, line_buf, written_bytes + 1), FILE_WRITE_ERR, FILE_WRITE_MSG);
    curr_line++;
}

static void print_buf(char buf[BUFFER_SIZE], ssize_t buf_size) {
    ssize_t write_index = 0;

    for (ssize_t i = 0; i < buf_size; i++) {
        if (is_line_start) {
            print_line_num();
            is_line_start = false;
        }

        if (buf[i] == '\n') {
            int status = write(STDOUT_FILENO, buf + write_index, i - write_index + 1); 
            sys_err(status, FILE_WRITE_ERR, FILE_WRITE_MSG);
            
            write_index = i + 1;
            is_line_start = true;
        }
    }
    
    if (write_index < buf_size) {
        if(is_line_start) {
            print_line_num();
            is_line_start = false;
        }

        int status = write(STDOUT_FILENO, buf + write_index, buf_size - write_index); 
        sys_err(status, FILE_WRITE_ERR, FILE_WRITE_MSG);
    }
}

static void print_file(const char* filename) {
    int file = open_file(filename, O_RDONLY, 0);
    
    ssize_t read_bytes;
    char buffer[BUFFER_SIZE];
    while ( (read_bytes = read(file, buffer, sizeof(buffer))) > 0) {
        print_buf(buffer, read_bytes); 
    }
    sys_err(read_bytes, FILE_READ_ERR, FILE_READ_MSG);
    close(file);
}

int main(int argc, char** argv) {
    size_t file_arg_start = 1;
     
    if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
        print_lines = true;
        file_arg_start = 2;
    }
    
    size_t args_count = (size_t) argc;
    if (file_arg_start == args_count) {
        print_file("-");
        return 0;
    }

    for (size_t i = file_arg_start; i < args_count; i++) {
        char* filename = argv[i];
        print_file(filename);
    }
    
    return 0;
}
