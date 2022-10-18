# fclip
In reference to the windows `clip` the `fclip` command copies files to the windows clipboard or pastes them from it.

# Usage

    fclip [-v | file1 [file2 [... [fileN]]]]

If you'd like a stable version use the [release](https://github.com/urbans0ft/fclip/releases)
provided. Currently the Initial Release does not support the `-v` option.


## Copying

    fclip.exe "C:\full\path\to\file.dat" "another_file.dat"

`fclip` now supports relative paths as well. Moreover the path existence is
checked before modifying the clipboard data. If a path does not exist `fclip`
returns `INVALID_FILE_ATTRIBUTES` wich is equivalent to 127.


That's it! Feel free to press `ctrl + v` to paste the files.

## Pasting

    fclip -v

`fclip -v` checks if the clipboard contains a file reference and pastes it to
the current location. It simulates pressing `ctrl + v`.

**Warning:**  
`ctrl + v` is currently not working within the Cygwin compilation.

# Compilation

## CMake

Example configuration and compilation assuming `g++` (MinGW-w64) is installed.

### Configuration

```
..\fclip>mkdir build && cd build
..\fclip\build>cmake -G "MinGW Makefiles" ..\
-- The C compiler identification is GNU 12.2.0
-- The CXX compiler identification is GNU 12.2.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/mingw64/bin/gcc.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: C:/mingw64/bin/g++.exe - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Configuring done
-- Generating done
-- Build files have been written to: C:/fclip/build
```

### Compilation

After the successfull configuration the project may be compiled. If no changes to the CMake configuration files are made
there's no need to (re-)run it again. Instead the project may be (re-)compiled any time.

```
..\fclip\build>cmake --build .
[ 33%] Building CXX object CMakeFiles/fclip.dir/fclip.cpp.obj
[ 66%] Building CXX object CMakeFiles/fclip.dir/pch.cpp.obj
[100%] Linking CXX executable fclip.exe
[100%] Built target fclip
```

## Visual Studio

Create a Visual C++ Console Project, add the fclip.ccp (delete the other predefined main source code file) and compile, that's it!

## MinGW

You need to have the [MinGW Compiler Collection (GCC)](https://osdn.net/projects/mingw/releases/). I prefer using [Cygwin](https://cygwin.com/) and install MinGW from the available packages.

**UNICODE 32bit**
```
i686-w64-mingw32-g++ -s -DUNICODE -D_UNICODE -o fclip -static fclip.cpp pch.cpp -lOle32 -lShlwapi
```
**UNICODE 64bit**
```
x86_64-w64-mingw32-g++ -s -DUNICODE -D_UNICODE -o fclip -static fclip.cpp pch.cpp -lOle32 -lShlwapi
```
~~**None UNICODE 32bit**~~
```
i686-w64-mingw32-g++ -s -o fclip -static fclip.cpp pch.cpp
```
~~**None UNICODE 64bit**~~
```
x86_64-w64-mingw32-g++ -s -o fclip -static fclip.cpp pch.cpp
```

## g++ under Cygwin
**Attention:** The g++ compiler from the GCC under Cygwin does **not** support Unicode encoding.
```
g++ -s -o fclip.exe fclip.cpp pch.cpp -lOle32 -lShlwapi
```

# References
* Inspired by [stackoverflow question](https://stackoverflow.com/q/25708895/10224443)
