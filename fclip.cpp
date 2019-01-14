#include "pch.h" // Pre-compiled header

#ifdef MINGW_UNICODE
int main(void)
#else
int _tmain(int argc, TCHAR* argv[])
#endif
{
#ifdef MINGW_UNICODE
    int     argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(),&argc);
#endif
    // calculate *bytes* needed for memory allocation
    int clpSize = sizeof(DROPFILES);
    for (int i = 1; i < argc; i++)
        clpSize += sizeof(TCHAR) * (_tcslen(argv[i]) + 1); // + 1 => '\0'
    clpSize += sizeof(TCHAR); // two \0 needed at the end

    // allocate the zero initialized memory
    HDROP hdrop   = (HDROP)GlobalAlloc(GHND, clpSize);
    DROPFILES* df = (DROPFILES*)GlobalLock(hdrop);
    df->pFiles    = sizeof(DROPFILES); // string offset
#ifdef _UNICODE
    df->fWide     = TRUE; // unicode file names
#endif // _UNICODE

    // copy the command line args to the allocated memory
    TCHAR* dstStart = (TCHAR*)&df[1];
    for (int i = 1; i < argc; i++)
    {
        _tcscpy(dstStart, argv[i]);
        dstStart = &dstStart[_tcslen(argv[i]) + 1]; // + 1 => get beyond '\0'
    }
    GlobalUnlock(hdrop);

    // prepare the clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_HDROP, hdrop);
    CloseClipboard();

    return 0;
}
