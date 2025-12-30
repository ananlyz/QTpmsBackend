#include "CarController.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include "../utils/JsonUtil.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

CarController& CarController::instance()
{
    static CarController instance;
    return instance;
}

void CarController::registerCar(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析请求体
        qDebug() << "api HTTP Request Body Raw:" << request.bodyRaw;
        
        QJsonDocument doc = QJsonDocument::fromJson(request.bodyRaw);
        qDebug() << "api HTTP Request Body JSON:" << doc;
        Logger::info("1");
        if (!doc.isObject()) {
            Logger::info("Invalid JSON format");
            response.badRequest("Invalid JSON format");
            return;
        }
        
        QJsonObject body = doc.object();
        Logger::info("2");
        // 验证必填字段
        if (!body.contains("plate")) {
            response.badRequest("Missing required fields: plate, type, owner");
            return;
        }
        Logger::info("3");
        QString plate = body["plate"].toString().trimmed().toUpper();
        QString type = body["type"].toString().trimmed().toLower();
        QString color = body["color"].toString().trimmed();
        
        // 验证字段格式
        if (plate.isEmpty()) {
            response.badRequest("Fields cannot be empty");
            return;
        }
        
        // 调用服务层
        QJsonObject result = CarService::instance().registerCar(plate, type, color);
        Logger::info("4");
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Car registration request: plate=%1, type=%2, color=%3").arg(plate).arg(type).arg(color));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in registerCar: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::getCarInfo(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径中提取车牌号
        QString plate = extractPlateFromPath(request.path);
        if (plate.isEmpty()) {
            response.badRequest("Plate number is required");
            return;
        }
        
        // 调用服务层
        QJsonObject result = CarService::instance().getCarInfo(plate);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.notFound("Car not found");
        }
        
        Logger::info(QString("Get car info request: plate=%1").arg(plate));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getCarInfo: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::getAllCars(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonArray cars = CarService::instance().getAllCars();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = cars;
        result["count"] = cars.size();
        result["message"] = "Cars retrieved successfully";
        
        response.ok(result);
        
        Logger::info("Get all cars request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getAllCars: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::getCarsByType(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径或查询参数中提取车辆类型
        QString type = extractTypeFromPath(request.path);
        if (type.isEmpty()) {
            type = request.getQueryParam("type");
        }
        
        if (type.isEmpty()) {
            response.badRequest("Car type is required");
            return;
        }
        
        // 调用服务层
        QJsonArray cars = CarService::instance().getCarsByType(type);
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = cars;
        result["count"] = cars.size();
        result["message"] = QString("Cars of type '%1' retrieved successfully").arg(type);
        
        response.ok(result);
        
        Logger::info(QString("Get cars by type request: type=%1").arg(type));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getCarsByType: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::updateCar(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径中提取车牌号
        QString plate = extractPlateFromPath(request.path);
        if (plate.isEmpty()) {
            response.badRequest("Plate number is required");
            return;
        }
        
        // 解析请求体
        QJsonDocument doc = QJsonDocument::fromJson(request.bodyRaw);
        if (!doc.isObject()) {
            response.badRequest("Invalid JSON format");
            return;
        }
        
        QJsonObject body = doc.object();
        
        // 验证必填字段
        if (!body.contains("type") || !body.contains("owner")) {
            response.badRequest("Missing required fields: type, owner");
            return;
        }
        
        QString type = body["type"].toString().trimmed().toLower();
        QString owner = body["owner"].toString().trimmed();
        
        // 验证字段格式
        if (type.isEmpty() || owner.isEmpty()) {
            response.badRequest("Fields cannot be empty");
            return;
        }
        
        // 调用服务层
        QJsonObject result = CarService::instance().updateCar(plate, type, owner);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Update car request: plate=%1, type=%2, owner=%3").arg(plate).arg(type).arg(owner));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in updateCar: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::deleteCar(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径中提取车牌号
        QString plate = extractPlateFromPath(request.path);
        if (plate.isEmpty()) {
            response.badRequest("Plate number is required");
            return;
        }
        
        // 调用服务层
        QJsonObject result = CarService::instance().deleteCar(plate);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Delete car request: plate=%1").arg(plate));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in deleteCar: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::checkCarExists(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径中提取车牌号
        QString plate = extractPlateFromPath(request.path);
        if (plate.isEmpty()) {
            response.badRequest("Plate number is required");
            return;
        }
        
        // 调用服务层
        bool exists = CarService::instance().exists(plate);
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = QJsonObject{{"exists", exists}};
        result["message"] = exists ? "Car exists" : "Car not found";
        
        response.ok(result);
        
        Logger::info(QString("Check car exists request: plate=%1, exists=%2").arg(plate).arg(exists));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in checkCarExists: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void CarController::getStatistics(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonObject result = CarService::instance().getStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info("Get car statistics request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getStatistics: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

QString CarController::extractPlateFromPath(const QString& path)
{
    // 从路径 /api/cars/ABC123 提取车牌号
    QStringList parts = path.split('/', QString::SkipEmptyParts);
    if (parts.size() >= 3 && parts[1] == "cars") {
        return parts[2].toUpper();
    }
    return "";
}

QString CarController::extractTypeFromPath(const QString& path)
{
    // 从路径 /api/cars/type/small 提取类型
    QStringList parts = path.split('/', QString::SkipEmptyParts);
    if (parts.size() >= 4 && parts[1] == "cars" && parts[2] == "type") {
        return parts[3].toLower();
    }
    return "";
}
