#include <sys/resource.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#define _GNU_SOURCE
#define __USE_GNU
#include <link.h>

#include "quickjs/quickjs-libc.h"

#include "PinTracer/FilterEntry.h"

// Performs target initialization steps.
// This function is called once in the very beginning for the first testcase file, to make sure that the target is entirely loaded.
// The call is included in the trace prefix.
extern void InitTarget(FILE* input);

// Executes the target function.
// Do not use global variables, since the trace generator will reuse the instrumented version of this executable for several different inputs.
extern void RunTarget(FILE* input);

// Pin notification functions.
// These functions (and their names) must not be optimized away by the compiler, so Pin can find and instrument them.
// The return values reduce the probability that the compiler uses these function in other places as no-ops (Visual C++ did do this in some experiments).
#pragma optimize("", off)
int PinNotifyTestcaseStart(int t) { return t + 42; }
int PinNotifyTestcaseEnd() { return 42; }
int PinNotifyStackPointer(uint64_t spMin, uint64_t spMax) { return (int)(spMin + spMax + 42); }
int PinNotifyAllocation(uint64_t address, uint64_t size) { return (int)(address + 23 * size); }
int PinNotifyFilter(FilterEntry *addr, size_t length) { return length + 42; }
#pragma optimize("", on)

FilterEntry *filterAddr;
size_t filterAddrSize = 0;

#define FilterTypeByteCodeHandler   FilterTypeWhiteList | FilterTypeControlFlow | FilterTypeJump
#define FilterTypeMainCall          FilterTypeWhiteList | FilterTypeControlFlow | FilterTypeCall
#define FilterTypeBinaryLookup      FilterTypeWhiteList | FilterTypeDataAccess  | FilterTypeRead

void ReadTargetAdresses()
{
    #if ENABLE_INSTR
    size_t opcodeLen = sizeof(quickjs_opcode_targets) / sizeof(quickjs_opcode_targets[0]);

    filterAddrSize = opcodeLen + 1;
    filterAddr = calloc(filterAddrSize, sizeof(FilterEntry));

    FILE *alias = fopen("./alias.txt", "w");

    if (alias == NULL) {
        fprintf(stderr, "Error opening metadata files");
        return;
    }

    uintptr_t quickjsAddr = 0;
    int callback(struct dl_phdr_info *info, size_t size, void *data)
    {
        char *name = strstr(info->dlpi_name, "libquickjs");
        if (!name)
            return 0;

        fprintf(stderr, "Detected %s at %p\n", name, (void *) info->dlpi_addr);
        fprintf(alias, "%s\n", name);

        quickjsAddr = info->dlpi_addr;

        return 0;
    }
    dl_iterate_phdr(callback, NULL);

    uintptr_t max = 0;
    uintptr_t min = UINTPTR_MAX;

    for (size_t i = 0; i < opcodeLen; ++i) {
        uintptr_t addr = (uintptr_t) quickjs_opcode_targets[i];

        FilterEntry entry = {
            .type = FilterTypeByteCodeHandler,
            .originStart = 0,
            .originEnd = 0,
            .targetStart = addr - 4,
            .targetEnd = addr,
        };
        filterAddr[i] = entry;

        if (addr == 0) continue;
        if (addr > max) max = addr;
        if (addr < min) min = addr;

        const char *name = quickjs_opcode_target_names[i];

        fprintf(alias, "%08" PRIxPTR " %s\n", addr - quickjsAddr - 4, name);
    }

    FilterEntry entry = {
        .type = FilterTypeMainCall,
        .originStart = 0,
        .originEnd = 0,
        .targetStart = (uintptr_t) &RunTarget,
        .targetEnd = (uintptr_t) &RunTarget,
    };
    filterAddr[opcodeLen] = entry;

    fclose(alias);

    PinNotifyFilter(filterAddr, filterAddrSize);
    #endif
}

// Reads the stack pointer base value and transmits it to Pin.
void ReadAndSendStackPointer()
{
    // There does not seem to be a reliable way to get the stack size, so we use an estimation
    // Compiling with -fno-split-stack may be desired, to avoid surprises during analysis

    // Take the current stack pointer as base value
    uintptr_t stackBase;
    asm("mov %%rsp, %0" : "=r"(stackBase));

    // Get full stack size
    struct rlimit stackLimit;
    if(getrlimit(RLIMIT_STACK, &stackLimit) != 0)
    {
        char errBuffer[128];
        strerror_r(errno, errBuffer, sizeof(errBuffer));
        fprintf(stderr, "Error reading stack limit: [%d] %s\n", errno, errBuffer);
    }

    uint64_t stackMin = (uint64_t)stackBase - (uint64_t)stackLimit.rlim_cur;
    uint64_t stackMax = ((uint64_t)stackBase + 0x10000) & ~0xFFFFull; // Round to next higher multiple of 64 kB (should be safe on x86 systems)
    PinNotifyStackPointer(stackMin, stackMax);
}

// Main trace target function. The following actions are performed:
//     The current action is read from stdin.
//     A line with "t" followed by a numeric ID, and another line with a file path determining a new testcase, that is subsequently loaded and fed into the target function, while calling PinNotifyNextFile() beforehand.
//     A line with "e 0" terminates the program.
void TraceFunc()
{
    // First transmit stack pointer information
    ReadAndSendStackPointer();

	PinNotifyAllocation((uint64_t)&errno, 8);

    // Run until exit is requested
    char inputBuffer[512];
    char errBuffer[128];
	int targetInitialized = 0;
    while(1)
    {
        // Read command and testcase ID (0 for exit command)
        char command;
        int testcaseId;
        fgets(inputBuffer, sizeof(inputBuffer), stdin);
        sscanf(inputBuffer, "%c %d", &command, &testcaseId);

        fprintf(stderr, "%d: before command\n", testcaseId);

        // Exit or process given testcase
        if(command == 'e')
            break;
        if(command == 't')
        {
            fprintf(stderr, "%d: test case\n", testcaseId);
            // Read testcase file name
            fgets(inputBuffer, sizeof(inputBuffer), stdin);
            int inputFileNameLength = strlen(inputBuffer);
            if(inputFileNameLength > 0 && inputBuffer[inputFileNameLength - 1] == '\n')
                inputBuffer[inputFileNameLength - 1] = '\0';

            fprintf(stderr, "%d: input %s\n", testcaseId, inputBuffer);

            // Load testcase file and run target function
            FILE* inputFile = fopen(inputBuffer, "rb");
            if(!inputFile)
            {
                strerror_r(errno, errBuffer, sizeof(errBuffer));
                fprintf(stderr, "Error opening input file '%s': [%d] %s\n", inputBuffer, errno, errBuffer);
                continue;
            }

            fprintf(stderr, "%d: init\n", testcaseId);
			InitTarget(inputFile);
			fseek(inputFile, 0, SEEK_SET);

            fprintf(stderr, "%d: notify\n", testcaseId);
            PinNotifyTestcaseStart(testcaseId);

            fprintf(stderr, "%d: run\n", testcaseId);
            RunTarget(inputFile);

            fprintf(stderr, "%d: notify end\n", testcaseId);
            PinNotifyTestcaseEnd();

            fprintf(stderr, "%d: done\n", testcaseId);
            fclose(inputFile);
        }
    }
}

// Wrapper entry point.
int main(int argc, const char** argv)
{
    // Run target function
    ReadTargetAdresses();
    TraceFunc();
    free(filterAddr);
    return 0;
}
