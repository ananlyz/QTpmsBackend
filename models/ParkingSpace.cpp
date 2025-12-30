#include "ParkingSpace.h"

ParkingSpace::ParkingSpace() : id(0), status(AVAILABLE), hourlyRate(5.0)
{
}

ParkingSpace::ParkingSpace(int id, const QString& location, Status status)
    : id(id), location(location), status(status), hourlyRate(5.0)
{
}

void ParkingSpace::occupy(const QString& plate)
{
    if (status == AVAILABLE) {
        status = OCCUPIED;
        currentPlate = plate;
        occupiedTime = QDateTime::currentDateTime();
    }
}

void ParkingSpace::release()
{
    if (status == OCCUPIED) {
        status = AVAILABLE;
        currentPlate.clear();
        occupiedTime = QDateTime();
    }
}

void ParkingSpace::reserve()
{
    if (status == AVAILABLE) {
        status = RESERVED;
    }
}

void ParkingSpace::enable()
{
    if (status == DISABLED) {
        status = AVAILABLE;
    }
}

void ParkingSpace::disable()
{
    if (status != OCCUPIED) {
        status = DISABLED;
        currentPlate.clear();
        occupiedTime = QDateTime();
    }
}

QJsonObject ParkingSpace::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["location"] = location;
    json["status"] = statusToString(status);
    json["currentPlate"] = currentPlate;
    json["occupiedTime"] = occupiedTime.toString(Qt::ISODate);
    json["type"] = type;
    json["hourlyRate"] = hourlyRate;
    json["isAvailable"] = isAvailable();
    json["isOccupied"] = isOccupied();
    return json;
}

ParkingSpace ParkingSpace::fromJson(const QJsonObject& json)
{
    ParkingSpace space;
    space.id = json["id"].toInt();
    space.location = json["location"].toString();
    space.status = stringToStatus(json["status"].toString());
    space.currentPlate = json["currentPlate"].toString();
    space.occupiedTime = QDateTime::fromString(json["occupiedTime"].toString(), Qt::ISODate);
    space.type = json["type"].toString();
    space.hourlyRate = json["hourlyRate"].toDouble(5.0);
    return space;
}

QString ParkingSpace::statusToString(Status status)
{
    switch (status) {
        case AVAILABLE: return "available";
        case OCCUPIED: return "occupied";
        case RESERVED: return "reserved";
        case DISABLED: return "disabled";
        default: return "unknown";
    }
}

ParkingSpace::Status ParkingSpace::stringToStatus(const QString& statusStr)
{
    if (statusStr == "available") return AVAILABLE;
    if (statusStr == "occupied") return OCCUPIED;
    if (statusStr == "reserved") return RESERVED;
    if (statusStr == "disabled") return DISABLED;
    return AVAILABLE;
}