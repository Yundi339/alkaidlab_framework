# alkaidlab_fw 架构说明

## 概述

alkaidlab_fw 是轻量级 C++11 HTTP 服务端框架抽象层，将业务逻辑与底层 HTTP 库 (libhv) 解耦。

## 模块结构

```
alkaidlab_fw/
├── include/fw/
│   ├── Application.hpp     # HTTP 服务器生命周期管理
│   ├── Base64.hpp          # Base64 编解码
│   ├── Context.hpp         # 请求/响应上下文
│   ├── HttpConstants.hpp   # HTTP 状态码/方法常量
│   ├── HvTransport.hpp     # libhv HTTP/1.1 传输实现
│   ├── IniConfig.hpp       # INI 配置解析
│   ├── LogConfig.hpp       # 日志配置
│   ├── Middleware.hpp       # 洋葱模型中间件链
│   ├── Router.hpp           # 统一路由注册
│   └── ServerTransport.hpp  # 传输层抽象接口
├── src/
│   ├── Application.cpp
│   ├── Base64.cpp
│   ├── Context.cpp
│   ├── HttpConstants.cpp
│   ├── HvTransport.cpp
│   ├── IniConfig.cpp
│   ├── LogConfig.cpp
│   ├── Middleware.cpp
│   └── Router.cpp
├── test/                    # 单元测试
├── doc/                     # 框架文档
├── build.sh                 # 独立构建脚本
└── CMakeLists.txt           # 独立构建 + 安装 + 测试
```

## 依赖

| 依赖 | 版本 | 用途 |
|------|------|------|
| libhv | ≥1.3 | HTTP 服务器核心 |
| OpenSSL | ≥1.1 | HTTPS 支持 |
| GTest | ≥1.13 | 单元测试（可选） |

## 构建

```bash
# 独立构建（推荐）
cd third_party/alkaidlab_fw
bash build.sh                    # 构建并安装到 ../alkaidlab_fw_install
bash build.sh --test             # 构建并运行测试
bash build.sh --clean            # 清空后重新构建

# 从项目根目录调用（代理脚本）
bash build-fw.sh                 # 等价于上面的 build.sh

# 手动 CMake 构建
cmake -B build \
    -DLIBHV_INCLUDE_DIR=/path/to/libhv/include \
    -DLIBHV_LIB=/path/to/libhv.a \
    -DCMAKE_INSTALL_PREFIX=../alkaidlab_fw_install
cmake --build build
cmake --install build
```

## 设计决策

### 1. Context 封装（非暴露 raw 指针）

Context 提供完整的请求/响应访问方法，handler 层无需接触 `HttpRequest*`/`HttpResponse*`。
好处：底层 HTTP 库替换时只需修改 Context 实现，业务代码零改动。

### 2. KV 存储替代 X-Internal-* Header

中间件间数据传递使用 `Context::set()`/`get()`，不污染 HTTP header。
避免内部 header 泄露到客户端的安全风险。

### 3. 洋葱模型中间件

`MiddlewareChain` 支持前置/后置逻辑，中间件可通过不调用 `next()` 中断请求链。
中间件签名统一为 `int(Context&, Next)`。

### 4. Router + libhv 桥接

`Router::bind(hv::HttpService&)` 将 fw 路由注册到 libhv 的路由系统，
自动将 libhv 的 `HttpRequest*/HttpResponse*` 包装为 `Context`。

## 版本历史

- **v1.0.0** — 初始版本，从 FileStar `src/fw/` 提取为独立库
