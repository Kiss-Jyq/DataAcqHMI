#pragma once

#include "datapoint.h"

#include <QVector>

/**
 * @brief 环形缓冲（固定容量，超出后覆盖最旧数据）
 *
 * 高频采集时避免内存无限增长；UI 线程按窗口大小取最近 N 点绘图。
 */
class DataBuffer {
public:
    explicit DataBuffer(int capacity = 300) : m_capacity(qMax(1, capacity)) {}

    void append(const QVector<DataPoint>& points) {
        for (const auto& p : points) {
            if (m_data.size() >= m_capacity) m_data.removeFirst();
            m_data.append(p);
        }
    }

    [[nodiscard]] QVector<DataPoint> points() const { return m_data; }
    [[nodiscard]] int size() const { return m_data.size(); }
    void clear() { m_data.clear(); }

private:
    QVector<DataPoint> m_data;
    int m_capacity;
};
