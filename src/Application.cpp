#include "fw/Application.hpp"
#include "fw/Context.hpp"
#include "fw/Router.hpp"
#include <hv/HttpServer.h>
#include <hv/HttpService.h>
#include <hv/hssl.h>
#include <hv/hasync.h>
#include <cstring>

namespace alkaidlab {
namespace fw {

struct Application::Impl {
    hv::HttpService service;
    hv::HttpServer server;
    int httpsPort;

    Impl() : httpsPort(0) {}
};

Application::Application() : m_impl(new Impl) {}

Application::~Application() {
    delete m_impl;
}

void Application::setHeaderHandler(std::function<int(Context&)> fn) {
    m_impl->service.headerHandler = http_sync_handler(
        [fn](HttpRequest* req, HttpResponse* resp) -> int {
            Context c(static_cast<void*>(req), static_cast<void*>(resp));
            return fn(c);
        });
}

void Application::use(std::function<int(Context&)> fn) {
    m_impl->service.Use(
        [fn](HttpRequest* req, HttpResponse* resp) -> int {
            Context c(static_cast<void*>(req), static_cast<void*>(resp));
            return fn(c);
        });
}

void Application::setErrorHandler(std::function<int(Context&)> fn) {
    m_impl->service.errorHandler =
        [fn](HttpRequest* req, HttpResponse* resp) -> int {
            Context c(static_cast<void*>(req), static_cast<void*>(resp));
            return fn(c);
        };
}

void Application::setPostprocessor(std::function<int(Context&)> fn) {
    m_impl->service.postprocessor =
        [fn](HttpRequest* req, HttpResponse* resp) -> int {
            Context c(static_cast<void*>(req), static_cast<void*>(resp));
            return fn(c);
        };
}

void Application::setDocumentRoot(const std::string& path) {
    m_impl->service.document_root = path;
}

void Application::mount(Router& router) {
    router.bind(m_impl->service);
}

void Application::setHost(const std::string& host) {
    strncpy(m_impl->server.host, host.c_str(), sizeof(m_impl->server.host) - 1);
    m_impl->server.host[sizeof(m_impl->server.host) - 1] = '\0';
}

void Application::setPort(int port) {
    m_impl->server.port = port;
}

void Application::setHttpsPort(int port) {
    m_impl->httpsPort = port;
}

void Application::setWorkerThreads(int n) {
    m_impl->server.setThreadNum(n);
}

bool Application::enableSsl(const std::string& certFile, const std::string& keyFile) {
    m_impl->server.port = 0;
    m_impl->server.https_port = m_impl->httpsPort > 0
        ? m_impl->httpsPort
        : m_impl->server.port;
    hssl_ctx_opt_t sslOpt;
    std::memset(&sslOpt, 0, sizeof(sslOpt));
    sslOpt.crt_file = certFile.c_str();
    sslOpt.key_file = keyFile.c_str();
    sslOpt.endpoint = 0; /* HSSL_SERVER */
    return m_impl->server.newSslCtx(&sslOpt) == 0;
}

int Application::start() {
    m_impl->server.service = &m_impl->service;
    return m_impl->server.start();
}

void Application::stop() {
    m_impl->server.stop();
}

int Application::connectionNum() const {
    return static_cast<int>(m_impl->server.connectionNum());
}

int Application::workerThreadsCount() const {
    return m_impl->server.worker_threads;
}

std::function<void(std::function<void()>)> Application::makeAsyncDispatcher() {
    return [](std::function<void()> task) {
        hv::async(task);
    };
}

void Application::cleanupAsync() {
    hv::async::cleanup();
}

} // namespace fw
} // namespace alkaidlab
