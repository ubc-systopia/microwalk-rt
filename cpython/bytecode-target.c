#include <link.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>

#include "Python.h"
#include "dist/include/python3.13/pylifecycle.h"

#define Py_BUILD_CORE
#define NEED_OPCODE_METADATA
#include "internal/pycore_opcode_metadata.h"
#include "internal/pycore_ceval.h"

#include "../wrapper.h"
#include "cpython/Include/longobject.h"
#include "dist/include/python3.13/pytypedefs.h"

#ifndef PYTHON_BIN
#error PYTHON_BIN not defined
#endif

const wchar_t *exe = L"" PYTHON_BIN;

PyObject* module = NULL;
PyObject* fn = NULL;

FilterEntry *filter_addr = NULL;
size_t filter_addr_size = 0;

void init_target(const char *file) {
    PyConfig config = {};
    PyConfig_InitIsolatedConfig(&config);
    PyConfig_SetString(&config, &config.executable, exe);

    const PyStatus status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);

    if (PyStatus_Exception(status)) {
        fprintf(stdout, "Failed to set up CPython environment in %s: %s", status.func, status.err_msg);
        Py_ExitStatusException(status);
    }

    FILE* fp = fopen(file, "r");
    if (fp == NULL) {
        fprintf(stdout, "Failed to read CPython init file: %s", file);
        exit(1);
    }

    const int res = PyRun_SimpleFileEx(fp, file, 1);
    if (res != 0) {
        fprintf(stdout, "Failed to set up initialize CPython context in %s: %s", status.func, status.err_msg);
        exit(1);
    }

    module = PyImport_ImportModule("__main__");
    fn = PyObject_GetAttrString(module, "test");
}

void init_target_run() {
    // Nothing to do
}

void target_run(const char *buffer, const size_t buffer_size) {
    if (buffer_size <= 0) {
        mw_exit_error("Buffer size must be non-empty");
    }

    char *end_ptr = NULL;
    const int input = (int) strtol(buffer, &end_ptr, 10);
    if (end_ptr == NULL) {
        mw_exit_error("Integer conversion from testcase failed");
    }

    PyObject* n = PyLong_FromLong(input);
    PyObject* ret = PyObject_CallOneArg(fn, n);
    Py_XDECREF(ret);
    Py_DECREF(n);
}

void cleanup_target_run() {
    // Nothing to do
}

uintptr_t cpython_addr = 0;
FILE *alias = NULL;

int phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    char *name = strstr(info->dlpi_name, "libpython3");
    if (!name)
        return 0;

    fprintf(stderr, "Detected %s at %p\n", name, (void *) info->dlpi_addr);
    fprintf(alias, "%s\n", name);

    cpython_addr = info->dlpi_addr;
    return 0;
}

void init_target_filter(void) {
    #if ENABLE_INSTR
    size_t opcodeLen = sizeof(python_opcode_targets) / sizeof(python_opcode_targets[0]);
    size_t binOpcodeLen = NB_OPARG_LAST + 1;

    filter_addr_size = opcodeLen + binOpcodeLen + 1;
    filter_addr = calloc(opcodeLen + binOpcodeLen + 1, sizeof(FilterEntry));

    FILE *alias = fopen("./alias.txt", "w");

    if (alias == NULL) {
        fprintf(stderr, "Error opening metadata files");
        return;
    }

    uintptr_t max = 0;
    uintptr_t min = UINTPTR_MAX;

    for (size_t i = 0; i < opcodeLen; ++i) {
        uintptr_t addr = (uintptr_t) python_opcode_targets[i];

        FilterEntry entry = {
            .type = FilterTypeByteCodeHandler,
            .originStart = 0,
            .originEnd = 0,
            .targetStart = addr - 4,
            .targetEnd = addr,
        };
        filter_addr[i] = entry;

        if (addr == 0) continue;
        if (addr > max) max = addr;
        if (addr < min) min = addr;

        const char *name = _PyOpcode_OpName[i];

        fprintf(alias, "%08" PRIxPTR " %s\n", addr - cpython_addr - 4, name);
    }

    for (size_t i = 0; i < binOpcodeLen; ++i) {
        uintptr_t addr = (uintptr_t) _PyEval_BinaryOps[i];

            uintptr_t start = (uintptr_t) python_opcode_target_sub;
            FilterEntry entry = {
                .type = FilterTypeMainCall | FilterTypeLinearize,
                .originStart = start,
                .originEnd = start + 0x20,
                .targetStart = addr,
                .targetEnd = addr,
            };
            filter_addr[i + opcodeLen] = entry;
        }

    FilterEntry entry = {
        .type = FilterTypeMainCall,
        .originStart = 0,
        .originEnd = 0,
        .targetStart = (uintptr_t) &target_run,
        .targetEnd = (uintptr_t) &target_run,
    };
    filter_addr[opcodeLen + binOpcodeLen] = entry;

    fclose(alias);

    PinNotifyFilter(filter_addr, filter_addr_size);
#endif
}

void cleanup_target_filter(void) {
    free(filter_addr);
}

void cleanup_target(void) {
    Py_DECREF(fn);
	Py_DECREF(module);

	Py_Finalize();
}
