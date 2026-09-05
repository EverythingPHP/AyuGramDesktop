#pragma once
#include <history\history.h>
#include <ayu/ui/settings/ayu_builder.h>
#include <settings/settings_builder.h>
#include <plengine/settings/settings_plugins.h>
#include <plengine/helpers/PseudoPopup.h>
#include <history\view\history_view_context_menu.h>
#include <ui\widgets\popup_menu.h>
#include <cstdint>

#ifdef AYUPLUGIN
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
#endif


/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026 // t.me/ayuplugg
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

typedef void (*ExAddToQueue)(const std::function<void()> &func);

struct MemData
{
	uintptr_t applicationAddr;
	uintptr_t activeUserPtr;
	uintptr_t activeSessionPtr;
	ExAddToQueue addToQueue;
};

struct AyuPlugin
{
	wchar_t name[128];
	wchar_t description[255];
	wchar_t moduleName[128];
	MemData memData;
};

enum FilteredState
{
	NoMatch,
	Filtered,
	RejectFurtherFiltering
};

typedef void (*InternalSetup)(void);
typedef void (*InternalLoop)(void);
typedef FilteredState (*InternalDoFilterHistoryItem)(HistoryItem *);
typedef bool (*InternalExcludeDeletion)(HistoryItem *);
typedef void (*InternalDoPreProcessMessage)(std::string *in, std::string* out);
typedef void (*InternalDrawPopupItem)(Ui::PseudoPopup* popupMenu, HistoryItem* i);
typedef void (*InternalDrawGUI)(Settings::Builder::SectionBuilder& builder, Settings::AyuBuilder::AyuSectionBuilder& ayu, Settings::PLEPlugins* ple);
typedef bool (*InternalIsOnline)(void);
typedef AyuPlugin *(*InternalPluginInfo)();
