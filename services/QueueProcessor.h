#ifndef QUEUEPROCESSOR_H
#define QUEUEPROCESSOR_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>

class QueueProcessor : public QObject
{
    Q_OBJECT

public:
    static QueueProcessor& instance();

    // 处理排队队列，为排队车辆分配可用车位
    QJsonObject processQueueForAvailableSpaces();
    
    // 检查并处理队列（线程安全）
    void checkAndProcessQueue();
    
    // 强制处理队列（重置状态并立即处理）
    void forceProcessQueue();

signals:
    void queueProcessed(int assignedCount);
    void queueProcessError(const QString& error);

private:
    QueueProcessor() = default;
    QueueProcessor(const QueueProcessor&) = delete;
    QueueProcessor& operator=(const QueueProcessor&) = delete;
    
    // 实际的队列处理逻辑
    QJsonObject processQueueLogic();
    
    bool m_processing = false;
    QMutex m_mutex;
    qint64 m_lastProcessTime = 0;  // 上次处理时间
    static const int MIN_PROCESS_INTERVAL = 500; // 最小处理间隔（毫秒）
};

#endif // QUEUEPROCESSOR_H