#include "CarService.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include "../dao/CarRepository.h"
#include <QRegularExpression>

CarService& CarService::instance()
{
    static CarService instance;
    return instance;
}

QJsonObject CarService::registerCar(const QString& plate, const QString& type, const QString& color)
{
    try {
        if (!validatePlate(plate)) {
            return ApiResponse::error("Invalid plate number");
        }
        
        //if (!validateCarType(type)) {
          //  return ApiResponse::error("Invalid car type");
       // }
        
        
        
        Car car;
        car.setPlate(plate);
        car.setColor(color);
        //car.setType(type);
        car.setCreateTime(QDateTime::currentDateTime());
        
        if (CarRepository::instance().insert(car)) {
            return ApiResponse::success("Car registered", carToJson(car));
        } else {
            return ApiResponse::error("Failed to register car");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error registering car: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject CarService::getCarInfo(const QString& plate)
{
    try {
        if (!CarRepository::instance().exists(plate)) {
            return ApiResponse::error("Car not found");
        }
        
        Car car = CarRepository::instance().findByPlate(plate);
        return ApiResponse::success(carToJson(car));
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting car info: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonArray CarService::getAllCars()
{
    try {
        QList<Car> cars = CarRepository::instance().findAll();
        return carsToJson(cars);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting all cars: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonArray CarService::getCarsByType(const QString& type)
{
    try {
        if (!validateCarType(type)) {
            return QJsonArray();
        }
        
        QList<Car> cars = CarRepository::instance().findByType(type);
        return carsToJson(cars);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting cars by type: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonObject CarService::updateCar(const QString& plate, const QString& type, const QString& owner)
{
    try {
        if (!CarRepository::instance().exists(plate)) {
            return ApiResponse::error("Car not found");
        }
        
        if (!validateCarType(type)) {
            return ApiResponse::error("Invalid car type");
        }
        
        Car car = CarRepository::instance().findByPlate(plate);
        car.setType(type);
        car.setColor(owner); // Assuming owner maps to color
        car.setUpdateTime(QDateTime::currentDateTime());
        
        if (CarRepository::instance().update(car)) {
            return ApiResponse::success("Car updated", carToJson(car));
        } else {
            return ApiResponse::error("Failed to update car");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error updating car: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject CarService::deleteCar(const QString& plate)
{
    try {
        if (!CarRepository::instance().exists(plate)) {
            return ApiResponse::error("Car not found");
        }
        
        if (CarRepository::instance().remove(plate)) {
            return ApiResponse::success("Car deleted");
        } else {
            return ApiResponse::error("Failed to delete car");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error deleting car: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

bool CarService::exists(const QString& plate)
{
    return CarRepository::instance().exists(plate);
}

QJsonObject CarService::getStatistics()
{
    try {
        int totalCars = CarRepository::instance().count();
        
        QJsonObject stats;
        stats["totalCars"] = totalCars;
        
        QStringList types = {"small", "medium", "large", "suv", "truck"};
        QJsonObject typeStats;
        for (const QString& type : types) {
            int count = CarRepository::instance().countByType(type);
            if (count > 0) {
                typeStats[type] = count;
            }
        }
        stats["byType"] = typeStats;
        
        return ApiResponse::success(stats);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting car statistics: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject CarService::carToJson(const Car& car)
{
    QJsonObject json;
    json["plate"] = car.getPlate();
    json["type"] = car.getType();
    json["color"] = car.getColor(); // Assuming owner maps to color
    json["createdAt"] = car.getCreateTime().toString(Qt::ISODate);
    return json;
}

QJsonArray CarService::carsToJson(const QList<Car>& cars)
{
    QJsonArray array;
    for (const Car& car : cars) {
        array.append(carToJson(car));
    }
    return array;
}

bool CarService::validatePlate(const QString& plate)
{
    QRegularExpression regex("^[A-Z]{1}[A-Z0-9]{4,5}[A-Z0-9��ѧ���۰�]{1}$");
    return regex.match(plate.toUpper()).hasMatch();
}

bool CarService::validateCarType(const QString& type)
{
    QStringList validTypes = {"small", "medium", "large", "suv", "truck", "van", "bus"};
    return validTypes.contains(type.toLower());
}
