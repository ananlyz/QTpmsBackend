#ifndef SPACEREPOSITORY_H
#define SPACEREPOSITORY_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QThreadStorage>
#include <QSqlDatabase>
#include "../models/ParkingSpace.h"

class SpaceRepository
{
public:
    static SpaceRepository& instance();
    
    // 初始化表结构
    bool initializeTable();
    
    // CRUD操作
    bool insert(const ParkingSpace& space);
    bool update(const ParkingSpace& space);
    bool remove(int id);
    
    // 查询操作
    ParkingSpace findById(int id);
    QList<ParkingSpace> findAll();
    QList<ParkingSpace> findByStatus(ParkingSpace::Status status);
    QList<ParkingSpace> findAvailableSpaces();
    QList<ParkingSpace> findOccupiedSpaces();
    
    // 更新状态
    bool updateStatus(int id, ParkingSpace::Status status);
    bool occupySpace(int id, const QString& plate);
    bool releaseSpace(int id);
    
    // 存在性检查
    bool exists(int id);
    bool existsByLocation(const QString& location);
    
    // 统计
    int count();
    int countByStatus(ParkingSpace::Status status);
    int countAvailable();
    int countOccupied();
    
private:
    SpaceRepository() = default;
    SpaceRepository(const SpaceRepository&) = delete;
    SpaceRepository& operator=(const SpaceRepository&) = delete;
    
    ParkingSpace mapToSpace(const QVariantMap& row);
    
    // 辅助方法
    bool executeQuery(const QString& queryStr, const QVariantMap& params = QVariantMap());
    QList<QVariantMap> executeQueryWithResults(const QString& queryStr, const QVariantMap& params = QVariantMap());
    QSqlDatabase getDatabase();
    QVariantMap spaceToMap(const ParkingSpace& space);
};

#endif // SPACEREPOSITORY_H