#include "QueueRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariantMap>
#include <QDebug>
#include <QThread>
#include <QThreadStorage>
#include <QDir>

QueueRepository& QueueRepository::instance()
{
    static QueueRepository instance;
    return instance;
}

bool QueueRepository::insert(const QueueItem& item)
{
    QString insertQuery = R"(
        INSERT OR IGNORE INTO parking_queue (plate, queue_time)
        VALUES (:plate, :queue_time)
    )";

    QVariantMap params;
    params["plate"] = item.plate;
    params["queue_time"] = item.queueTime;

    return instance().executeQuery(insertQuery, params);
}

bool QueueRepository::remove(const QString& plate)
{
    QString deleteQuery = "DELETE FROM parking_queue WHERE plate = :plate";
    QVariantMap params;
    params["plate"] = plate;

    return instance().executeQuery(deleteQuery, params);
}

QueueItem QueueRepository::findFirst()
{
    QString selectQuery = "SELECT * FROM parking_queue ORDER BY queue_time ASC LIMIT 1";

    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery);

    if (!results.isEmpty()) {
        return mapToItem(results.first());
    }

    return QueueItem();
}

QList<QueueItem> QueueRepository::findAll()
{
    QString selectQuery = "SELECT * FROM parking_queue ORDER BY queue_time ASC";

    QList<QVariantMap> results = instance().executeQueryWithResults(selectQuery);

    QList<QueueItem> items;
    for (const auto& result : results) {
        items.append(mapToItem(result));
    }

    return items;
}

bool QueueRepository::exists(const QString& plate)
{
    QString countQuery = "SELECT COUNT(*) as count FROM parking_queue WHERE plate = :plate";
    QVariantMap params;
    params["plate"] = plate;

    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery, params);

    if (!results.isEmpty()) {
        return results.first()["count"].toInt() > 0;
    }

    return false;
}

int QueueRepository::count()
{
    QString countQuery = "SELECT COUNT(*) as count FROM parking_queue";

    QList<QVariantMap> results = instance().executeQueryWithResults(countQuery);

    if (!results.isEmpty()) {
        return results.first()["count"].toInt();
    }

    return 0;
}

QueueItem QueueRepository::mapToItem(const QVariantMap& map)
{
    QueueItem item;
    item.id = map["id"].toInt();
    item.plate = map["plate"].toString();
    item.queueTime = map["queue_time"].toDateTime();
    return item;
}

QueueItem QueueRepository::mapToItem(const QSqlQuery& query)
{
    QueueItem item;
    item.id = query.value("id").toInt();
    item.plate = query.value("plate").toString();
    item.queueTime = query.value("queue_time").toDateTime();
    return item;
}

QSqlDatabase QueueRepository::getDatabase()
{
    QString connectionName = QString("queue_repo_%1").arg((quintptr)QThread::currentThreadId());

    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen()) {
            return db;
        } else {
            QSqlDatabase::removeDatabase(connectionName);
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    QString dbPath = QDir::currentPath() + "/data/parking_server.db";
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Failed to open database in QueueRepository:" << db.lastError().text();
    }

    return db;
}

bool QueueRepository::executeQuery(const QString& queryStr, const QVariantMap& params)
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

QList<QVariantMap> QueueRepository::executeQueryWithResults(const QString& queryStr, const QVariantMap& params)
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