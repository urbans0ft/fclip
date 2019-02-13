#include "pch.h" // Pre-compiled header

void paste();
void olePaste();
void olePaste2();

int run(int argc, TCHAR* argv[])
{
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
	//olePaste();
	olePaste2();
	std::string wait;
	std::getline(std::cin, wait);

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

void olePaste2()
{//49366
	int cntClipFormat = CountClipboardFormats();
	wprintf(L"Found %i Clipboard Formats.\n", cntClipFormat);
	UINT iClipFormat = 0;
	OpenClipboard(NULL);
	while (iClipFormat = EnumClipboardFormats(iClipFormat))
	{
		const int cntMax = 255;
		TCHAR szFormatName[255];
		GetClipboardFormatName(iClipFormat, szFormatName, cntMax);
		wprintf(L"\t %i: %ls\n", iClipFormat, szFormatName);
	}
	CloseClipboard();

	UINT iFileContentsFormat = RegisterClipboardFormat(CFSTR_FILECONTENTS);
	if (!IsClipboardFormatAvailable(iFileContentsFormat))
	{
		wprintf(L"CFSTR_FILECONTENTS not available!\n");
		return;
	}
	UINT iFileDescriptorFormat = RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
	if (!IsClipboardFormatAvailable(iFileDescriptorFormat))
	{
		wprintf(L"CFSTR_FILEDESCRIPTOR not available!\n");
		return;
	}
	
	HRESULT result;
	IDataObject* dataObject;
	result = OleInitialize(NULL);
	result = OleGetClipboard(&dataObject);

	FORMATETC fmtFileDescriptor{
		iFileDescriptorFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
	};
	FORMATETC fmtFileContents{
		iFileContentsFormat, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM
	};
	STGMEDIUM stgFileDescriptor{ 0 };
	STGMEDIUM stgFileContents{ 0 };
	
	result = dataObject->GetData(&fmtFileDescriptor, &stgFileDescriptor);
	
	IDataObject* dataObject2;
	result = OleGetClipboard(&dataObject2);
	result = dataObject2->GetData(&fmtFileContents, &stgFileContents);
	
	FILEGROUPDESCRIPTOR* fileGrpDescriptor =
		(FILEGROUPDESCRIPTOR*)GlobalLock(stgFileDescriptor.hGlobal);

	LARGE_INTEGER pos = { 0, 0 };
	const int cntBuffer = 1024;
	BYTE buffer[cntBuffer];
	ULONG read;
	result = stgFileContents.pstm->Seek(pos, STREAM_SEEK_SET, NULL);
	result = stgFileContents.pstm->Read(buffer, cntBuffer, &read);

	GlobalUnlock(stgFileDescriptor.hGlobal);
	OleUninitialize();
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
			wprintf(L"The value for lindex is not valid; currently, only -1 is supported.\n");
			break;
		case DV_E_FORMATETC:
			wprintf(L"The value for pformatetcIn is not valid.\n");
			break;
		case DV_E_TYMED:
			wprintf(L"The tymed value is not valid.\n");
			break;
		case DV_E_DVASPECT:
			wprintf(L"The dwAspect value is not valid.\n");
			break;
		case OLE_E_NOTRUNNING:
			wprintf(L"The object application is not running.\n");
			break;
		case STG_E_MEDIUMFULL:
			wprintf(L"An error occurred when allocating the medium.\n");
			break;
		case E_UNEXPECTED:
			wprintf(L"An unexpected error has occurred.\n");
			break;
		case E_INVALIDARG:
			wprintf(L"The dwDirection value is not valid.\n");
			break;
		case E_OUTOFMEMORY:
			wprintf(L"There was insufficient memory available for this operation.\n");
			break;
		case CLIPBRD_E_BAD_DATA:
			wprintf(L"Data on clipboard is invalid.\n");
			break;
		default:
			wprintf(L"Unknown error\n");
		}
		wprintf(L"\n###\n");
	}
}