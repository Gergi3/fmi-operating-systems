#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdint.h>

#define ARGS_ERR 100, "Invalid arguments"
#define READ_ERR 101, "Couldn't read from fd"
#define WRITE_ERR 102, "Couldn't write to fd"
#define WAIT_ERR 103, "Couldn't wait for child"
#define WAIT_EXIT_ERR 108, "Child was interrupted"
#define WAIT_STATUS_ERR 109, "Child didn't exit with status code 0"
#define FORK_ERR 104, "Couldn't fork a child"
#define EXEC_ERR 105, "Couldn't execute external command"
#define DUP_ERR 106, "Couldn't duplicate fd"
#define OPEN_ERR 107, "Couldn't open fd"
#define SEEK_ERR 110, "Couldn't seek"
#define PIPE_ERR 111, "Couldn't create a pipe"

#define ARGS_COUNT 3
#define BUF_SIZE 4096

int sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
void start_read(const char* filename);
void wait_children(size_t n);
bool is_special(char symb);

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

static int pfd[2];

void start_read(const char* filename) {
    
    sys_err(pipe(pfd), PIPE_ERR);

    pid_t child_pid = sys_err(fork(), FORK_ERR);
    
    if (child_pid == 0) {
        close(pfd[0]);
        sys_err(dup2(pfd[1], STDOUT_FILENO), DUP_ERR);
        close(pfd[1]);

        int err = execlp("cat", "cat", filename, NULL);
        sys_err(err, EXEC_ERR);
    }

    close(pfd[1]);
}

void wait_children(size_t n) {
    for (size_t i = 0; i < n; i++) {
        int wstatus;
        sys_err(wait(&wstatus), WAIT_ERR);
    
        assert_true(WIFEXITED(wstatus), WAIT_EXIT_ERR);
        assert_true(WEXITSTATUS(wstatus) == 0, WAIT_EXIT_ERR);
    }
}

bool is_special(char symb) {
    return symb == '\x00' || symb == '\xFF' || symb == '\x55' || symb == '\x7D';
}

int main(int argc, char** argv) {
    assert_true(argc == ARGS_COUNT, ARGS_ERR);
    
    const char* output_fn = argv[2];
    int output = sys_err(open(output_fn, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR), OPEN_ERR);
    
    const char* input_fn = argv[1];
    start_read(input_fn);

    int input = pfd[0];

    char curr;
    ssize_t read_bytes;
    char prev = '\x0';
    while ( (read_bytes = read(input, &curr, sizeof(curr))) > 0 ) {
        if (curr == '\x55') {
            continue;
        }

        char to_write = curr;
        if (prev == '\x7D' && is_special(curr ^ '\x20')) {
            sys_err(lseek(output, -1 * sizeof(char), SEEK_CUR), SEEK_ERR);
            to_write ^= '\x20';
        }
        prev = curr;
        sys_err(write(output, &to_write, sizeof(to_write)), WRITE_ERR);
    }
    sys_err(read_bytes, READ_ERR);

    close(pfd[0]);
    wait_children(1);
}
