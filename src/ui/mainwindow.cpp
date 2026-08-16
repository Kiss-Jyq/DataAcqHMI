#include "mainwindow.h"

#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QPainter>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

using namespace QtCharts;

MainWindow::MainWindow(SerialWorker* worker, QWidget* parent)
    : QMainWindow(parent), m_worker(worker), m_buffer(300) {
    setWindowTitle("DataAcqHMI — 工业数据采集上位机（Qt6 示例）");
    resize(900, 600);

    // ---- 串口参数 ----
    m_portCombo = new QComboBox(this);
    const auto ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty()) m_portCombo->addItem("COM1");
    for (const auto& p : ports) m_portCombo->addItem(p.portName());

    m_baudCombo = new QComboBox(this);
    for (int b : {9600, 19200, 38400, 115200}) m_baudCombo->addItem(QString::number(b));
    m_baudCombo->setCurrentText("9600");

    m_connectBtn = new QPushButton("连接", this);
    m_pollBtn    = new QPushButton("开始采集", this);
    m_pollBtn->setEnabled(false);

    auto* toolBar = new QWidget(this);
    auto* hbox = new QHBoxLayout(toolBar);
    hbox->addWidget(new QLabel("串口：", this));
    hbox->addWidget(m_portCombo);
    hbox->addWidget(new QLabel("波特率：", this));
    hbox->addWidget(m_baudCombo);
    hbox->addWidget(m_connectBtn);
    hbox->addWidget(m_pollBtn);
    hbox->addStretch(1);

    // ---- 实时曲线 ----
    m_series = new QLineSeries(this);
    m_chart = new QChart();
    m_chart->addSeries(m_series);
    m_chart->setTitle("实时采集（最近 300 点）");
    m_chart->legend()->hide();

    // 显式创建数值坐标轴（避免使用基类指针调用 setRange 编译失败）
    auto* axisX = new QValueAxis();
    auto* axisY = new QValueAxis();
    axisX->setRange(0, 300);
    axisY->setRange(0, 100);
    axisX->setTitleText("采样点");
    axisY->setTitleText("工程量值");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisX);
    m_series->attachAxis(axisY);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    m_statusLabel = new QLabel("未连接", this);
    statusBar()->addWidget(m_statusLabel);

    auto* central = new QWidget(this);
    auto* vbox = new QVBoxLayout(central);
    vbox->addWidget(toolBar);
    vbox->addWidget(m_chartView);
    setCentralWidget(central);

    // ---- 信号连接（跨线程自动队列连接）----
    connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(m_pollBtn, &QPushButton::clicked, this, &MainWindow::onTogglePolling);
    connect(m_worker, &SerialWorker::dataReady, this, &MainWindow::onDataReady);
    connect(m_worker, &SerialWorker::errorOccurred, this, &MainWindow::onWorkerError);
    connect(m_worker, &SerialWorker::connected, this, &MainWindow::onConnected);
    connect(m_worker, &SerialWorker::disconnected, this, &MainWindow::onDisconnected);
}

void MainWindow::onConnect() {
    if (m_connectBtn->text() == "连接") {
        const QString port = m_portCombo->currentText();
        const qint32 baud  = m_baudCombo->currentText().toInt();
        // 调用工作线程的 open（队列连接到其线程）
        QMetaObject::invokeMethod(m_worker, "open",
            Qt::QueuedConnection, Q_ARG(QString, port), Q_ARG(qint32, baud));
    } else {
        QMetaObject::invokeMethod(m_worker, "close", Qt::QueuedConnection);
    }
}

void MainWindow::onTogglePolling() {
    static bool polling = false;
    polling = !polling;
    QMetaObject::invokeMethod(m_worker, "setPolling",
        Qt::QueuedConnection, Q_ARG(bool, polling));
    m_pollBtn->setText(polling ? "停止采集" : "开始采集");
    m_statusLabel->setText(polling ? "采集中…" : "已停止");
}

void MainWindow::onDataReady(const QVector<DataPoint>& points) {
    m_buffer.append(points);

    const auto& data = m_buffer.points();
    m_series->clear();
    for (int i = 0; i < data.size(); ++i)
        m_series->append(i, data.at(i).value);

    if (!data.isEmpty())
        m_statusLabel->setText(QString("最新值：%1  ｜ 点数：%2")
            .arg(data.last().value, 0, 'f', 2).arg(data.size()));
}

void MainWindow::onWorkerError(const QString& msg) {
    QMessageBox::warning(this, "采集错误", msg);
}

void MainWindow::onConnected(const QString& info) {
    m_connectBtn->setText("断开");
    m_pollBtn->setEnabled(true);
    m_statusLabel->setText("已连接：" + info);
}

void MainWindow::onDisconnected() {
    m_connectBtn->setText("连接");
    m_pollBtn->setEnabled(false);
    m_pollBtn->setText("开始采集");
    m_statusLabel->setText("未连接");
}
