#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_SSL_ENABLED
#include "httplib.h"
#include <Shlwapi.h>
#include <string>

class MixedServer
{
	public:
        inline void Init(char* host = nullptr, char* cert = nullptr, char* key = nullptr) {

            host != nullptr && host[0] ? strcpy(_host, host) : strcpy(_host, "127.0.0.1");
            cert != nullptr && cert[0] ? strcpy(_cert, cert) : strcpy(_cert, "");
            key != nullptr && key[0] ? strcpy(_key, key) : strcpy(_key, "");

            if (PathFileExistsA(_cert) && PathFileExistsA(_key)) {
                svrssl = new httplib::SSLServer(_cert, _key);
                std::cout << "Using SSL at " << _cert;
            }
            else {
                svr = new httplib::Server();
            }
        }

        inline void Listen(int port=8080) {
            (svrssl == nullptr) ? svr->listen(_host, port) : svrssl->listen(_host, port);
        }

        inline void Get(const std::string& pattern, httplib::Server::Handler handler) {
            svrssl == nullptr ? svr->Get(pattern, handler) : svrssl->Get(pattern, handler);
        }

        inline void Post(const std::string& pattern, httplib::Server::Handler handler) {
            svrssl == nullptr ? svr->Post(pattern, handler) : svrssl->Post(pattern, handler);
        }

        inline void Post(const std::string& pattern,
            httplib::Server::HandlerWithContentReader handler) {
            svrssl == nullptr ? svr->Post(pattern, handler) : svrssl->Post(pattern, handler);
        }

        inline void Put(const std::string& pattern, httplib::Server::Handler handler) {
            svrssl == nullptr ? svr->Put(pattern, handler) : svrssl->Put(pattern, handler);
        }

        inline void Put(const std::string& pattern,
            httplib::Server::HandlerWithContentReader handler) {
            svrssl == nullptr ? svr->Put(pattern, handler) : svrssl->Put(pattern, handler);
        }

        inline void Patch(const std::string& pattern, httplib::Server::Handler handler) {
            svrssl == nullptr ? svr->Patch(pattern, handler) : svrssl->Patch(pattern, handler);
        }

        inline void Patch(const std::string& pattern,
            httplib::Server::HandlerWithContentReader handler) {
            svrssl == nullptr ? svr->Patch(pattern, handler) : svrssl->Patch(pattern, handler);
        }

        inline void Delete(const std::string& pattern, httplib::Server::Handler handler) {
            svrssl == nullptr ? svr->Delete(pattern, handler) : svrssl->Delete(pattern, handler);
        }

        inline void Delete(const std::string& pattern,
            httplib::Server::HandlerWithContentReader handler) {
            svrssl == nullptr ? svr->Delete(pattern, handler) : svrssl->Delete(pattern, handler);
        }

	private:
		httplib::SSLServer* svrssl = nullptr;
		httplib::Server* svr = nullptr;
        char _host[128];
        char _cert[1024];
        char _key[1024];
};

