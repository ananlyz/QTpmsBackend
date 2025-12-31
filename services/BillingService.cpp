#include "BillingService.h"
#include "../api/ApiResponse.h"
#include "../utils/Logger.h"
#include "../dao/ParkingRecordRepository.h"
#include "../dao/SpaceRepository.h"
#include <QRegularExpression>
#include <QDateTime>

BillingService& BillingService::instance()
{
    static BillingService instance;
    return instance;
}

QJsonObject BillingService::startParking(const QString& plate, int spaceId)
{
    try {
        if (!validatePlate(plate)) {
            return ApiResponse::error("Invalid plate number");
        }

        if (!SpaceRepository::instance().exists(spaceId)) {
            return ApiResponse::error("Space not found");
        }

        ParkingSpace space = SpaceRepository::instance().findById(spaceId);
        if (space.getStatus() != ParkingSpace::AVAILABLE) {
            return ApiResponse::error("Space not available");
        }

        QList<ParkingRecord> activeRecords = ParkingRecordRepository::instance().findActiveByPlate(plate);
        if (!activeRecords.isEmpty()) {
            return ApiResponse::error("Car already parking");
        }

        if (!SpaceRepository::instance().occupySpace(spaceId, plate)) {
            return ApiResponse::error("Failed to occupy space");
        }

        ParkingRecord record;
        record.setPlate(plate);
        record.setSpaceId(spaceId);
        record.setEnterTime(QDateTime::currentDateTime());
        record.setIsPaid(false);

        if (ParkingRecordRepository::instance().insert(record)) {
            return ApiResponse::success("Parking started", recordToJson(record));
        } else {
            SpaceRepository::instance().releaseSpace(spaceId);
            return ApiResponse::error("Failed to create record");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error starting parking: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::endParking(int recordId)
{
    try {
        ParkingRecord record = ParkingRecordRepository::instance().findById(recordId);
        if (record.getId() == 0) {
            return ApiResponse::error("Record not found");
        }

        if (record.getExitTime().isValid()) {
            return ApiResponse::error("Parking already ended");
        }

        ParkingSpace space = SpaceRepository::instance().findById(record.getSpaceId());
        QDateTime endTime = QDateTime::currentDateTime();
        double fee = record.calculateFee(space.getHourlyRate());

        record.setExitTime(endTime);
        record.setFee(fee);

        if (ParkingRecordRepository::instance().update(record)) {
            SpaceRepository::instance().releaseSpace(record.getSpaceId());
            return ApiResponse::success("Parking ended", recordToJson(record));
        } else {
            return ApiResponse::error("Failed to update record");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error ending parking: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::getParkingRecord(int recordId)
{
    try {
        ParkingRecord record = ParkingRecordRepository::instance().findById(recordId);
        if (record.getId() == 0) {
            return ApiResponse::error("Record not found");
        }
        return ApiResponse::success(recordToJson(record));
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting record: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonArray BillingService::getParkingRecordsByPlate(const QString& plate)
{
    try {
        if (!validatePlate(plate)) {
            return QJsonArray();
        }
        QList<ParkingRecord> records = ParkingRecordRepository::instance().findByPlate(plate);
        return recordsToJson(records);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting records by plate: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonArray BillingService::getActiveParkingRecords()
{
    try {
        QList<ParkingRecord> records = ParkingRecordRepository::instance().findActive();
        return recordsToJson(records);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting active records: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonArray BillingService::getAllParkingRecords(int limit)
{
    try {
        QList<ParkingRecord> records = ParkingRecordRepository::instance().findAll(limit);
        return recordsToJson(records);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting all records: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonObject BillingService::calculateFee(int recordId)
{
    try {
        ParkingRecord record = ParkingRecordRepository::instance().findById(recordId);
        if (record.getId() == 0) {
            return ApiResponse::error("Record not found");
        }
        ParkingSpace space = SpaceRepository::instance().findById(record.getSpaceId());
        double fee = record.calculateFee(space.getHourlyRate());
        QJsonObject result;
        result["fee"] = fee;
        return ApiResponse::success(result);
    } catch (const std::exception& e) {
        Logger::error(QString("Error calculating fee: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::processPayment(int recordId, double amount, const QString& paymentMethod)
{
    try {
        ParkingRecord record = ParkingRecordRepository::instance().findById(recordId);
        if (record.getId() == 0) {
            return ApiResponse::error("Record not found");
        }
        if (!validatePaymentMethod(paymentMethod)) {
            return ApiResponse::error("Invalid payment method");
        }
        record.setFee(amount);
        record.setIsPaid(true);
        record.setPayMethod(paymentMethod);
        record.setPayTime(QDateTime::currentDateTime());
        if (ParkingRecordRepository::instance().update(record)) {
            return ApiResponse::success("Payment processed", recordToJson(record));
        } else {
            return ApiResponse::error("Failed to process payment");
        }
    } catch (const std::exception& e) {
        Logger::error(QString("Error processing payment: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::getRevenueStatistics(const QDateTime& startTime, const QDateTime& endTime)
{
    try {
        QJsonObject stats;
        stats["totalRevenue"] = 0.0; // TODO: implement actual calculation
        stats["period"] = QString("%1 to %2").arg(startTime.toString(), endTime.toString());
        return ApiResponse::success(stats);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting revenue stats: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::getParkingStatistics(const QDateTime& startTime, const QDateTime& endTime)
{
    try {
        QJsonObject stats;
        stats["totalParkings"] = 0; // TODO
        return ApiResponse::success(stats);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting parking stats: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::getPaymentStatistics(const QDateTime& startTime, const QDateTime& endTime)
{
    try {
        QJsonObject stats;
        stats["totalPayments"] = 0; // TODO
        return ApiResponse::success(stats);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting payment stats: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonArray BillingService::getUnpaidRecords()
{
    try {
        QList<ParkingRecord> records = ParkingRecordRepository::instance().findAll();
        QJsonArray unpaid;
        for (const auto& record : records) {
            if (!record.getIsPaid()) {
                unpaid.append(recordToJson(record));
            }
        }
        return unpaid;
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting unpaid records: %1").arg(e.what()));
        return QJsonArray();
    }
}

QJsonObject BillingService::getUnpaidAmount(const QString& plate)
{
    try {
        QList<ParkingRecord> records = ParkingRecordRepository::instance().findByPlate(plate);
        double total = 0.0;
        for (const auto& record : records) {
            if (!record.getIsPaid()) {
                total += record.getFee();
            }
        }
        QJsonObject result;
        result["unpaidAmount"] = total;
        return ApiResponse::success(result);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting unpaid amount: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::sendPaymentReminder(const QString& plate)
{
    try {
        return ApiResponse::success("Reminder sent", QJsonObject());
    } catch (const std::exception& e) {
        Logger::error(QString("Error sending reminder: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

QJsonObject BillingService::getCurrentRates()
{
    try {
        QJsonObject rates;
        rates["defaultRate"] = 5.0;
        return ApiResponse::success(rates);
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting rates: %1").arg(e.what()));
        return ApiResponse::error("Internal server error");
    }
}

double BillingService::getHourlyRate(int spaceId)
{
    try {
        ParkingSpace space = SpaceRepository::instance().findById(spaceId);
        return space.getHourlyRate();
    } catch (const std::exception& e) {
        Logger::error(QString("Error getting rate: %1").arg(e.what()));
        return 0.0;
    }
}

QJsonObject BillingService::recordToJson(const ParkingRecord& record)
{
    QJsonObject json;
    json["id"] = record.getId();
    json["plate"] = record.getPlate();
    json["spaceId"] = record.getSpaceId();
    json["startTime"] = record.getEnterTime().toString(Qt::ISODate);
    json["endTime"] = record.getExitTime().isValid() ? record.getExitTime().toString(Qt::ISODate) : "";
    json["fee"] = record.getFee();
    json["paidAmount"] = record.getFee(); // Assuming fee is paid amount
    json["paymentStatus"] = record.getIsPaid() ? "paid" : "unpaid";
    json["paymentMethod"] = record.getPayMethod();
    json["createdAt"] = record.getEnterTime().toString(Qt::ISODate); // Using enter time as created
    return json;
}

QJsonArray BillingService::recordsToJson(const QList<ParkingRecord>& records)
{
    QJsonArray array;
    for (const ParkingRecord& record : records) {
        array.append(recordToJson(record));
    }
    return array;
}

bool BillingService::validatePlate(const QString& plate)
{
    // 第1位汉字 + 第2位大写字母 + 后5位大写字母或数字
    static QRegularExpression reg(
        R"(^[\x{4e00}-\x{9fa5}][A-Z][A-Z0-9]{5}$)"
    );
    
    return reg.match(plate).hasMatch();
}

bool BillingService::validatePaymentMethod(const QString& method)
{
    QStringList validMethods = {"cash", "card", "mobile", "online"};
    return validMethods.contains(method.toLower());
}

double BillingService::calculateParkingFee(const QDateTime& startTime, const QDateTime& endTime, double hourlyRate)
{
    qint64 seconds = startTime.secsTo(endTime);
    double hours = seconds / 3600.0;
    if (hours < 1) hours = 1;
    double fee = hours * hourlyRate;
    return qRound(fee * 100) / 100.0;
}

bool BillingService::isOverdue(const ParkingRecord& record)
{
    if (record.getExitTime().isValid() && !record.getIsPaid()) {
        QDateTime now = QDateTime::currentDateTime();
        return record.getExitTime().addDays(1) < now;
    }
    return false;
}

QJsonObject BillingService::generatePaymentReminder(const QString& plate, double amount)
{
    QJsonObject reminder;
    reminder["plate"] = plate;
    reminder["unpaidAmount"] = amount;
    reminder["reminderDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    reminder["message"] = QString("Unpaid parking fee: %1").arg(amount, 0, 'f', 2);
    return reminder;
}
