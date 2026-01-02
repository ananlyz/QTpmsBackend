#ifndef PARKINGRECORDREPOSITORY_H
#define PARKINGRECORDREPOSITORY_H

#include "../models/ParkingRecord.h"
#include <QList>
#include <QSqlQuery>
#include <QThreadStorage>
#include <QSqlDatabase>

class ParkingRecordRepository
{
public:
    static ParkingRecordRepository& instance();

    bool insert(const ParkingRecord& record);
    bool update(const ParkingRecord& record);
    bool remove(int id);
    ParkingRecord findById(int id);
    QList<ParkingRecord> findAll(int limit = -1);
    QList<ParkingRecord> findByPlate(const QString& plate);
    QList<ParkingRecord> findActive();
    QList<ParkingRecord> findActiveByPlate(const QString& plate);
    QList<ParkingRecord> findActiveBySpaceId(int spaceId);
    QList<ParkingRecord> findUnpaidByPlateAndSpace(const QString& plate, int spaceId);
    
    // 统计查询方法
    int count();
    int countByDateRange(const QDateTime& startTime, const QDateTime& endTime);
    double sumRevenueByDateRange(const QDateTime& startTime, const QDateTime& endTime);
    int countByPaymentStatus(bool isPaid, const QDateTime& startTime, const QDateTime& endTime);
    double sumRevenueByPaymentStatus(bool isPaid, const QDateTime& startTime, const QDateTime& endTime);

private:
    ParkingRecordRepository() = default;
    ParkingRecordRepository(const ParkingRecordRepository&) = delete;
    ParkingRecordRepository& operator=(const ParkingRecordRepository&) = delete;

    ParkingRecord mapToRecord(const QSqlQuery& query);
    QSqlDatabase getDatabase();
};

#endif // PARKINGRECORDREPOSITORY_H