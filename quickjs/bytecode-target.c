#include <inttypes.h>
#include <link.h>

#include "quickjs/quickjs-libc.h"

#include "../wrapper.h"

JSRuntime *rtm = NULL;
JSContext *ctx = NULL;
JSValue global_obj = {};
JSValue fn = {};

FilterEntry *filter_addr = NULL;
size_t filter_addr_size = 0;

int js_std_eval_file_global(JSContext *ctx, const char *filename) {
    size_t buf_len = 0;
    uint8_t *buf = js_load_file(ctx, &buf_len, filename);
    const JSValue val = JS_Eval(ctx, (const char *) buf, buf_len, filename, JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(val)) {
        js_std_dump_error(ctx);
        return -1;
    }
    JS_FreeValue(ctx, val);
    js_free(ctx, buf);
    return 0;
}

void init_target(const char *file) {
    rtm = JS_NewRuntime();
    js_std_set_worker_new_context_func(JS_NewContext);
    js_std_init_handlers(rtm);
    ctx = JS_NewContext(rtm);
    JS_SetModuleLoaderFunc2(rtm, NULL, js_module_loader, js_module_check_attributes, NULL);

    js_std_eval_file_global(ctx, file);
    global_obj = JS_GetGlobalObject(ctx);
    fn = JS_GetPropertyStr(ctx, global_obj, "test");
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

    JSValue arg = JS_NewInt32(ctx, input);
    JS_Call(ctx, fn, global_obj, 1, &arg);
    JS_FreeValue(ctx, arg);
}

void cleanup_target_run() {
    // Nothing to do
}

uintptr_t quickjs_addr = 0;
FILE *alias = NULL;

int phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    char *name = strstr(info->dlpi_name, "libquickjs");
    if (!name)
        return 0;

    fprintf(stderr, "Detected %s at %p\n", name, (void *) info->dlpi_addr);
    fprintf(alias, "%s\n", name);

    quickjs_addr = info->dlpi_addr;
    return 0;
}

void init_target_filter(void) {
#if ENABLE_HANDLER_EXPORT
    const size_t opcode_len = sizeof(quickjs_opcode_targets) / sizeof(quickjs_opcode_targets[0]);

    filter_addr_size = opcode_len + 1;
    filter_addr = calloc(filter_addr_size, sizeof(FilterEntry));

    alias = fopen("./alias.txt", "w");

    if (alias == NULL) {
        fprintf(stderr, "Error opening metadata files");
        return;
    }

    dl_iterate_phdr(phdr_callback, NULL);

    uintptr_t max = 0;
    uintptr_t min = UINTPTR_MAX;

    for (size_t i = 0; i < opcode_len; ++i) {
        const uintptr_t addr = (uintptr_t) quickjs_opcode_targets[i];

        const FilterEntry entry = {
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

        const char *name = quickjs_opcode_target_names[i];

        fprintf(alias, "%08" PRIxPTR " %s\n", addr - quickjs_addr - 4, name);
    }

    const FilterEntry entry = {
        .type = FilterTypeMainCall,
        .originStart = 0,
        .originEnd = 0,
        .targetStart = (uintptr_t) &target_run,
        .targetEnd = (uintptr_t) &target_run,
    };
    filter_addr[opcode_len] = entry;

    fclose(alias);

    PinNotifyFilter(filter_addr, filter_addr_size);
#endif
}

void cleanup_target_filter(void) {
    free(filter_addr);
}

void cleanup_target(void) {
    JS_FreeValue(ctx, fn);
    JS_FreeValue(ctx, global_obj);

    js_std_free_handlers(rtm);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rtm);
}
