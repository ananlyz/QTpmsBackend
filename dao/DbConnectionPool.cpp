#include "DbConnectionPool.h"
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlField>

DbConnectionPool& DbConnectionPool::instance()
{
    static DbConnectionPool instance;
    return instance;
}

DbConnectionPool::DbConnectionPool() : initialized(false)
{
}

DbConnectionPool::~DbConnectionPool()
{
    close();
}

bool DbConnectionPool::initialize(const QString& host, int port, const QString& database,
                                   const QString& username, const QString& password,
                                   int poolSize)
{
    QMutexLocker locker(&mutex);
    
    if (initialized) {
        return true;
    }
    
    this->host = host;
    this->port = port;
    this->database = database;
    this->username = username;
    this->password = password;
    this->poolSize = poolSize;
    this->connectionName = "ParkingServer";
    
    // 创建连接池
    for (int i = 0; i < poolSize; ++i) {
        QSqlDatabase connection = createConnection();
        if (connection.isValid() && connection.isOpen()) {
            availableConnections.enqueue(connection);
        } else {
            qWarning() << "Failed to create database connection" << i;
            return false;
        }
    }
    
    initialized = true;
    qDebug() << "Database connection pool initialized with" << poolSize << "connections";
    return true;
}

QSqlDatabase DbConnectionPool::getConnection()
{
    QMutexLocker locker(&mutex);
    
    if (!initialized) {
        qWarning() << "Database connection pool not initialized";
        return QSqlDatabase();
    }
    
    // 等待可用连接
    while (availableConnections.isEmpty()) {
        waitCondition.wait(&mutex, 5000); // 最多等待5秒
        if (availableConnections.isEmpty()) {
            qWarning() << "Timeout waiting for database connection";
            return QSqlDatabase();
        }
    }
    
    QSqlDatabase connection = availableConnections.dequeue();
    
    // 测试连接是否仍然有效
    if (!testConnection(connection)) {
        qWarning() << "Database connection is invalid, creating new one";
        connection = createConnection();
    }
    
    usedConnections.enqueue(connection);
    return connection;
}

void DbConnectionPool::releaseConnection(QSqlDatabase connection)
{
    QMutexLocker locker(&mutex);
    
    if (!initialized) {
        return;
    }
    
    // 从使用队列中移除
    for (int i = 0; i < usedConnections.size(); ++i) {
        if (usedConnections[i].connectionName() == connection.connectionName()) {
            usedConnections.removeAt(i);
            break;
        }
    }
    
    // 归还到可用队列
    availableConnections.enqueue(connection);
    waitCondition.wakeOne();
}

void DbConnectionPool::close()
{
    QMutexLocker locker(&mutex);
    
    if (!initialized) {
        return;
    }
    
    // 关闭所有连接
    while (!availableConnections.isEmpty()) {
        QSqlDatabase connection = availableConnections.dequeue();
        connection.close();
    }
    
    while (!usedConnections.isEmpty()) {
        QSqlDatabase connection = usedConnections.dequeue();
        connection.close();
    }
    
    initialized = false;
    qDebug() << "Database connection pool closed";
}

bool DbConnectionPool::executeQuery(const QString& queryStr, const QVariantMap& params)
{
    QSqlDatabase connection = getConnection();
    if (!connection.isValid() || !connection.isOpen()) {
        return false;
    }
    
    QSqlQuery query(connection);
    query.prepare(queryStr);
    
    // 绑定参数
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    bool success = query.exec();
    if (!success) {
        qWarning() << "Query execution failed:" << query.lastError().text();
        qWarning() << "Query:" << queryStr;
    }
    
    releaseConnection(connection);
    return success;
}

QList<QVariantMap> DbConnectionPool::executeQueryWithResults(const QString& queryStr, const QVariantMap& params)
{
    QList<QVariantMap> results;
    
    QSqlDatabase connection = getConnection();
    if (!connection.isValid() || !connection.isOpen()) {
        return results;
    }
    
    QSqlQuery query(connection);
    query.prepare(queryStr);
    
    // 绑定参数
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(":" + it.key(), it.value());
    }
    
    if (query.exec()) {
        while (query.next()) {
            QVariantMap row;
            QSqlRecord record = query.record();
            
            for (int i = 0; i < record.count(); ++i) {
                row[record.fieldName(i)] = record.value(i);
            }
            
            results.append(row);
        }
    } else {
        qWarning() << "Query execution failed:" << query.lastError().text();
        qWarning() << "Query:" << queryStr;
    }
    
    releaseConnection(connection);
    return results;
}

qint64 DbConnectionPool::getLastInsertId()
{
    QSqlDatabase connection = getConnection();
    if (!connection.isValid() || !connection.isOpen()) {
        return -1;
    }
    
    QSqlQuery query("SELECT LAST_INSERT_ID()", connection);
    qint64 id = -1;
    
    if (query.next()) {
        id = query.value(0).toLongLong();
    }
    
    releaseConnection(connection);
    return id;
}

QSqlDatabase DbConnectionPool::createConnection()
{
    QString connectionName = QString("%1_%2").arg(this->connectionName).arg(QDateTime::currentMSecsSinceEpoch());
    
    QSqlDatabase connection = QSqlDatabase::addDatabase("QMYSQL", connectionName);
    connection.setHostName(host);
    connection.setPort(port);
    connection.setDatabaseName(database);
    connection.setUserName(username);
    connection.setPassword(password);
    
    if (!connection.open()) {
        qWarning() << "Failed to open database connection:" << connection.lastError().text();
        return QSqlDatabase();
    }
    
    return connection;
}

bool DbConnectionPool::testConnection(QSqlDatabase connection)
{
    if (!connection.isValid() || !connection.isOpen()) {
        return false;
    }
    
    QSqlQuery query("SELECT 1", connection);
    return query.exec();
}