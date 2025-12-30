#include "SpaceController.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include "../utils/JsonUtil.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SpaceController& SpaceController::instance()
{
    static SpaceController instance;
    return instance;
}

void SpaceController::addSpace(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析请求体
        QJsonDocument doc = QJsonDocument::fromJson(request.bodyRaw);
        if (!doc.isObject()) {
            response.badRequest("Invalid JSON format");
            return;
        }
        
        QJsonObject body = doc.object();
        
        // 验证必填字段
        if (!body.contains("location") || !body.contains("type") || !body.contains("hourlyRate")) {
            response.badRequest("Missing required fields: location, type, hourlyRate");
            return;
        }
        
        QString location = body["location"].toString().trimmed();
        QString type = body["type"].toString().trimmed().toLower();
        double hourlyRate = body["hourlyRate"].toDouble();
        
        // 验证字段格式
        if (location.isEmpty() || type.isEmpty() || hourlyRate <= 0) {
            response.badRequest("Invalid field values");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().addSpace(location, type, hourlyRate);
        
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Add space request: location=%1, type=%2, rate=%3").arg(location).arg(type).arg(hourlyRate));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in addSpace: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::updateSpace(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径参数中提取停车位ID
        QString spaceIdStr = request.getPathParam("id");
        if (spaceIdStr.isEmpty()) {
            response.badRequest("Missing space ID in path");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
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
        if (!body.contains("location") || !body.contains("type") || !body.contains("hourlyRate")) {
            response.badRequest("Missing required fields: location, type, hourlyRate");
            return;
        }
        
        QString location = body["location"].toString().trimmed();
        QString type = body["type"].toString().trimmed().toLower();
        double hourlyRate = body["hourlyRate"].toDouble();
        
        // 验证字段格式
        if (location.isEmpty() || type.isEmpty() || hourlyRate <= 0) {
            response.badRequest("Invalid field values");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().updateSpace(spaceId, location, type, hourlyRate);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Update space request: id=%1, location=%2, type=%3, rate=%4").arg(spaceId).arg(location).arg(type).arg(hourlyRate));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in updateSpace: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::deleteSpace(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径参数中提取停车位ID
        QString spaceIdStr = request.getPathParam("id");
        if (spaceIdStr.isEmpty()) {
            response.badRequest("Missing space ID in path");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().deleteSpace(spaceId);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Delete space request: id=%1").arg(spaceId));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in deleteSpace: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getSpaceInfo(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径参数中提取停车位ID
        QString spaceIdStr = request.getPathParam("id");
        if (spaceIdStr.isEmpty()) {
            response.badRequest("Missing space ID in path");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().getSpaceInfo(spaceId);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.notFound("Space not found");
        }
        
        Logger::info(QString("Get space info request: id=%1").arg(spaceId));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getSpaceInfo: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getAllSpaces(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonArray spaces = SpaceService::instance().getAllSpaces();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = spaces;
        result["count"] = spaces.size();
        result["message"] = "Spaces retrieved successfully";
        
        response.ok(result);
        
        Logger::info("Get all spaces request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getAllSpaces: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getSpacesByStatus(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径或查询参数中提取状态
        QString status = extractStatusFromPath(request.path);
        if (status.isEmpty()) {
            status = request.getQueryParam("status");
        }
        
        if (status.isEmpty()) {
            response.badRequest("Status is required");
            return;
        }
        
        // 调用服务层
        QJsonArray spaces = SpaceService::instance().getSpacesByStatus(status);
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = spaces;
        result["count"] = spaces.size();
        result["message"] = QString("Spaces with status '%1' retrieved successfully").arg(status);
        
        response.ok(result);
        
        Logger::info(QString("Get spaces by status request: status=%1").arg(status));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getSpacesByStatus: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getAvailableSpaces(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonArray spaces = SpaceService::instance().getAvailableSpaces();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = spaces;
        result["count"] = spaces.size();
        result["message"] = "Available spaces retrieved successfully";
        
        response.ok(result);
        
        Logger::info("Get available spaces request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getAvailableSpaces: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getOccupiedSpaces(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonArray spaces = SpaceService::instance().getOccupiedSpaces();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = spaces;
        result["count"] = spaces.size();
        result["message"] = "Occupied spaces retrieved successfully";
        
        response.ok(result);
        
        Logger::info("Get occupied spaces request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getOccupiedSpaces: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::occupySpace(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径参数中提取停车位ID
        QString spaceIdStr = request.getPathParam("id");
        if (spaceIdStr.isEmpty()) {
            response.badRequest("Missing space ID in path");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
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
        if (!body.contains("plate")) {
            response.badRequest("Missing required field: plate");
            return;
        }
        
        QString plate = body["plate"].toString().trimmed().toUpper();
        
        // 验证车牌号格式
        if (plate.isEmpty()) {
            response.badRequest("Plate number cannot be empty");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().occupySpace(spaceId, plate);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Occupy space request: spaceId=%1, plate=%2").arg(spaceId).arg(plate));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in occupySpace: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::releaseSpace(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径参数中提取停车位ID
        QString spaceIdStr = request.getPathParam("id");
        if (spaceIdStr.isEmpty()) {
            response.badRequest("Missing space ID in path");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().releaseSpace(spaceId);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Release space request: spaceId=%1").arg(spaceId));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in releaseSpace: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::updateSpaceStatus(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从路径参数中提取停车位ID
        QString spaceIdStr = request.getPathParam("id");
        if (spaceIdStr.isEmpty()) {
            response.badRequest("Missing space ID in path");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
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
        if (!body.contains("status")) {
            response.badRequest("Missing required field: status");
            return;
        }
        
        QString status = body["status"].toString().trimmed().toLower();
        
        // 验证状态值
        QStringList validStatuses = {"available", "occupied", "reserved", "maintenance"};
        if (!validStatuses.contains(status)) {
            response.badRequest("Invalid status value");
            return;
        }
        
        // 调用服务层
        QJsonObject result = SpaceService::instance().updateSpaceStatus(spaceId, status);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info(QString("Update space status request: spaceId=%1, status=%2").arg(spaceId).arg(status));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in updateSpaceStatus: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getStatistics(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonObject result = SpaceService::instance().getStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info("Get space statistics request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getStatistics: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::getUsageStatistics(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 调用服务层
        QJsonObject result = SpaceService::instance().getUsageStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest(result["msg"].toString());
        }
        
        Logger::info("Get space usage statistics request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getUsageStatistics: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void SpaceController::calculateParkingFee(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从查询参数获取参数
        QString spaceIdStr = request.getQueryParam("spaceId");
        QString plate = request.getQueryParam("plate");

        qDebug() << "Calculating parking fee for spaceId:" << spaceIdStr << "plate:" << plate;
        
        if (spaceIdStr.isEmpty() || plate.isEmpty()) {
            response.badRequest("Missing required parameters: spaceId, plate");
            return;
        }
        
        bool ok;
        int spaceId = spaceIdStr.toInt(&ok);
        if (!ok || spaceId <= 0) {
            response.badRequest("Invalid space ID");
            return;
        }
        
        // 查找未支付的停车记录
        QList<ParkingRecord> unpaidRecords = ParkingRecordRepository::instance().findUnpaidByPlateAndSpace(plate, spaceId);
        
        if (unpaidRecords.isEmpty()) {
            response.notFound("No unpaid parking record found for this plate and space");
            return;
        }
        
        // 返回第一个未支付记录（通常只有一个）
        ParkingRecord record = unpaidRecords.first();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = QJsonObject{
            {"recordId", record.getId()},
            {"plate", record.getPlate()},
            {"spaceId", record.getSpaceId()},
            {"enterTime", record.getEnterTime().toString(Qt::ISODate)},
            {"exitTime", record.getExitTime().isValid() ? record.getExitTime().toString(Qt::ISODate) : ""},
            {"fee", record.getFee()},
            {"paid", record.getIsPaid()}
        };
        result["message"] = "Unpaid parking record found";
        
        response.ok(result);
        
        Logger::info(QString("Calculate parking fee request: spaceId=%1, plate=%2, recordId=%3")
                    .arg(spaceId).arg(plate).arg(record.getId()));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in calculateParkingFee: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

int SpaceController::extractSpaceIdFromPath(const QString& path)
{
    // 从路径 /api/spaces/123 提取停车位ID
    QStringList parts = path.split('/', QString::SkipEmptyParts);
    if (parts.size() >= 3 && parts[1] == "spaces") {
        bool ok;
        int id = parts[2].toInt(&ok);
        return ok ? id : -1;
    }
    return -1;
}

QString SpaceController::extractStatusFromPath(const QString& path)
{
    // 从路径 /api/spaces/status/available 提取状态
    QStringList parts = path.split('/', QString::SkipEmptyParts);
    if (parts.size() >= 4 && parts[1] == "spaces" && parts[2] == "status") {
        return parts[3].toLower();
    }
    return "";
}
