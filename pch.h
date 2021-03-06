#ifndef PCH_H
#define PCH_H

// TODO: add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>   // DROPFILES
#include <ShellApi.h> // HDROP
#include <tchar.h>    // e.g. _tcslen
#include <Shlwapi.h>  // PathStripPath
// debug
#include <iostream>
#include <string>

#ifdef __CYGWIN__
#if defined(_UNICODE) || defined(UNICODE)
#error UNICODE is not supported for GCC under Cygwin. \
       Use MinGW GCC instead.
#endif
#define _tcscpy strcpy
#define _tcslen strlen
#define _tcscmp strcmp
#define _tmain main
#endif

#if defined(__MINGW32__) && defined(_UNICODE)
#define MINGW_UNICODE
#endif

// 1 MB
#define ISTREAM_BUF_SIZE 1048576 // Byte

// New entry point, instead of main().
int run(int argc, TCHAR* argv[]);

#endif //PCH_H
