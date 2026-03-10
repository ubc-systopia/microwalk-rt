#include <inttypes.h>

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

void init_target_filter(void) {
    PinNotifyFilterAdd(&(FilterEntry){
        .type = FilterTypeMainCall,
        .originStart = 0,
        .originEnd = 0,
        .targetStart = (uintptr_t) &target_run,
        .targetEnd = (uintptr_t) &target_run,
    });
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
