#pragma once

#include "datapoint.h"

#include <QObject>
#include <QSerialPort>
#include <QTimer>

/**
 * @brief 串口采集工作线程（运行在独立 QThread）
 *
 * 设计原则：所有串口 IO 都在本对象所在线程执行，UI 线程通过信号拿到数据，
 * 绝不跨线程直接访问 QSerialPort。
 */
class SerialWorker : public QObject {
    Q_OBJECT

public:
    explicit SerialWorker(QObject* parent = nullptr);
    ~SerialWorker() override;

public slots:
    void open(const QString& port, qint32 baud);
    void close();
    void setPolling(bool on);
    void setPollingInterval(int ms);

signals:
    void connected(const QString& info);
    void disconnected();
    void errorOccurred(const QString& msg);
    void dataReady(const QVector<DataPoint>& points);

private slots:
    void onPoll();

private:
    QSerialPort* m_port = nullptr;
    QTimer*      m_timer = nullptr;
    bool         m_polling    = false;
    int          m_interval   = 1000;
    quint8       m_slave      = 1;
    quint16      m_startAddr  = 0;
    quint16      m_regCount   = 4;
};
