#include <memory>

#include "mbedtls/net_sockets.h"

#include "ssgx_http_t_u.h"

#include "../../../common/auxilliary.h"
#include "../../share/ObjectRegistry.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/URI.h"
#include "RequestHandler.h"

using namespace ssgx::http_u;

extern "C" int ssgx_ocall_http_create_listener(uint64_t sgx_eid, const char* url, uint64_t timeout_seconds,
                                               uint64_t max_queued, uint64_t max_threads) {
    if (!IsNonEmptyString(url)) {
        return -1;
    }

    std::string server_id = std::to_string(sgx_eid) + "_" + url;

    // Destroy the listener with the same name.
    if (ssgx::internal::ObjectRegistry<std::string, HTTPServer>::Contains(server_id)) {
        auto last_listener = ssgx::internal::ObjectRegistry<std::string, HTTPServer>::Query(server_id);
        last_listener->stop();
        ssgx::internal::ObjectRegistry<std::string, HTTPServer>::Unregister(server_id);
    }

    // Set server parameters
    Poco::AutoPtr<HTTPServerParams> params = new HTTPServerParams();
    params->setMaxThreads(max_threads);
    params->setMaxQueued(static_cast<int>(max_queued));
    params->setTimeout(Poco::Timespan(static_cast<long>(timeout_seconds), 0));

    // Create a new server listener
    int ret = 0;
    HTTPServer* the_server = nullptr;
    try {
        Poco::URI uri(url);
        const std::string& host = uri.getHost();
        Poco::UInt16 port = uri.getPort();
        Poco::Net::SocketAddress svs(host, port);
        the_server = new HTTPServer(new RequestHandlerFactory(sgx_eid, server_id), svs, params);
        the_server->start();
        ssgx::internal::ObjectRegistry<std::string, HTTPServer>::Register(server_id, the_server, true);
        ret = 0;
    } catch (const Poco::Exception& e) {
        ret = -2;
    } catch (const std::exception& e) {
        ret = -3;
    } catch (...) {
        ret = -4;
    }

    if (ret < 0) {
        delete the_server;
        the_server = nullptr;
    }
    return ret;
}

extern "C" void ssgx_ocall_http_release_listener(const char* server_id) {
    if (!IsNonEmptyString(server_id)) {
        return;
    }

    try {
        if (auto the_server = ssgx::internal::ObjectRegistry<std::string, HTTPServer>::Query(server_id); !the_server) {
            the_server->stop();
        }
    } catch (const Poco::Exception& e) {
        // Encounter a Poco exception, ignore it silently.
    } catch (const std::exception& e) {
        // Encounter a normal exception, ignore it silently.
    } catch (...) {
        // Silently ignore other exceptions
    }
    ssgx::internal::ObjectRegistry<std::string, HTTPServer>::Unregister(server_id);
}
