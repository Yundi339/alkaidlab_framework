# alkaidlab_fw API 参考

## Context

请求/响应封装 + KV 中间件数据传递。

### 构造

| 签名 | 场景 |
|------|------|
| `Context(HttpRequest*, HttpResponse*)` | 同步 handler |
| `Context(const HttpContextPtr&)` | 异步 / 流式 handler |

### 请求读取

| 方法 | 返回 | 说明 |
|------|------|------|
| `method()` | `const char*` | HTTP 方法 ("GET", "POST", …) |
| `path()` | `const string&` | 请求路径 (e.g. "/api/fs/list") |
| `param(key)` | `string` | URL path/query 参数 |
| `header(key)` | `string` | 请求头 |
| `body()` | `const string&` | 请求体 |
| `clientIp()` | `string` | Socket 对端 IP |
| `clientPort()` | `int` | Socket 对端端口 |
| `formData(key)` | `string` | Multipart 表单字段值 |
| `contentLength()` | `int64_t` | 请求 Content-Length |
| `fullPath()` | `string` | 完整路径含 query string |

### 响应设置

| 方法 | 说明 |
|------|------|
| `setStatus(code)` | 设置 HTTP 状态码 |
| `status()` | 获取当前状态码 |
| `setHeader(key, val)` | 设置响应头 |
| `responseHeader(key)` | 读取响应头 |
| `setBody(body)` | 设置响应体 |
| `setContentType(ct)` | 设置 Content-Type |
| `serveFile(path)` | 读取文件内容到响应体 |
| `json(code, body)` | JSON 响应 (Content-Type + status + body) |
| `error(code, errCode, msg)` | JSON 错误响应 |
| `responseBodySize()` | 响应体大小 (bytes) |
| `removeHeader(key)` | 移除响应头 |

### 流式写入 (ctx_handler 场景)

| 方法 | 说明 |
|------|------|
| `hasWriter()` | 是否有 writer |
| `writeStatus(code)` | 写状态行 |
| `writeHeader(key, val)` | 写响应头 |
| `endHeaders(key, len)` | 结束头部 |
| `writeBody(data, len)` | 写响应体片段 |
| `writerConnected()` | 连接是否仍然活跃 |
| `end()` | 结束响应 |

### KV 存储 (中间件 → handler 数据传递)

| 方法 | 说明 |
|------|------|
| `set(key, val)` | 存储键值对 |
| `get(key, def)` | 读取值，不存在返回 def |
| `has(key)` | 键是否存在 |

---

## MiddlewareChain

洋葱模型中间件链。

| 方法 | 说明 |
|------|------|
| `use(fn)` | 添加匿名中间件 |
| `use(name, fn)` | 添加命名中间件 |
| `execute(ctx, finalHandler)` | 执行完整链 |

中间件签名：`int(Context&, Next)` — 调用 `next()` 进入下一层，不调用则中断链。

---

## Router

统一路由注册 + libhv 桥接。

### 路由注册

| 方法 | 说明 |
|------|------|
| `get(path, handler)` | 注册 GET 路由 |
| `post(path, handler)` | 注册 POST 路由 |
| `put(path, handler)` | 注册 PUT 路由 |
| `del(path, handler)` | 注册 DELETE 路由 |
| `patch(path, handler)` | 注册 PATCH 路由 |
| `getAsync(path, handler)` | 注册异步 GET (ctx_handler) |

Handler 签名：`void(Context&)`

### 中间件

| 方法 | 说明 |
|------|------|
| `use(name, fn)` | 全局中间件 |

### libhv 桥接

| 方法 | 说明 |
|------|------|
| `bind(hv::HttpService&)` | 绑定到 libhv 路由系统 |
| `setAsyncDispatcher(fn)` | 设置异步任务派发器 |

---

## ServerTransport / HvServerTransport

传输层抽象。`HvServerTransport` 封装 libhv 的 `hv::HttpServer`。

| 方法 | 说明 |
|------|------|
| `start(config)` | 启动 HTTP 服务 |
| `stop()` | 停止服务 |
| `isRunning()` | 是否运行中 |

---

## 设计原则

1. **C++11 兼容** — 不使用 C++14/17 特性
2. **零业务耦合** — 仅封装 HTTP 协议抽象，不包含业务逻辑
3. **Handler 层零 raw 指针** — 通过 Context 方法访问所有请求/响应数据
4. **KV 替代 X-Internal-*** — 中间件间数据传递使用 `set()`/`get()`，不污染 HTTP header
5. **libhv 隔离** — 公开头文件零 libhv 类型，上游可替换

---

## Application

HTTP 服务器生命周期管理。封装 libhv 的 HttpService + HttpServer，提供纯 Context 回调。

### 回调注册

| 方法 | 说明 |
|------|------|
| `setHeaderHandler(fn)` | 设置 header 预处理回调；返回 0 放行，负数关闭连接 |
| `use(fn)` | 添加全局中间件（在路由匹配前执行）；返回 0 放行 |
| `setErrorHandler(fn)` | 设置 404 / 未匹配路由处理 |
| `setPostprocessor(fn)` | 设置请求后处理回调 |

所有回调签名：`std::function<int(Context&)>`

### 路由 & 静态文件

| 方法 | 说明 |
|------|------|
| `setDocumentRoot(path)` | 设置静态文件根目录 |
| `mount(Router&)` | 挂载路由到当前应用 |

### 服务器配置

| 方法 | 说明 |
|------|------|
| `setHost(host)` | 监听地址 |
| `setPort(port)` | HTTP 端口 |
| `setHttpsPort(port)` | HTTPS 端口 |
| `setWorkerThreads(n)` | IO 线程数 |
| `enableSsl(cert, key)` | 启用 HTTPS；返回 false 表示失败 |

### 生命周期

| 方法 | 说明 |
|------|------|
| `start()` | 启动服务（非阻塞）；返回 0 成功 |
| `stop()` | 停止服务 |
| `cleanupAsync()` | 清理全局异步线程池 |

### 监控

| 方法 | 说明 |
|------|------|
| `connectionNum()` | 当前活跃连接数 |
| `workerThreadsCount()` | 工作线程数 |

### 异步

| 方法 | 说明 |
|------|------|
| `makeAsyncDispatcher()` | 返回异步任务派发器，用于 `Router::setAsyncDispatcher()` |

---

## HttpConstants

HTTP 状态码和方法的命名空间常量。值与 libhv 枚举保持一致（通过 `static_assert` 编译期校验）。

### HttpStatus

| 常量 | 值 | 说明 |
|------|-----|------|
| `Next` | 0 | 中间件放行 |
| `Close` | -100 | 关闭连接，不发送 HTTP 响应 |
| `Ok` | 200 | |
| `NoContent` | 204 | |
| `BadRequest` | 400 | |
| `Unauthorized` | 401 | |
| `Forbidden` | 403 | |
| `NotFound` | 404 | |
| `TooManyRequests` | 429 | |
| `InternalError` | 500 | |

### HttpMethod

| 常量 | 值 |
|------|-----|
| `DELETE_` | 0 |
| `GET` | 1 |
| `HEAD` | 2 |
| `POST` | 3 |
| `PUT` | 4 |
| `CONNECT` | 5 |
| `OPTIONS` | 6 |
| `TRACE` | 7 |
| `PATCH` | 28 |

---

## IniConfig

INI 配置文件解析，opaque pointer 隐藏底层实现。

### 加载 / 保存

| 方法 | 说明 |
|------|------|
| `loadFromFile(path)` | 从文件加载；返回 true 成功 |
| `loadFromMem(content)` | 从内存字符串加载 |
| `save()` | 保存到原文件 |
| `saveAs(path)` | 保存到指定文件 |
| `unload()` | 卸载 / 清空 |

### 读取

| 方法 | 说明 |
|------|------|
| `getString(key, section)` | 获取字符串值 |
| `getInt(key, section, def)` | 获取整数值 |
| `getBool(key, section, def)` | 获取布尔值 |
| `getFloat(key, section, def)` | 获取浮点值 |

### 写入

| 方法 | 说明 |
|------|------|
| `setString(key, val, section)` | 设置字符串值 |
| `setInt(key, val, section)` | 设置整数值 |
| `setBool(key, val, section)` | 设置布尔值 |
| `setFloat(key, val, section)` | 设置浮点值 |

### 枚举

| 方法 | 说明 |
|------|------|
| `sections()` | 返回所有 section 名称 |
| `keys(section)` | 返回指定 section 下所有键名 |

---

## Base64

Base64 编解码。

| 方法 | 说明 |
|------|------|
| `encode(data, len)` | 编码 `unsigned char*` → `std::string` |
| `decode(data, len)` | 解码 `const char*` → `std::string` |

注意：`len=0` 时行为未定义（上游 libhv 约束）。

---

## LogConfig

日志配置。

| 方法 | 说明 |
|------|------|
| `setFile(filepath)` | 设置日志文件路径 |
| `setMaxFileSize(bytes)` | 设置单文件最大大小 |
| `setRemainDays(days)` | 设置日志保留天数 |
| `setLevel(level)` | 设置日志级别 |

### Level 枚举

| 值 | 名称 |
|----|------|
| 0 | `kDebug` |
| 1 | `kInfo` |
| 2 | `kWarn` |
| 3 | `kError` |
