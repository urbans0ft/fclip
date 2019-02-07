#include "pch.h" // Pre-compiled header

void paste();
void olePaste();

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
    //OpenClipboard(NULL);
    //EmptyClipboard();
    //SetClipboardData(CF_HDROP, hdrop);
    //CloseClipboard();

	//paste();
	olePaste();

    return 0;
}

/**
 * OleGetClipboard() => IDataObject
 * IDataObject::EnumFormatEtc => IEnumFORMATETC
 * IEnumFORMATETC::next() iterate and search for appropriate IDs
 * 1. Call RegisterClipboardFormat() with
 *   - CFSTR_FILEDESCRIPTOR
 *   - CFSTR_FILECONTENTS
 * 2. Call pDataObject->GetData() with received ID from 1.
 * 3. This creates STGMEDIUM structures which might provide IStream
 */

void paste()
{
	OpenClipboard(NULL);
	UINT format = 0;
	while ((format = EnumClipboardFormats(format)) != 0)
	{
		TCHAR formatName[255];
		GetClipboardFormatName(
			format,
			formatName,
			255
		);
		wprintf(L"%i - %ls\n",format, formatName);
	}
	HANDLE hHdrop = GetClipboardData(CF_HDROP);
	DROPFILES* df = (DROPFILES*)GlobalLock(hHdrop);
	GlobalUnlock(hHdrop);
	CloseClipboard();

	return;
	TCHAR* startFiles;
	int    fileCount = 0;
	
	startFiles = (TCHAR*)&df[1];
	while (startFiles[0] != '\0')
	{
		fileCount++;
		startFiles += _tcslen(startFiles) + 1; // get beyond \0
	}	
	TCHAR** oldFiles = new TCHAR*[fileCount];
	TCHAR** newFiles = new TCHAR*[fileCount];

	startFiles = (TCHAR*)&df[1];
	for (int i = 0; i < fileCount; i++)
	{
		int fileNameLength = _tcslen(startFiles) + 1; // + 1 => \0
		oldFiles[i] = startFiles;
		newFiles[i] = new TCHAR[fileNameLength];
		_tcscpy(newFiles[i], oldFiles[i]);
		PathStripPath(newFiles[i]);
		startFiles += fileNameLength;
		wprintf(L"copy %ls %ls\n", oldFiles[i], newFiles[i]);
		CopyFile(oldFiles[i], newFiles[i], FALSE);
	}

	int lala = 0;
}
// http://netez.com/2xExplorer/shellFAQ/adv_clip.html
void olePaste()
{
	HRESULT result;
	wprintf(L"\nFormats needed:\n");
	UINT ufileDesc = RegisterClipboardFormat(
		CFSTR_FILEDESCRIPTOR
	);
	UINT ufileContent = RegisterClipboardFormat(
		CFSTR_FILECONTENTS
	);
	wprintf(L"%i - %ls\n", ufileDesc, CFSTR_FILEDESCRIPTOR);
	wprintf(L"%i - %ls\n", ufileContent, CFSTR_FILECONTENTS);

	IDataObject* dataObject;
	result = OleGetClipboard(&dataObject);
	IEnumFORMATETC* enumFormats;
	result = dataObject->EnumFormatEtc(DATADIR::DATADIR_GET, &enumFormats);
	FORMATETC format;
	ULONG fetched;
	wprintf(L"Formats on the clipboard:\n");
	FORMATETC* formatDescriptor = nullptr;
	FORMATETC* formatContents = nullptr;
	while (SUCCEEDED(enumFormats->Next(1, &format, &fetched))
		&& fetched != 0)
	{
		const int cchMaxCount = 255;
		TCHAR lpszFormatName[cchMaxCount];
		GetClipboardFormatName(
			format.cfFormat,
			lpszFormatName,
			cchMaxCount
		);
		wprintf(L"%i - %ls\n", format.cfFormat, lpszFormatName);
		if (format.cfFormat == ufileDesc)
		{
			formatDescriptor = &format;
			continue;
		}
		if (format.cfFormat == ufileContent)
		{
			formatContents = &format;
			continue;
		}
	}
	if (formatDescriptor == nullptr || formatContents == nullptr)
	{
		wprintf(L"Needed formats not within the clipboard.");
		return;
	}
	STGMEDIUM mediumDescriptor;
	result = dataObject->GetData(formatDescriptor, &mediumDescriptor);
	LPCTSTR ptxt = (LPCTSTR)GlobalLock(mediumDescriptor.hGlobal); // dont' forget GlobalUnlock(stgm.hGlobal);

	STGMEDIUM mediumContents;
	result = dataObject->GetData(formatContents, &mediumContents);
	ptxt = (LPCTSTR)GlobalLock(mediumContents.hGlobal); // dont' forget GlobalUnlock(stgm.hGlobal);
	int lala = 0;
}