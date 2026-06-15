#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ARGS_ERR 101
#define ARGS_MSG "Invalid required args"
#define WRITE_ERR 102
#define WRITE_MSG "Couldn't write"
#define READ_ERR 103
#define READ_MSG "Couldn't read"
#define STAT_ERR 104
#define STAT_MSG "Couldn't stat"
#define OPEN_ERR 106
#define OPEN_MSG "Couldn't open file"

#define ARGS_COUNT 3
#define BUF_SIZE 4096

void sys_err(int status, int code, const char* message) {
    if (status == -1) {
        err(code, message);
    }
}

void assert_true(bool success, int code, const char* message) {
    if (!success) {
        errx(code, message);
    }
}

int open_file(const char* filename, int flags, mode_t mode) {
    int fd = open(filename, flags, mode);
    sys_err(fd, OPEN_ERR, OPEN_MSG);
    return fd;
}

int main(int argc, char** argv) {
    
    assert_true(argc == ARGS_COUNT, ARGS_ERR, ARGS_MSG);

    int input = open_file(argv[1], O_RDONLY, 0);
    int output = open_file(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    char buf[BUF_SIZE];
    ssize_t bytes_read;
    while ( (bytes_read = read(input, buf, sizeof(buf))) > 0 ) {
        size_t bytes_read_positive = (size_t)bytes_read;
        uint8_t res = 0;
        for (size_t i = 0; i < bytes_read_positive; i++) {
            for (uint8_t j = 0; j < 8; j+=2) {
                res = res << 1;
                res = res | ((buf[i] >> (7 - j)) & 1);
            }
            if (i % 2 == 1) {
                sys_err(write(output, &res, sizeof(uint8_t)), WRITE_ERR, WRITE_MSG);
                res = 0;
            }
        }
    }
    sys_err(bytes_read, READ_ERR, READ_MSG);
    close(input);
    close(output);
    return 0;
}
