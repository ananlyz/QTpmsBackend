#include "ParkingRecord.h"
#include "../utils/DateTimeUtil.h"

ParkingRecord::ParkingRecord() : id(0), spaceId(0), fee(0.0), isPaid(false)
{
    enterTime = QDateTime::currentDateTime();
}

ParkingRecord::ParkingRecord(const QString& plate, int spaceId)
    : id(0), plate(plate), spaceId(spaceId), fee(0.0), isPaid(false)
{
    enterTime = QDateTime::currentDateTime();
}

qint64 ParkingRecord::getParkingDuration() const
{
    if (enterTime.isNull()) return 0;
    
    QDateTime endTime = exitTime.isNull() ? QDateTime::currentDateTime() : exitTime;
    return DateTimeUtil::getDurationMinutes(enterTime, endTime);
}

double ParkingRecord::calculateFee(double hourlyRate) const
{
    qint64 durationMinutes = getParkingDuration();
    if (durationMinutes <= 0) return 0.0;
    
    // 不足一小时按一小时计算
    qint64 hours = (durationMinutes + 59) / 60;
    return hours * hourlyRate;
}

QJsonObject ParkingRecord::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["plate"] = plate;
    json["spaceId"] = spaceId;
    json["enterTime"] = enterTime.toString(Qt::ISODate);
    json["exitTime"] = exitTime.toString(Qt::ISODate);
    json["fee"] = fee;
    json["isPaid"] = isPaid;
    json["payTime"] = payTime.toString(Qt::ISODate);
    json["payMethod"] = payMethod;
    json["duration"] = getParkingDuration();
    json["isActive"] = isActive();
    json["isCompleted"] = isCompleted();
    return json;
}

ParkingRecord ParkingRecord::fromJson(const QJsonObject& json)
{
    ParkingRecord record;
    record.id = json["id"].toInt();
    record.plate = json["plate"].toString();
    record.spaceId = json["spaceId"].toInt();
    record.enterTime = QDateTime::fromString(json["enterTime"].toString(), Qt::ISODate);
    record.exitTime = QDateTime::fromString(json["exitTime"].toString(), Qt::ISODate);
    record.fee = json["fee"].toDouble();
    record.isPaid = json["isPaid"].toBool();
    record.payTime = QDateTime::fromString(json["payTime"].toString(), Qt::ISODate);
    record.payMethod = json["payMethod"].toString();
    return record;
}