# fclip
In reference to the windows `clip` the `fclip` command copies files to the windows clipboard so one can insert these files later on using ctrl + v.

# Usage
`fclip` now supports relative paths as well. Moreover the path existence is
checked before modifying the clipboard data. If a path does not exist `fclip`
returns `INVALID_FILE_ATTRIBUTES` wich is equivalent to 127.
```
fclip.exe "C:\full\path\to\file.dat" "another_file.dat"
```
That's it! Feel free to press ctrl + v to paste the files.

# Compilation

## Visual Studio

Create a Visual C++ Console Project, add the fclip.ccp (delete the other predefined main source code file) and compile, that's it!

## MinGW

You need to have the [MinGW Compiler Collection (GCC)](https://osdn.net/projects/mingw/releases/). I prefer using [Cygwin](https://cygwin.com/) and install MinGW from the available packages.

**UNICODE 32bit**
```
i686-w64-mingw32-g++ -DUNICODE -D_UNICODE -o fclip -static fclip.cpp 
```
**UNICODE 64bit**
```
x86_64-w64-mingw32-g++ -DUNICODE -D_UNICODE -o fclip -static fclip.cpp 
```
**None UNICODE 32bit**
```
i686-w64-mingw32-g++ -o fclip -static fclip.cpp
```
**None UNICODE 64bit**
```
x86_64-w64-mingw32-g++ -o fclip -static fclip.cpp
```

## g++ under Cygwin
**Attention:** The g++ compiler from the GCC under Cygwin does **not** support Unicode encoding.
```
g++ -o fclip fclip.cpp
```

# References
* Inspired by [stackoverflow question](https://stackoverflow.com/q/25708895/10224443)
