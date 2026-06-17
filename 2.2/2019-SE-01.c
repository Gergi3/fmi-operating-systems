#include <stdint.h>
#include <stdbool.h>
#include <err.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

#define SEEK_ERR 99, "Couldn't seek"
#define FORK_ERR 100, "Couldn't fork"
#define EXEC_ERR 101, "Couldn't exec"
#define WAIT_ERR 102, "Couldn't wait"
#define PIPE_ERR 103, "Couldn't pipe"
#define READ_ERR 104, "Couldn't read"
#define WRITE_ERR 105, "Couldn't write"
#define ARGS_ERR 106, "Invalid args"
#define OPEN_ERR 107, "Couldn't open file"

#define REQUIRED_ARGS 3

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

void write_s(int fd, const void* buf, size_t len) {
    const char* p = buf;
    sys_err(write(fd, p, len), WRITE_ERR);
}

void read_s(int fd, void* buf, size_t len) {
    char* p = buf;
    sys_err(read(fd, p, len), READ_ERR);
}

void write_time(int fd) {
    time_t now = time(NULL);
    write_s(fd, &now, sizeof(now));
    write_s(fd, &" ", 1);
}

time_t get_difference(int fd) {
    time_t start;
    time_t end;
    sys_err(lseek(fd, -2 * sizeof(time_t) - 2, SEEK_CUR), SEEK_ERR);
    read_s(fd, &start, sizeof(start));
    char discard;
    read_s(fd, &discard, sizeof(discard));
    read_s(fd, &end, sizeof(end));
    read_s(fd, &discard, sizeof(discard));

    return end - start;
}

void solve(int num, const char* q, char** q_args) {
    int fd = open("run.log", O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
    sys_err(fd, OPEN_ERR);
    
    bool last_failed = false;
    while (true) {
        pid_t child_pid = fork();
        sys_err(child_pid, FORK_ERR);
        
        if (child_pid == 0) {
            write_time(fd);
            close(fd);
            
            sys_err(execv(q, q_args), EXEC_ERR);
        }
        
        int wstatus;
        sys_err(wait(&wstatus), WAIT_ERR);
        write_time(fd);

        if (!WIFEXITED(wstatus)) {
            wstatus = 129;
        } else {
            wstatus = WEXITSTATUS(wstatus);
        }
        int diff = (int)get_difference(fd);
        
        write_s(fd, &wstatus, sizeof(wstatus));
        write_s(fd, &"\n", 1);
        
        if (wstatus != 0 && diff < num) {
            if (last_failed) {
                close(fd);
                return;
            } else {
                last_failed = true;
            }
        }
    }
}

int main(int argc, char** argv) {
    assert_true(argc >= REQUIRED_ARGS, ARGS_ERR);

    int num = atoi(argv[1]);
    assert_true(num >= 1 && num <= 9, ARGS_ERR);

    const char* q = argv[2];
    char** q_args = argv + 2;

    solve(num, q, q_args); 
}
