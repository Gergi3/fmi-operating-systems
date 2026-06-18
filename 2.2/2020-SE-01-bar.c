#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/wait.h>

#define ARGS_ERR 100, "Invalid args"
#define MKFIFO_ERR 101, "Couldn't create a named pipe"
#define EXEC_ERR 102, "Couldn't execute command"
#define READ_ERR 104, "Couldn't read"
#define WRITE_ERR 105, "Couldn't write"
#define OPEN_ERR 106, "Couldn't open fd"
#define DUP_ERR 107, "Couldn't dup fds"

#define REQUIRED_ARGS 2
#define FIFO_NAME "somefifo"

void sys_err(int status, int code, const char* message);
int open_fifo(int flags);
void assert_true(bool success, int code, const char* message);

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

int open_fifo(int flags) {
    int fd = open(FIFO_NAME, flags);
    sys_err(fd, OPEN_ERR);
    return fd;
}

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR);

    int fd = open_fifo(O_RDONLY);
    sys_err(dup2(fd, STDIN_FILENO), DUP_ERR);    
    close(fd);

    const char* cmd = argv[1];
    int err = execl(cmd, cmd, NULL);
    sys_err(err, EXEC_ERR);
}
