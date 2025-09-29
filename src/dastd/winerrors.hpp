/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 11-OCT-2024
*
* This module provides with a simple conversion of Windows "FormatMessage" to
* standard strings.
* It can be included also in other platforms, but it will be excluded by the `DASTD_WIN` macro.
**/
#pragma once
#include "defs.hpp"
#ifdef DASTD_WIN

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <string>
namespace dastd {

/// @brief Translates a message (typically the return of "GetLastError()") into a readable string
inline std::string win_format_message(DWORD msg)
{
	char* errBuf = 0;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, NULL, msg, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errBuf, 0, NULL);
	std::string s = errBuf;
	LocalFree (errBuf);
	return s;
}

/// @brief Translates a the return of "GetLastError()" into a readable string
inline std::string win_format_last_error() {return win_format_message(GetLastError());}

} // namespace dastd
#endif
