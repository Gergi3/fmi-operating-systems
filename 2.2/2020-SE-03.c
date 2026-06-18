#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

#define ARGS_ERR 99, "Invalid args count"
#define ENTRY_FILE_ERR 100, "Invalid entry file"
#define TOO_MANY_ENTRIES_ERR 101, "Too many entries"
#define WAIT_ERR 102, "Couldn't wait child"
#define FORK_ERR 103, "Couldn't fork a process"
#define PIPE_ERR 104, "Couldn't create a pipe"
#define READ_ERR 105, "Couldn't read from fd"
#define WRITE_ERR 106, "Couldn't write to fd"
#define OPEN_ERR 107, "Couldn't open fd"
#define SEEK_ERR 108, "Couldn't lseek in fd"
#define STAT_ERR 109, "Coudln't stat fd"
#define DUP_ERR 110, "Couldn't dup2"

#define REQUIRED_ARGS 2
#define MAX_ENTRIES 8
#define FILENAME_SIZE 8
#define BUF_SIZE 4096


int sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
size_t get_file_size(int fd);
size_t read_s(int fd, void* buf, size_t len);
void write_s(int fd, const void* buf, size_t len);
int open_s(const char* filename, int flags, mode_t mode);
void start_child(int fd);
void wait_children(size_t n);

int sys_err(int status, int code, const char* message) {
    if (status == -1) {
        err(code, message);
    }
    return status;
}

void assert_true(bool success, int code, const char* message) {
    if (!success) {
        errx(code, message);
    }
}

size_t get_file_size(int fd) {
    struct stat s;
    sys_err(fstat(fd, &s), STAT_ERR);
    assert_true(s.st_size > 0, STAT_ERR);
    return (size_t)s.st_size;
}

size_t read_s(int fd, void* buf, size_t len) {
    char* p = buf;
    ssize_t read_bytes = read(fd, p, len);
    sys_err(read_bytes, READ_ERR);
    return read_bytes;
}

void write_s(int fd, const void* buf, size_t len) {
    const char* p = buf;
    ssize_t written_bytes = write(fd, p, len);
    sys_err(written_bytes, WRITE_ERR);
}

int open_s(const char* filename, int flags, mode_t mode) {
    return sys_err(open(filename, flags, mode), OPEN_ERR);
}

static int child_result_pipe[2];

void start_child(int fd) {
    char filename[FILENAME_SIZE];
    read_s(fd, filename, sizeof(filename));
    uint32_t offset;
    read_s(fd, &offset, sizeof(offset));
    uint32_t length;
    read_s(fd, &length, sizeof(length));

    pid_t child_pid = sys_err(fork(), FORK_ERR);

    if (child_pid == 0) {
        close(fd);
        close(child_result_pipe[0]);

        int entry_file = open_s(filename, O_RDONLY, 0);
        sys_err(lseek(entry_file, offset * sizeof(uint16_t), SEEK_SET), SEEK_ERR);

        uint16_t result = 0;
        for (size_t i = 0; i < length; i++) {
            uint16_t num;
            read_s(entry_file, &num, sizeof(num));
            result ^= num;
        }

        write_s(child_result_pipe[1], &result, sizeof(result)); 
        close(child_result_pipe[1]);
        exit(0);
    }
}

void wait_children(size_t n) {
    for (size_t i = 0; i < n; i++) {
        int wstatus;
        sys_err(wait(&wstatus), WAIT_ERR);

        assert_true(WIFEXITED(wstatus), WAIT_ERR);
        assert_true(WEXITSTATUS(wstatus) == 0, WAIT_ERR);
    }
}

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR);

    int fd = open(argv[1], O_RDONLY, 0);

    size_t size = get_file_size(fd);
    size_t entry_size = FILENAME_SIZE + sizeof(uint32_t) * 2;
    assert_true(size % entry_size == 0, ENTRY_FILE_ERR);

    size_t entries = size / entry_size;
    assert_true(entries <= MAX_ENTRIES, TOO_MANY_ENTRIES_ERR);

    sys_err(pipe(child_result_pipe), PIPE_ERR);

    for (size_t i = 0; i < entries; i++) {
        start_child(fd);
    }
    close(fd);
    close(child_result_pipe[1]);

    uint16_t result = 0;
    uint16_t curr;
    ssize_t read_bytes = 0;
    while ( (read_bytes = read_s(child_result_pipe[0], &curr, sizeof(curr))) > 0 ) {
        result ^= curr;
    }

    close(child_result_pipe[0]);
    wait_children(entries);
    
    char hex[5];
    snprintf(hex, sizeof(hex), "%04X", result);
    write(STDOUT_FILENO, "result: ", 9);
    write(STDOUT_FILENO, hex, 4);
    write(STDOUT_FILENO, "\n", 1);
}
