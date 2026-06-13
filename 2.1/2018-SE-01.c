#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// ERROR CONSTANTS
#define FILE_READ_ERR 100
#define FILE_READ_MSG "Couldn't read from standard input"
#define FILE_WRITE_ERR 101
#define FILE_WRITE_MSG "Couldn't write to standard output"
#define ARGS_COUNT_ERR 103
#define ARGS_COUNT_MSG "Invalid args count"
#define SET_ERR 104
#define SET_MSG "Incompatible sets of characters"

// LOGIC CONSTANTS
#define ARGS_COUNT 3
#define CHARS_COUNT 256 

// We precompute which characters belong to the delete/squeeze set
// into a 256-element bool lookup table, giving O(1) per character.
// A simpler approach would be to scan the set string each time
// with str_contains (O(n) per character, O(n*m) total), 
// but since all chars fit in 0-255, the lookup table is better.

//bool str_contains(const char* str, char symb) {
//    size_t len = strlen(str);
//    for (size_t i = 0; i < len; i++) {
//        if (symb == str[i]) {
//            return true;
//        }
//    }
//    return false;
//}

void sys_err(int status, int code, const char* message);
void assert_true(bool success, int code, const char* message);
bool streq(const char* lhs, const char* rhs);
void write_all(int fd, const void* buf, ssize_t len);
void write_symb(char symb);
ssize_t read_symb(char* symb);
void fill_set(bool set[CHARS_COUNT], const char* str);
void perform_delete(const char* set);
void perform_squeeze(const char* set);
void perform_translate(const char* set1, const char* set2);

// ERROR HANDLING
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

// STRING FUNCS
bool streq(const char* lhs, const char* rhs) {
    return strcmp(lhs, rhs) == 0;
}

// FD OPERATIONS
// Template function to prevent partial writes (probably non needed)
void write_all(int fd, const void* buf, ssize_t len) {
    ssize_t written = 0;
    while (written < len) {
        ssize_t curr_written = write(fd, buf, len - written);
        sys_err(curr_written, FILE_WRITE_ERR, FILE_WRITE_MSG);
        written += curr_written;
    }
}

void write_symb(char symb) {
    write_all(STDOUT_FILENO, &symb, sizeof(symb));
}

ssize_t read_symb(char* symb) {
    ssize_t bytes_read = read(STDIN_FILENO, symb, sizeof(*symb));
    sys_err(bytes_read, FILE_READ_ERR, FILE_READ_MSG);
    return bytes_read;
}

// TR ACTIONS
void fill_set(bool set[CHARS_COUNT], const char* str) {
    while (*str) {
        set[(size_t)*str] = true;
        str++;
    }
}

void perform_delete(const char* set) {
    bool delete_set[CHARS_COUNT] = {false};
    fill_set(delete_set, set);

    char symb;
    while ( (read_symb(&symb)) > 0) {
        if (!delete_set[(size_t)symb]) {
            write_symb(symb);
        }
    }
}

void perform_squeeze(const char* set) {
    bool squeeze_set[CHARS_COUNT] = {false};
    fill_set(squeeze_set, set);

    char last_symb;
    read_symb(&last_symb);
    write_symb(last_symb);

    char symb;
    while ( (read_symb(&symb)) > 0) {
        if (symb != last_symb || !squeeze_set[(size_t)symb]) {
            write_symb(symb);
        }

        last_symb = symb;
    }
}

void perform_translate(const char* set1, const char* set2) {
    size_t len = strlen(set1);
    assert_true(len == strlen(set2), SET_ERR, SET_MSG);
    
    char translation_set[CHARS_COUNT] = {'\0'};
    for (size_t i = 0; i < CHARS_COUNT; i++) {
        translation_set[i] = (char)i;
    }

    for (size_t i = 0; i < len; i++) {
        translation_set[(size_t)set1[i]] = set2[i];
    }

    char symb;
    while ( (read_symb(&symb)) > 0) {
        char new_symb = translation_set[(size_t)symb];
        write_symb(new_symb);
    }
}

int main(int argc, char** argv) {
    assert_true(argc == ARGS_COUNT, ARGS_COUNT_ERR, ARGS_COUNT_MSG);

    if (streq(argv[1], "-d")) {
        perform_delete(argv[2]);
    } else if (streq(argv[1], "-s")) {
        perform_squeeze(argv[2]);
    } else {
        perform_translate(argv[1], argv[2]);
    }
}
