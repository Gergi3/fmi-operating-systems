#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

// ERR CONSTANTS
#define FILE_OPEN_ERR 100
#define FILE_OPEN_MSG "Couldn't open file"
#define FILE_READ_ERR 101
#define FILE_READ_MSG "Couldn't read from file"
#define FILE_WRITE_ERR 102
#define FILE_WRITE_MSG "Couldn't write to file"
#define FILE_SEEK_ERR 103
#define FILE_SEEK_MSG "Couldn't lseek in file"
#define FILE_STAT_ERR 104
#define FILE_STAT_MSG "Couldn't stat file"
#define FILE_NEGATIVE_SIZE_ERR 105
#define FILE_NEGATIVE_SIZE_MSG "File size was negative indication system or file corruption"
#define ARGS_ERR 105
#define ARGS_MSG "Invalid args"
#define FILE_SIZE_ERR 106
#define FILE_SIZE_MSG "File size was invalid"

// LOGIC CONSTANTS
#define REQUIRED_ARGS 3
#define BUFFER_SIZE 4096

void sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
int open_file(const char* filename, int flags, mode_t mode);
void write_all(int fd, const void* buf, size_t len);
void cpy_file(int src, int dest);
void seek_to(int fd, size_t pos);
void write_num(int fd, uint32_t num, size_t pos);
void bubble_sort(int file);
uint32_t read_num(int fd, size_t pos);
size_t get_file_size(int fd);

// ERROR HANDLING
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

// FD OPERATIONS
int open_file(const char* filename, int flags, mode_t mode) {
    int fd = open(filename, flags, mode);
    sys_err(fd, FILE_OPEN_ERR, FILE_OPEN_MSG);
    return fd;
}

void write_all(int fd, const void* buf, size_t len) {
    const char* p = buf;

    size_t written = 0;
    while (written < len) {
        ssize_t written_bytes = write(fd, p + written, len - written);
        sys_err(written_bytes, FILE_WRITE_ERR, FILE_WRITE_MSG);
        written += (size_t)written_bytes;
    }
}

void cpy_file(int src, int dest) {
    char buf[BUFFER_SIZE];

    ssize_t read_bytes;
    while ( (read_bytes = read(src, buf, sizeof(buf))) > 0) {
        write_all(dest, buf, read_bytes);
    }
    sys_err(read_bytes, FILE_READ_ERR, FILE_READ_MSG);

    seek_to(src, 0);
    seek_to(dest, 0);
}

void seek_to(int fd, size_t pos) {
    off_t res = lseek(fd, pos * sizeof(uint32_t), SEEK_SET);
    sys_err(res, FILE_SEEK_ERR, FILE_SEEK_MSG);
}

size_t get_file_size(int fd) {
    struct stat stat;
    sys_err(fstat(fd, &stat), FILE_STAT_ERR, FILE_STAT_MSG);

    off_t file_size = stat.st_size;
    assert_true(file_size >= 0, FILE_NEGATIVE_SIZE_ERR, FILE_NEGATIVE_SIZE_MSG);
    return (size_t)file_size;
}

uint32_t read_num(int fd, size_t pos) {
    seek_to(fd, pos);
    uint32_t num;
    ssize_t read_bytes = read(fd, &num, sizeof(num));
    sys_err(read_bytes, FILE_READ_ERR, FILE_READ_MSG);
    return num;
}

void write_num(int fd, uint32_t num, size_t pos) {
    seek_to(fd, pos);
    write_all(fd, &num, sizeof(num));
}

// ALGORITHM
void bubble_sort(int file) {
    size_t nums_count = get_file_size(file) / sizeof(uint32_t);    
    
    for (size_t i = 0; i < nums_count; i++) {
        for (size_t j = 0; j < nums_count - i - 1; j++) {
            uint32_t lhs = read_num(file, j);
            uint32_t rhs = read_num(file, j + 1);
            
            if (lhs > rhs) {
                write_num(file, lhs, j + 1);
                write_num(file, rhs, j);
            }
        }
    }
}

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR, ARGS_MSG); 

    int src = open_file(argv[1], O_RDONLY, 0);
    size_t src_size = get_file_size(src);
    assert_true(src_size % sizeof(uint32_t) == 0, FILE_SIZE_ERR, FILE_SIZE_MSG);

    int dest = open_file(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    cpy_file(src, dest);
    bubble_sort(dest);

    close(src);
    close(dest);
}
