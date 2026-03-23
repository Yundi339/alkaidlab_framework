#ifndef ALKAIDLAB_FW_TRANSPORT_FACTORY_HPP
#define ALKAIDLAB_FW_TRANSPORT_FACTORY_HPP

#include "fw/ITransport.hpp"
#include "fw/TransportTypes.hpp"

#include <memory>
#include <string>

namespace alkaidlab {
namespace fw {

class TransportFactory {
public:
    /**
     * 创建传输实例
     * @param type 传输类型（"http", "https", "tcp", "websocket"）
     * @return 传输指针，失败返回nullptr
     */
    static std::unique_ptr<ITransport> create(const std::string& type);

    /**
     * 创建HTTP传输
     * @return HTTP传输指针
     */
    static std::unique_ptr<ITransport> createHttp();

    /**
     * 创建HTTPS传输
     * @param sslConfig SSL配置（可选）
     * @return HTTPS传输指针
     */
    static std::unique_ptr<ITransport> createHttps(const SslConfig& sslConfig = SslConfig());

    /**
     * 创建TCP传输
     * @return TCP传输指针
     */
    static std::unique_ptr<ITransport> createTcp();

    /**
     * 创建WebSocket传输
     * @param sslConfig SSL配置（用于wss://，可选）
     * @return WebSocket传输指针
     */
    static std::unique_ptr<ITransport> createWebSocket(const SslConfig& sslConfig = SslConfig());
};

} // namespace fw
} // namespace alkaidlab

#endif
