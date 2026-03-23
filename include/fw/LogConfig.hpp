#ifndef ALKAIDLAB_FW_LOG_CONFIG_HPP
#define ALKAIDLAB_FW_LOG_CONFIG_HPP

#include <cstddef>
#include <string>

namespace alkaidlab {
namespace fw {

class LogConfig {
public:
    static void setFile(const char* filepath);
    static void setMaxFileSize(size_t bytes);
    static void setRemainDays(int days);
    static void setLevel(int level);

    enum Level { kDebug = 0, kInfo = 1, kWarn = 2, kError = 3 };

    /** 一次性初始化日志：确保目录存在 + 设置文件/大小/保留天数/级别。
     *  重复调用安全（仅首次生效）。 */
    static bool initialize(const std::string& logDir = "log",
                           size_t maxFileSize = 10ULL * 1024 * 1024,
                           int remainDays = 7);

    /** 确保目录存在（递归创建），可用于任意路径。 */
    static bool ensureDirectory(const std::string& dir);
};

} // namespace fw
} // namespace alkaidlab

#endif // ALKAIDLAB_FW_LOG_CONFIG_HPP
