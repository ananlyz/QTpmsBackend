#ifndef CARSERVICE_H
#define CARSERVICE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "../models/Car.h"
#include "../dao/CarRepository.h"

class CarService : public QObject
{
    Q_OBJECT

public:
    static CarService& instance();

    // 车辆注册
    QJsonObject registerCar(const QString& plate, const QString& type, const QString& owner);
    
    // 车辆信息查询
    QJsonObject getCarInfo(const QString& plate);
    QJsonArray getAllCars();
    QJsonArray getCarsByType(const QString& type);
    
    // 车辆信息更新
    QJsonObject updateCar(const QString& plate, const QString& type, const QString& owner);
    
    // 车辆删除
    QJsonObject deleteCar(const QString& plate);
    
    // 车辆存在性检查
    bool exists(const QString& plate);
    
    // 统计信息
    QJsonObject getStatistics();

signals:
    void carRegistered(const QString& plate);
    void carUpdated(const QString& plate);
    void carDeleted(const QString& plate);

private:
    CarService() = default;
    CarService(const CarService&) = delete;
    CarService& operator=(const CarService&) = delete;
    
    QJsonObject carToJson(const Car& car);
    QJsonArray carsToJson(const QList<Car>& cars);
    bool validatePlate(const QString& plate);
    bool validateCarType(const QString& type);
};

#endif // CARSERVICE_H