#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <err.h>
#include <sys/types.h>

#define ARGS_ERR 100
#define ARGS_MSG "Wrong args count"
#define FORK_ERR 101
#define FORK_MSG "Couldn't fork"
#define F_READ_ERR 102
#define F_READ_MSG "Couldn't read"
#define DUP_ERR 103
#define DUP_MSG "Couldn't dup2"
#define EXEC_ERR 104
#define EXEC_MSG "Couldn't exec"
#define PIPE_ERR 105
#define PIPE_MSG "Couldn't create a pipe"

#define REQUIRED_ARGS 2

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

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR, ARGS_MSG); 
    
    const char* filename = argv[1];

    int pfd[2]; 
    sys_err(pipe(pfd), PIPE_ERR, PIPE_MSG);

    pid_t pid = fork();
    sys_err(pid, FORK_ERR, FORK_MSG);
    
    if (pid > 0) {
        // cat
        close(pfd[0]);
        sys_err(dup2(pfd[1], STDOUT_FILENO), DUP_ERR, DUP_MSG);
        close(pfd[1]);
        sys_err(execlp("cat", "cat", filename, NULL), EXEC_ERR, EXEC_MSG);
    }
    
    if (pid == 0) {
        // sort
        close(pfd[1]);
        sys_err(dup2(pfd[0], STDIN_FILENO), DUP_ERR, DUP_MSG);
        close(pfd[0]);
        sys_err(execlp("sort", "sort", NULL), EXEC_ERR, EXEC_MSG);
    }
}
