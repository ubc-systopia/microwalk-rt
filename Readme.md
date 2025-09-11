# MicrowalkRT

MicrowalkRT is an extension to the [Microwalk project](https://github.com/microwalk-project/Microwalk) that facilliates side channel analysis of programs written in interpreted programming languages by analyzing the control flow of language runtimes.

MicrowalkRT is designed to require minimal changes to supported language runtimes and leverages the minimally modified Microwalk framework.

Currently supported language runtimes are:
- [CPython v3.13.1](https://github.com/ubc-systopia/cpython/tree/cpsc-508)
- [QuickJS 0d7aaed](https://github.com/ubc-systopia/quickjs/tree/cpsc-508)

## Usage

1. Clone the repository including its submodules (incl. dependencies and language runtimes)
  ```bash
  git clone --recurse-submodules -j$(nproc) git@github.com:ubc-systopia/microwalk-rt.git
  ```
2. Download [Intel Pin](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-dynamic-binary-instrumentation-tool.html) and set the `PIN_ROOT` environment variable to the directory containing the `pin` executable.
3. Move into the language runtime directory.
4. Run `./init.sh`
5. Run `./build.sh`
6. Run `./analyze.sh`



## Acknowledgements

 - [Microwalk project](https://github.com/microwalk-project/Microwalk)
 - [Intel Pin](https://www.intel.com/content/www/us/en/developer/articles/tool/pin-a-dynamic-binary-instrumentation-tool.html)
 - [CPython](https://github.com/python/cpython)
 - [QuickJS](https://github.com/bellard/quickjs)


## License

[MIT](https://choosealicense.com/licenses/mit/)
