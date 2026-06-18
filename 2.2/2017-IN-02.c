#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/wait.h>
#include <err.h>

#define ARGS_ERR 100, "Invalid args"
#define READ_ERR 101, "Couldn't read from fd"
#define WRITE_ERR 102, "Couldnt write to fd"
#define WAIT_ERR 103, "Couldn't wait for child"
#define READ_ARG_TOO_LONG_ERR 104, "Given arg was too long"
#define EXEC_ERR 105, "Couldn't execute program"
#define FORK_ERR 106, "Couldn't fork"

#define MAX_CMD_LEN 4
#define MAX_PARAM_LEN 4

bool read_arg(char arg[MAX_PARAM_LEN + 1]);
void wait_for_children(size_t n);
void assert_true(bool success, int code, const char* message);
int sys_err(int status, int code, const char* message);

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

void wait_for_children(size_t n) {
    for (size_t i = 0; i < n; i++) {
        int wstatus;
        wait(&wstatus);
        assert_true(WIFEXITED(wstatus), WAIT_ERR); 
        assert_true(WEXITSTATUS(wstatus) == 0, WAIT_ERR); 
    }
}

bool read_arg(char arg[MAX_PARAM_LEN + 1]) {
    size_t curr_len = 0;
    char curr;
    ssize_t read_bytes;
    while ( (read_bytes = read(STDIN_FILENO, &curr, sizeof(curr))) > 0) {
        if (curr == '\x20' || curr == '\x0A') {
            break;
        }

        arg[curr_len++] = curr;
        assert_true(curr_len <= MAX_PARAM_LEN, READ_ARG_TOO_LONG_ERR); 
    }
    sys_err(read_bytes, READ_ERR);
    if (read_bytes == 0 && curr_len == 0) {
        // no more args
        return false;
    }

    arg[curr_len] = '\0';
    return true;
}

int main(int argc, char** argv) {
    assert_true(argc <= 2, ARGS_ERR);

    const char* cmd = "echo";
    if (argc == 2) {
        assert_true(strlen(argv[1]) <= MAX_CMD_LEN, ARGS_ERR);
        cmd = argv[1];
    }
    
    while (true) {
        char arg1[5];
        char* arg1_ptr = NULL;
        char arg2[5];
        char* arg2_ptr = NULL;
        
        if (read_arg(arg1)) {
            arg1_ptr = arg1;
        }
        if (read_arg(arg2)) {
            arg2_ptr = arg2;
        }
        
        if (arg1_ptr == NULL && arg2_ptr == NULL) {
            break;
        }
                
        pid_t child_pid = sys_err(fork(), FORK_ERR);
        if (child_pid == 0) {
            sys_err(execlp(cmd, cmd, arg1_ptr, arg2_ptr, NULL), EXEC_ERR);
        }

        wait_for_children(1);
    }
}
