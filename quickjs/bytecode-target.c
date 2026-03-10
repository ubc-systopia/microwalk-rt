#include <inttypes.h>

#include "quickjs/quickjs-libc.h"

#include "../wrapper.h"

JSRuntime *rt = NULL;
JSContext *ctx = NULL;
JSValue fn = {};

FilterEntry *filter_addr = NULL;
size_t filter_addr_size = 0;

JSRuntime *js_std_new_runtime() {
    JSRuntime* rt = JS_NewRuntime();
    js_std_set_worker_new_context_func(JS_NewContext);
    js_std_init_handlers(rt);
    return rt;
}

JSContext *js_std_new_context(JSRuntime *rt) {
    JSContext *ctx = JS_NewContext(rt);
    JS_SetModuleLoaderFunc2(rt, NULL, js_module_loader, js_module_check_attributes, NULL);
    js_std_add_helpers(ctx, -1, NULL);
    return ctx;
}

JSValue js_std_load_file(JSContext *ctx, const char *filename) {
    size_t buf_len = 0;
    uint8_t *buf = js_load_file(ctx, &buf_len, filename);
    const JSValue module = JS_Eval(ctx, (const char *) buf, buf_len, filename, JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    if (JS_IsException(module)) {
        js_std_dump_error(ctx);
        mw_exit_error("Script compilation failed");
    }
    js_module_set_import_meta(ctx, module, 1, 1);
    JSValue promise = JS_EvalFunction(ctx, module);
    if (JS_IsException(promise)) {
        js_std_dump_error(ctx);
        mw_exit_error("Script initialization failed");
    }
    JSValue res = js_std_await(ctx, promise);
    if (JS_IsException(res)) {
        js_std_dump_error(ctx);
        mw_exit_error("Script async initialization failed");
    }
    JS_FreeValue(ctx, res);
    js_free(ctx, buf);
    return module;
}

JSValue js_std_get_fn(JSContext *ctx, JSValue module, const char *name) {
    if (JS_VALUE_GET_TAG(module) != JS_TAG_MODULE) {
        return JS_UNDEFINED;
    }
    JSModuleDef *m = JS_VALUE_GET_PTR(module);
    JSValue ns = JS_GetModuleNamespace(ctx, m);
    JSValue fn = JS_GetPropertyStr(ctx, ns, name);
    JS_Call(ctx, fn, JS_UNDEFINED, 0, NULL);
    JS_FreeValue(ctx, ns);
    return fn;
}

void init_target(const char *file) {
    rt = js_std_new_runtime();
    ctx = js_std_new_context(rt);

    JSValue module = js_std_load_file(ctx, file);
    fn = js_std_get_fn(ctx, module, "test");
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
    JSValue res = JS_Call(ctx, fn, JS_UNDEFINED, 1, &arg);
    if (JS_IsException(res)) {
        js_std_dump_error(ctx);
        mw_exit_error("Script evaluation failed");
    }

    JS_FreeValue(ctx, res);
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
    JS_FreeContext(ctx);
    js_std_free_handlers(rt);
    JS_FreeRuntime(rt);
}
