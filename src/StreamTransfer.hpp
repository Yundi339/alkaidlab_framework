// StreamTransfer — 事件驱动文件传输策略（onwrite 状态机）
// send() 注册 EPOLLOUT 回调后立即返回，IO 线程驱动后续分块发送。
// 零线程占用，每次 pump() <1ms（256KB 磁盘读取），IO 线程可同时服务千级连接。

#ifndef ALKAIDLAB_FW_STREAM_TRANSFER_HPP
#define ALKAIDLAB_FW_STREAM_TRANSFER_HPP

#include "fw/IFileTransfer.hpp"

namespace alkaidlab {
namespace fw {

class StreamTransfer : public IFileTransfer {
public:
    void send(Context& c, const TransferParams& params);
    const char* name() const { return "stream"; }
};

} // namespace fw
} // namespace alkaidlab

#endif
