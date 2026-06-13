#include <sys/types.h>
#include <stdbool.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>

// ERROR HANDLING CONSTANTS
#define OPEN_FILE_ERR 400
#define WRONG_ARG_COUNT_ERR 401
#define READ_FILE_ERR 402
#define WRITE_FILE_ERR 403
#define STAT_FILE_ERR 404
#define WRONG_FILE_SIZE_ERR 405
#define SEEK_FILE_ERR 406

// LOGIC CONSTANTS
#define ARGS_REQUIRED 2 

// ERROR HANDLING
static void sys_err(const int exit_code, const int status_code, const char* filename) {
    if (exit_code == -1) {
        err(status_code, filename);
    }
}

static void assert_true(const bool success, const int status_code, const char* filename) {
    if (!success) {
        errx(status_code, filename);
    }
}

// GENERIC FILE OPERATIONS
static int open_file(const char* filename, const int flags, const mode_t mode) {
    int fd = open(filename, flags, mode);
    sys_err(fd, OPEN_FILE_ERR, "Couldn't open file");
    return fd;
}

static size_t get_file_size(int fd) {
    struct stat file_stat;
    sys_err(fstat(fd, &file_stat), STAT_FILE_ERR, "Couldn't stat");
    
    off_t file_size = file_stat.st_size;
    assert_true(file_size >= 0, STAT_FILE_ERR, "st_size was negative. System/File is probably corrupted");
    return (size_t)file_size;
}

static void assert_file_size_divisible_by(int fd, size_t by) {
    size_t file_size = get_file_size(fd);
    assert_true(file_size % by == 0, WRONG_FILE_SIZE_ERR, "Wrong file size");
}

static void seek_to(int fd, size_t pos) {
    sys_err(lseek(fd, pos * sizeof(uint32_t), SEEK_SET), SEEK_FILE_ERR, "Couldn't seek");
}

static void write_to_file(int fd, uint32_t toWrite) {
    sys_err(write(fd, &toWrite, sizeof(toWrite)), WRITE_FILE_ERR, "Couldn't write");
}

static uint32_t read_from_file(int fd) {
    uint32_t toRead;
    sys_err(read(fd, &toRead, sizeof(toRead)), READ_FILE_ERR, "Couldn't read");
    return toRead;
}

// SPECIFIC FILE OPERATIONS
static uint32_t read_el(int fd, size_t pos) {
    seek_to(fd, pos);
    return read_from_file(fd);
}

static void write_el(int fd, uint32_t el, size_t pos) {
    seek_to(fd, pos);
    write_to_file(fd, el);
}

// ALGORITHM
static void bubble_sort_file(int fd) {
    size_t file_size = get_file_size(fd);
    for (size_t i = 0; i < file_size; i++) {
        for (size_t j = i; j < file_size - 1; j++) {
            uint32_t curr_el = read_el(fd, j);
            uint32_t next_el = read_el(fd, j + 1);
            if (curr_el > next_el) {
                write_el(fd, next_el, j);
                write_el(fd, curr_el, j + 1);
            }
        }
    }
}

int main(int argc, char** argv) {
    assert_true(argc == ARGS_REQUIRED, WRONG_ARG_COUNT_ERR, "Wrong arg count");

    int file = open_file(argv[1], O_RDWR, 0);
    assert_file_size_divisible_by(file, sizeof(uint32_t));
    
    bubble_sort_file(file);

    close(file);

    return 0;
}
