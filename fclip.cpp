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
    // create / fill full file path buffer and calculate global buffer
    TCHAR** files    = new TCHAR*[argc-1];
    DWORD*  bufSizes = new DWORD[argc-1];
    int     clpSize  = sizeof(DROPFILES);
    
    for (int i = 1; i < argc; i++) {
        bufSizes[i-1] = GetFullPathName(argv[i], 0, NULL, NULL);
        files[i-1]    = new TCHAR[bufSizes[i-1]];
        GetFullPathName(argv[i], bufSizes[i-1], files[i-1], NULL);
        if (GetFileAttributes(files[i-1]) == INVALID_FILE_ATTRIBUTES)
            return INVALID_FILE_ATTRIBUTES;
        // calculate *bytes* needed for memory allocation
        clpSize += sizeof(TCHAR) * (bufSizes[i-1]);
    }
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
    for (int i = 0; i < argc - 1; i++)
    {
        _tcscpy(dstStart, files[i]);
        dstStart = &dstStart[bufSizes[i]];
    }
    GlobalUnlock(hdrop);

    // prepare the clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_HDROP, hdrop);
    CloseClipboard();

    return 0;
}
