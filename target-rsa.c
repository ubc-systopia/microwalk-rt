#include <stdint.h>
#include <stdio.h>

#include <Python.h>

PyConfig config;

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


    PyConfig_InitPythonConfig(&config);
    config.home = L"/home/kdankert/508/cpython-microwalk/cpython";
    config.pythonpath_env = L"/home/kdankert/508/cpython-microwalk/cpython/Lib";
    config.run_filename = L"/home/kdankert/508/cpython-microwalk/rsa.py";

    char* argv[2] = { "/home/kdankert/508/cpython-microwalk/rsa.py", plain };
    PyConfig_SetBytesArgv(&config, 2, argv);

    Py_InitializeFromConfig(&config);
}
