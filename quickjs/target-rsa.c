#include "quickjs/quickjs-libc.h"
#include "quickjs/quickjs.h"
#include <stdint.h>
#include <stdio.h>

JSContext *ctx;
JSRuntime *rtm;

extern void RunTarget(FILE* input)
{
    js_std_eval_file(ctx, "../../../../js-rsa.js", 0);

    js_std_free_handlers(rtm);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rtm);
}

extern void InitTarget(FILE* input)
{
    rtm = JS_NewRuntime();
	js_std_set_worker_new_context_func(JS_NewContext);
	js_std_init_handlers(rtm);
	ctx = JS_NewContext(rtm);
	JS_SetModuleLoaderFunc(rtm, NULL, js_module_loader, NULL);

	char plain[2];
    if(fread(plain, 1, 1, input) != 1)
        return;
    plain[1] = '\0';

	char* args[2] = {"../../../../ct.js", plain };
	js_std_add_helpers(ctx, 2, args);
}
