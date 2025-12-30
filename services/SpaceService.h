#ifndef SPACESERVICE_H
#define SPACESERVICE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "../models/ParkingSpace.h"
#include "../dao/SpaceRepository.h"

class SpaceService : public QObject
{
    Q_OBJECT

public:
    static SpaceService& instance();

    // 停车位管理
    QJsonObject addSpace(const QString& location, const QString& type, double hourlyRate);
    QJsonObject updateSpace(int id, const QString& location, const QString& type, double hourlyRate);
    QJsonObject deleteSpace(int id);
    
    // 停车位查询
    QJsonObject getSpaceInfo(int id);
    QJsonArray getAllSpaces();
    QJsonArray getSpacesByStatus(const QString& status);
    QJsonArray getAvailableSpaces();
    QJsonArray getOccupiedSpaces();
    
    // 停车位状态管理
    QJsonObject occupySpace(int id, const QString& plate);
    QJsonObject releaseSpace(int id);
    QJsonObject updateSpaceStatus(int id, const QString& status);
    
    // 停车位存在性检查
    bool exists(int id);
    bool existsByLocation(const QString& location);
    
    // 统计信息
    QJsonObject getStatistics();
    QJsonObject getUsageStatistics();
    
    // 计费相关
    double calculateParkingFee(int spaceId, const QDateTime& startTime, const QDateTime& endTime);

signals:
    void spaceAdded(int id);
    void spaceUpdated(int id);
    void spaceDeleted(int id);
    void spaceOccupied(int id, const QString& plate);
    void spaceReleased(int id);

private:
    SpaceService() = default;
    SpaceService(const SpaceService&) = delete;
    SpaceService& operator=(const SpaceService&) = delete;
    
    QJsonObject spaceToJson(const ParkingSpace& space);
    QJsonArray spacesToJson(const QList<ParkingSpace>& spaces);
    bool validateLocation(const QString& location);
    bool validateType(const QString& type);
    bool validateHourlyRate(double rate);
    ParkingSpace::Status parseStatus(const QString& status);
    QString statusToString(ParkingSpace::Status status);
};

#endif // SPACESERVICE_H