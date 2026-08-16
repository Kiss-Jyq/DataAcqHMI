#include "serialworker.h"
#include "protocolparser.h"

#include <QSerialPortInfo>
#include <QDateTime>

SerialWorker::SerialWorker(QObject* parent) : QObject(parent) {
    // 注意：m_port / m_timer 在 open() 中创建，
    // 因为 open() 经由队列连接在"本线程"执行，确保对象亲和性正确。
}

SerialWorker::~SerialWorker() {
    close();
}

void SerialWorker::open(const QString& port, qint32 baud) {
    if (!m_port) {
        m_port = new QSerialPort(this);
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &SerialWorker::onPoll);
    }
    m_port->setPortName(port);
    m_port->setBaudRate(baud);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port->open(QIODevice::ReadWrite)) {
        emit errorOccurred("打开串口失败：" + m_port->errorString());
        return;
    }
    emit connected(QString("%1 @ %2").arg(port).arg(baud));
    if (m_polling) m_timer->start(m_interval);
}

void SerialWorker::close() {
    if (m_timer) m_timer->stop();
    if (m_port && m_port->isOpen()) m_port->close();
    emit disconnected();
}

void SerialWorker::setPolling(bool on) {
    m_polling = on;
    if (!m_port || !m_port->isOpen()) return;
    if (on) m_timer->start(m_interval);
    else    m_timer->stop();
}

void SerialWorker::setPollingInterval(int ms) {
    m_interval = qMax(50, ms);
    if (m_timer && m_timer->isActive()) m_timer->start(m_interval);
}

void SerialWorker::onPoll() {
    if (!m_port || !m_port->isOpen()) return;

    const QByteArray req =
        ProtocolParser::buildReadHoldingRequest(m_slave, m_startAddr, m_regCount);

    m_port->clear(QSerialPort::Input);
    if (m_port->write(req) != req.size()) {
        emit errorOccurred("发送请求失败：" + m_port->errorString());
        return;
    }
    if (!m_port->waitForBytesWritten(500)) {
        emit errorOccurred("等待发送超时");
        return;
    }

    // 预期响应长度：1(从站)+1(功能)+1(字节数)+N*2+2(CRC)
    const int expected = 5 + static_cast<int>(m_regCount) * 2;
    QByteArray resp;
    const qint64 deadline = QDateTime::currentMSecsSinceEpoch() + 800;
    while (QDateTime::currentMSecsSinceEpoch() < deadline) {
        if (m_port->waitForReadyRead(100)) {
            resp.append(m_port->readAll());
            if (resp.size() >= expected) break;
        }
    }

    QVector<quint16> regs;
    if (!ProtocolParser::parseHoldingRegisters(resp, regs) || regs.isEmpty()) {
        emit errorOccurred("响应解析失败或超时");
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    QVector<DataPoint> points;
    for (int i = 0; i < regs.size(); ++i) {
        const double scaled = static_cast<double>(regs.at(i)) / 10.0; // 示例：工程量缩放
        points.append(DataPoint(now, scaled, QString("Reg%1").arg(i)));
    }
    emit dataReady(points);
}
