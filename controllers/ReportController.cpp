#include "ReportController.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include "../utils/JsonUtil.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

ReportController& ReportController::instance()
{
    static ReportController instance;
    return instance;
}

void ReportController::getRevenueReport(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析日期范围
        auto [startTime, endTime] = parseDateRange(request);
        
        if (!startTime.isValid() || !endTime.isValid()) {
            response.badRequest("Invalid date range");
            return;
        }
        
        // 调用服务层
        QJsonObject result = BillingService::instance().getRevenueStatistics(startTime, endTime);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve revenue report");
        }
        
        Logger::info(QString("Revenue report request: start=%1, end=%2")
                    .arg(startTime.toString(Qt::ISODate))
                    .arg(endTime.toString(Qt::ISODate)));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getRevenueReport: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getParkingStatistics(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析日期范围
        auto [startTime, endTime] = parseDateRange(request);
        
        if (!startTime.isValid() || !endTime.isValid()) {
            response.badRequest("Invalid date range");
            return;
        }
        
        // 调用服务层
        QJsonObject result = BillingService::instance().getParkingStatistics(startTime, endTime);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve parking statistics");
        }
        
        Logger::info(QString("Parking statistics request: start=%1, end=%2")
                    .arg(startTime.toString(Qt::ISODate))
                    .arg(endTime.toString(Qt::ISODate)));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getParkingStatistics: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getPaymentStatistics(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析日期范围
        auto [startTime, endTime] = parseDateRange(request);
        
        if (!startTime.isValid() || !endTime.isValid()) {
            response.badRequest("Invalid date range");
            return;
        }
        
        // 调用服务层
        QJsonObject result = BillingService::instance().getPaymentStatistics(startTime, endTime);
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve payment statistics");
        }
        
        Logger::info(QString("Payment statistics request: start=%1, end=%2")
                    .arg(startTime.toString(Qt::ISODate))
                    .arg(endTime.toString(Qt::ISODate)));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getPaymentStatistics: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getSpaceUsageReport(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 获取使用率统计
        QJsonObject result = SpaceService::instance().getUsageStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve space usage report");
        }
        
        Logger::info("Space usage report request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getSpaceUsageReport: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getOccupancyRate(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 获取基本统计信息
        QJsonObject result = SpaceService::instance().getStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve occupancy rate");
        }
        
        Logger::info("Occupancy rate request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getOccupancyRate: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getCarStatistics(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 获取车辆统计信息
        QJsonObject result = CarService::instance().getStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve car statistics");
        }
        
        Logger::info("Car statistics request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getCarStatistics: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getCarTypeDistribution(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 获取车辆统计信息中的类型分布
        QJsonObject result = CarService::instance().getStatistics();
        
        // 设置响应
        if (result["code"] == 0) {
            response.ok(result);
        } else {
            response.badRequest("Failed to retrieve car type distribution");
        }
        
        Logger::info("Car type distribution request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getCarTypeDistribution: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getUnpaidReport(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 获取未支付记录
        QJsonArray unpaidRecords = BillingService::instance().getUnpaidRecords();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = unpaidRecords;
        result["count"] = unpaidRecords.size();
        result["message"] = "Unpaid records retrieved successfully";
        
        response.ok(result);
        
        Logger::info(QString("Unpaid report request: count=%1").arg(unpaidRecords.size()));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getUnpaidReport: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getOverdueReport(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析日期范围
        auto [startTime, endTime] = parseDateRange(request);
        
        if (!startTime.isValid() || !endTime.isValid()) {
            response.badRequest("Invalid date range");
            return;
        }
        
        // 获取指定时间范围内的停车记录
        QJsonArray records = BillingService::instance().getAllParkingRecords(1000);
        
        QJsonArray overdueRecords;
        QDateTime now = QDateTime::currentDateTime();
        
        for (const QJsonValue& value : records) {
            QJsonObject record = value.toObject();
            
            // 检查是否欠费（停车结束超过24小时且未完全支付）
            if (record.contains("endTime") && !record["endTime"].toString().isEmpty()) {
                QDateTime endTime = QDateTime::fromString(record["endTime"].toString(), Qt::ISODate);
                if (endTime.isValid() && endTime.secsTo(now) > 24 * 3600) { // 超过24小时
                    double fee = record["fee"].toDouble();
                    double paidAmount = record["paidAmount"].toDouble();
                    if (paidAmount < fee) {
                        QJsonObject overdueRecord = record;
                        overdueRecord["overdueDays"] = endTime.daysTo(now);
                        overdueRecord["unpaidAmount"] = fee - paidAmount;
                        overdueRecords.append(overdueRecord);
                    }
                }
            }
        }
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = overdueRecords;
        result["count"] = overdueRecords.size();
        result["message"] = "Overdue records retrieved successfully";
        
        response.ok(result);
        
        Logger::info(QString("Overdue report request: count=%1").arg(overdueRecords.size()));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getOverdueReport: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getDashboardSummary(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 生成仪表板数据
        QJsonObject dashboardData = generateDashboardData();
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = dashboardData;
        result["message"] = "Dashboard data retrieved successfully";
        
        response.ok(result);
        
        Logger::info("Dashboard summary request");
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getDashboardSummary: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

void ReportController::getDetailedReport(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 解析日期范围
        auto [startTime, endTime] = parseDateRange(request);
        
        if (!startTime.isValid() || !endTime.isValid()) {
            response.badRequest("Invalid date range");
            return;
        }
        
        // 生成详细报告数据
        QJsonObject detailedData = generateDetailedData(startTime, endTime);
        
        QJsonObject result;
        result["success"] = true;
        result["data"] = detailedData;
        result["message"] = "Detailed report generated successfully";
        
        response.ok(result);
        
        Logger::info(QString("Detailed report request: start=%1, end=%2")
                    .arg(startTime.toString(Qt::ISODate))
                    .arg(endTime.toString(Qt::ISODate)));
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in getDetailedReport: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}

QPair<QDateTime, QDateTime> ReportController::parseDateRange(const HttpRequest& request)
{
    QString startDateStr = request.getQueryParam("startDate");
    QString endDateStr = request.getQueryParam("endDate");
    
    QDateTime startTime;
    QDateTime endTime;
    
    if (!startDateStr.isEmpty() && !endDateStr.isEmpty()) {
        startTime = QDateTime::fromString(startDateStr, Qt::ISODate);
        endTime = QDateTime::fromString(endDateStr, Qt::ISODate);
    } else {
        // 默认返回最近30天的数据
        endTime = QDateTime::currentDateTime();
        startTime = endTime.addDays(-30);
    }
    
    return qMakePair(startTime, endTime);
}

QJsonObject ReportController::generateDashboardData()
{
    QJsonObject dashboard;
    
    // 获取当前时间
    QDateTime now = QDateTime::currentDateTime();
    QDateTime todayStart = QDateTime(now.date(), QTime(0, 0, 0));
    QDateTime todayEnd = QDateTime(now.date(), QTime(23, 59, 59));
    
    // 基本统计
    QJsonObject spaceStats = SpaceService::instance().getStatistics()["data"].toObject();
    QJsonObject carStats = CarService::instance().getStatistics()["data"].toObject();
    QJsonObject revenueStats = BillingService::instance().getRevenueStatistics(todayStart, todayEnd)["data"].toObject();
    
    dashboard["spaces"] = spaceStats;
    dashboard["cars"] = carStats;
    dashboard["revenue"] = revenueStats;
    
    // 实时数据
    QJsonArray activeParkings = BillingService::instance().getActiveParkingRecords();
    dashboard["activeParkings"] = activeParkings;
    dashboard["activeParkingCount"] = activeParkings.size();
    
    // 未支付订单
    QJsonArray unpaidRecords = BillingService::instance().getUnpaidRecords();
    dashboard["unpaidRecords"] = unpaidRecords;
    dashboard["unpaidCount"] = unpaidRecords.size();
    
    // 系统状态
    dashboard["systemTime"] = now.toString(Qt::ISODate);
    dashboard["serverStatus"] = "running";
    
    return dashboard;
}

QJsonObject ReportController::generateDetailedData(const QDateTime& startTime, const QDateTime& endTime)
{
    QJsonObject detailed;
    
    // 收入统计
    QJsonObject revenueStats = BillingService::instance().getRevenueStatistics(startTime, endTime)["data"].toObject();
    detailed["revenue"] = revenueStats;
    
    // 停车统计
    QJsonObject parkingStats = BillingService::instance().getParkingStatistics(startTime, endTime)["data"].toObject();
    detailed["parking"] = parkingStats;
    
    // 支付统计
    QJsonObject paymentStats = BillingService::instance().getPaymentStatistics(startTime, endTime)["data"].toObject();
    detailed["payment"] = paymentStats;
    
    // 空间使用率
    QJsonObject usageStats = SpaceService::instance().getUsageStatistics()["data"].toObject();
    detailed["spaceUsage"] = usageStats;
    
    // 车辆统计
    QJsonObject carStats = CarService::instance().getStatistics()["data"].toObject();
    detailed["cars"] = carStats;
    
    // 欠费统计
    QJsonArray unpaidRecords = BillingService::instance().getUnpaidRecords();
    
    double totalUnpaid = 0;
    QJsonObject unpaidByPlate;
    
    for (const QJsonValue& value : unpaidRecords) {
        QJsonObject record = value.toObject();
        QString plate = record["plate"].toString();
        double unpaidAmount = record["unpaidAmount"].toDouble();
        
        totalUnpaid += unpaidAmount;
        
        if (!unpaidByPlate.contains(plate)) {
            unpaidByPlate[plate] = 0;
        }
        unpaidByPlate[plate] = unpaidByPlate[plate].toDouble() + unpaidAmount;
    }
    
    QJsonObject unpaidStats;
    unpaidStats["totalUnpaid"] = totalUnpaid;
    unpaidStats["totalCount"] = unpaidRecords.size();
    unpaidStats["byPlate"] = unpaidByPlate;
    detailed["unpaid"] = unpaidStats;
    
    // 报告元数据
    detailed["reportPeriod"] = QJsonObject{
        {"startTime", startTime.toString(Qt::ISODate)},
        {"endTime", endTime.toString(Qt::ISODate)},
        {"generatedAt", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
    
    return detailed;
}

void ReportController::processPayment(const HttpRequest& request, HttpResponse& response)
{
    try {
        // 从请求体获取参数
        QJsonDocument jsonDoc = QJsonDocument::fromJson(request.bodyRaw);
        if (!jsonDoc.isObject()) {
            response.badRequest("Invalid JSON format");
            return;
        }
        
        QJsonObject json = jsonDoc.object();
        int recordId = json["recordId"].toInt();
        QString paymentMethod = json["paymentMethod"].toString();
        
        if (recordId <= 0) {
            response.badRequest("Invalid record ID");
            return;
        }
        
        if (paymentMethod.isEmpty()) {
            paymentMethod = "cash"; // 默认现金支付
        }
        
        // 获取记录以获取费用
        ParkingRecord record = ParkingRecordRepository::instance().findById(recordId);
        if (record.getId() == 0) {
            response.notFound("Parking record not found");
            return;
        }
        
        if (record.getIsPaid()) {
            response.badRequest("Record is already paid");
            return;
        }
        
        // 处理支付
        QJsonObject result = BillingService::instance().processPayment(recordId, record.getFee(), paymentMethod);
        
        if (result["code"] == 0) {
            response.ok(result);
            Logger::info(QString("Payment processed: recordId=%1, amount=%2, method=%3")
                        .arg(recordId).arg(record.getFee()).arg(paymentMethod));
        } else {
            response.badRequest("Payment processing failed");
        }
        
    } catch (const std::exception& e) {
        Logger::error(QString("Error in processPayment: %1").arg(e.what()));
        response.serverError("Internal server error");
    }
}