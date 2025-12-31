#include "SpaceRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariantMap>
#include <QDebug>
#include <QThread>
#include <QThreadStorage>
#include <QDir>

SpaceRepository& SpaceRepository::instance()
{
    static SpaceRepository instance;
    return instance;
}

bool SpaceRepository::initializeTable()
{
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS parking_spaces (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            location TEXT NOT NULL UNIQUE,
            status TEXT DEFAULT 'available',
            current_plate TEXT,
            occupied_time TEXT,
            type TEXT DEFAULT 'normal',
            hourly_rate REAL DEFAULT 5.0,
            create_time TEXT DEFAULT CURRENT_TIMESTAMP,
            update_time TEXT DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    return instance().executeQuery(createTableQuery);
}

bool SpaceRepository::insert(const ParkingSpace& space)
{
    QString insertQuery = R"(
        INSERT INTO parking_spaces (location, status, current_plate, occupied_time, type, hourly_rate)
        VALUES (:location, :status, :current_plate, :occupied_time, :type, :hourly_rate)
    )";
    
    QVariantMap params;
    params["location"] = space.getLocation();
    params["status"] = ParkingSpace::statusToString(space.getStatus());
    params["current_plate"] = space.getCurrentPlate();
    params["occupied_time"] = space.getOccupiedTime();
    params["type"] = space.getType();
    params["hourly_rate"] = space.getHourlyRate();
    
    return instance().executeQuery(insertQuery, params);
}

bool SpaceRepository::update(const ParkingSpace& space)
{
    QString updateQuery = R"(
        UPDATE parking_spaces 
        SET location = :location, status = :status, current_plate = :current_plate, 
            occupied_time = :occupied_time, type = :type, hourly_rate = :hourly_rate
        WHERE id = :id
    )";
    
    QVariantMap params;
    params["id"] = space.getId();
    params["location"] = space.getLocation();
    params["status"] = ParkingSpace::statusToString(space.getStatus());
    params["current_plate"] = space.getCurrentPlate();
    params["occupied_time"] = space.getOccupiedTime();
    params["type"] = space.getType();
    params["hourly_rate"] = space.getHourlyRate();
    
    return instance().executeQuery(updateQuery, params);
}

bool SpaceRepository::remove(int id)
{
    QString deleteQuery = "DELETE FROM parking_spaces WHERE id = :id";
    QVariantMap params;
    params["id"] = id;
    
    return instance().executeQuery(deleteQuery, params);
}

ParkingSpace SpaceRepository::findById(int id)
{
    QString selectQuery = "SELECT * FROM parking_spaces WHERE id = :id";
    QVariantMap params;
    params["id"] = id;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery, params);
    
    if (!results.isEmpty()) {
        return mapToSpace(results.first());
    }
    
    return ParkingSpace(); // 返回空对象
}

QList<ParkingSpace> SpaceRepository::findAll()
{
    QString selectQuery = "SELECT * FROM parking_spaces ORDER BY id ASC";
    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery);
    
    QList<ParkingSpace> spaces;
    for (const auto& row : results) {
        spaces.append(mapToSpace(row));
    }
    
    return spaces;
}

QList<ParkingSpace> SpaceRepository::findByStatus(ParkingSpace::Status status)
{
    QString selectQuery = "SELECT * FROM parking_spaces WHERE status = :status ORDER BY id ASC";
    QVariantMap params;
    params["status"] = ParkingSpace::statusToString(status);
    
    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery, params);
    
    QList<ParkingSpace> spaces;
    for (const auto& row : results) {
        spaces.append(mapToSpace(row));
    }
    
    return spaces;
}

QList<ParkingSpace> SpaceRepository::findAvailableSpaces()
{
    return findByStatus(ParkingSpace::AVAILABLE);
}

QList<ParkingSpace> SpaceRepository::findOccupiedSpaces()
{
    return findByStatus(ParkingSpace::OCCUPIED);
}

bool SpaceRepository::updateStatus(int id, ParkingSpace::Status status)
{
    QString updateQuery = "UPDATE parking_spaces SET status = :status WHERE id = :id";
    QVariantMap params;
    params["id"] = id;
    params["status"] = ParkingSpace::statusToString(status);
    
    return instance().executeQuery(updateQuery, params);
}

bool SpaceRepository::occupySpace(int id, const QString& plate)
{
    QString updateQuery = R"(
        UPDATE parking_spaces 
        SET status = 'occupied', current_plate = :plate, occupied_time = strftime('%Y-%m-%d %H:%M:%S', 'now', 'localtime')
        WHERE id = :id AND status = 'available'
    )";
    
    QVariantMap params;
    params["id"] = id;
    params["plate"] = plate;
    
    return instance().executeQuery(updateQuery, params);
}

bool SpaceRepository::releaseSpace(int id)
{
    QString updateQuery = R"(
        UPDATE parking_spaces 
        SET status = 'available', current_plate = NULL, occupied_time = NULL
        WHERE id = :id AND status = 'occupied'
    )";
    
    QVariantMap params;
    params["id"] = id;
    
    return instance().executeQuery(updateQuery, params);
}

bool SpaceRepository::exists(int id)
{
    QString countQuery = "SELECT COUNT(*) as count FROM parking_spaces WHERE id = :id";
    QVariantMap params;
    params["id"] = id;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery, params);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt() > 0;
    }
    
    return false;
}

bool SpaceRepository::existsByLocation(const QString& location)
{
    QString countQuery = "SELECT COUNT(*) as count FROM parking_spaces WHERE location = :location";
    QVariantMap params;
    params["location"] = location;
    
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery, params);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt() > 0;
    }
    
    return false;
}

int SpaceRepository::count()
{
    QString countQuery = "SELECT COUNT(*) as count FROM parking_spaces";
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt();
    }
    
    return 0;
}

int SpaceRepository::countByStatus(ParkingSpace::Status status)
{
    QString countQuery = "SELECT COUNT(*) as count FROM parking_spaces WHERE status = :status";
    QVariantMap params;
    params["status"] = ParkingSpace::statusToString(status);
    
    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery, params);
    
    if (!results.isEmpty()) {
        return results.first()["count"].toInt();
    }
    
    return 0;
}

int SpaceRepository::countAvailable()
{
    return countByStatus(ParkingSpace::AVAILABLE);
}

int SpaceRepository::countOccupied()
{
    return countByStatus(ParkingSpace::OCCUPIED);
}

ParkingSpace SpaceRepository::mapToSpace(const QVariantMap& row)
{
    ParkingSpace space;
    space.setId(row["id"].toInt());
    space.setLocation(row["location"].toString());
    space.setStatus(ParkingSpace::stringToStatus(row["status"].toString()));
    space.setCurrentPlate(row["current_plate"].toString());
    space.setOccupiedTime(row["occupied_time"].toDateTime());
    space.setType(row["type"].toString());
    space.setHourlyRate(row["hourly_rate"].toDouble());
    return space;
}

QVariantMap SpaceRepository::spaceToMap(const ParkingSpace& space)
{
    QVariantMap map;
    map["id"] = space.getId();
    map["location"] = space.getLocation();
    map["status"] = ParkingSpace::statusToString(space.getStatus());
    map["current_plate"] = space.getCurrentPlate();
    map["occupied_time"] = space.getOccupiedTime();
    map["type"] = space.getType();
    map["hourly_rate"] = space.getHourlyRate();
    return map;
}

bool SpaceRepository::executeQuery(const QString& queryStr, const QVariantMap& params)
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

QList<QVariantMap> SpaceRepository::executeQueryWithResults(const QString& queryStr, const QVariantMap& params)
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

QSqlDatabase SpaceRepository::getDatabase()
{
    QString dbPath = QDir::currentPath() + "/data/parking_server.db";
    QString connectionName = QString("space_repo_%1").arg(QDateTime::currentMSecsSinceEpoch());
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return QSqlDatabase();
    }
    return db;
}