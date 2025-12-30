#ifndef CARREPOSITORY_H
#define CARREPOSITORY_H

#include <QObject>
#include <QString>
#include <QList>
#include <QVariantMap>
#include <QThreadStorage>
#include <QSqlDatabase>
#include "../models/Car.h"

class CarRepository
{
public:
    static CarRepository& instance();
    
    // 初始化表结构
    bool initializeTable();
    
    // CRUD操作
    bool insert(const Car& car);
    bool update(const Car& car);
    bool remove(const QString& plate);
    
    // 查询操作
    Car findByPlate(const QString& plate);
    QList<Car> findAll();
    QList<Car> findByType(const QString& type);
    
    // 存在性检查
    bool exists(const QString& plate);
    
    // 统计
    int count();
    int countByType(const QString& type);
    
private:
    CarRepository() = default;
    CarRepository(const CarRepository&) = delete;
    CarRepository& operator=(const CarRepository&) = delete;
    
    Car mapToCar(const QVariantMap& row);
    QVariantMap carToMap(const Car& car);
    
    // 辅助方法
    bool executeQuery(const QString& queryStr, const QVariantMap& params = QVariantMap());
    QList<QVariantMap> executeQueryWithResults(const QString& queryStr, const QVariantMap& params = QVariantMap());
    QSqlDatabase getDatabase();
};

#endif // CARREPOSITORY_H