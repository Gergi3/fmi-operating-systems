#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <wait.h>

#define PIPE_ERR 101, "Couldn't create a pipe"
#define EXEC_ERR 102, "Couldn't exec command"
#define DUP_ERR 103, "Couldn't duplicate pipe"
#define FORK_ERR 104, "Couldn't fork process"
#define WAIT_ERR 105, "Couldn't wait for childs"

void sys_err(int status, int code, const char* message) {
    if (status == -1) {
        err(code, message);
    }
}

void pipe_exec_cmds(char** cmds[], size_t size) {

    int prev_read = -1;
    int pfd[2];
    
    for (size_t i = 0; i < size; i++) {
        char* cmd = cmds[i][0];
        char** args = cmds[i];

        if (i != size - 1) {
            sys_err(pipe(pfd), PIPE_ERR);
        }

        pid_t child_pid = fork();
        sys_err(child_pid, FORK_ERR);

        if (child_pid == 0) {
            if (i != 0) {
                sys_err(dup2(prev_read, STDIN_FILENO), DUP_ERR);
                close(prev_read);
            }

            if (i != size - 1) {
                close(pfd[0]);
                sys_err(dup2(pfd[1], STDOUT_FILENO), DUP_ERR);
                close(pfd[1]);
            }

            sys_err(execvp(cmd, args), EXEC_ERR);
        }
        
        if (i != 0) {
            close(prev_read);
        }

        if (i != size - 1) {
            close(pfd[1]);
            prev_read = pfd[0];
        }
    }

    for (size_t i = 0; i < size; i++) {
        int status;
        pid_t child_pid = wait(&status);
        sys_err(child_pid, WAIT_ERR);

        if (!WIFEXITED(status)) {
            errx(WAIT_ERR);
        } else if (WEXITSTATUS(status) != 0) {
            errx(WAIT_ERR);
        }
    }
}

int main(void) {
    char cmd1_0[] = "cat", cmd1_1[] = "/etc/passwd";
    char cmd2_0[] = "cut", cmd2_1[] = "-d", cmd2_2[] = ":", cmd2_3[] = "-f", cmd2_4[] = "7";
    char cmd3_0[] = "sort";
    char cmd4_0[] = "uniq", cmd4_1[] = "-c";
    char cmd5_0[] = "sort";

    char **cmds[] = {
        (char *[]){cmd1_0, cmd1_1, NULL},
        (char *[]){cmd2_0, cmd2_1, cmd2_2, cmd2_3, cmd2_4, NULL},
        (char *[]){cmd3_0, NULL},
        (char *[]){cmd4_0, cmd4_1, NULL},
        (char *[]){cmd5_0, NULL},
    };
    pipe_exec_cmds(cmds, 5);
}
