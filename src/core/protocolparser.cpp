#include "protocolparser.h"

namespace ProtocolParser {

quint16 crc16(const QByteArray& data) {
    quint16 crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<quint8>(c);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;   // 0x8005 按位反转
            } else {
                crc >>= 1;
            }
        }
    }
    return crc; // 返回小端顺序（低字节在前）
}

QByteArray buildReadHoldingRequest(quint8 slave, quint16 addr, quint16 count) {
    QByteArray frame;
    frame.append(static_cast<char>(slave));
    frame.append(static_cast<char>(0x03));          // 功能码：读保持寄存器
    frame.append(static_cast<char>((addr >> 8) & 0xFF));
    frame.append(static_cast<char>(addr & 0xFF));
    frame.append(static_cast<char>((count >> 8) & 0xFF));
    frame.append(static_cast<char>(count & 0xFF));

    const quint16 crc = crc16(frame);
    frame.append(static_cast<char>(crc & 0xFF));     // CRC 低字节
    frame.append(static_cast<char>((crc >> 8) & 0xFF)); // CRC 高字节
    return frame;
}

bool parseHoldingRegisters(const QByteArray& frame, QVector<quint16>& out) {
    out.clear();
    if (frame.size() < 5) return false;

    const quint8 slave = static_cast<quint8>(frame.at(0));
    const quint8 func  = static_cast<quint8>(frame.at(1));
    if (func != 0x03) return false; // 异常码或功能不匹配
    Q_UNUSED(slave);

    const quint8 byteCount = static_cast<quint8>(frame.at(2));
    if (frame.size() != 3 + byteCount + 2) return false; // 头(3) + 数据 + CRC(2)

    // 校验 CRC
    const QByteArray payload = frame.left(frame.size() - 2);
    const quint16 crcRecv = static_cast<quint8>(frame.at(frame.size() - 2))
                          | (static_cast<quint8>(frame.at(frame.size() - 1)) << 8);
    if (crc16(payload) != crcRecv) return false;

    const int regCount = byteCount / 2;
    for (int i = 0; i < regCount; ++i) {
        const int base = 3 + i * 2;
        const quint16 hi = static_cast<quint8>(frame.at(base));
        const quint16 lo = static_cast<quint8>(frame.at(base + 1));
        out.append(static_cast<quint16>((hi << 8) | lo));
    }
    return true;
}

} // namespace ProtocolParser
