#include "CarRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariantMap>
#include <QDebug>
#include <QThread>
#include <QThreadStorage>
#include <QDir>

CarRepository& CarRepository::instance()
{
    static CarRepository instance;
    return instance;
}

bool CarRepository::initializeTable()
{
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS cars (
            plate VARCHAR(20) PRIMARY KEY,
            type VARCHAR(50),
            color VARCHAR(20),
            create_time DATETIME DEFAULT CURRENT_TIMESTAMP,
            update_time DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
        )
    )";
    
    return instance().executeQuery(createTableQuery);
}

bool CarRepository::insert(const Car& car)
{
    QString insertQuery = R"(
        INSERT INTO cars (plate, type, color, create_time, update_time)
        VALUES (:plate, :type, :color, :create_time, :update_time)
    )";
    
    QVariantMap params;
    params["plate"] = car.getPlate();
    params["type"] = car.getType();
    params["color"] = car.getColor();
    params["create_time"] = car.getCreateTime();
    params["update_time"] = car.getUpdateTime();
    
    return instance().executeQuery(insertQuery, params);
}

bool CarRepository::update(const Car& car)
{
    QString updateQuery = R"(
        UPDATE cars 
        SET type = :type, color = :color, update_time = :update_time
        WHERE plate = :plate
    )";
    
    QVariantMap params;
    params["plate"] = car.getPlate();
    params["type"] = car.getType();
    params["color"] = car.getColor();
    params["update_time"] = car.getUpdateTime();
    
    return instance().executeQuery(updateQuery, params);
}

bool CarRepository::remove(const QString& plate)
{
    QString deleteQuery = "DELETE FROM cars WHERE plate = :plate";
    QVariantMap params;
    params["plate"] = plate;
    
    return instance().executeQuery(deleteQuery, params);
}

Car CarRepository::findByPlate(const QString& plate)
{
    QString selectQuery = "SELECT * FROM cars WHERE plate = :plate";
    QVariantMap params;
    params["plate"] = plate;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery, params);
    
    if (!results.isEmpty()) {
        return mapToCar(results.first());
    }
    
    return Car(); // 返回空对象
}

QList<Car> CarRepository::findAll()
{
    QString selectQuery = "SELECT * FROM cars ORDER BY create_time DESC";
    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery);
    
    QList<Car> cars;
    for (const auto& row : results) {
        cars.append(mapToCar(row));
    }
    
    return cars;
}

QList<Car> CarRepository::findByType(const QString& type)
{
    QString selectQuery = "SELECT * FROM cars WHERE type = :type ORDER BY create_time DESC";
    QVariantMap params;
    params["type"] = type;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery, params);
    
    QList<Car> cars;
    for (const auto& row : results) {
        cars.append(mapToCar(row));
    }
    
    return cars;
}

bool CarRepository::exists(const QString& plate)
{
    QString countQuery = "SELECT COUNT(*) as count FROM cars WHERE plate = :plate";
    QVariantMap params;
    params["plate"] = plate;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery, params);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt() > 0;
    }
    
    return false;
}

int CarRepository::count()
{
    QString countQuery = "SELECT COUNT(*) as count FROM cars";
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt();
    }
    
    return 0;
}

int CarRepository::countByType(const QString& type)
{
    QString countQuery = "SELECT COUNT(*) as count FROM cars WHERE type = :type";
    QVariantMap params;
    params["type"] = type;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery, params);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt();
    }
    
    return 0;
}

Car CarRepository::mapToCar(const QVariantMap& row)
{
    Car car;
    car.setPlate(row["plate"].toString());
    car.setType(row["type"].toString());
    car.setColor(row["color"].toString());
    car.setCreateTime(row["create_time"].toDateTime());
    car.setUpdateTime(row["update_time"].toDateTime());
    return car;
}

QVariantMap CarRepository::carToMap(const Car& car)
{
    QVariantMap map;
    map["plate"] = car.getPlate();
    map["type"] = car.getType();
    map["color"] = car.getColor();
    map["create_time"] = car.getCreateTime();
    map["update_time"] = car.getUpdateTime();
    return map;
}

bool CarRepository::executeQuery(const QString& queryStr, const QVariantMap& params)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        qDebug() << "Database connection is not open";
        return false;
    }
    
    bool success = false;
    {
        QSqlQuery query(db);
        query.prepare(queryStr);
        
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.bindValue(":" + it.key(), it.value());
        }
        
        if (!query.exec()) {
            qDebug() << "Query failed:" << query.lastError().text() << "Query:" << queryStr;
            success = false;
        } else {
            success = true;
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return success;
}

QList<QVariantMap> CarRepository::executeQueryWithResults(const QString& queryStr, const QVariantMap& params)
{
    QList<QVariantMap> results;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        qDebug() << "Database connection is not open";
        return results;
    }
    
    {
        QSqlQuery query(db);
        query.prepare(queryStr);
        
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.bindValue(":" + it.key(), it.value());
        }
        
        if (!query.exec()) {
            qDebug() << "Query failed:" << query.lastError().text() << "Query:" << queryStr;
            QSqlDatabase::removeDatabase(db.connectionName());
            return results;
        }
        
        while (query.next()) {
            QVariantMap row;
            QSqlRecord record = query.record();
            for (int i = 0; i < record.count(); ++i) {
                row[record.fieldName(i)] = query.value(i);
            }
            results.append(row);
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return results;
}

QSqlDatabase CarRepository::getDatabase()
{
    QString dbPath = QDir::currentPath() + "/data/parking_server.db";
    QString connectionName = QString("car_repo_%1").arg(QDateTime::currentMSecsSinceEpoch());
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return QSqlDatabase();
    }
    return db;
}