#pragma once

#include <QByteArray>
#include <QVector>

/**
 * @brief Modbus RTU 协议解析（纯函数，便于单元测试、零依赖）
 *
 * 只实现上位机最常用的"读保持寄存器 (Function 0x03)"请求构造与响应解析。
 */
namespace ProtocolParser {

/// Modbus RTU CRC16（多项式 0x8005，初始 0xFFFF，低位在前）
quint16 crc16(const QByteArray& data);

/// 构造读保持寄存器请求帧：slave 从站地址，addr 起始寄存器，count 寄存器数量
QByteArray buildReadHoldingRequest(quint8 slave, quint16 addr, quint16 count);

/**
 * @brief 解析响应帧，提取寄存器值（大端）
 * @return 成功返回 true，out 填入 count 个 quint16
 */
bool parseHoldingRegisters(const QByteArray& frame, QVector<quint16>& out);

} // namespace ProtocolParser
