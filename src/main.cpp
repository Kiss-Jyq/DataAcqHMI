#include "serialworker.h"
#include "mainwindow.h"

#include <QApplication>
#include <QThread>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("DataAcqHMI");
    QApplication::setOrganizationName("qiuyoujun");

    // 采集工作线程：SerialWorker 在其中运行，UI 永不阻塞
    QThread workerThread;
    SerialWorker worker;
    worker.moveToThread(&workerThread);
    QObject::connect(&workerThread, &QThread::finished, &worker, &SerialWorker::deleteLater);

    MainWindow w(&worker);
    w.show();

    workerThread.start();

    const int rc = app.exec();

    workerThread.quit();
    workerThread.wait();
    return rc;
}
