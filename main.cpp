#include <QCoreApplication>
#include <QTimer>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>
#include <memory>

#include "ParkingServerApplication.h"
#include "utils/Logger.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Parking Server");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Parking Management System");
    
    // 初始化日志系统
    Logger::initialize(QDir::currentPath() + "/logs", "parking_server");
    LOG_INFO("Starting Parking Server v1.0.0");
    
    // 创建数据目录
    QDir dataDir(QDir::currentPath() + "/data");
    if (!dataDir.exists()) {
        if (!dataDir.mkpath(".")) {
            LOG_ERROR("Failed to create data directory");
            return 1;
        }
    }
    
    // 创建日志目录
    QDir logDir(QDir::currentPath() + "/logs");
    if (!logDir.exists()) {
        if (!logDir.mkpath(".")) {
            LOG_ERROR("Failed to create logs directory");
            return 1;
        }
    }
    
    // 创建应用程序实例
    ParkingServerApplication serverApp;
    
    // 初始化应用程序
    if (!serverApp.initialize()) {
        LOG_ERROR("Failed to initialize application");
        return 1;
    }
    
    // 启动服务器
    if (!serverApp.startServer()) {
        LOG_ERROR("Failed to start server");
        return 1;
    }
    
    // 设置信号处理
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&serverApp]() {
        LOG_INFO("Application shutting down...");
        serverApp.stopServer();
    });
    
    // 启动事件循环
    LOG_INFO("Parking Server is running. Press Ctrl+C to stop.");
    
    int result = app.exec();
    
    LOG_INFO("Parking Server stopped");
    Logger::shutdown();
    
    return result;
}