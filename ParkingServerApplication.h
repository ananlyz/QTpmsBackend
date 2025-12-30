#ifndef PARKINGSERVERAPPLICATION_H
#define PARKINGSERVERAPPLICATION_H

#include <QObject>
#include <QCoreApplication>
#include <memory>
#include <QSqlDatabase>

class HttpServer;

class ParkingServerApplication : public QObject
{
    Q_OBJECT

public:
    ParkingServerApplication(QObject* parent = nullptr);
    ~ParkingServerApplication();

    bool initialize();
    bool startServer();
    void stopServer();
    bool isRunning() const;

private:
    bool initializeDatabase();
    bool createDatabaseTables();
    bool initializeBaseData();
    void debugDatabaseContent(QSqlDatabase& db);
    bool initializeServices();
    bool initializeControllers();
    bool initializeHttpServer();
    bool registerApiRoutes();

private:
    std::unique_ptr<HttpServer> m_server;
    bool m_isRunning;
};

#endif // PARKINGSERVERAPPLICATION_H