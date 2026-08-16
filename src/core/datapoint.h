#pragma once

#include <QString>

/**
 * @brief 单条采集数据点（线程安全值对象）
 */
struct DataPoint {
    qint64 timestamp = 0;   // 毫秒时间戳（QDateTime::currentMSecsSinceEpoch）
    double value     = 0.0; // 数值
    QString tag;            // 点位标签，如 "Reg0" / "温度"

    DataPoint() = default;
    DataPoint(qint64 ts, double v, const QString& t)
        : timestamp(ts), value(v), tag(t) {}
};
