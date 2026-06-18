#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define ARGS_COUNT_ERR 100, "Invalid arg count"
#define OPEN_ERR 101, "Couldn't open fd"
#define READ_ERR 102, "Couldn't read from fd"
#define WRITE_ERR 103, "Couldnt write to fd"
#define WAIT_ERR 108, "Couldnt wait for process to finish"
#define FORK_ERR 104, "Couldn't for process"
#define EXEC_ERR 105, "Couldnt execute process"
#define PIPE_ERR 106, "Couldn't create pipe"
#define KILL_ERR 107, "Couldn't send signal to process"

#define MIN_ARGS 2
#define MAX_ARGS 11
#define MAX_PROGRAMS 10

void assert_true(bool success, int code, const char* message);
void start_program(const char* program, size_t program_no);
void kill_all_and_wait(size_t programs_count);
int sys_err(int status, int code, const char* message);
pid_t fork_s(void);
void watch_programs(char** program_names, size_t programs_count);

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

pid_t fork_s(void) {
    return sys_err(fork(), FORK_ERR);
}

static pid_t programs[MAX_PROGRAMS];
static bool alive_p[MAX_PROGRAMS];

void start_program(const char* program, size_t program_no) {
    pid_t child_pid = fork_s();

    if (child_pid > 0) {
        programs[program_no] = child_pid;
        alive_p[program_no] = true;
    }

    if (child_pid == 0) {
        execl(program, program, NULL);
    }
}

void kill_all_and_wait(size_t programs_count) {
    size_t killed = 0;
    for (size_t i = 0; i < programs_count; i++) {
        if (alive_p[i]) {
            killed++;
            sys_err(kill(programs[i], SIGTERM), KILL_ERR);
        }
    }

    for (size_t i = 0; i < killed; i++) {
        wait(NULL);
    }
}

void watch_programs(char** program_names, size_t programs_count) {
    for (size_t i = 0; i < programs_count; i++) {
        start_program(program_names[i], i);
    }

    size_t finished_succesfully = 0;
    while (finished_succesfully < programs_count) {
        int wstatus;
        pid_t child_pid = wait(&wstatus);
        
        size_t indx = -1;
        for (size_t i = 0; i < programs_count; i++) {
            if (child_pid == programs[i]) {
                indx = i;
            }
        }

        if (!WIFEXITED(wstatus)) {
            alive_p[indx] = false;
            kill_all_and_wait(programs_count); 
            exit(indx + 1);
        } else if (WEXITSTATUS(wstatus) != 0) {
            start_program(program_names[indx], indx);
        } else {
            finished_succesfully++;
            alive_p[indx] = false;
        }
    }

}

int main(int argc, char** argv) {
    assert_true(argc >= MIN_ARGS && argc <= MAX_ARGS, ARGS_COUNT_ERR);

    watch_programs(argv + 1, argc - 1);
}
