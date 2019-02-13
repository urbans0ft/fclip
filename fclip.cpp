#include "pch.h" // Pre-compiled header

void paste();
void pasteByHdrop();
void pasteByFileContents(CLIPFORMAT clFileDescriptor, CLIPFORMAT clFileContents);
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

	olePaste();
	paste();

    return 0;
}

void paste()
{
	if (IsClipboardFormatAvailable(CF_HDROP))
	{
		pasteByHdrop();
		return;
	}
	UINT formatFileDescriptor = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	UINT formatFileContents   = RegisterClipboardFormat(CFSTR_FILECONTENTS);
	if (IsClipboardFormatAvailable(formatFileDescriptor)
		&& IsClipboardFormatAvailable(formatFileContents))
	{
		pasteByFileContents(formatFileDescriptor, formatFileContents);
		return;
	}
	wprintf(L"Nothing to paste!\n");
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

void pasteByHdrop()
{
	OpenClipboard(NULL);

	HANDLE hHdrop = GetClipboardData(CF_HDROP);
	DROPFILES* df = (DROPFILES*)GlobalLock(hHdrop);
	GlobalUnlock(hHdrop);

	CloseClipboard();

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
		//CopyFile(oldFiles[i], newFiles[i], FALSE);
	}

	int lala = 0;
}

void pasteByFileContents(CLIPFORMAT clFileDescriptor, CLIPFORMAT clFileContents)
{
	HRESULT result;
	OleInitialize(NULL);
	IDataObject* dataObject; OleGetClipboard(&dataObject);
	FORMATETC formatDescriptor{
		clFileDescriptor,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};
	STGMEDIUM mediumDescriptor;
	result = dataObject->GetData(&formatDescriptor, &mediumDescriptor);
	FILEGROUPDESCRIPTOR* pFileGrpDescriptor = 
		(FILEGROUPDESCRIPTOR*)GlobalLock(mediumDescriptor.hGlobal);
	for (int i = 0; i < pFileGrpDescriptor->cItems; i++)
	{
		const FILEDESCRIPTOR& fDescriptor = pFileGrpDescriptor->fgd[i];
		wprintf(L"File in group descriptor: %ls\n", fDescriptor.cFileName);
		HANDLE hFile = CreateFile(
			fDescriptor.cFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_NEW,
			fDescriptor.dwFileAttributes,
			NULL
		);
		FORMATETC formatFileContents{
			clFileContents,
			NULL,
			DVASPECT_CONTENT,
			-1,
			TYMED_ISTREAM
		};
		STGMEDIUM mediumFile;
		result = dataObject->GetData(&formatFileContents, &mediumFile);
		void * p = mediumFile.pstm;

		BYTE* pBuf = new BYTE[ISTREAM_BUF_SIZE]; // 1 MB
		ULONG read;
		result = mediumFile.pstm->Seek({0}, STREAM_SEEK_SET, NULL);
		do
		{
			DWORD written;
			DWORD toWrite;
			result = mediumFile.pstm->Read(pBuf, ISTREAM_BUF_SIZE, &read);
			toWrite = read;
			do {
				WriteFile(hFile, &pBuf[read-toWrite], toWrite, &written, NULL);
				toWrite -= written;
			} while (toWrite);

		} while (read == ISTREAM_BUF_SIZE);
		CloseHandle(hFile);
	}
}

// http://netez.com/2xExplorer/shellFAQ/adv_clip.html
void olePaste()
{
	HRESULT result;
	result = OleInitialize(NULL);
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
	wprintf(L"\n\nFormats on the clipboard:\n");
	FORMATETC formatDescriptor; ZeroMemory(&formatDescriptor, sizeof(FORMATETC));
	FORMATETC formatContents; ZeroMemory(&formatContents, sizeof(FORMATETC));
	while (enumFormats->Next(1, &format, NULL) == S_OK)
	{
		DWORD tymed;
		STGMEDIUM medium;
		const int cchMaxCount = 255;
		TCHAR lpszFormatName[cchMaxCount];
		GetClipboardFormatName(
			format.cfFormat,
			lpszFormatName,
			cchMaxCount
		);
		tymed = format.tymed;
		wprintf(L"Found: %i - %ls\n", format.cfFormat, lpszFormatName);
		if (TYMED_HGLOBAL & tymed) {
			wprintf(L"\tTYMED_HGLOBAL\n");
		}
		if (TYMED_FILE & tymed) {
			wprintf(L"\tTYMED_FILE\n");
		}
		if (TYMED_ISTREAM & tymed) {
			wprintf(L"\tTYMED_ISTREAM\n");
		}
		if (TYMED_ISTORAGE & tymed) {
			wprintf(L"\tTYMED_ISTORAGE\n");
		}
		if (TYMED_GDI & tymed) {
			wprintf(L"\tTYMED_GDI\n");
		}
		if (TYMED_MFPICT & tymed) {
			wprintf(L"\tTYMED_MFPICT\n");
		}
		if (TYMED_ENHMF & tymed) {
			wprintf(L"\tTYMED_ENHMF\n");
		}
		if (TYMED_NULL & tymed) {
			wprintf(L"\tTYMED_NULL\n");
		}
		result = dataObject->GetData(&format, &medium);
		tymed = medium.tymed;
		wprintf(L"Medium result:\n");
		void* p;
		LARGE_INTEGER pos = { 0, 0 };
		BYTE pszBuf[1024];
		ULONG read;
		FILEGROUPDESCRIPTOR* fileDesc;
		switch (result)
		{
		case S_OK:
			wprintf(L"S_OK\n");
			if (TYMED_HGLOBAL & tymed) {
				wprintf(L"\tTYMED_HGLOBAL\n");
				HGLOBAL hGlobal = GlobalLock(medium.hGlobal);
				//fileDesc = (FILEGROUPDESCRIPTOR*)GlobalLock(medium.hGlobal);
				if (format.cfFormat == RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT))
				{
					const DWORD& operation = *(DWORD*)hGlobal;
					
					if (DROPEFFECT_NONE & operation)
						wprintf(L"\tNONE\n");
						
					if (DROPEFFECT_COPY & operation)
						wprintf(L"\tCOPY\n");
						
					if ( DROPEFFECT_MOVE & operation)
						wprintf(L"\tMOVE\n");
						
					if ( DROPEFFECT_LINK & operation)
						wprintf(L"\tLINK\n");
						
					if ( DROPEFFECT_SCROLL & operation)
						wprintf(L"\tSCROLL\n");
						
				}
			}
			if (TYMED_FILE & tymed) {
				wprintf(L"\tTYMED_FILE\n");
			}
			if (TYMED_ISTREAM & tymed) {
				wprintf(L"\tTYMED_ISTREAM\n");
				//result = medium.pstm->Seek(pos, STREAM_SEEK_SET, NULL);
				//result = medium.pstm->Read(pszBuf, 1024, &read);
			}
			if (TYMED_ISTORAGE & tymed) {
				wprintf(L"\tTYMED_ISTORAGE\n");
			}
			if (TYMED_GDI & tymed) {
				wprintf(L"\tTYMED_GDI\n");
			}
			if (TYMED_MFPICT & tymed) {
				wprintf(L"\tTYMED_MFPICT\n");
			}
			if (TYMED_ENHMF & tymed) {
				wprintf(L"\tTYMED_ENHMF\n");
			}
			if (TYMED_NULL & tymed) {
				wprintf(L"\tTYMED_NULL\n");
			}
			break;
		case DV_E_LINDEX:
			wprintf(L"The value for lindex is not valid; currently, only -1 is supported.");
			break;
		case DV_E_FORMATETC:
			wprintf(L"The value for pformatetcIn is not valid.");
			break;
		case DV_E_TYMED:
			wprintf(L"The tymed value is not valid.");
			break;
		case DV_E_DVASPECT:
			wprintf(L"The dwAspect value is not valid.");
			break;
		case OLE_E_NOTRUNNING:
			wprintf(L"The object application is not running.");
			break;
		case STG_E_MEDIUMFULL:
			wprintf(L"An error occurred when allocating the medium.");
			break;
		case E_UNEXPECTED:
			wprintf(L"An unexpected error has occurred.");
			break;
		case E_INVALIDARG:
			wprintf(L"The dwDirection value is not valid.");
			break;
		case E_OUTOFMEMORY:
			wprintf(L"There was insufficient memory available for this operation.");
			break;
		default:
			wprintf(L"Unknown error");
		}
		wprintf(L"\n###\n");
	}
	if (!*(int*)&formatDescriptor || !*(int*)&formatContents)
	{
		wprintf(L"Needed formats not within the clipboard.");
		return;
	}
//	tagTYMED
//	STGMEDIUM medium;
//	format.cfFormat = 49265;
//	format.dwAspect = DVASPECT_CONTENT;
//	format.lindex = 0;
//	format.ptd = NULL;
//	format.tymed = TYMED_ISTREAM;
//	result = dataObject->GetData(&format, &medium);
//
//	STGMEDIUM mediumDescriptor;
//	result = dataObject->GetData(&formatDescriptor, &mediumDescriptor);
//	LPCTSTR ptxt = (LPCTSTR)GlobalLock(mediumDescriptor.hGlobal); // dont' forget GlobalUnlock(stgm.hGlobal);
//
//	LARGE_INTEGER pos = { 0, 0 };
//	ULONG i = 0;
//	BYTE pszBuf[1024];
//	int nLength = 1024;
//	STGMEDIUM mediumContents;
//	//formatContents.cfFormat = 49265;
//	//formatContents.dwAspect = DVASPECT_CONTENT;
//	//formatContents.tymed = TYMED_ISTREAM;
//	result = dataObject->GetData(&formatContents, &mediumContents);
//	result = mediumContents.pstm->Seek(pos, STREAM_SEEK_SET, NULL);
//	result = mediumContents.pstm->Read(pszBuf, nLength, &i);
	int lala = 0;
}