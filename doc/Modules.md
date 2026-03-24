# fw 模块 API 参考

> 核心框架（Context/Router/Middleware/Application）见 [API.md](API.md)，模块总表见 [Architecture.md](Architecture.md)

## 无锁并发

### LockfreeQueue\<T\>

`fw/LockfreeQueue.hpp` — MPMC 多生产者多消费者无锁队列（基于 `boost::lockfree::queue`）

| 方法 | 返回 | 说明 |
|------|------|------|
| `LockfreeQueue(capacity)` | — | 构造，指定容量 |
| `push(item)` | `bool` | 入队（const& 或 &&），满时返回 false |
| `pop(item)` | `bool` | 出队到 item，空时返回 false |
| `empty()` | `bool` | 是否为空 |
| `size()` | `size_t` | 当前元素数（近似） |
| `capacity()` | `size_t` | 容量 |
| `clear()` | `void` | 清空 |

内部跟踪 `m_pushCount`/`m_popCount`/`m_dropCount`（原子计数器）。

### SPSCQueue\<T\>

`fw/SPSCQueue.hpp` — SPSC 单生产者单消费者队列（自实现环形缓冲区，性能优于 MPMC）

接口与 `LockfreeQueue` 相同。容量自动对齐到 2 的幂次。

### AtomicCounter

`fw/AtomicCounter.hpp` — 原子计数器（`boost::atomic<int64_t>`）

| 方法 | 返回 | 说明 |
|------|------|------|
| `AtomicCounter()` | — | 初始 0 |
| `AtomicCounter(val)` | — | 指定初始值 |
| `add(delta=1)` | `int64_t` | 加 delta，返回新值 |
| `sub(delta=1)` | `int64_t` | 减 delta，返回新值 |
| `get()` | `int64_t` | 当前值 |
| `set(val)` | `void` | 设置值 |
| `reset()` | `void` | 归零 |

支持 `++`/`--`（前缀/后缀）和 `int64_t` 隐式转换。

### FlowController

`fw/FlowController.hpp` — 队列流控（防溢出 + 丢包统计）

| 方法 | 返回 | 说明 |
|------|------|------|
| `FlowController(maxQueueSize, dropOnFull=true)` | — | 构造 |
| `canPush(currentSize)` | `bool` | 是否可入队 |
| `tryPush(currentSize)` | `bool` | 尝试入队（含统计） |
| `recordDrop()` | `void` | 记录一次丢弃 |
| `getDropCount()` | `uint64_t` | 累计丢弃数 |
| `resetDropCount()` | `void` | 重置丢弃计数 |
| `setMaxQueueSize(max)` | `void` | 运行时调整上限 |
| `setDropOnFull(bool)` | `void` | 设置满时策略 |

---

## 线程池

### SupervisedThreadPool

`fw/SupervisedThreadPool.hpp` — 受监督线程池（worker 异常退出时 supervisor 自动拉起新线程替代）

| 方法 | 返回 | 说明 |
|------|------|------|
| `getInstance()` | `SupervisedThreadPool&` | 应用级单例（静态） |
| `start(numWorkers)` | `void` | 启动 N 个 worker + 1 个 supervisor |
| `setMaxQueueSize(max)` | `void` | 任务队列上限（start 前设置，0=不限制） |
| `submit(task)` | `bool` | 提交任务；false=池未启动或队列满 |
| `stop()` | `void` | 停止所有线程并 join |
| `workerCount()` | `size_t` | 配置的 worker 数 |
| `queueSize()` | `size_t` | 当前任务队列深度 |
| `isRunning()` | `bool` | 是否已启动 |

`Task = boost::function<void()>`。worker 从共享任务队列持续取任务执行。异常退出时通过 finished 队列通知 supervisor，supervisor join 旧线程后 spawn 新线程替代。

---

## 工具类

### Logger

`fw/Logger.hpp` — spdlog 封装（控制台 + 滚动文件）

| 方法 | 说明 |
|------|------|
| `getInstance()` | 单例 |
| `setLevel(level)` | 设置日志级别（TRACE/DEBUG/INFO/WARN/ERROR） |
| `setLogFile(dir, maxSize, maxFiles)` | 启用文件日志（默认 100MB/5 文件） |
| `setConsoleEnabled(bool)` | 控制台输出开关 |
| `trace/debug/info/warn/error(msg)` | 按级别记录（接受 `std::string`） |
| `tracef/debugf/infof/warnf/errorf(fmt, ...)` | printf 风格记录 |
| `flush()` | 刷新缓冲 |

宏：`LOG_TRACE`、`LOG_DEBUG`、`LOG_INFO`、`LOG_WARN`、`LOG_ERROR`

### JsonUtil

`fw/JsonUtil.hpp` — nlohmann::json 封装

| 方法 | 返回 | 说明 |
|------|------|------|
| `safeParse(body)` | `json` | 解析失败返回空对象 |
| `getString(body, key, def)` | `string` | 取字符串（数字自动转字符串） |
| `getInt(body, key, def)` | `int` | 取整数 |
| `getBool(body, key, def)` | `bool` | 取布尔（兼容 "true"/1） |
| `escape(s)` | `string` | JSON 字符串转义 |

`using json = nlohmann::json;`

### TimeUtil

`fw/TimeUtil.hpp` — 时间工具（`boost::chrono`）

| 方法 | 返回 | 说明 |
|------|------|------|
| `nowMs()` | `int64_t` | 当前时间戳（毫秒） |
| `nowISO8601(tz)` | `string` | 当前时间 ISO 8601 |
| `toISO8601(timestamp, us, tz)` | `string` | time_t → ISO 8601 |
| `durationMs(start, end)` | `int64_t` | 两时间点间隔（毫秒） |
| `getSystemTimezone()` | `string` | 系统时区 |
| `toMillisecondsString(tp)` | `string` | time_point → 毫秒字符串 |

### IdUtil

`fw/IdUtil.hpp` — ID 生成

| 方法 | 返回 | 说明 |
|------|------|------|
| `generateV4()` | `string` | UUID v4 |
| `generateTimedV4()` | `string` | `<微秒时间戳>-<UUID_v4>` |
| `generateSnowflakeId()` | `int64_t` | 雪花 ID |
| `initSnowflake(dc, worker)` | `void` | 初始化雪花生成器 |
| `isSafeId(s)` | `bool` | ID 安全性校验（≤64 字符，仅 `[a-zA-Z0-9_-]`） |

### 其他工具

| 头文件 | 类 | 说明 |
|--------|-----|------|
| `fw/HashUtil.hpp` | HashUtil | SHA256/MD5 哈希 |
| `fw/PasswordUtil.hpp` | PasswordUtil | bcrypt 密码哈希 |
| `fw/JwtUtil.hpp` | JwtUtil | JWT HMAC-SHA256 签发/验证 |
| `fw/Base64.hpp` | Base64 | Base64 编解码 |
| `fw/CertUtil.hpp` | CertUtil | SSL/TLS 证书工具 |
| `fw/PathGuard.hpp` | PathGuard | 路径安全校验（系统目录黑名单） |
| `fw/IniConfig.hpp` | IniConfig | INI 配置解析（opaque pointer）— 详见 [API.md](API.md) |
| `fw/LogConfig.hpp` | LogConfig | 日志配置 — 详见 [API.md](API.md) |
