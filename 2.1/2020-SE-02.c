#include <unistd.h>
#include <err.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ARGS_ERR 100
#define ARGS_MSG "Invalid args"
#define FILE_OPEN_ERR 102
#define FILE_OPEN_MSG "Cannot open file"
#define FILE_READ_ERR 103
#define FILE_READ_MSG "Cannot read file"
#define FILE_WRITE_ERR 104
#define FILE_WRITE_MSG "Cannot write file"
#define FILE_STAT_ERR 105
#define FILE_STAT_MSG "Cannot stat file"
#define FILE_SIZE_ERR 106
#define FILE_SIZE_MSG "File is either of incorrect size or incompatible compared to the other"
#define FILE_SEEK_ERR 107
#define FILE_SEEK_MSG "Couldn't seek file"

#define REQUIRED_ARGS 4
#define BUF_SIZE 4096

void sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
int open_file(const char* filename, int flags, mode_t mode);
void write_all(int fd, const void* buf, size_t len);
size_t get_file_size(int fd);
void seek_to(int fd, size_t pos);

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
    sys_err(fd, FILE_OPEN_ERR, FILE_OPEN_MSG);
    return fd;
}

void write_all(int fd, const void* buf, size_t len) {
    const char* p = buf;
    size_t written = 0;
    while (written < len) {
        ssize_t curr_written = write(fd, p, len - written);
        sys_err(curr_written, FILE_WRITE_ERR, FILE_WRITE_MSG);
        written += (size_t)curr_written;
    }
}

size_t get_file_size(int fd) {
    struct stat stat;
    sys_err(fstat(fd, &stat), FILE_STAT_ERR, FILE_STAT_MSG);
    assert_true(stat.st_size > 0, FILE_STAT_ERR, FILE_STAT_MSG);
    return (size_t)stat.st_size;
}

void seek_to(int fd, size_t pos) {
    sys_err(lseek(fd, pos * sizeof(uint16_t), SEEK_SET), FILE_SEEK_ERR, FILE_SEEK_MSG);
}

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR, ARGS_MSG);

    int scl = open_file(argv[1], O_RDONLY, 0);
    size_t scl_size = get_file_size(scl);

    int sdl = open_file(argv[2], O_RDONLY, 0);
    size_t sdl_size = get_file_size(sdl);

    assert_true(sdl_size % sizeof(uint16_t) == 0, FILE_SIZE_ERR, FILE_SIZE_MSG);
    assert_true(scl_size == sdl_size / 16, FILE_SIZE_ERR, FILE_SIZE_MSG);

    int output = open_file(argv[3], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    size_t pos_sdl = 0;
    char buf[BUF_SIZE];
    ssize_t bytes_read;
    while ( (bytes_read = read(scl, buf, sizeof(buf))) > 0) {
        for (size_t i = 0; i < (size_t)bytes_read; i++) {
            for (uint8_t j = 0; j < 8; j++) {
                bool is_high = (bool)((buf[i] >> (7 - j)) & 1);
                
                if (is_high) {
                    seek_to(sdl, pos_sdl);
                    uint16_t num;
                    sys_err(read(sdl, &num, sizeof(num)), FILE_READ_ERR, FILE_READ_MSG);
                    write_all(output, &num, sizeof(num));
                }
                pos_sdl++;
            }
        }
    }
    sys_err(bytes_read, FILE_READ_ERR, FILE_READ_MSG);

    close(scl);
    close(sdl);
    close(output);
}
