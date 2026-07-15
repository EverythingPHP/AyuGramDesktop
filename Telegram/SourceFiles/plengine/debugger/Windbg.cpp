// Source - https://stackoverflow.com/a/25442301
// Posted by Praveen
// Retrieved 2026-07-15, License - CC BY-SA 3.0
/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026 // t.me/ayuplugg
Follows GNU GPL v3 and Telegram Desktop licensing.
*/
#include <windows.h>
#include <Dbghelp.h>
#include <tchar.h>
#include <timeapi.h>
#include <string>


typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

void create_minidump(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
    HMODULE mhLib = ::LoadLibrary(_T("dbghelp.dll"));
    MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(mhLib, "MiniDumpWriteDump");
   
    HANDLE  hFile = ::CreateFile(_T(std::string(std::to_string(timeGetTime()) + "ayugram.dmp").c_str()), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
    ExInfo.ThreadId = ::GetCurrentThreadId();
    ExInfo.ExceptionPointers = apExceptionInfo;
    ExInfo.ClientPointers = FALSE;

    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
    ::CloseHandle(hFile);
}

LONG WINAPI unhandled_handler(struct _EXCEPTION_POINTERS* apExceptionInfo)
{
    create_minidump(apExceptionInfo);
    return EXCEPTION_CONTINUE_SEARCH;
}