#pragma once
#include <history\history.h>

#ifdef AYUPLUGIN
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
#endif

/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

#include <cstdint>
struct MemData
{
	uintptr_t applicationAddr;
	uintptr_t activeUserPtr;
};

struct AyuPlugin
{
	wchar_t name[128];
	wchar_t description[255];
	wchar_t moduleName[128];
	bool sharedFiltersEnabled;
	MemData memData;
};

typedef void (*InternalLoop)(void);
typedef bool (*InternalDoFilterHistoryItem)(HistoryItem *);
typedef void (*InternalDoPreProcessMessage)(char* in, char* out);
typedef bool (*InternalIsOnline)(void);
typedef AyuPlugin *(*InternalPluginInfo)();