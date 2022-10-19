# fclip v2.0

In reference to the windows `clip` the `fclip` command copies files to the windows clipboard or pastes them from it.

**Major Changes since v2.0**

- UNICODE-only support:

	> Windows natively supports Unicode strings for UI elements, file names,
	  and so forth. Unicode is the preferred character encoding, because it
	  supports all character sets and languages. Windows represents Unicode
	  characters using UTF-16 encoding, in which each character is encoded as
	  one or two 16-bit values.
	
	_&mdash;[microsoft.com](https://learn.microsoft.com/en-us/windows/win32/learnwin32/working-with-strings)_

- 64-bit only (since its 2022)
- CMake build system

# Usage

    fclip [-v | file1 [file2 [... [fileN]]]]

If you'd like a stable version use the [release](https://github.com/urbans0ft/fclip/releases)
provided.

## Copying

**Example:**

```
>fclip file1.dat ..\file2.dat "C:\long folder\file3.dat" "C:\file4.dat"
```

`fclip` supports relative and absolute paths. Moreover the path existence is
checked before modifying the clipboard data. If a path does not exist `fclip`
returns `INVALID_FILE_ATTRIBUTES` wich is equivalent to -1.

```
>fclip file-does-not-exist.dat
>echo %ERRORLEVEL%
-1
```

Press `ctrl + v` to paste the files.

## Pasting

    fclip -v

`fclip -v` checks if the clipboard contains a file reference and pastes it to
the current location. It simulates pressing `ctrl + v`.

# Changes

- v2.0.0
	- Bug(s):
		- support of relative file names (..\..\file.dat)
		- return value on copy failure (-1)
	- Change(s):
		- Removed non UNICODE support
		- cmake version control
		- fclip version info
	- Develop:
		- Increased verbosity

# Compilation

## CMake

Example configuration and compilation assuming `g++` (MinGW-w64) is installed.

### Configuration

Generating the debug configuration:

```
..\fclip>cmake -G "MinGW Makefiles" -S . -B .\bin\debug\ -DCMAKE_BUILD_TYPE=Debug
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
```

Generating the release configuration: 

```
..\fclip>cmake -G "MinGW Makefiles" -S . -B .\bin\release\ -DCMAKE_BUILD_TYPE=Release
-- The C compiler identification is GNU 12.2.0
-- The CXX compiler identification is GNU 12.2.0
...
-- Configuring done
-- Generating done
```

### Compilation

After the successfull configuration the project may be compiled. If no changes to the CMake configuration files are made
there's no need to (re-)run it again. Instead the project may be (re-)compiled any time.

```
..\fclip>>cmake --build bin\debug
[ 33%] Building CXX object CMakeFiles/fclip.dir/fclip.cpp.obj
[ 66%] Building CXX object CMakeFiles/fclip.dir/pch.cpp.obj
[100%] Linking CXX executable fclip.exe
[100%] Built target fclip
```
```
..\fclip>cmake --build bin\release
[ 33%] Building CXX object CMakeFiles/fclip.dir/fclip.cpp.obj
[ 66%] Building CXX object CMakeFiles/fclip.dir/pch.cpp.obj
[100%] Linking CXX executable fclip.exe
[100%] Built target fclip
```

## Manual

With MinGW-w64 installed you may compile the application via command line:

```
g++ -s -DUNICODE -D_UNICODE -o fclip -static fclip.cpp pch.cpp -lOle32 -lShlwapi
```

# References
* Inspired by [stackoverflow question](https://stackoverflow.com/q/25708895/10224443)
