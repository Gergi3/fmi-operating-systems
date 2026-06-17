#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include <stdlib.h>

#define FORK_ERR 100, "Couldn't fork"
#define EXEC_ERR 101, "Couldn't exec command"
#define WRITE_ERR 102, "Couldn't write"
#define READ_ERR 103, "Couldn't read"

#define PROMPT_MSG "Prompt: "
#define BUF_SIZE 4096

void sys_err(int status, int code, const char* message);
void write_s(int fd, const void* buf, size_t len);
size_t read_s(int fd, void* buf, size_t len);

void sys_err(int status, int code, const char* message) {
    if (status == -1) {
        err(code, message);
    }
}

void write_s(int fd, const void* buf, size_t len) {
    sys_err(write(fd, buf, len), WRITE_ERR);
}

size_t read_s(int fd, void* buf, size_t len) {
    ssize_t read_bytes = read(fd, buf, len);
    sys_err(read_bytes, READ_ERR);
    return (size_t)read_bytes;
}

int main(void) {
    while (true) {
        write_s(STDOUT_FILENO, &PROMPT_MSG, strlen(PROMPT_MSG)); 
        
        char cmd[BUF_SIZE];
        size_t read = read_s(STDIN_FILENO, cmd, sizeof(cmd));
        cmd[read - 1] = '\0';
        if (strcmp(cmd, "exit") == 0) {
            exit(0);
        }

        pid_t child_pid = fork();
        if (child_pid == 0) {
            sys_err(execlp(cmd, cmd, NULL), EXEC_ERR);
        }
    }
}
