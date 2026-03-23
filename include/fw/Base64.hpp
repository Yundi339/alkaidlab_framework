#ifndef ALKAIDLAB_FW_BASE64_HPP
#define ALKAIDLAB_FW_BASE64_HPP

#include <string>
#include <vector>

namespace alkaidlab {
namespace fw {

class Base64 {
public:
    /** 编码原始字节指针 */
    static std::string encode(const unsigned char* data, size_t len);
    /** 解码 Base64 字符指针 */
    static std::string decode(const char* data, size_t len);

    /** 编码 std::string */
    static std::string encode(const std::string& data);
    /** 解码 std::string */
    static std::string decode(const std::string& encoded);
    /** 编码字节向量 */
    static std::string encode(const std::vector<unsigned char>& data);
};

} // namespace fw
} // namespace alkaidlab

#endif // ALKAIDLAB_FW_BASE64_HPP
