/*
Emil Kh, AKA Pomorgite - t.me/Pomorgite // pmrgt.com
AyuGram Plugin engine, 2026 // t.me/ayuplugg
Follows GNU GPL v3 and Telegram Desktop licensing.
*/

#pragma once
#include <stdlib.h>
#include <string>
#include <iostream>
class PLESettings
{
	private:
		bool _consoleEnabled = isEnvSet("AYUPL_CONSOLE");
		bool _apiEnabled = isEnvSet("AYUPL_PORT");
		std::string _apiCert = std::string(getenv("AYUPL_SSL_CERT") != nullptr ? getenv("AYUPL_SSL_CERT") : "");
		std::string _apiKey = std::string(getenv("AYUPL_SSL_KEY") != nullptr ? getenv("AYUPL_SSL_KEY") : "");
		bool _apiSecEnabled = isEnvSet("AYUPL_DISABLESECURITYFEATURES");
		inline static PLESettings* instancePtr = nullptr;

		PLESettings() {
			_consoleEnabled = isEnvSet("AYUPL_CONSOLE");
			_apiEnabled = isEnvSet("AYUPL_PORT");
			_apiSecEnabled = isEnvSet("AYUPL_DISABLESECURITYFEATURES");
			_apiCert = std::string(getenv("AYUPL_SSL_CERT") != nullptr ? getenv("AYUPL_SSL_CERT") : "");
			_apiKey = std::string(getenv("AYUPL_SSL_KEY") != nullptr ? getenv("AYUPL_SSL_KEY") : "");
		}
	public:
		PLESettings(const PLESettings& obj) = delete;
		static PLESettings* getInstance() {
			if (instancePtr == nullptr) {
				instancePtr = new PLESettings();
			}
			return instancePtr;
		}

		static bool isEnvSet(std::string name) {
			return getenv(name.c_str()) != nullptr && std::string(getenv(name.c_str())) != "" && std::string(getenv(name.c_str())) != "0";
		}
		inline std::string apiCert() {
			return _apiCert;
		}
		inline std::string apiKey() {
			return _apiKey;
		}
		inline void setCert(std::string val) {
			_apiCert = val;
			writeRegistryEnv("AYUPL_SSL_CERT", _apiCert);
		}
		inline void setKey(std::string val) {
			_apiKey = val;
			writeRegistryEnv("AYUPL_SSL_KEY", _apiKey);
		}

		inline bool consoleEnabled() {
			return _consoleEnabled;
		}
		inline void setConsoleEnabled(bool val) {
			_consoleEnabled = val;
			val ? writeRegistryEnv("AYUPL_CONSOLE", "1") : writeRegistryEnv("AYUPL_CONSOLE", "");
		}
		inline bool apiEnabled() {
			return _apiEnabled;
		}
		inline void setapiEnabled(bool val) {
			_apiEnabled = val;
			if (val) {
				writeRegistryEnv("AYUPL_PORT", "8081");
				writeRegistryEnv("AYUPL_HOST", "127.0.0.1");
			}
			else {
				writeRegistryEnv("AYUPL_PORT", "");
				writeRegistryEnv("AYUPL_HOST", "");
			}
		}
		inline bool apiSecEnabled() {
			return _apiSecEnabled;
		}
		inline void setapiSecEnabled(bool val) {
			_apiSecEnabled = val;
			val ? writeRegistryEnv("AYUPL_DISABLESECURITYFEATURES", "1") : writeRegistryEnv("AYUPL_DISABLESECURITYFEATURES", "");
		}

		static inline void writeRegistryEnv(std::string name, std::string value) { 
			std::thread t([name, value] {
				// this is a shady workaround for what we're doing, but i'm slim shady, all you other slim shadys are just imitating
				if (value == "") {
					system(("setx " + name + " 0").c_str()); // apply rn
					system(("REG DELETE \"HKCU\\Environment\" /V \"" + name + "\" /F").c_str());
					return;
				}
				system(("setx \"" + name + "\"	" + value).c_str()); // apply rn
				std::cout << "Saved setting " << name << " with value " << value << std::endl;
			});
			t.detach();
			
		}
};

