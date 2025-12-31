#include "SpaceService.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include "../dao/SpaceRepository.h"
#include "../dao/ParkingRecordRepository.h"
#include "../dao/QueueRepository.h"
#include "../services/BillingService.h"
#include <QRegularExpression>

SpaceService& SpaceService::instance()
{
    static SpaceService instance;
    return instance;
}

QJsonObject SpaceService::addSpace(const QString& location, const QString& type, double hourlyRate)
{
    try {
        if (!validateLocation(location)) {
            return ApiResponse::error("Invalid location");
        }
        
        
        if (!validateHourlyRate(hourlyRate)) {
            return ApiResponse::error("Invalid rate");
        }
        
        if (SpaceRepository::instance().existsByLocation(location)) {
            return ApiResponse::error("Location exists");
        }
        
        ParkingSpace space;
        space.setLocation(location);
        space.setType(type);
        space.setHourlyRate(hourlyRate);
        space.setStatus(ParkingSpace::AVAILABLE);
        // No setCreatedAt, assuming it's set in repository
        
        if (SpaceRepository::instance().insert(space)) {
            return ApiResponse::success("Space added", spaceToJson(space));
        } else {
            return ApiResponse::error("Failed to add space");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error adding space: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject SpaceService::updateSpace(int id, const QString& location, const QString& type, double hourlyRate)
{
    try {
        if (!SpaceRepository::instance().exists(id)) {
            return ApiResponse::error("Space not found");
        }
        
        if (!validateLocation(location)) {
            return ApiResponse::error("Invalid location");
        }
        
        if (!validateHourlyRate(hourlyRate)) {
            return ApiResponse::error("Invalid rate");
        }
        
        ParkingSpace space = SpaceRepository::instance().findById(id);
        space.setLocation(location);
        space.setType(type);
        space.setHourlyRate(hourlyRate);
        
        if (SpaceRepository::instance().update(space)) {
            return ApiResponse::success("Space updated", spaceToJson(space));
        } else {
            return ApiResponse::error("Failed to update space");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error updating space: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject SpaceService::deleteSpace(int id)
{
    try {
        if (!SpaceRepository::instance().exists(id)) {
            return ApiResponse::error("Space not found");
        }
        
        ParkingSpace space = SpaceRepository::instance().findById(id);
        if (space.getStatus() == ParkingSpace::OCCUPIED) {
            return ApiResponse::error("Space occupied");
        }
        
        if (SpaceRepository::instance().remove(id)) {
            return ApiResponse::success("Space deleted");
        } else {
            return ApiResponse::error("Failed to delete space");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error deleting space: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject SpaceService::getSpaceInfo(int id)
{
    try {
        if (!SpaceRepository::instance().exists(id)) {
            return ApiResponse::error("Space not found");
        }
        
        ParkingSpace space = SpaceRepository::instance().findById(id);
        return ApiResponse::success(spaceToJson(space));
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting space info: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonArray SpaceService::getAllSpaces()
{
    try {
        QList<ParkingSpace> spaces = SpaceRepository::instance().findAll();
        return spacesToJson(spaces);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting all spaces: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonArray SpaceService::getSpacesByStatus(const QString& status)
{
    try {
        ParkingSpace::Status spaceStatus = parseStatus(status);
        QList<ParkingSpace> spaces = SpaceRepository::instance().findByStatus(spaceStatus);
        return spacesToJson(spaces);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting spaces by status: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonArray SpaceService::getAvailableSpaces()
{
    try {
        QList<ParkingSpace> spaces = SpaceRepository::instance().findAvailableSpaces();
        return spacesToJson(spaces);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting available spaces: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonArray SpaceService::getOccupiedSpaces()
{
    try {
        QList<ParkingSpace> spaces = SpaceRepository::instance().findOccupiedSpaces();
        return spacesToJson(spaces);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting occupied spaces: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonObject SpaceService::occupySpace(int id, const QString& plate)
{
    try {
        if (!SpaceRepository::instance().exists(id)) {
            return ApiResponse::error("Space not found");
        }
        
        QRegularExpression regex("^[A-Z]{1}[A-Z0-9]{4,5}[A-Z0-9��ѧ���۰�]{1}$");
        if (!regex.match(plate.toUpper()).hasMatch()) {
            return ApiResponse::error("Invalid plate");
        }
        
        ParkingSpace space = SpaceRepository::instance().findById(id);
        if (space.getStatus() != ParkingSpace::AVAILABLE) {
            return ApiResponse::error("Space not available");
        }
        
        // 调用 BillingService 开始停车
        QJsonObject billingResult = BillingService::instance().startParking(plate, id);
        if (billingResult["code"] != 0) {
            return billingResult; // 返回计费服务的错误
        }
        
        // 重新获取车位信息
        space = SpaceRepository::instance().findById(id);
        return ApiResponse::success("Space occupied", spaceToJson(space));
    } catch (const std::exception& e) {
        Logger::error(QString("Error occupying space: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject SpaceService::releaseSpace(int id)
{
    try {
        if (!SpaceRepository::instance().exists(id)) {
            return ApiResponse::error("Space not found");
        }
        
        // 查找该车位的活跃停车记录
        QList<ParkingRecord> activeRecords = ParkingRecordRepository::instance().findActiveBySpaceId(id);
        if (activeRecords.isEmpty()) {
            return ApiResponse::error("No active parking record found for this space");
        }
        
        if (activeRecords.size() > 1) {
            Logger::warning(QString("Multiple active records for space %1, using first one").arg(id));
        }
        
        ParkingRecord record = activeRecords.first();
        
        // 调用 BillingService 结束停车
        QJsonObject billingResult = BillingService::instance().endParking(record.getId());
        if (billingResult["code"] != 0) {
            return billingResult; // 返回计费服务的错误
        }
        
        // 检查排队队列，如果有排队的车辆，自动占用该车位
        QueueItem firstInQueue = QueueRepository::instance().findFirst();
        if (firstInQueue.id != 0) {
            // 有排队的车辆，自动占用
            QString plate = firstInQueue.plate;
            if (SpaceRepository::instance().occupySpace(id, plate)) {
                // 占用成功，从队列中移除
                QueueRepository::instance().remove(plate);
                Logger::info(QString("Auto-assigned space %1 to queued vehicle %2").arg(id).arg(plate));
                
                // 重新获取车位信息
                ParkingSpace space = SpaceRepository::instance().findById(id);
                return ApiResponse::success("Space released and auto-assigned to queued vehicle", spaceToJson(space));
            } else {
                Logger::error(QString("Failed to auto-assign space %1 to queued vehicle %2").arg(id).arg(plate));
            }
        }
        
        // 重新获取车位信息
        ParkingSpace space = SpaceRepository::instance().findById(id);
        return ApiResponse::success("Space released", spaceToJson(space));
    } catch (const std::exception& e) {
        Logger::error(QString("Error releasing space: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject SpaceService::updateSpaceStatus(int id, const QString& status)
{
    try {
        if (!SpaceRepository::instance().exists(id)) {
            return ApiResponse::error("Space not found");
        }
        
        ParkingSpace::Status spaceStatus = parseStatus(status);
        ParkingSpace space = SpaceRepository::instance().findById(id);
        space.setStatus(spaceStatus);
        
        if (SpaceRepository::instance().update(space)) {
            return ApiResponse::success("Status updated", spaceToJson(space));
        } else {
            return ApiResponse::error("Failed to update status");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error updating status: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

bool SpaceService::exists(int id)
{
    return SpaceRepository::instance().exists(id);
}

bool SpaceService::existsByLocation(const QString& location)
{
    return SpaceRepository::instance().existsByLocation(location);
}

QJsonObject SpaceService::getStatistics()
{
    try {
        int total = SpaceRepository::instance().count();
        int available = SpaceRepository::instance().countByStatus(ParkingSpace::AVAILABLE);
        int occupied = SpaceRepository::instance().countByStatus(ParkingSpace::OCCUPIED);
        
        QJsonObject stats;
        stats["totalSpaces"] = total;
        stats["availableSpaces"] = available;
        stats["occupiedSpaces"] = occupied;
        stats["occupancyRate"] = total > 0 ? (double)occupied / total : 0.0;
        
        return ApiResponse::success(stats);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting statistics: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject SpaceService::getUsageStatistics()
{
    try {
        QJsonObject stats;
        stats["usageRate"] = 0.0; // TODO
        return ApiResponse::success(stats);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting usage stats: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

double SpaceService::calculateParkingFee(int spaceId, const QDateTime& startTime, const QDateTime& endTime)
{
    try {
        ParkingSpace space = SpaceRepository::instance().findById(spaceId);
        qint64 seconds = startTime.secsTo(endTime);
        double hours = seconds / 3600.0;
        if (hours < 1) hours = 1;
        return hours * space.getHourlyRate();
    } catch (const std::exception& e) {
        Logger::error(QString("Error calculating fee: %1").arg(e.what()));
        return 0.0;
    }
}

QJsonObject SpaceService::spaceToJson(const ParkingSpace& space)
{
    QJsonObject json;
    json["id"] = space.getId();
    json["location"] = space.getLocation();
    json["type"] = space.getType();
    json["hourlyRate"] = space.getHourlyRate();
    json["status"] = statusToString(space.getStatus());
    json["currentPlate"] = space.getCurrentPlate();
    json["occupiedTime"] = space.getOccupiedTime().isValid() ? space.getOccupiedTime().toString(Qt::ISODate) : "";
    // No createdAt in model
    return json;
}

QJsonArray SpaceService::spacesToJson(const QList<ParkingSpace>& spaces)
{
    QJsonArray array;
    for (const ParkingSpace& space : spaces) {
        array.append(spaceToJson(space));
    }
    return array;
}

bool SpaceService::validateLocation(const QString& location)
{
    return !location.trimmed().isEmpty() && location.length() <= 50;
}

bool SpaceService::validatePlate(const QString& plate)
{
    QRegularExpression regex("^[A-Z]{1}[A-Z0-9]{4,5}[A-Z0-9��ѧ���۰�]{1}$");
    return regex.match(plate.toUpper()).hasMatch();
}


bool SpaceService::validateHourlyRate(double rate)
{
    return rate >= 0.1 && rate <= 1000.0;
}

ParkingSpace::Status SpaceService::parseStatus(const QString& status)
{
    if (status.toLower() == "available") return ParkingSpace::AVAILABLE;
    if (status.toLower() == "occupied") return ParkingSpace::OCCUPIED;
    if (status.toLower() == "reserved") return ParkingSpace::RESERVED;
    if (status.toLower() == "maintenance") return ParkingSpace::DISABLED;
    return ParkingSpace::AVAILABLE;
}

QString SpaceService::statusToString(ParkingSpace::Status status)
{
    switch (status) {
        case ParkingSpace::AVAILABLE: return "available";
        case ParkingSpace::OCCUPIED: return "occupied";
        case ParkingSpace::RESERVED: return "reserved";
        case ParkingSpace::DISABLED: return "maintenance";
        default: return "unknown";
    }
}

QJsonObject SpaceService::joinQueue(const QString& plate)
{
    try {
        if (!SpaceService::instance().validatePlate(plate)) {
            return ApiResponse::error("Invalid plate number");
        }
        
        // 检查车辆是否已经在排队
        if (QueueRepository::instance().exists(plate)) {
            return ApiResponse::error("Vehicle already in queue");
        }
        
        // 检查车辆是否已经在停车
        QList<ParkingRecord> activeRecords = ParkingRecordRepository::instance().findActiveByPlate(plate);
        if (!activeRecords.isEmpty()) {
            return ApiResponse::error("Vehicle already parking");
        }
        
        // 检查是否有空闲车位
        QJsonArray availableSpaces = getAvailableSpaces();
        if (!availableSpaces.isEmpty()) {
            // 有空闲车位，直接占用第一个
            QJsonObject firstSpace = availableSpaces.first().toObject();
            int spaceId = firstSpace["id"].toInt();
            
            QJsonObject occupyResult = occupySpace(spaceId, plate);
            if (occupyResult["code"] == 0) {
                return ApiResponse::success("Space assigned directly", occupyResult["data"].toObject());
            } else {
                // 占用失败，加入队列
                Logger::warning(QString("Failed to occupy available space %1 for plate %2, adding to queue").arg(spaceId).arg(plate));
            }
        }
        
        // 加入排队队列
        QueueItem item(plate);
        if (QueueRepository::instance().insert(item)) {
            QJsonObject queueInfo;
            queueInfo["plate"] = plate;
            queueInfo["queueTime"] = item.queueTime.toString(Qt::ISODate);
            queueInfo["position"] = QueueRepository::instance().count();
            
            return ApiResponse::success("Added to queue", queueInfo);
        } else {
            return ApiResponse::error("Failed to join queue");
        }
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error joining queue: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}
