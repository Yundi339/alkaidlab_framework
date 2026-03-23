#include "fw/Application.hpp"
#include "fw/Context.hpp"
#include "fw/HttpConstants.hpp"
#include "fw/Router.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

namespace alkaidlab {
namespace fw {

/* ── 基础构造/配置 ── */

TEST(Application, ConstructDestruct) {
    Application app;
    // 构造和析构不应崩溃
}

TEST(Application, SetDocumentRoot) {
    Application app;
    app.setDocumentRoot("/tmp");
}

TEST(Application, SetHostAndPort) {
    Application app;
    app.setHost("127.0.0.1");
    app.setPort(19339);
}

TEST(Application, SetWorkerThreads) {
    Application app;
    app.setWorkerThreads(2);
    EXPECT_EQ(app.workerThreadsCount(), 2);
}

TEST(Application, WorkerThreadsDefaultZero) {
    Application app;
    // libhv 默认 worker_threads = 0（由系统决定）
    EXPECT_EQ(app.workerThreadsCount(), 0);
}

TEST(Application, SetWorkerThreadsMultipleTimes) {
    Application app;
    app.setWorkerThreads(4);
    EXPECT_EQ(app.workerThreadsCount(), 4);
    app.setWorkerThreads(8);
    EXPECT_EQ(app.workerThreadsCount(), 8);
}

/* ── 回调注册 ── */

TEST(Application, UseMiddleware) {
    Application app;
    app.use([](Context& c) -> int {
        c.setHeader("X-Test", "ok");
        return HttpStatus::Next;
    });
}

TEST(Application, SetErrorHandler) {
    Application app;
    app.setErrorHandler([](Context& c) -> int {
        c.json(404, "{\"error\":\"not_found\"}");
        return HttpStatus::NotFound;
    });
}

TEST(Application, SetPostprocessor) {
    Application app;
    app.setPostprocessor([](Context& c) -> int {
        return c.status();
    });
}

TEST(Application, SetHeaderHandler) {
    Application app;
    app.setHeaderHandler([](Context& c) -> int {
        return 0; // continue
    });
}

/* ── 路由挂载 ── */

TEST(Application, MountRouter) {
    Application app;
    Router router;
    router.get("/test", [](Context& c) -> int {
        c.json(200, "{\"ok\":true}");
        return 200;
    });
    app.mount(router);
}

TEST(Application, MountEmptyRouter) {
    Application app;
    Router router;
    app.mount(router);
    // 空路由挂载不应崩溃
}

TEST(Application, MountMultipleRouters) {
    Application app;
    Router r1, r2;
    r1.get("/api/a", [](Context& c) { c.json(200, "{}"); });
    r2.post("/api/b", [](Context& c) { c.json(201, "{}"); });
    app.mount(r1);
    app.mount(r2);
}

/* ── 异步调度器 ── */

TEST(Application, MakeAsyncDispatcher) {
    Application app;
    auto dispatcher = app.makeAsyncDispatcher();
    EXPECT_TRUE(static_cast<bool>(dispatcher));
}

TEST(Application, AsyncDispatcherRunsTask) {
    Application app;
    auto dispatcher = app.makeAsyncDispatcher();
    std::atomic<bool> ran(false);
    dispatcher([&ran]() { ran.store(true); });
    // 等待异步任务完成
    for (int i = 0; i < 50 && !ran.load(); ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(ran.load());
    app.cleanupAsync();
}

/* ── 监控 ── */

TEST(Application, ConnectionNumBeforeStart) {
    Application app;
    EXPECT_EQ(app.connectionNum(), 0);
}

/* ── 生命周期 ── */

TEST(Application, StartStopLifecycle) {
    Application app;
    app.setHost("127.0.0.1");
    app.setPort(39339);
    app.setWorkerThreads(1);

    Router router;
    router.get("/health", [](Context& c) -> int {
        c.json(200, "{\"ok\":true}");
        return 200;
    });
    app.mount(router);

    EXPECT_EQ(app.start(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_EQ(app.workerThreadsCount(), 1);
    // connectionNum >= 0（服务器刚启动，无外部连接）
    EXPECT_GE(app.connectionNum(), 0);

    app.stop();
}

TEST(Application, StartStopWithMiddleware) {
    Application app;
    app.setHost("127.0.0.1");
    app.setPort(39340);
    app.setWorkerThreads(1);

    app.use([](Context& c) -> int {
        c.setHeader("X-Powered-By", "fw");
        return HttpStatus::Next;
    });
    app.setPostprocessor([](Context& c) -> int {
        return c.status();
    });

    Router router;
    router.get("/ping", [](Context& c) -> int {
        c.json(200, "{\"pong\":true}");
        return 200;
    });
    app.mount(router);

    EXPECT_EQ(app.start(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    app.stop();
}

/* ── 完整配置场景 ── */

TEST(Application, FullConfigurationScenario) {
    Application app;
    app.setHost("127.0.0.1");
    app.setPort(39341);
    app.setWorkerThreads(2);
    app.setDocumentRoot("/tmp");

    app.setHeaderHandler([](Context& c) -> int {
        return 0;
    });
    app.use([](Context& c) -> int {
        return HttpStatus::Next;
    });
    app.setErrorHandler([](Context& c) -> int {
        c.json(404, "{\"error\":\"not_found\"}");
        return HttpStatus::NotFound;
    });
    app.setPostprocessor([](Context& c) -> int {
        return c.status();
    });

    Router router;
    router.get("/api/test", [](Context& c) -> int {
        c.json(200, "{\"ok\":true}");
        return 200;
    });
    router.post("/api/data", [](Context& c) -> int {
        c.json(201, "{\"created\":true}");
        return 201;
    });
    app.mount(router);

    EXPECT_EQ(app.workerThreadsCount(), 2);
}

} // namespace fw
} // namespace alkaidlab
