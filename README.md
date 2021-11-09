# CMAKEGEN

## Building on Windows

First step is to [install Visual Studio](https://visualstudio.microsoft.com/free-developer-offers) in order to get the MSVC compiler and Windows SDK which are required for the next steps

Next step is to download and install [cmake](https://cmake.org/download/)

Next prepare the environment so that cmake can find all dependencies during setup
We will need the following
```

INCLUDE=VS_INSTALL_PATH\VC\Tools\MSVC\14.29.30133\include;WINDOWS_SDK_PATH\10\Include\10.0.19041.0\ucrt;WINDOWS_SDK_PATH\10\Include\10.0.19041.0\um

LIB=VS_INSTALL_PATH\VC\Tools\MSVC\14.29.30133\lib\x64;WINDOWS_SDK_PATH\Lib\10.0.19041.0\um\x64;WINDOWS_SDK_PATH\Lib\10.0.19041.0\ucrt\x64

CXX=VS_INSTALL_PATH\VC\Tools\MSVC\14.29.30133\bin\Hostx64\x64\cl.exe
```

Then add the following to the PATH
```
WINDOWS_SDK_PATH\bin\10.0.19041.0\x64
```

This will allow us to run rc.exe, the root directories for me was:
```
VS_INSTALL_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community

WINDOWS_SDK_PATH=C:\Program Files (x86)\Windows Kits\10
```
In order to use Ninja as build tool we can either use the one provided by VS or provide it ourselves, I downloaded it from the  [official Github](https://github.com/ninja-build/ninja/releases) and add it to the PATH.

Once all of this is done create the directory used for output:
```
# mkdir _build
```

and then start the build with:
```
# cd _build
# cmake -GNinja -DCMAKE_BUILD_TYPE=Release ..
# ninja
```

This should provide a cmakegen.exe that can be added to the PATH for convenience

