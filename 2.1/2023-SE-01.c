#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdio.h>
#define F_OPEN_ERR 100
#define F_OPEN_MSG "Couldn't open fd"
#define F_READ_ERR 101
#define F_READ_MSG "Couldn't read from fd"
#define F_WRITE_ERR 102
#define F_WRITE_MSG "Couldn't write to fd"
#define ARGS_ERR 103
#define ARGS_MSG "Invalid args"
#define ARGS_COUNT_ERR 104
#define ARGS_COUNT_MSG "Invalid args count"
#define F_SEEK_ERR 105
#define F_SEEK_MSG "Couldn't seek in fd"

#define ARGS_COUNT 3
#define BUF_SIZE 4096

#define STAGE_START 1
#define STAGE_CL 2
#define STAGE_DATA 3
#define STAGE_CHECKSUM 4
#define STAGE_FINISHED 5
#define STAGE_FAILED 6

struct state {
    int stage;
    uint8_t actual_checksum;
    // byte 1
    char start_byte;
    // byte 2
    uint8_t content_length;
    // byte 3-N + helper size + acumulated checksum
    uint8_t data[252];
    uint8_t data_size;
    // byte N
    uint8_t checksum;
};

void sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
int open_file(const char* filename, int flags, mode_t mode);
void write_to_state(struct state* state, uint8_t byte);
void write_state(struct state* state, int output);
void write_all(int fd, const void* buf, size_t len);
void clear_state(struct state* state);

int main(int argc, char** argv) {
    assert_true(argc == ARGS_COUNT, ARGS_ERR, ARGS_MSG);

    int input = open_file(argv[1], O_RDONLY, 0);
    int output = open_file(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    struct state state;
    clear_state(&state);

    char buf[BUF_SIZE];
    ssize_t bytes_read;
    off_t start = 0;
    while ( (bytes_read = read(input, buf, sizeof(buf))) > 0) {
        size_t ubytes_read = (size_t)bytes_read;
        for (size_t i = 0; i < ubytes_read; i++) {
            write_to_state(&state, (uint8_t)buf[i]);

            if (state.stage == STAGE_FAILED) {
                start = start + 1;
                sys_err(lseek(input, start, SEEK_SET), F_SEEK_ERR, F_SEEK_MSG);
                clear_state(&state);
                break;
            }

            if (state.stage == STAGE_CL) {
                off_t curr_in_pos = lseek(input, 0, SEEK_CUR);
                sys_err(start, F_SEEK_ERR, F_SEEK_MSG);
                start = curr_in_pos - ubytes_read + i;
            }

            if (state.stage == STAGE_FINISHED) {   
                write_state(&state, output);
                clear_state(&state);
            }
        }
    }
    sys_err(bytes_read, F_READ_ERR, F_READ_MSG);

    close(input);
    close(output);
}

void write_to_state(struct state *state, uint8_t byte) {
    if (state->stage == STAGE_START && byte == '\x55') {
        state->stage = STAGE_CL;
        state->start_byte = byte;
        state->actual_checksum ^= byte;
        return;
    }

    if (state->stage == STAGE_CL) {
        state->stage = STAGE_DATA;
        state->content_length = byte;
        state->actual_checksum ^= byte;
        return;
    }

    if (state->content_length <= 4) { 
        state->stage = STAGE_FAILED; 
        return;
    }

    if (state->stage == STAGE_DATA) {
        state->data[state->data_size] = byte;
        state->data_size++;
        state->actual_checksum ^= byte;
        if (state->data_size + 3 == state->content_length) {
            state->stage = STAGE_CHECKSUM;    
        }
        return;
    }

    if (state->stage == STAGE_CHECKSUM) {
        state->checksum = byte;
        if (state->checksum == state->actual_checksum) {
            state->stage = STAGE_FINISHED;
        } else {
            state->stage = STAGE_FAILED;
        }
    }
}

void write_state(struct state* state, int output) {
    write_all(output, &state->start_byte, sizeof(state->start_byte));  
    write_all(output, &state->content_length, sizeof(state->content_length));  
    write_all(output, state->data, state->data_size);  
    write_all(output, &state->checksum, sizeof(state->checksum));  
}

void clear_state(struct state* state) {
    *state = (struct state){ .stage = STAGE_START, .data_size = 0, .actual_checksum = 0, .checksum = 0 };

}

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

int open_file(const char* filename, int flags, mode_t mode) {
    int fd = open(filename, flags, mode);
    sys_err(fd, F_OPEN_ERR, F_OPEN_MSG);
    return fd;
}

void write_all(int fd, const void* buf, size_t len) {
    size_t written = 0;
    const char* p = buf;
    while (written < len) {
        ssize_t curr_written = write(fd, p + written, len - written);
        sys_err(curr_written, F_WRITE_ERR, F_WRITE_MSG);
        written += (size_t)curr_written;
    }
}
