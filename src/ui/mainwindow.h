#pragma once

#include "databuffer.h"
#include "serialworker.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QLabel;
class QLineSeries;
class QChart;
class QChartView;
QT_END_NAMESPACE

/**
 * @brief 主界面（UI 线程）
 *
 * 负责：串口参数选择、启停采集、实时曲线绘制。
 * 不直接碰串口 —— 数据来自 SerialWorker 的 dataReady 信号（队列连接，跨线程安全）。
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(SerialWorker* worker, QWidget* parent = nullptr);

private slots:
    void onConnect();
    void onTogglePolling();
    void onDataReady(const QVector<DataPoint>& points);
    void onWorkerError(const QString& msg);
    void onConnected(const QString& info);
    void onDisconnected();

private:
    SerialWorker* m_worker;
    DataBuffer    m_buffer;

    QComboBox*  m_portCombo;
    QComboBox*  m_baudCombo;
    QPushButton* m_connectBtn;
    QPushButton* m_pollBtn;
    QLabel*     m_statusLabel;

    QLineSeries* m_series;
    QChart*      m_chart;
    QChartView*  m_chartView;
};
