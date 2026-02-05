#pragma once

/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

#include <vector>
#include <AyuPlugin.h>

inline extern std::vector<InternalDoFilterHistoryItem> FunctionsOnFilter{};
inline extern std::vector<InternalDoPreProcessMessage> FunctionsOnPrepare{};
inline extern std::vector<InternalIsOnline> FunctionsOnIsOnline{};