#include "ParkingServerApplication.h"
#include "api/ApiRegister.h"
#include "core/HttpServer.h"
#include "core/Router.h"
#include "utils/Logger.h"
#include "services/CarService.h"
#include "services/SpaceService.h"
#include "services/BillingService.h"
#include "controllers/CarController.h"
#include "controllers/SpaceController.h"
#include "controllers/ReportController.h"
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

ParkingServerApplication::ParkingServerApplication(QObject* parent) : QObject(parent)
{
    m_server = nullptr;
    m_isRunning = false;
}

ParkingServerApplication::~ParkingServerApplication()
{
    stopServer();
}

bool ParkingServerApplication::initialize()
{
    LOG_INFO("Initializing Parking Server Application...");
    
    // 1. 初始化数据库
    if (!initializeDatabase()) {
        LOG_ERROR("Failed to initialize database");
        return false;
    }
    
    // 2. 初始化服务
    if (!initializeServices()) {
        LOG_ERROR("Failed to initialize services");
        return false;
    }
    
    // 3. 初始化控制器
    if (!initializeControllers()) {
        LOG_ERROR("Failed to initialize controllers");
        return false;
    }
    
    // 4. 初始化HTTP服务器
    if (!initializeHttpServer()) {
        LOG_ERROR("Failed to initialize HTTP server");
        return false;
    }
    
    // 5. 注册API路由
    if (!registerApiRoutes()) {
        LOG_ERROR("Failed to register API routes");
        return false;
    }
    
    LOG_INFO("Parking Server Application initialized successfully");
    return true;
}

bool ParkingServerApplication::startServer()
{
    if (m_isRunning) {
        LOG_WARNING("Server is already running");
        return true;
    }
    
    if (!m_server) {
        LOG_ERROR("HTTP server not initialized");
        return false;
    }
    
    if (!m_server->start()) {
        LOG_ERROR("Failed to start HTTP server");
        return false;
    }
    
    m_isRunning = true;
    LOG_INFO("Parking Server started successfully");
    LOG_INFO(QString("Server listening on port %1").arg(m_server->port()));
    
    return true;
}

void ParkingServerApplication::stopServer()
{
    if (m_server && m_server->isRunning()) {
        m_server->stop();
        m_isRunning = false;
        LOG_INFO("Parking Server stopped");
    }
}

bool ParkingServerApplication::isRunning() const
{
    return m_isRunning;
}

bool ParkingServerApplication::initializeDatabase()
{
    LOG_INFO("Initializing database...");
    
    // 确保data目录存在
    QDir dir;
    if (!dir.mkpath("data")) {
        LOG_ERROR("Failed to create data directory");
        return false;
    }
    
    // 创建临时数据库连接用于初始化
    QString dbPath = QDir::currentPath() + "/data/parking_server.db";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "init_db");
    db.setDatabaseName(dbPath);
    
    if (!db.open()) {
        LOG_ERROR("Failed to open database: " + db.lastError().text());
        return false;
    }
    
    LOG_INFO("Database opened successfully: " + dbPath);
    
    // 创建数据表
    if (!createDatabaseTables()) {
        LOG_ERROR("Failed to create database tables");
        db.close();
        QSqlDatabase::removeDatabase("init_db");
        return false;
    }
    
    // 初始化基础数据
    if (!initializeBaseData()) {
        LOG_WARNING("Failed to initialize base data");
    }
    
    // 调试数据库内容
    debugDatabaseContent(db);
    
    // 关闭临时连接
    db.close();
    QSqlDatabase::removeDatabase("init_db");
    
    LOG_INFO("Database initialized successfully");
    return true;
}

bool ParkingServerApplication::createDatabaseTables()
{
    LOG_INFO("Creating database tables...");
    
    QSqlDatabase db = QSqlDatabase::database("init_db"); // 使用初始化连接
    if (!db.isOpen()) {
        LOG_ERROR("Database not open");
        return false;
    }
    
    QSqlQuery query(db);
    
    // 创建车辆表 - 与Car模型保持一致
    QString createCarTable = R"(
        CREATE TABLE IF NOT EXISTS cars (
            plate TEXT PRIMARY KEY,
            type TEXT DEFAULT '小型车',
            color TEXT,
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            update_time DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createCarTable)) {
        LOG_ERROR("Failed to create cars table: " + query.lastError().text());
        return false;
    }
    
    // 创建停车位表 - 与ParkingSpace模型保持一致
    QString createSpaceTable = R"(
        CREATE TABLE IF NOT EXISTS parking_spaces (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            location TEXT NOT NULL,
            status TEXT DEFAULT 'available',
            current_plate TEXT,
            occupied_time DATETIME,
            type TEXT DEFAULT '普通',
            hourly_rate REAL DEFAULT 5.0
        )
    )";
    
    if (!query.exec(createSpaceTable)) {
        LOG_ERROR("Failed to create spaces table: " + query.lastError().text());
        return false;
    }
    
    // 创建停车记录表 - 与ParkingRecord模型保持一致
    QString createParkingRecordTable = R"(
        CREATE TABLE IF NOT EXISTS parking_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            plate TEXT NOT NULL,
            space_id INTEGER NOT NULL,
            enter_time DATETIME NOT NULL,
            exit_time DATETIME,
            fee REAL DEFAULT 0.0,
            is_paid INTEGER DEFAULT 0,
            pay_time DATETIME,
            pay_method TEXT,
            FOREIGN KEY (plate) REFERENCES cars(plate),
            FOREIGN KEY (space_id) REFERENCES parking_spaces(id)
        )
    )";
    
    if (!query.exec(createParkingRecordTable)) {
        LOG_ERROR("Failed to create parking_records table: " + query.lastError().text());
        return false;
    }
    
    // 创建支付记录表
    QString createPaymentTable = R"(
        CREATE TABLE IF NOT EXISTS payments (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            parking_record_id INTEGER NOT NULL,
            amount REAL NOT NULL,
            payment_method TEXT NOT NULL,
            payment_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            status TEXT DEFAULT '已完成',
            transaction_id TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (parking_record_id) REFERENCES parking_records(id)
        )
    )";
    
    if (!query.exec(createPaymentTable)) {
        LOG_ERROR("Failed to create payments table: " + query.lastError().text());
        return false;
    }
    
    // 创建计费规则表
    QString createBillingTable = R"(
        CREATE TABLE IF NOT EXISTS billing_rules (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            rule_name TEXT NOT NULL,
            space_type TEXT DEFAULT '普通',
            hourly_rate REAL DEFAULT 5.0,
            daily_rate REAL DEFAULT 50.0,
            monthly_rate REAL DEFAULT 800.0,
            night_rate REAL DEFAULT 3.0,
            weekend_rate REAL DEFAULT 6.0,
            holiday_rate REAL DEFAULT 8.0,
            min_fee REAL DEFAULT 5.0,
            max_fee REAL DEFAULT 200.0,
            is_active INTEGER DEFAULT 1,
            effective_date DATETIME DEFAULT CURRENT_TIMESTAMP,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createBillingTable)) {
        LOG_ERROR("Failed to create billing_rules table: " + query.lastError().text());
        return false;
    }
    
    LOG_INFO("Database tables created successfully");
    return true;
}

bool ParkingServerApplication::initializeBaseData()
{
    LOG_INFO("Initializing base data...");
    
    QSqlDatabase db = QSqlDatabase::database("init_db"); // 使用初始化连接
    QSqlQuery query(db);
    
    // 检查是否已经存在基础数据
    query.prepare("SELECT COUNT(*) FROM billing_rules WHERE is_active = 1");
    if (query.exec() && query.next() && query.value(0).toInt() > 0) {
        LOG_INFO("Base data already exists");
        return true;
    }
    
    // 插入默认计费规则
    QString insertDefaultRules = R"(
        INSERT INTO billing_rules (rule_name, space_type, hourly_rate, daily_rate, monthly_rate, 
                                 night_rate, weekend_rate, holiday_rate, min_fee, max_fee, is_active)
        VALUES 
        ('普通车位标准费率', '普通', 5.0, 50.0, 800.0, 3.0, 6.0, 8.0, 5.0, 200.0, 1),
        ('VIP车位标准费率', 'VIP', 8.0, 80.0, 1200.0, 5.0, 10.0, 12.0, 8.0, 300.0, 1),
        ('临时车位标准费率', '临时', 6.0, 60.0, 900.0, 4.0, 8.0, 10.0, 6.0, 250.0, 1)
    )";
    
    if (!query.exec(insertDefaultRules)) {
        LOG_WARNING("Failed to insert default billing rules: " + query.lastError().text());
    }
    
    // 插入一些默认停车位 - 使用与ParkingSpace模型一致的字段
    QString insertDefaultSpaces = R"(
        INSERT INTO parking_spaces (location, status, type, hourly_rate)
        VALUES 
        ('A区-1号', 'available', '普通', 5.0),
        ('A区-2号', 'available', '普通', 5.0),
        ('A区-3号', 'available', '普通', 5.0),
        ('B区-1号', 'available', 'VIP', 8.0),
        ('B区-2号', 'available', 'VIP', 8.0),
        ('C区-1号', 'available', '临时', 6.0)
    )";
    
    if (!query.exec(insertDefaultSpaces)) {
        LOG_WARNING("Failed to insert default spaces: " + query.lastError().text());
    }
    
    LOG_INFO("Base data initialized successfully");
    return true;
}

void ParkingServerApplication::debugDatabaseContent(QSqlDatabase& db)
{
    LOG_INFO("=== Database Debug Information ===");
    
    if (!db.isOpen()) {
        LOG_ERROR("Database not open for debugging");
        return;
    }
    
    // 获取所有表
    QStringList tables = db.tables();
    LOG_INFO(QString("Total tables: %1").arg(tables.size()));
    
    for (const QString& tableName : tables) {
        LOG_INFO(QString("Table: %1").arg(tableName));
        
        // 获取表结构
        QSqlRecord record = db.record(tableName);
        QStringList fields;
        for (int i = 0; i < record.count(); ++i) {
            fields.append(record.fieldName(i));
        }
        LOG_INFO(QString("Fields: %1").arg(fields.join(", ")));
        
        // 获取表数据
        QSqlQuery query(db);
        query.prepare(QString("SELECT * FROM %1").arg(tableName));
        if (query.exec()) {
            int rowCount = 0;
            while (query.next()) {
                rowCount++;
                QStringList values;
                for (int i = 0; i < record.count(); ++i) {
                    QVariant value = query.value(i);
                    values.append(QString("%1: %2").arg(record.fieldName(i), value.toString()));
                }
                LOG_INFO(QString("Row %1: %2").arg(rowCount).arg(values.join(", ")));
            }
            LOG_INFO(QString("Total rows in %1: %2").arg(tableName).arg(rowCount));
        } else {
            LOG_ERROR(QString("Failed to query table %1: %2").arg(tableName, query.lastError().text()));
        }
        
        LOG_INFO("---");
    }
    
    LOG_INFO("=== End Database Debug ===");
}

bool ParkingServerApplication::initializeServices()
{
    LOG_INFO("Initializing services...");
    
    // 初始化各个服务
    CarService& carService = CarService::instance();
    SpaceService& spaceService = SpaceService::instance();
    BillingService& billingService = BillingService::instance();
    
    LOG_INFO("Services initialized successfully");
    return true;
}

bool ParkingServerApplication::initializeControllers()
{
    LOG_INFO("Initializing controllers...");
    
    // 初始化各个控制器
    CarController& carController = CarController::instance();
    SpaceController& spaceController = SpaceController::instance();
    ReportController& reportController = ReportController::instance();
    
    LOG_INFO("Controllers initialized successfully");
    return true;
}

bool ParkingServerApplication::initializeHttpServer()
{
    LOG_INFO("Initializing HTTP server...");
    
    m_server = std::make_unique<HttpServer>(this);
    
    // 配置服务器
    m_server->setPort(8080);
    m_server->setMaxConnections(100);
    m_server->setRequestTimeout(30000); // 30秒
    
    // 创建并设置路由器 - 这是关键步骤！
    Router* router = new Router(this);
    m_server->setRouter(router);
    
    LOG_INFO("HTTP server initialized successfully");
    return true;
}

bool ParkingServerApplication::registerApiRoutes()
{
    LOG_INFO("Registering API routes...");
    
    if (!m_server) {
        LOG_ERROR("HTTP server not initialized");
        return false;
    }
    
    // 注册所有路由
    ApiRegister& apiRegister = ApiRegister::instance();
    if (!apiRegister.initialize(*m_server)) {
        LOG_ERROR("Failed to register API routes");
        return false;
    }
    
    LOG_INFO("API routes registered successfully");
    return true;
}
