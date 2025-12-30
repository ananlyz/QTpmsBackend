#include "ApiRegister.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

ApiRegister& ApiRegister::instance()
{
    static ApiRegister instance;
    return instance;
}

bool ApiRegister::initialize(HttpServer& server)
{
    try {
        LOG_INFO("Initializing API routes...");
        
        // 获取路由器
        Router* router = server.router();
        
        // 注册所有路由
        registerRoutes(*router);
        
        LOG_INFO("API routes initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR(QString("Failed to initialize API: %1").arg(e.what()));
        return false;
    }
}

void ApiRegister::registerRoutes(Router& router)
{
    // 注册系统路由
    registerSystemRoutes(router);
    
    // 注册车辆管理路由
    registerCarRoutes(router);
    
    // 注册停车位管理路由
    registerSpaceRoutes(router);
    
    // 注册报告统计路由
    registerReportRoutes(router);
    
    Logger::info("All API routes registered");
}

void ApiRegister::registerSystemRoutes(Router& router)
{
    qDebug() << "Registering system routes...";
    
    // 健康检查
    router.get("/api/health", {}, [this](const HttpRequest& req, HttpResponse& res) {
        qDebug() << "Health check route called!";
        this->handleHealthCheck(req, res);
    });
    
    // API信息
    router.get("/api/info", {}, [this](const HttpRequest& req, HttpResponse& res) {
        qDebug() << "API info route called!";
        this->handleApiInfo(req, res);
    });
    
    Logger::info("System routes registered");
}

void ApiRegister::registerCarRoutes(Router& router)
{
    // 车辆注册
    router.post("/api/cars", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().registerCar(req, res);
    });
    
    // 获取车辆信息
    router.get("/api/cars/:plate", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().getCarInfo(req, res);
    });
    
    // 获取所有车辆
    router.get("/api/cars", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().getAllCars(req, res);
    });
    
    // 按类型获取车辆
    router.get("/api/cars/type/:type", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().getCarsByType(req, res);
    });
    
    // 更新车辆信息
    router.put("/api/cars/:plate", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().updateCar(req, res);
    });
    
    // 删除车辆
    router.del("/api/cars/:plate", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().deleteCar(req, res);
    });

    
    // 车辆统计
    router.get("/api/cars/statistics/overview", {}, [](const HttpRequest& req, HttpResponse& res) {
        CarController::instance().getStatistics(req, res);
    });
    
    Logger::info("Car routes registered");
}

void ApiRegister::registerSpaceRoutes(Router& router)
{
    // 添加停车位
    router.post("/api/spaces", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().addSpace(req, res);
    });
    
    // 计算停车费用
    router.get("/api/spaces/calculate-fee", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().calculateParkingFee(req, res);
    });
    
    // 获取停车位信息
    router.get("/api/spaces/:id", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getSpaceInfo(req, res);
    });
    
    // 获取所有停车位
    router.get("/api/spaces", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getAllSpaces(req, res);
    });
    
    // 按状态获取停车位
    router.get("/api/spaces/status/:status", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getSpacesByStatus(req, res);
    });
    
    // 获取可用停车位
    router.get("/api/spaces/available", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getAvailableSpaces(req, res);
    });
    
    // 获取已占用停车位
    router.get("/api/spaces/occupied", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getOccupiedSpaces(req, res);
    });
    
    // 更新停车位信息
    router.put("/api/spaces/:id", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().updateSpace(req, res);
    });
    
    // 删除停车位
    router.del("/api/spaces/:id", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().deleteSpace(req, res);
    });
    
    // 占用停车位
    router.post("/api/spaces/:id/occupy", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().occupySpace(req, res);
    });
    
    // 释放停车位
    router.post("/api/spaces/:id/release", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().releaseSpace(req, res);
    });
    
    // 更新停车位状态
    router.put("/api/spaces/:id/status", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().updateSpaceStatus(req, res);
    });
    
    // 停车位统计
    router.get("/api/spaces/statistics/overview", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getStatistics(req, res);
    });
    
    // 停车位使用率统计
    router.get("/api/spaces/statistics/usage", {}, [](const HttpRequest& req, HttpResponse& res) {
        SpaceController::instance().getUsageStatistics(req, res);
    });
    
    Logger::info("Space routes registered");
}

void ApiRegister::registerReportRoutes(Router& router)
{
    // 收入报告
    router.get("/api/reports/revenue", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getRevenueReport(req, res);
    });
    
    // 停车统计报告
    router.get("/api/reports/parking", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getParkingStatistics(req, res);
    });
    
    // 支付统计报告
    router.get("/api/reports/payment", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getPaymentStatistics(req, res);
    });
    
    // 空间使用率报告
    router.get("/api/reports/space-usage", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getSpaceUsageReport(req, res);
    });
    
    // 占用率报告
    router.get("/api/reports/occupancy-rate", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getOccupancyRate(req, res);
    });
    
    // 车辆统计报告
    router.get("/api/reports/car-statistics", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getCarStatistics(req, res);
    });
    
    // 车辆类型分布报告
    router.get("/api/reports/car-type-distribution", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getCarTypeDistribution(req, res);
    });
    
    // 欠费报告
    router.get("/api/reports/unpaid", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getUnpaidReport(req, res);
    });
    
    // 逾期报告
    router.get("/api/reports/overdue", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getOverdueReport(req, res);
    });
    
    // 仪表板摘要
    router.get("/api/reports/dashboard", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getDashboardSummary(req, res);
    });
    
    // 详细报告
    router.get("/api/reports/detailed", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().getDetailedReport(req, res);
    });
    
    // 处理支付
    router.post("/api/payments/pay", {}, [](const HttpRequest& req, HttpResponse& res) {
        ReportController::instance().processPayment(req, res);
    });
    
    Logger::info("Report routes registered");
}

void ApiRegister::handleHealthCheck(const HttpRequest& request, HttpResponse& response)
{
    QJsonObject health;
    health["status"] = "healthy";
    health["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    health["service"] = "ParkingServer";
    health["version"] = "1.0.0";
    
    response.ok(health);
}

void ApiRegister::handleApiInfo(const HttpRequest& request, HttpResponse& response)
{
    QJsonObject info;
    info["name"] = "ParkingServer API";
    info["version"] = "1.0.0";
    info["description"] = "Parking Management System API";
    info["author"] = "Parking Server Team";
    
    QJsonObject endpoints;
    
    // 系统端点
    QJsonArray systemEndpoints;
    systemEndpoints.append(QJsonObject{{"path", "/api/health"}, {"method", "GET"}, {"description", "Health check"}});
    systemEndpoints.append(QJsonObject{{"path", "/api/info"}, {"method", "GET"}, {"description", "API information"}});
    endpoints["system"] = systemEndpoints;
    
    // 车辆管理端点
    QJsonArray carEndpoints;
    carEndpoints.append(QJsonObject{{"path", "/api/cars"}, {"method", "POST"}, {"description", "Register a new car"}});
    carEndpoints.append(QJsonObject{{"path", "/api/cars"}, {"method", "GET"}, {"description", "Get all cars"}});
    carEndpoints.append(QJsonObject{{"path", "/api/cars/:plate"}, {"method", "GET"}, {"description", "Get car by plate"}});
    carEndpoints.append(QJsonObject{{"path", "/api/cars/type/:type"}, {"method", "GET"}, {"description", "Get cars by type"}});
    carEndpoints.append(QJsonObject{{"path", "/api/cars/:plate"}, {"method", "PUT"}, {"description", "Update car information"}});
    carEndpoints.append(QJsonObject{{"path", "/api/cars/:plate"}, {"method", "DELETE"}, {"description", "Delete car"}});
    carEndpoints.append(QJsonObject{{"path", "/api/cars/statistics/overview"}, {"method", "GET"}, {"description", "Get car statistics"}});
    endpoints["cars"] = carEndpoints;
    
    // 停车位管理端点
    QJsonArray spaceEndpoints;
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces"}, {"method", "POST"}, {"description", "Add a new parking space"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces"}, {"method", "GET"}, {"description", "Get all spaces"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/:id"}, {"method", "GET"}, {"description", "Get space by ID"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/status/:status"}, {"method", "GET"}, {"description", "Get spaces by status"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/available"}, {"method", "GET"}, {"description", "Get available spaces"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/occupied"}, {"method", "GET"}, {"description", "Get occupied spaces"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/:id"}, {"method", "PUT"}, {"description", "Update space information"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/:id"}, {"method", "DELETE"}, {"description", "Delete space"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/:id/occupy"}, {"method", "POST"}, {"description", "Occupy a space"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/:id/release"}, {"method", "POST"}, {"description", "Release a space"}});
    spaceEndpoints.append(QJsonObject{{"path", "/api/spaces/statistics/overview"}, {"method", "GET"}, {"description", "Get space statistics"}});
    endpoints["spaces"] = spaceEndpoints;
    
    // 报告统计端点
    QJsonArray reportEndpoints;
    reportEndpoints.append(QJsonObject{{"path", "/api/reports/revenue"}, {"method", "GET"}, {"description", "Get revenue report"}});
    reportEndpoints.append(QJsonObject{{"path", "/api/reports/parking"}, {"method", "GET"}, {"description", "Get parking statistics"}});
    reportEndpoints.append(QJsonObject{{"path", "/api/reports/payment"}, {"method", "GET"}, {"description", "Get payment statistics"}});
    reportEndpoints.append(QJsonObject{{"path", "/api/reports/space-usage"}, {"method", "GET"}, {"description", "Get space usage report"}});
    reportEndpoints.append(QJsonObject{{"path", "/api/reports/dashboard"}, {"method", "GET"}, {"description", "Get dashboard summary"}});
    reportEndpoints.append(QJsonObject{{"path", "/api/reports/detailed"}, {"method", "GET"}, {"description", "Get detailed report"}});
    endpoints["reports"] = reportEndpoints;
    
    info["endpoints"] = endpoints;
    
    response.ok(info);
}