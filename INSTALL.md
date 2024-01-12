# Requirements:

## Requirements
1. [mingw-w64](https://winlibs.com/#download-release)
    - for windows OS.
    provides make build system.
    download,install and add the bin folder to PATH.

2. [arm-gnu-toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
    required to compile source code for arm chips (cortex-m 3 in this case)
    download,install and add the bin folder to PATH.

3. [ST-Link driver](https://www.st.com/en/development-tools/stsw-link009.html#get-software)
    needed to interface with the ST-Link programmer.

4. [ST-Link tools](https://github.com/stlink-org/stlink/releases/tag/v1.7.0)
    cmdline tools to interact with ST-Link device.
    download, extract and add the bin folder to PATH.

5. [Rust](https://www.rust-lang.org/)
    used to compile the firmware updater.

6. [Python](https://www.python.org/downloads/)
    used to run various scripts for generation signing keys and signing firmware.

# Installation
1. clone the git repo on your machine and navigate to it.
2. run the following commands to fetch libopencm3
```bash
git submodule init && git submodule update
```
