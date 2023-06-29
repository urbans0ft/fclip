#pragma once
/**
 * @file fclip.h
 * @author Nico V. Urbanczyk (nico@urbansoft.eu)
 * @brief Declaration and documentation of functions used in fclip.cpp.
 * @version 0.1
 * @date 2023-04-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/**
 * @brief Print usage information.
 * 
 * Displays usage information on the console.
 * 
 */
void usage();
/**
 * @brief Copy the files specified by the command line to the clipboard.
 * 
 * @param argc The command line argument count
 * @param argv The command line arguments
 * @return int `0` indicates success; `INVALID_FILE_ATTRIBUTES` (`-1`) failure.
 */
int copy(int argc, wchar_t* argv[]);
/**
 * @brief paste files
 * 
 * The function checks if there are files present on the clipboard. If so
 * it calls either pasteByFileContents() or pasteByCfHDrop().
 */
void paste();
/**
 * @brief paste file(s) from the clipboard
 * 
 * This function gets called by paste() do not use it directly.
 * 
 * @param clFileDescriptor The clipboard format used with the CFSTR_FILECONTENTS format to transfer
 *                         data as a group of files.
 * @param clFileContents   The clipboard format used with the CFSTR_FILEDESCRIPTOR format to
 *                         transfer data as if it were a file, regardless of how it is actually
 *                         stored.
 */
void pasteByFileContents(CLIPFORMAT clFileDescriptor, CLIPFORMAT clFileContents);
/**
 * @brief paste file(s) from the clipboard
 * 
 * This function gets called by paste() do not use it directly. The paste operation
 * is done via `CF_HDROP`.
 * 
 * @note No checks are implemented. The caller must ensure that the `CF_HDROP`
 *       clipboard format is available.
 * 
 * @sa https://learn.microsoft.com/en-us/windows/win32/shell/clipboard
 * 
 */
void pasteByCfHDrop();
/**
 * @brief Prints the currently available clipboard formats.
 * 
 * Used for debugging: Prints all currently available clipboard formats to
 * the console.
 */
#ifdef DEBUG
void printClipboardFormats();
#endif

