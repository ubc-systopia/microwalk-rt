#include "wrapper.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/resource.h>

int PinNotifyTestcaseStart(const int t) { return t; }
int PinNotifyTestcaseEnd() { return 42; }
int PinNotifyStackPointer(const uint64_t spMin, const uint64_t spMax) { return (int) (spMin + spMax + 42); }
int PinNotifyAllocation(const uint64_t address, const uint64_t size) { return (int) (address + 23 * size); }
int PinNotifyFilter(FilterEntry *addr, const size_t length) { return (int) (length + (intptr_t) addr + 42); }

void mw_exit_error(const char *format, ...) {
    fprintf(stderr, "ERROR in wrapper: ");

    va_list args = {};
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fputc('\n', stderr);
    exit(1);
}

static void read_and_send_stack_pointer(void) {
    uintptr_t stack_base = 0;
    asm volatile("mov %%rsp, %0" : "=r"(stack_base));

    struct rlimit stack_limit = {};
    if (getrlimit(RLIMIT_STACK, &stack_limit) != 0) {
        mw_exit_error("Error reading stack limit: %s (%d)", strerror(errno), errno);
    }

    uint64_t stack_min = stack_base - stack_limit.rlim_cur;
    uint64_t stack_max = stack_base + 0x10000 & ~0xFFFFull;

    PinNotifyStackPointer(stack_min, stack_max);
}

int redirectStdOutToStdErr(void) {
    fflush(stdout);
    const int stdout_fd = dup(STDOUT_FILENO);
    const int res = dup2(STDERR_FILENO, STDOUT_FILENO);
    if (res == -1) {
        mw_exit_error("Error redirecting stdout to stderr: %s (%d)", strerror(errno), errno);
    }
    return stdout_fd;
}

void redirectStdOutToStdOut(const int stdout_fd) {
    fflush(stdout);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdout_fd);
}

static void execute(const char *buffer, const size_t buffer_size) {
    const int stdout_fd = redirectStdOutToStdErr();
    target_run(buffer, buffer_size);
    redirectStdOutToStdOut(stdout_fd);
}

// Trace generation controller function. After some setup, actions are read from stdin:
//   - A line with "t" followed by a numeric ID, and another line with a file path determining a new testcase, that is subsequently loaded and fed into the target function, while calling PinNotifyNextFile() beforehand.
//   - A line with "e 0" terminates the program.
static void trace(const char *script_path) {
    // First transmit stack pointer information
    read_and_send_stack_pointer();

    // Tell Pin the errno address so accesses are correctly resolved
    // ReSharper disable once CppRedundantDereferencingAndTakingAddress
    PinNotifyAllocation((uint64_t) &errno, 8);

    // Set up target. Allocations made here will end up in the trace prefix
    const int stdout_fd = redirectStdOutToStdErr();
    init_target(script_path);
    redirectStdOutToStdOut(stdout_fd);

    // Setup target filter
    init_target_filter();

    char command_buffer[512] = {};
    bool target_initialized = false;
    const size_t input_buffer_size = 16 * 1024;
    char *input_buffer = malloc(input_buffer_size);

    // Run until exit is requested
    while (true) {
        // Read command and testcase ID (0 for exit command)

        if (!fgets(command_buffer, sizeof(command_buffer), stdin)) {
            mw_exit_error("Could not read command from stdin, error '%s' (%d)", strerror(errno), errno);
        }
        const char command = command_buffer[0];
        const int testcase_id = (int) strtol(command_buffer + 2, NULL, 10);

        // Exit or process given testcase
        if (command == 'e') {
            break;
        }

        if (command == 't') {
            // Read testcase file name
            if (!fgets(command_buffer, sizeof(command_buffer), stdin)) {
                mw_exit_error("Could not read testcase file name from stdin, error '%s' (%d)", strerror(errno), errno);
            }
            const uint64_t input_file_length = strlen(command_buffer);
            if (input_file_length > 0 && command_buffer[input_file_length - 1] == '\n') {
                command_buffer[input_file_length - 1] = '\0';
            }

            // Try to read testcase file
            FILE *input_file = fopen(command_buffer, "rb");
            if (!input_file) {
                mw_exit_error("Could not open testcase file '%s', error '%s' (%d)", command_buffer, strerror(errno),
                              errno);
            }
            fread(input_buffer, 1, input_buffer_size, input_file);
            if (ferror(input_file)) {
                mw_exit_error("Could not read testcase file '%s'. Error '%s' (%d)", command_buffer, strerror(errno),
                              errno);
            }
            fclose(input_file);

            FILE *script_file = fopen(script_path, "r");
            if (!script_file) {
                mw_exit_error("Could not find script file '%s'", script_path);
            }
            fclose(script_file);

            // Initialize runtime
            init_target_run();

            // If the target was not yet initialized, do a dummy run of the subtarget
            if (!target_initialized) {
                execute(input_buffer, input_file_length);
                target_initialized = true;
            }

            PinNotifyTestcaseStart(testcase_id);
            execute(input_buffer, input_file_length);
            PinNotifyTestcaseEnd();

            cleanup_target_run();
        }
    }

    cleanup_target_filter();

    cleanup_target();

    free(input_buffer);
}

int main(const int argc, const char *argv[]) {
    if (argc != 2) {
        mw_exit_error("Script file not provided");
    }

    trace(argv[1]);
}
