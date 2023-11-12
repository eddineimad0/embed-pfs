# Requirements:

## Windows
1. [mingw-w64](https://winlibs.com/#download-release)

    download,install and add the bin folder to PATH.
2. [arm-gnu-toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

    download,install and add the bin folder to PATH.
3. [ST-Link driver](https://www.st.com/en/development-tools/stsw-link009.html#get-software)

    this guide uses ST-Link USB programmer.
    
    if you are using other development-tools
    you some steps might be different for you.
4. [ST-Link tools](https://github.com/stlink-org/stlink/releases/tag/v1.7.0)
    download, extract and add the bin folder to PATH.

## Linux


# Installation
1. clone the git repo on your machine and navigate to it.
2. run the following commands to fetch libopencm3
```bash
git submodule init && git submodule update
```

## Windows
If you already installed the Requirements run the `build.ps1` file with Powershell.
