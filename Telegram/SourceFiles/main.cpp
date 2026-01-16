/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/

/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026
Follows GNU GPL v3 and Telegram Desktop licensing.
*/
#include "core/launcher.h"
#include "httplib.h"
#include <thread>
#include <json.hpp>
#include <core/application.h>
#include "main/main_domain.h"
#include "main/main_account.h"
#include "data/data_user.h"
#include "main/main_session.h"
#include <stddef.h>
#include "apiwrap.h"
#include "AyuPlugin.h"
#include "FunctionsOnFilter.h"
using json = nlohmann::json;
httplib::Server svr;

namespace ns {
	struct DefaultJSONResponse
	{
		bool ok;
		std::string error;
		std::string result_json;
	};

	void to_json(json &j, const DefaultJSONResponse &p) {
		j = json{{"ok", p.ok}, {"error", p.error}, {"result_json", p.result_json}};
	}

	void from_json(const json &j, DefaultJSONResponse &p) {
		j.at("ok").get_to(p.ok);
		j.at("error").get_to(p.error);
		j.at("result_json").get_to(p.result_json);
	}

	struct SimplifiedTGAccount
	{
		int index;
		std::string auth_key;
		int dc;
		long user_id;
		std::string username;
		std::string first_name;
		std::string last_name;
	};

	void to_json(json &j, const SimplifiedTGAccount &p) {
		j = json{{"index", p.index},
				 {"auth_key", p.auth_key},
				 {"dc", p.dc},
				 {"user_id", p.user_id},
				 {"username", p.username},
				 {"first_name", p.first_name},
				 {"last_name", p.last_name}
		};
	}

	void from_json(const json &j, SimplifiedTGAccount &p) {
		j.at("index").get_to(p.index);
		j.at("auth_key").get_to(p.auth_key);
		j.at("dc").get_to(p.dc);
		j.at("user_id").get_to(p.user_id);
		j.at("username").get_to(p.username);
		j.at("first_name").get_to(p.first_name);
		j.at("last_name").get_to(p.last_name);
	}
	}

std::string OKResponse(bool ok = true, std::string errors = "", std::string res = "") { 
	ns::DefaultJSONResponse aok = {ok, errors, res}; 
	json aokj = aok;
	return aokj.dump().c_str();
}

std::string to_hex_string(bytes::const_span data) {
	std::ostringstream oss;
	oss << std::hex << std::setfill('0');
	for (auto b : data) {
		oss << std::setw(2) << static_cast<int>(gsl::to_integer<unsigned char>(b));
	}
	return oss.str();
}

struct TrustedUAs
{
	std::vector<std::string> runtime;
	std::vector<std::string> session;
	std::vector<std::string> events; // todo
};

enum TypeTrust
{
	runtime,
	session,
	events
};

struct SharedMemHelper
{
	uintptr_t applicationAddr; 
	UserData* activeUserPtr;
	wchar_t currentAccountName[255];
	bool isSharingScreen; 
	bool isEnabled;
	// to be extended
};

TrustedUAs trusted;
ApiWrap* activeAccWrapper;
SharedMemHelper memHelper;

std::string GetTrustedNameByType(TypeTrust t) {
	return std::string(t == TypeTrust::runtime ? "Remote Code Execution" : t == TypeTrust::session
						   ? "Sessions and accounts"
						   : "Updates & Events");
}

std::vector<std::string>* GetTrustedByType(TypeTrust t) {
	return &(t == TypeTrust::runtime ? trusted.runtime : t == TypeTrust::session ? trusted.session : trusted.events);
 }

std::string getUA(const httplib::Request &req) {
	return (req.has_header("User-Agent")
		 ? req.get_header_value("User-Agent")
		 : (req.has_header("user-agent") ? req.get_header_value("user-agent") : "No user agent provided"));
}

bool IsTrustedUA(std::string ua, TypeTrust trust) {
	auto trustedScope = GetTrustedByType(trust);
	bool good = std::find(trustedScope->begin(), trustedScope->end(), ua) != trustedScope->end();
	if (!good) {
		wchar_t a[2048];
		mbstowcs(a,
				 std::string("Some application or plug-in is trying to elevate their permissions. Type of elevation: " +
							 GetTrustedNameByType(trust) +
							 ".  Please confirm, or "
							 "decline this request. U/A:" +
							 ua)
					 .c_str(),
				 2048);

		good = IDOK == MessageBox(NULL, a, L"Security warning", MB_OKCANCEL);
		if (good) {
			trustedScope->push_back(ua);
		}
	}
	
	return good;
}

int pollingRate = 1000; // ms

void updMemHelp() { 
	while (1) {
		memHelper.applicationAddr = ((uintptr_t) &Core::App());
		memHelper.isSharingScreen = Core::App().isSharingScreen();
		memHelper.activeUserPtr = Core::App().activeAccount().session().user();
		mbstowcs(memHelper.currentAccountName, memHelper.activeUserPtr->name().toStdString().c_str(), 255);
		Sleep(pollingRate);
	}
}

void loadMem() {
	if (memHelper.isEnabled == false) {
		memHelper.isEnabled = true;
		std::thread th(updMemHelp);
		th.detach();
	}
}

std::string uintToJSON(uintptr_t addr) {
	json b;
	char addrv[128];
	sprintf(addrv, "%p", addr);
	b["address"] = addrv;
	return b.dump();
}

void processDLL(std::string dll) { 

	wchar_t a[2048];
	mbstowcs(a, dll.c_str(), 2048);
	HINSTANCE dllInstance =
		LoadLibrary(a);

	if (dllInstance == NULL) {
		wchar_t a[512];
		mbstowcs(a,
				 std::string("DLL " +
							 dll + " failed to load with code " + std::to_string(GetLastError())
				 )
					 .c_str(),
				 512);

		MessageBox(NULL, a, L"Plugin load error", MB_OK);
		return;
	}

	AyuPlugin *pl = ((InternalPluginInfo) GetProcAddress(dllInstance, "pluginInfo"))();
	if (pl == NULL) {
		wchar_t a[512];
		mbstowcs(
			a, std::string("DLL " + dll + "'s global structure failed to load with code " + std::to_string(GetLastError())).c_str(), 512);

		MessageBox(NULL, a, L"Plugin execution error", MB_OK);
		return;
	}

	MessageBox(NULL, pl->name, L"Plugin loaded successfully", MB_OK);
	InternalLoop loop_func = (InternalLoop) GetProcAddress(dllInstance, "internalLoop");
	InternalDoFilterHistoryItem func = (InternalDoFilterHistoryItem) GetProcAddress(dllInstance, "doFilterHistoryItem");
	if (func != NULL && pl->sharedFiltersEnabled) {
		FunctionsOnFilter.push_back(func);
	}
	pl->memData.activeUserPtr = (uintptr_t) memHelper.activeUserPtr;
	pl->memData.applicationAddr = (uintptr_t) memHelper.applicationAddr;
	if (loop_func != NULL) {
		while (true) {
			loop_func();
		}
	}
}


void listenHTTP() {
	svr.Get("/api/ping",
			[](const httplib::Request &, httplib::Response &res)
			{ res.set_content(OKResponse(), "application/json"); });

	svr.Get("/api/runtime/setPolling",
			[](const httplib::Request &req, httplib::Response &res)
			{
				std::string ua = getUA(req);
				std::vector<ns::SimplifiedTGAccount> accs_2;
				bool good = IsTrustedUA(ua, TypeTrust::runtime);
				if (good && memHelper.isEnabled) {
					int r = std::stoi(req.get_param_value("rate"));
					r = r <= 10 ? 10 : r;
					pollingRate = r;
				}
				res.set_content(
					OKResponse(good,
							   good ? (memHelper.isEnabled ? "" : "Nobody's updating MemHelper, though.") : "User rejected trust elevation request.",
										   good ? uintToJSON((uintptr_t) &memHelper) : "null"),
					"application/json");
			});

	svr.Get("/api/runtime/load",
			[](const httplib::Request &req, httplib::Response &res)
			{
				std::string ua = getUA(req);
				std::vector<ns::SimplifiedTGAccount> accs_2;
				bool good = IsTrustedUA(ua, TypeTrust::runtime);
				if (good) {
					loadMem();
					std::string libName = req.get_param_value("name");
					std::thread t(processDLL, libName);
					t.detach();

				}
				res.set_content(OKResponse(good,
										   good ? "" : "User rejected trust elevation request.",
										   good ? uintToJSON((uintptr_t) &memHelper) : "null"),
								"application/json");
			});

	svr.Get("/api/runtime/export",
		[](const httplib::Request &req, httplib::Response &res)
		{
			std::string ua = getUA(req);
			std::vector<ns::SimplifiedTGAccount> accs_2;
			bool good = IsTrustedUA(ua, TypeTrust::runtime);
			if (good) {
				loadMem();
			}
			res.set_content(OKResponse(good,
									   good ? "" : "User rejected trust elevation request.",
									   good ? uintToJSON((uintptr_t) &memHelper) : "null") ,
				"application/json");
		});

	svr.Get("/api/session/get",
			[](const httplib::Request &req, httplib::Response &res)
			{
				std::string ua = getUA(req);
				std::vector<ns::SimplifiedTGAccount> accs_2;
				bool good = IsTrustedUA(ua, TypeTrust::session);
				if (good) {
					for (const auto &[index, account] : Core::App().domain().accounts()) {
						if (!account->sessionExists()) {
							continue;
						}
						auto session = &account->session();
						auto userdata = session->user().get();
						auto keylist = session->mtp().getKeysForWrite();
						auto dcid = session->mainDcId();
						std::string hexedKey = "";
						for (int ii = 0; ii < keylist.size(); ii++) {
							if (keylist.at(ii)->dcId() == dcid) {
								hexedKey = to_hex_string(keylist.at(ii)->data());
							}
						}
						ns::SimplifiedTGAccount a{index,
												  hexedKey,
												  dcid,
												  session->userPeerId().value & UserId::kShift,
												  userdata->username().toStdString(),
												  userdata->firstName.toStdString(),
												  userdata->lastName.toStdString()};
						accs_2.push_back(a);
					}
				}
				json response = accs_2;
				res.set_content(
					OKResponse(good, good ? "" : "User rejected trust elevation request.", good ? response.dump() : "null"),
								"application/json"); 
		});
	svr.listen("0.0.0.0", 8080);
}

int main(int argc, char *argv[]) {
	
	std::thread t(listenHTTP);
	t.detach();
	const auto launcher = Core::Launcher::Create(argc, argv);
	
	return launcher ? launcher->exec() : 1;
}
