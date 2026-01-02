#include "QueueProcessor.h"
#include "../utils/Logger.h"
#include "../dao/QueueRepository.h"
#include "../dao/SpaceRepository.h"
#include "../dao/ParkingRecordRepository.h"
#include "../api/ApiResponse.h"
#include <QtConcurrent/QtConcurrent>
#include <QMutexLocker>
#include <QTimer>
#include <QDateTime>

QueueProcessor& QueueProcessor::instance()
{
    static QueueProcessor instance;
    return instance;
}

QJsonObject QueueProcessor::processQueueForAvailableSpaces()
{
    QMutexLocker locker(&m_mutex);
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 检查处理间隔
    if (currentTime - m_lastProcessTime < MIN_PROCESS_INTERVAL) {
        QJsonObject result;
        result["assignedCount"] = 0;
        result["message"] = "Queue processing too frequent";
        result["success"] = false;
        return result;
    }
    
    if (m_processing) {
        QJsonObject result;
        result["assignedCount"] = 0;
        result["message"] = "Queue processing already in progress";
        result["success"] = false;
        return result;
    }
    
    m_processing = true;
    m_lastProcessTime = currentTime;
    QJsonObject result;
    
    try {
        // 直接处理队列分配逻辑，避免递归调用
        result = processQueueLogic();
        int assignedCount = result["assignedCount"].toInt();
        
        if (assignedCount > 0) {
            emit queueProcessed(assignedCount);
            Logger::info(QString("Queue processor assigned %1 spaces to queued vehicles").arg(assignedCount));
        }
        
        result["success"] = true;
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in queue processor: %1").arg(e.what()));
        result["assignedCount"] = 0;
        result["message"] = "Error processing queue";
        result["success"] = false;
        result["error"] = e.what();
        emit queueProcessError(e.what());
    }
    
    m_processing = false;
    return result;
}

void QueueProcessor::checkAndProcessQueue()
{
    // 使用延迟处理，避免在高频调用时阻塞
    // 延迟100ms执行，让当前操作先完成
    QTimer::singleShot(100, [this]() {
        processQueueForAvailableSpaces();
    });
}

void QueueProcessor::forceProcessQueue()
{
    QMutexLocker locker(&m_mutex);
    
    // 强制重置处理状态
    m_processing = false;
    
    // 延迟处理，避免立即阻塞
    QTimer::singleShot(50, [this]() {
        processQueueForAvailableSpaces();
    });
}

QJsonObject QueueProcessor::processQueueLogic()
{
    QJsonObject result;
    int assignedCount = 0;
    
    try {
        // 获取所有排队车辆
        QList<QueueItem> queueItems = QueueRepository::instance().findAll();
        if (queueItems.isEmpty()) {
            result["assignedCount"] = 0;
            result["message"] = "No vehicles in queue";
            return result;
        }
        
        // 获取可用车位
        QList<ParkingSpace> availableSpaces = SpaceRepository::instance().findAvailableSpaces();
        if (availableSpaces.isEmpty()) {
            result["assignedCount"] = 0;
            result["message"] = "No available spaces";
            return result;
        }
        
        Logger::info(QString("Processing queue: %1 vehicles, %2 available spaces")
                    .arg(queueItems.size()).arg(availableSpaces.size()));
        
        // 为排队车辆分配车位，限制每次处理的数量避免阻塞
        int maxProcessBatch = 10; // 每次最多处理10辆车
        int spacesToAssign = qMin(qMin(queueItems.size(), availableSpaces.size()), maxProcessBatch);
        
        for (int i = 0; i < spacesToAssign; ++i) {
            QueueItem queueItem = queueItems[i];
            ParkingSpace space = availableSpaces[i];
            
            try {
                // 检查车辆是否还在停车
                QList<ParkingRecord> activeRecords = ParkingRecordRepository::instance().findActiveByPlate(queueItem.plate);
                if (!activeRecords.isEmpty()) {
                    Logger::info(QString("Vehicle %1 already parking, removing from queue").arg(queueItem.plate));
                    QueueRepository::instance().remove(queueItem.plate);
                    continue;
                }
                
                // 占用车位
                if (SpaceRepository::instance().occupySpace(space.getId(), queueItem.plate)) {
                    // 创建停车记录
                    ParkingRecord record;
                    record.setPlate(queueItem.plate);
                    record.setSpaceId(space.getId());
                    record.setEnterTime(QDateTime::currentDateTime());
                    record.setIsPaid(false);
                    
                    if (ParkingRecordRepository::instance().insert(record)) {
                        assignedCount++;
                        QueueRepository::instance().remove(queueItem.plate);
                        Logger::info(QString("Assigned space %1 to vehicle %2 from queue")
                                   .arg(space.getId()).arg(queueItem.plate));
                    } else {
                        // 如果创建记录失败，释放车位
                        SpaceRepository::instance().releaseSpace(space.getId());
                        Logger::warning(QString("Failed to create parking record for vehicle %1").arg(queueItem.plate));
                    }
                } else {
                    Logger::warning(QString("Failed to occupy space %1 for vehicle %2").arg(space.getId()).arg(queueItem.plate));
                }
                
            } catch (const std::exception& e) {
                Logger::error(QString("Error assigning space to vehicle %1: %2").arg(queueItem.plate).arg(e.what()));
            }
        }
        
        result["assignedCount"] = assignedCount;
        result["message"] = QString("Assigned %1 spaces to queued vehicles").arg(assignedCount);
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in queue processing logic: %1").arg(e.what()));
        result["assignedCount"] = 0;
        result["message"] = "Error processing queue";
        result["error"] = e.what();
    }
    
    return result;
}