/**
 * @file      fclip.cpp
 * @author    Nico V. Urbanczyk (nico@urbansoft.eu)
 * @brief     Complete source code with all functionality.
 * @version   0.1
 * @date      2022-10-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "pch.h" // Pre-compiled header

/**
 * @brief The main entry point
 * 
 * @param argc The command line argument count
 */
int main(int argc, char** /*argv*/)
{
	DWORD procId; DWORD processCount = GetConsoleProcessList(&procId, 1);	
	DBGPRINT(L"%S %s", VERSION, __DATE__ " " __TIME__);
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
	if (argc == 1) {
		std::filesystem::path exe = argv[0];
		std::wcout << exe.stem().wstring() << L" Version " << VERSION << std::endl;
		usage();
		DBGPRINT(L"GetConsoleProcessList => %d", processCount);
		if (processCount < 2) { // < 2 => GUI
			system("pause");
		}
		return 0;
	}
	// parse command line parameter
	if (argc == 2 && !wcscmp(argv[1], L"-v"))
		paste();
	else
		return copy(argc, argv);
	return 0;
}

int copy(int argc, wchar_t* argv[])
{
	DBGPRINT(L"Copy file(s) to clipboard (count: %d).", argc - 1);
	// create / fill full file path buffer and calculate global buffer
	wchar_t** files = new wchar_t*[argc - 1];
	DWORD*  bufSizes = new DWORD[argc - 1];
	int     clpSize = sizeof(DROPFILES);

	for (int i = 1; i < argc; i++) {
		bufSizes[i - 1] = GetFullPathName(argv[i], 0, NULL, NULL);
		DBGPRINT(L"Buffer size for '%S' == %d", argv[i], bufSizes[i - 1]);
		files[i - 1] = new wchar_t[bufSizes[i - 1]];
		// (re-)setting bufSize because calculation of relative paths (containing '..\')
		// migth be to long. The actual setting of the variable is done by eleminating relative
		// path elements (e.g.: ..\..\file.dat might become C:\file.dat).
		bufSizes[i - 1] = GetFullPathName(argv[i], bufSizes[i - 1], files[i - 1], NULL) + 1; // +1 => \0
		DBGPRINT(L"(Re-)setting buffer size to %d", bufSizes[i - 1]);
		if (GetFileAttributes(files[i - 1]) == INVALID_FILE_ATTRIBUTES) {
			const auto& err = LastError::New();
			std::wcerr << files[i - 1] << ": " << err << std::endl;
			return INVALID_FILE_ATTRIBUTES;
		}
		// calculate *bytes* needed for memory allocation
		clpSize += sizeof(wchar_t) * (bufSizes[i - 1]);
		DBGPRINT(L"Added '%S' to buffer.", files[i - 1]);
	}
	clpSize += sizeof(wchar_t); // two \0 needed at the end

	DBGPRINT(L"Alloc global memory %d", clpSize);
	// allocate the zero initialized memory
	HDROP hdrop = (HDROP)GlobalAlloc(GHND, clpSize);
	DROPFILES* df = (DROPFILES*)GlobalLock(hdrop);
	df->pFiles = sizeof(DROPFILES); // string offset
	df->fWide = TRUE; // unicode file names

	DBGPRINT(L"Copy filename(s) to global memory.");
	// copy the command line args to the allocated memory
	wchar_t* dstStart = (wchar_t*)&df[1];
	for (int i = 0; i < argc - 1; i++)
	{
		wcscpy(dstStart, files[i]);
		dstStart = &dstStart[bufSizes[i]];
	}
	GlobalUnlock(hdrop);

	// prepare the clipboard
	DBGPRINT(L"Set clipboard data (CF_HDROP).");
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_HDROP, hdrop);
	CloseClipboard();
	DBGPRINT(L"Clibpoard data set!");
	return 0;
}

void pasteByCfHDrop()
{
	UNUSED(HRESULT result);
	result = OleInitialize(NULL);
	IDataObject* dataObject;
	result = OleGetClipboard(&dataObject);
	FORMATETC formatDescriptor{
		CF_HDROP,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};
	STGMEDIUM mediumDescriptor;
	result = dataObject->GetData(&formatDescriptor, &mediumDescriptor);
	UINT fileCount = DragQueryFile((HDROP)mediumDescriptor.hGlobal, 0xFFFFFFFF, NULL, 0);
	DBGPRINT(L"Found %d files in in clipboard.", fileCount);
	for (UINT i = 0; i < fileCount; i++)
	{
		UINT cch = DragQueryFile((HDROP)mediumDescriptor.hGlobal, i, NULL, 0) + 1;
		DBGPRINT(L"File[%d].Length => %d", i, cch-1);
		wchar_t* fileName = new wchar_t[cch];
		DragQueryFile((HDROP)mediumDescriptor.hGlobal, i, fileName, cch);
		DBGPRINT(L"File[%d].Name => %S", i, fileName);
		std::filesystem::path filePath = fileName;
		DBGPRINT(L"Copy '%S' to '%S'", filePath.c_str(), filePath.filename().c_str());
		std::filesystem::copy(filePath, filePath.filename());

		delete[] fileName;
	}
}

void paste()
{
	#ifdef DEBUG
	printClipboardFormats();
	#endif

	DBGPRINT(L"Check if files need to be pasted.");
	UINT formatFileDescriptor = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	UINT formatFileContents   = RegisterClipboardFormat(CFSTR_FILECONTENTS);
	// prefer pasteByFileContents() over pasteByCfHDrop()
	if (IsClipboardFormatAvailable(formatFileDescriptor)
		&& IsClipboardFormatAvailable(formatFileContents))
	{
		DBGPRINT(L"Pasting by FILECONTENTS / FILEDESCRIPTOR");
		pasteByFileContents(formatFileDescriptor, formatFileContents);
		return;
	}
	else if (IsClipboardFormatAvailable(CF_HDROP))
	{
		DBGPRINT(L"Pasting by CF_HDROP");
		pasteByCfHDrop();
		return;
	}

	DBGPRINT(L"Nothing to paste.");
}

void pasteByFileContents(CLIPFORMAT clFileDescriptor, CLIPFORMAT clFileContents)
{
	UNUSED(HRESULT result);
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
	DBGPRINT(L"Paste file(s) from clipboard (count: %d).", pFileGrpDescriptor->cItems);
	for (UINT i = 0; i < pFileGrpDescriptor->cItems; i++)
	{
		const FILEDESCRIPTOR& fDescriptor = pFileGrpDescriptor->fgd[i];
		DBGPRINT(L"File in group descriptor: %ls\n", fDescriptor.cFileName);
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
			(LONG)i, // index of the desired file
			TYMED_ISTREAM
		};
		STGMEDIUM mediumFile;
		result = dataObject->GetData(&formatFileContents, &mediumFile);

		BYTE* pBuf = new BYTE[ISTREAM_BUF_SIZE];
		ULONG read;
		result = mediumFile.pstm->Seek({}, STREAM_SEEK_SET, NULL);
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

void usage()
{
	std::wcout << L"" << std::endl;
	std::wcout << L"Copys files to the clipbard as if done by pressing <ctrl>+c" << std::endl;
	std::wcout << L"or pastes files from the clipboard by copying them to the current location." << std::endl;
	std::wcout << std::endl;
	std::wcout << L"fileclip [-v | file1 [file2 [... [fileN]]]]" << std::endl;
	std::wcout << std::endl;
	std::wcout << L"-v        Paste files previously copyied to the cliboard into the current folder." << std::endl;
	std::wcout << L"file1 ... The relative and/or absolute file paths to copy to the clipboard." << std::endl;
	std::wcout << std::endl;
	std::wcout << L"example:" << std::endl;
	std::wcout << L"> fileclip -v" << std::endl;
	std::wcout << L"> fileclip C:\\path\\to\\file1 file2 ..\\file3 \"folder\\file 4\"" << std::endl;
}

#ifdef DEBUG
void printClipboardFormats()
{
	DWORD errCode = 0;
	UINT format   = 0;
	DBGPRINT(L"Enumarating clipboard formats");
	OpenClipboard(NULL);
	UNUSED(HANDLE hHdrop);
	hHdrop = GetClipboardData(CF_HDROP);
	errCode = GetLastError();

	while ((format = EnumClipboardFormats(format)))
	{
		const static int cchMaxCount = 255;
		wchar_t lpszFormatName[cchMaxCount];
		GetClipboardFormatName(format, lpszFormatName, cchMaxCount);
		DBGPRINT(L"Format: %d, %S", format, lpszFormatName);
	}
	errCode = GetLastError();
	if (errCode != ERROR_SUCCESS)
	{
		DBGPRINT(L"GetLastError: %d", errCode);
	}
	CloseClipboard();
}
#endif