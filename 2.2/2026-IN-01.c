#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ARGS_ERR 100, "Invalid args"
#define PIPE_ERR 101, "Couldn't create pipe"
#define WAIT_ERR 102, "Couldn't wait"
#define FORK_ERR 103, "Couldn't fork process"
#define EXEC_ERR 104, "Couldn't execute command"
#define READ_ERR 105, "Couldn't read"
#define WRITE_ERR 107, "Couldn't write"
#define DUP_ERR 106, "Couldn't dup2"

#define REQUIRED_ARGS 4
#define ENCODE_XOR_SYMB '\x20'
#define SPECIAL_REPEATING_APPEND_SYMB '\x55'
#define PIPES_COUNT 6
#define PROGRAMS_COUNT 3


void sys_err(int status, int code, const char* message);
void assert_true(bool status, int code, const char* message);
bool is_special(char byte);
void init_pipes(void);
void write_encoded(char curr, char prev, bool has_prev, int pfd_read);
void close_pipes_except(size_t indx);
void prepare_pipes(char** argv);
void prepare_pipe(const char* program_name, size_t indx);

static int pfds[PIPES_COUNT][2];

void sys_err(int status, int code, const char* message) {
    if (status == -1) {
        err(code, message);
    }
}

void assert_true(bool status, int code, const char* message) {
    if (!status) {
        errx(code, message);
    }
}

bool is_special(char byte) {
    return byte == '\x55' || byte == '\x00' || byte == '\x7D' || byte == '\xFF';
}


void write_encoded(char curr, char prev, bool has_prev, int pfd_read) {
    char to_write[2];
    size_t size = 2;

    if (is_special(curr)) {
        to_write[0] = SPECIAL_REPEATING_APPEND_SYMB; 
        to_write[1] = curr ^ ENCODE_XOR_SYMB;
    } else if(has_prev && prev == curr) {
        to_write[0] = SPECIAL_REPEATING_APPEND_SYMB;
        to_write[1] = curr;
    } else {
        to_write[0] = curr;
        size = 1;
    }

    sys_err(write(pfd_read, to_write, size), WRITE_ERR);
}

void init_pipes(void) {
    for (size_t i = 0; i < PIPES_COUNT; i++) {
        sys_err(pipe(pfds[i]), PIPE_ERR);
    }
}

void close_pipes_except(size_t indx) {
    for (size_t i = 0; i < PIPES_COUNT; i++) {
        if (i == indx || i == indx + 1) {
            continue;
        }
        close(pfds[i][0]);
        close(pfds[i][1]);
    }
}

void prepare_pipes(char** argv) {
    for (size_t i = 0; i < PROGRAMS_COUNT; i++) {
        prepare_pipe(argv[i + 1], i * 2);
    }
}

void prepare_pipe(const char* program_name, size_t indx) {
    int* to_pipe = pfds[indx];
    int* from_pipe = pfds[indx + 1];

    pid_t child_pid = fork();
    if (child_pid == 0) {
        close_pipes_except(indx);

        close(to_pipe[1]);
        sys_err(dup2(to_pipe[0], STDIN_FILENO), DUP_ERR);
        close(to_pipe[0]);

        close(from_pipe[0]);
        sys_err(dup2(from_pipe[1], STDOUT_FILENO), DUP_ERR);
        close(from_pipe[1]);

        sys_err(execl(program_name, program_name, NULL), EXEC_ERR);
    }
    close(to_pipe[0]);
    close(from_pipe[1]);
}

int main(int argc, char** argv) {
    assert_true(REQUIRED_ARGS == argc, ARGS_ERR);
    
    init_pipes();
    prepare_pipes(argv);
     
    bool has_prev = false;
    char curr;
    char prev = 'a';
    ssize_t bytes_read;
    while ( (bytes_read = read(STDIN_FILENO, &curr, sizeof(curr))) > 0 ) {
         
        for (size_t i = 0; i < PIPES_COUNT; i+=2) {
            int to_pipe_write_fd = pfds[i][1];
            write_encoded(curr, prev, has_prev, to_pipe_write_fd);
        }
        
        for (size_t i = 1; i < PIPES_COUNT; i+=2) {
            int from_pipe_read_fd = pfds[i][0];
            char status;
            sys_err(read(from_pipe_read_fd, &status, sizeof(status)), READ_ERR);
            
            assert_true(status == '\x1', 120, "LOLO BIG PROBLEM");
        }

        prev = curr;
        if (!has_prev) {
            has_prev = true;
        }
    }
    sys_err(bytes_read, READ_ERR);

    for (size_t i = 0; i < PIPES_COUNT; i+=2) {
        if (i % 2 == 0) {
            close(pfds[i][1]);
        } else {
            close(pfds[i][0]);
        }
    }

    for (int i = 0; i < 3; i++) {
        int wstatus;

        sys_err(wait(&wstatus), WAIT_ERR);

        if (!WIFEXITED(wstatus)) {
            errx(WAIT_ERR);
        } else if (WEXITSTATUS(wstatus) != 0) {
            errx(WAIT_ERR);
        }
    }
}
