#include "ParkingRecordRepository.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QThread>
#include <QThreadStorage>
#include <QDir>
#include "../utils/Logger.h"

ParkingRecordRepository& ParkingRecordRepository::instance()
{
    static ParkingRecordRepository instance;
    return instance;
}

bool ParkingRecordRepository::insert(const ParkingRecord& record)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return false;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO parking_records (plate, space_id, enter_time, exit_time, fee, is_paid, pay_time, pay_method) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(record.getPlate());
    query.addBindValue(record.getSpaceId());
    query.addBindValue(record.getEnterTime());
    query.addBindValue(record.getExitTime().isValid() ? record.getExitTime() : QVariant());
    query.addBindValue(record.getFee());
    query.addBindValue(record.getIsPaid());
    query.addBindValue(record.getPayTime().isValid() ? record.getPayTime() : QVariant());
    query.addBindValue(record.getPayMethod());

    if (!query.exec()) {
        Logger::error(QString("Failed to insert parking record: %1").arg(query.lastError().text()));
        QSqlDatabase::removeDatabase(db.connectionName());
        return false;
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return true;
}

bool ParkingRecordRepository::update(const ParkingRecord& record)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return false;
    }
    QSqlQuery query(db);
    query.prepare("UPDATE parking_records SET plate=?, space_id=?, enter_time=?, exit_time=?, fee=?, is_paid=?, pay_time=?, pay_method=? WHERE id=?");
    query.addBindValue(record.getPlate());
    query.addBindValue(record.getSpaceId());
    query.addBindValue(record.getEnterTime());
    query.addBindValue(record.getExitTime().isValid() ? record.getExitTime() : QVariant());
    query.addBindValue(record.getFee());
    query.addBindValue(record.getIsPaid());
    query.addBindValue(record.getPayTime().isValid() ? record.getPayTime() : QVariant());
    query.addBindValue(record.getPayMethod());
    query.addBindValue(record.getId());

    if (!query.exec()) {
        Logger::error(QString("Failed to update parking record: %1").arg(query.lastError().text()));
        QSqlDatabase::removeDatabase(db.connectionName());
        return false;
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return true;
}

bool ParkingRecordRepository::remove(int id)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return false;
    }
    QSqlQuery query(db);
    query.prepare("DELETE FROM parking_records WHERE id=?");
    query.addBindValue(id);

    if (!query.exec()) {
        Logger::error(QString("Failed to delete parking record: %1").arg(query.lastError().text()));
        QSqlDatabase::removeDatabase(db.connectionName());
        return false;
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return true;
}

ParkingRecord ParkingRecordRepository::findById(int id)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return ParkingRecord();
    }
    QSqlQuery query(db);
    query.prepare("SELECT * FROM parking_records WHERE id=?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        ParkingRecord record = mapToRecord(query);
        QSqlDatabase::removeDatabase(db.connectionName());
        return record;
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return ParkingRecord(); // Return empty record if not found
}

QList<ParkingRecord> ParkingRecordRepository::findAll(int limit)
{
    QList<ParkingRecord> records;
    QString sql = "SELECT * FROM parking_records ORDER BY enter_time DESC";
    if (limit > 0) {
        sql += " LIMIT ?";
    }

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return records;
    }
    {
        QSqlQuery query(db);
        query.prepare(sql);
        if (limit > 0) {
            query.addBindValue(limit);
        }

        if (query.exec()) {
            while (query.next()) {
                records.append(mapToRecord(query));
            }
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return records;
}

QList<ParkingRecord> ParkingRecordRepository::findByPlate(const QString& plate)
{
    QList<ParkingRecord> records;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return records;
    }
    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM parking_records WHERE plate=? ORDER BY enter_time DESC");
        query.addBindValue(plate);

        if (query.exec()) {
            while (query.next()) {
                records.append(mapToRecord(query));
            }
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return records;
}

QList<ParkingRecord> ParkingRecordRepository::findActive()
{
    QList<ParkingRecord> records;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return records;
    }
    QSqlQuery query(db);
    query.prepare("SELECT * FROM parking_records WHERE exit_time IS NULL");

    if (query.exec()) {
        while (query.next()) {
            records.append(mapToRecord(query));
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return records;
}

QList<ParkingRecord> ParkingRecordRepository::findActiveByPlate(const QString& plate)
{
    QList<ParkingRecord> records;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return records;
    }
    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM parking_records WHERE plate=? AND exit_time IS NULL");
        query.addBindValue(plate);

        if (query.exec()) {
            while (query.next()) {
                records.append(mapToRecord(query));
            }
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return records;
}

QList<ParkingRecord> ParkingRecordRepository::findActiveBySpaceId(int spaceId)
{
    QList<ParkingRecord> records;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return records;
    }
    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM parking_records WHERE space_id=? AND exit_time IS NULL");
        query.addBindValue(spaceId);

        if (query.exec()) {
            while (query.next()) {
                records.append(mapToRecord(query));
            }
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return records;
}

QList<ParkingRecord> ParkingRecordRepository::findUnpaidByPlateAndSpace(const QString& plate, int spaceId)
{
    QList<ParkingRecord> records;
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return records;
    }
    {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM parking_records WHERE plate=? AND space_id=? AND is_paid=0 AND exit_time IS NOT NULL ORDER BY exit_time DESC");
        query.addBindValue(plate);
        query.addBindValue(spaceId);

        if (query.exec()) {
            while (query.next()) {
                records.append(mapToRecord(query));
            }
        }
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return records;
}

int ParkingRecordRepository::count()
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        Logger::error("Database connection is not open");
        return 0;
    }
    QSqlQuery query(db);
    query.prepare("SELECT COUNT(*) FROM parking_records");
    if (query.exec() && query.next()) {
        int result = query.value(0).toInt();
        QSqlDatabase::removeDatabase(db.connectionName());
        return result;
    }
    QSqlDatabase::removeDatabase(db.connectionName());
    return 0;
}

ParkingRecord ParkingRecordRepository::mapToRecord(const QSqlQuery& query)
{
    ParkingRecord record;
    record.setId(query.value("id").toInt());
    record.setPlate(query.value("plate").toString());
    record.setSpaceId(query.value("space_id").toInt());
    record.setEnterTime(query.value("enter_time").toDateTime());
    if (!query.value("exit_time").isNull()) {
        record.setExitTime(query.value("exit_time").toDateTime());
    }
    record.setFee(query.value("fee").toDouble());
    record.setIsPaid(query.value("is_paid").toBool());
    if (!query.value("pay_time").isNull()) {
        record.setPayTime(query.value("pay_time").toDateTime());
    }
    record.setPayMethod(query.value("pay_method").toString());
    return record;
}

QSqlDatabase ParkingRecordRepository::getDatabase()
{
    QString dbPath = QDir::currentPath() + "/data/parking_server.db";
    QString connectionName = QString("record_repo_%1").arg(QDateTime::currentMSecsSinceEpoch());
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        Logger::error("Failed to open database:" + db.lastError().text());
        return QSqlDatabase();
    }
    return db;
}