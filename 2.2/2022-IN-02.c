#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define PIPE_ERR 100, "Couldn't create pipe"
#define EXEC_ERR 101, "Couldn't execute process"
#define WRITE_ERR 102, "Couldn't write"
#define READ_ERR 103, "Couldn't read"
#define WAIT_ERR 104, "Couldn't wait"
#define FORK_ERR 105, "Couldn't fork"
#define ARGS_ERR 106, "Wrong args" 

#define REQUIRED_ARGS 3
#define DING "DING "
#define DONG "DONG\n"
#define SIGNAL "0"

void sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
void ding(void);
void dong(void);
void sem_signal(int sem[2]);
bool sem_wait(int sem[2]);
void solve(int n, int d);

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

static int s1[2];
static int s2[2];

void ding(void) {
    sys_err(write(STDOUT_FILENO, &DING, strlen(DING)), WRITE_ERR);
}

void dong(void) {
    sys_err(write(STDOUT_FILENO, &DONG, strlen(DONG)), WRITE_ERR);
}

void sem_signal(int sem[2]) {
    sys_err(write(sem[1], &SIGNAL, 1), WRITE_ERR);
}

bool sem_wait(int sem[2]) {
    char bs;
    ssize_t read_bytes = read(sem[0], &bs, 1);
    sys_err(read_bytes, READ_ERR);
    
    return read_bytes == 0;
}

void solve(int n, int d) {
    
    sys_err(pipe(s1), PIPE_ERR);
    sys_err(pipe(s2), PIPE_ERR);

    pid_t child_pid = fork();
    sys_err(child_pid, FORK_ERR);
    
    int count = 0;

    if (child_pid > 0) {
        while(count < n) {
            close(s1[0]);
            close(s2[1]);

            ding();
            sem_signal(s1);
            sem_wait(s2);

            sys_err(sleep(d), 120, "MARZI ME SLEEP ERR");
            count++;
        }
        
        close(s1[1]);
        close(s2[0]);

        int status;
        sys_err(wait(&status), WAIT_ERR);
        
        if (!WIFEXITED(status)) {
            errx(WAIT_ERR);
        }

        if (WEXITSTATUS(status) != 0) {
            errx(WAIT_ERR);
        }
    }

    if (child_pid == 0) {
        close(s1[1]);
        close(s2[0]);
        
        while (true) {
            if (sem_wait(s1)) {
                close(s1[0]);
                close(s2[1]);
                exit(0);
            }
            dong();
            sem_signal(s2);
        }
    }
}

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR);
    
    int n = atoi(argv[1]);
    int d = atoi(argv[2]);
    
    solve(n, d);
}
