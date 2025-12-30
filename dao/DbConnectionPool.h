#ifndef DBCONNECTIONPOOL_H
#define DBCONNECTIONPOOL_H

#include <QObject>
#include <QQueue>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QDebug>

class DbConnectionPool : public QObject
{
    Q_OBJECT

public:
    static DbConnectionPool& instance();
    
    // 初始化连接池
    bool initialize(const QString& host, int port, const QString& database,
                   const QString& username, const QString& password,
                   int poolSize = 10);
    
    // 获取数据库连接
    QSqlDatabase getConnection();
    
    // 归还数据库连接
    void releaseConnection(QSqlDatabase connection);
    
    // 关闭连接池
    void close();
    
    // 执行查询
    bool executeQuery(const QString& queryStr, const QVariantMap& params = QVariantMap());
    
    // 执行查询并返回结果
    QList<QVariantMap> executeQueryWithResults(const QString& queryStr, const QVariantMap& params = QVariantMap());
    
    // 获取最后插入的ID
    qint64 getLastInsertId();
    
private:
    DbConnectionPool();
    ~DbConnectionPool();
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;
    
    QQueue<QSqlDatabase> availableConnections;
    QQueue<QSqlDatabase> usedConnections;
    QMutex mutex;
    QWaitCondition waitCondition;
    
    QString connectionName;
    QString host;
    int port;
    QString database;
    QString username;
    QString password;
    int poolSize;
    bool initialized;
    
    QSqlDatabase createConnection();
    bool testConnection(QSqlDatabase connection);
};

#endif // DBCONNECTIONPOOL_H