#include "pch.h" // Pre-compiled header

int copy(int argc, TCHAR* argv[]);
void paste();
void pasteByHdrop();
void pasteByFileContents(CLIPFORMAT clFileDescriptor, CLIPFORMAT clFileContents);

// usage: fclip [-v|[file1 [file2 [...]]]]
int run(int argc, TCHAR* argv[])
{
	if (argc == 1) {
		std::wcout << argv[0] << L" Version " << VERSION << std::endl;
		return 0;
	}
	// parse command line parameter
	if (argc == 2 && !_tcscmp(argv[1], _T("-v")))
		paste();
	else
		copy(argc, argv);

    return 0;
}

int copy(int argc, TCHAR* argv[])
{
	// create / fill full file path buffer and calculate global buffer
	TCHAR** files = new TCHAR*[argc - 1];
	DWORD*  bufSizes = new DWORD[argc - 1];
	int     clpSize = sizeof(DROPFILES);

	for (int i = 1; i < argc; i++) {
		bufSizes[i - 1] = GetFullPathName(argv[i], 0, NULL, NULL);
		files[i - 1] = new TCHAR[bufSizes[i - 1]];
		GetFullPathName(argv[i], bufSizes[i - 1], files[i - 1], NULL);
		if (GetFileAttributes(files[i - 1]) == INVALID_FILE_ATTRIBUTES)
			return INVALID_FILE_ATTRIBUTES;
		// calculate *bytes* needed for memory allocation
		clpSize += sizeof(TCHAR) * (bufSizes[i - 1]);
	}
	clpSize += sizeof(TCHAR); // two \0 needed at the end

							  // allocate the zero initialized memory
	HDROP hdrop = (HDROP)GlobalAlloc(GHND, clpSize);
	DROPFILES* df = (DROPFILES*)GlobalLock(hdrop);
	df->pFiles = sizeof(DROPFILES); // string offset
#ifdef _UNICODE
	df->fWide = TRUE; // unicode file names
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

void paste()
{
//	if (IsClipboardFormatAvailable(CF_HDROP))
//	{
//		pasteByHdrop();
//		return;
//	}
	UINT formatFileDescriptor = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	UINT formatFileContents   = RegisterClipboardFormat(CFSTR_FILECONTENTS);
	if (IsClipboardFormatAvailable(formatFileDescriptor)
		&& IsClipboardFormatAvailable(formatFileContents))
	{
		pasteByFileContents(formatFileDescriptor, formatFileContents);
		return;
	}
	_tprintf(_T("Nothing to paste!\n"));
}

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
		_tprintf(_T("copy %ls %ls\n"), oldFiles[i], newFiles[i]);
		CopyFile(oldFiles[i], newFiles[i], FALSE);
	}
	
	int lala = 0;
}

void pasteByFileContents(CLIPFORMAT clFileDescriptor, CLIPFORMAT clFileContents)
{
	HRESULT result;
	result = OleInitialize(NULL);
	IDataObject* dataObject;
	result = OleGetClipboard(&dataObject);
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
		_tprintf(_T("File in group descriptor: %ls\n"), fDescriptor.cFileName);
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
			i, // index of the desired file
			TYMED_ISTREAM
		};
		STGMEDIUM mediumFile;
		result = dataObject->GetData(&formatFileContents, &mediumFile);
		void * p = mediumFile.pstm;

		BYTE* pBuf = new BYTE[ISTREAM_BUF_SIZE];
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
