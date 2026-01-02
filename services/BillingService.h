#ifndef BILLINGSERVICE_H
#define BILLINGSERVICE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "../models/ParkingRecord.h"
#include "../dao/SpaceRepository.h"
#include "SpaceService.h"

class BillingService : public QObject
{
    Q_OBJECT

public:
    static BillingService& instance();

    // 停车记录管理
    QJsonObject startParking(const QString& plate, int spaceId);
    QJsonObject endParking(int recordId);
    QJsonObject getParkingRecord(int recordId);
    QJsonArray getParkingRecordsByPlate(const QString& plate);
    QJsonArray getActiveParkingRecords();
    QJsonArray getAllParkingRecords(int limit = 100);
    
    // 计费管理
    QJsonObject calculateFee(int recordId);
    QJsonObject processPayment(int recordId, double amount, const QString& paymentMethod);
    
    // 查询统计
    QJsonObject getRevenueStatistics(const QDateTime& startTime, const QDateTime& endTime);
    QJsonObject getParkingStatistics(const QDateTime& startTime, const QDateTime& endTime);
    QJsonObject getPaymentStatistics(const QDateTime& startTime, const QDateTime& endTime);
    
    // 欠费管理
    QJsonArray getUnpaidRecords();
    QJsonObject getUnpaidAmount(const QString& plate);
    QJsonObject sendPaymentReminder(const QString& plate);
    
    // 队列处理
    QJsonObject processQueueForAvailableSpaces();
    
    // 费率管理
    QJsonObject getCurrentRates();
    double getHourlyRate(int spaceId);

signals:
    void parkingStarted(int recordId, const QString& plate, int spaceId);
    void parkingEnded(int recordId, double fee);
    void paymentProcessed(int recordId, double amount);
    void paymentOverdue(const QString& plate, double amount);

private:
    BillingService() = default;
    BillingService(const BillingService&) = delete;
    BillingService& operator=(const BillingService&) = delete;
    
    QJsonObject recordToJson(const ParkingRecord& record);
    QJsonArray recordsToJson(const QList<ParkingRecord>& records);
    bool validatePlate(const QString& plate);
    bool validatePaymentMethod(const QString& method);
    double calculateParkingFee(const QDateTime& startTime, const QDateTime& endTime, double hourlyRate);
    bool isOverdue(const ParkingRecord& record);
    QJsonObject generatePaymentReminder(const QString& plate, double amount);
};

#endif // BILLINGSERVICE_H
