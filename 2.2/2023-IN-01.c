#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define WRITE_ERR 100, "Couldn't write to fd"
#define READ_ERR 101, "Couldn't read from fd"
#define EXEC_ERR 102, "Couldn't exec program"
#define FORK_ERR 103, "Couldn't fork a child"
#define DUP_ERR 104, "Couldn't dup2 a fd"
#define WAIT_ERR 105, "Couldn't wait for child"
#define WAIT_EXITED_ERR 105, "Child was interrupted"
#define WAIT_STATUS_ERR 106, "Child didn't finish successfully"
#define PIPE_ERR 107, "Couldn't create a pipe"
#define ARGS_ERR 108, "Invalid args"

#define SIGNAL "\0"
#define REQUIRED_ARGS 3
#define MIN_NC 1
#define MAX_NC 7
#define MIN_WC 1
#define MAX_WC 35

#define WORD_SIZE 4
#define WORD_COUNT 3

static char words[WORD_COUNT][WORD_SIZE] = { "tic ", "tac ", "toe\n" };

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

void write_s(int fd, const void* buf, size_t len) {
    sys_err(write(fd, (const char*)buf, len), WRITE_ERR);
}

size_t read_s(int fd, void* buf, size_t len) {
    ssize_t read_bytes = read(fd, (char*)buf, len);
    sys_err(read_bytes, READ_ERR);
    return (size_t)read_bytes;
}

void pipe_s(int pfd[2]) {
    sys_err(pipe(pfd), PIPE_ERR);
}

void sem_signal(int pfd[2]) {
    write_s(pfd[1], &SIGNAL, sizeof(char)); 
}

void sem_wait(int pfd[2]) {
    char discard;
    read_s(pfd[0], &discard, sizeof(char));
}

void wait_children(size_t n) {
    for (size_t i = 0; i < n; i++) {
        int wstatus;
        sys_err(wait(&wstatus), WAIT_ERR);
        assert_true(WIFEXITED(wstatus), WAIT_EXITED_ERR);
        assert_true(WEXITSTATUS(wstatus) == 0, WAIT_STATUS_ERR);
    }
}

pid_t fork_s() {
    pid_t child_pid = fork();
    sys_err(child_pid, FORK_ERR);
    return child_pid;
}

void dup2_s(int oldfd, int newfd) {
    sys_err(dup2(oldfd, newfd), DUP_ERR);
}

size_t get_offset(size_t process_count) {
    return process_count % WORD_COUNT;
}

static int semaphores[MAX_NC + 1][2];

void close_except(size_t process_count, size_t wait_indx, size_t signal_indx) {
    for (size_t i = 0; i < process_count; i++) {
        if (i == wait_indx || i == signal_indx) continue;
        
        close(semaphores[i][0]);
        close(semaphores[i][1]);
    }
}

void do_job(bool is_parent_first, size_t process_count, size_t word_count, size_t indx) {
    size_t wait_indx = indx;
    size_t signal_indx = (indx + 1) % process_count;

    int* curr_process_sem = semaphores[wait_indx];
    int* next_process_sem = semaphores[signal_indx];

    close_except(process_count, wait_indx, signal_indx);
    close(curr_process_sem[1]);
    close(next_process_sem[0]);
    
    size_t offset = process_count % WORD_COUNT;
    size_t curr_indx = indx % WORD_COUNT;
    size_t words_printed = indx;
    while (words_printed < word_count) {
        
        if (is_parent_first) {
            is_parent_first = false;
        } else {
            sem_wait(curr_process_sem);
        }

        write_s(STDOUT_FILENO, words[curr_indx], WORD_SIZE);
        curr_indx = (curr_indx + offset) % WORD_COUNT;
        
        // if this child printed the last word, dont signal
        if (words_printed + 1 < word_count) {
            sem_signal(next_process_sem);
        }

        // predict word count next time we get to this child
        words_printed += process_count;
    }

    close(curr_process_sem[0]);
    close(next_process_sem[1]);
}

void solve(size_t process_count, size_t word_count) {
    for (size_t i = 0; i < process_count; i++) {
        pipe_s(semaphores[i]);
    }

    for (size_t i = 1; i < process_count; i++) {
        pid_t child_pid = fork_s();
        if (child_pid == 0) {
            do_job(false, process_count, word_count, i);
            exit(0);
        }
    }
    do_job(true, process_count, word_count, 0);

    wait_children(process_count - 1);
}

int main(int argc, char** argv) {
    assert_true(argc == REQUIRED_ARGS, ARGS_ERR);
    
    int nc = atoi(argv[1]);
    assert_true(nc >= MIN_NC && nc <= MAX_NC, ARGS_ERR);
    
    int wc = atoi(argv[2]);
    assert_true(wc >= MIN_WC && wc <= MAX_WC, ARGS_ERR);

    solve(nc + 1, wc); 
}
