#include <stdint.h>
#include <stdio.h>

#include <Python.h>

extern void RunTarget(FILE* input)
{
    Py_RunMain();
}

extern void InitTarget(FILE* input)
{
    uint8_t plain[2];
    if(fread(plain, 1, 1, input) != 1)
        return;
    plain[1] = '\0';

    PyConfig config;
	PyConfig_InitIsolatedConfig(&config);
	PyConfig_SetString(&config, &config.executable, L"../../../../venv/bin/python");
	PyConfig_SetString(&config, &config.run_filename, L"../../../../rsa.py");

	char* argv[2] = { "../../../../rsa.py", plain };
    PyConfig_SetBytesArgv(&config, 2, argv);

	const PyStatus status = Py_InitializeFromConfig(&config);
	PyConfig_Clear(&config);

	if (PyStatus_Exception(status)) {
	    fprintf(stderr, "Failed to set up CPython environment in %s: %s", status.func, status.err_msg);
		Py_ExitStatusException(status);
	}
}
