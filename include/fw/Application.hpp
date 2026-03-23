#ifndef ALKAIDLAB_FW_APPLICATION_HPP
#define ALKAIDLAB_FW_APPLICATION_HPP

/**
 * fw::Application — HTTP server lifecycle abstraction.
 *
 * Encapsulates libhv's HttpService + HttpServer behind a clean API:
 *   - Context-based callbacks (no raw request/response pointers in user code)
 *   - Service-level middleware, header handler, postprocessor
 *   - Static file serving, SSL, IO threads
 *   - Async task dispatch
 *
 * Usage:
 *   fw::Application app;
 *   app.setDocumentRoot("web");
 *   app.setHeaderHandler([](fw::Context& c) -> int { ... });
 *   app.use([](fw::Context& c) -> int { ... });
 *   app.setErrorHandler([](fw::Context& c) -> int { ... });
 *   fw::Router router;
 *   // ... register routes ...
 *   app.mount(router);
 *   app.setHost("0.0.0.0");
 *   app.setPort(9339);
 *   app.setWorkerThreads(4);
 *   app.start();
 *   // ... wait for shutdown ...
 *   app.stop();
 */

#include <functional>
#include <string>

namespace alkaidlab {
namespace fw {

class Context;
class Router;

class Application {
public:
    Application();
    ~Application();

    /* -- Service-level callbacks (Context-based) -- */

    /** Set header handler (called after headers parsed, before body received).
     *  Return 0 to continue, negative to close connection. */
    void setHeaderHandler(std::function<int(Context&)> fn);

    /** Add a service-level middleware (executed for ALL requests, before routing).
     *  Return 0 to continue, non-zero to short-circuit. */
    void use(std::function<int(Context&)> fn);

    /** Set error handler (called for unmatched routes / 404).
     *  Return HTTP status code or fw::HttpStatus::Close. */
    void setErrorHandler(std::function<int(Context&)> fn);

    /** Set postprocessor (called after every request for cleanup/logging).
     *  Return the response status code. */
    void setPostprocessor(std::function<int(Context&)> fn);

    /* -- Static file serving -- */

    void setDocumentRoot(const std::string& path);

    /* -- Route mounting -- */

    /** Mount a Router's routes + route-level middleware into this application. */
    void mount(Router& router);

    /* -- Server configuration -- */

    void setHost(const std::string& host);
    void setPort(int port);
    void setHttpsPort(int port);
    void setWorkerThreads(int n);

    /** Enable HTTPS with cert/key PEM files. Returns false on failure. */
    bool enableSsl(const std::string& certFile, const std::string& keyFile);

    /* -- Lifecycle -- */

    /** Start the server (non-blocking). Returns 0 on success. */
    int start();

    /** Stop the server. */
    void stop();

    /* -- Monitoring -- */

    int connectionNum() const;
    int workerThreadsCount() const;

    /* -- Async support -- */

    /** Returns an async dispatcher that submits tasks to the global thread pool.
     *  Suitable for Router::setAsyncDispatcher(). */
    std::function<void(std::function<void()>)> makeAsyncDispatcher();

    /** Cleanup the global async thread pool (call during shutdown). */
    void cleanupAsync();

private:
    Application(const Application&);
    Application& operator=(const Application&);

    struct Impl;
    Impl* m_impl;
};

} // namespace fw
} // namespace alkaidlab

#endif // ALKAIDLAB_FW_APPLICATION_HPP
