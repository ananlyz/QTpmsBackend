#include "Car.h"
#include <QJsonDocument>

Car::Car()
{
    createTime = QDateTime::currentDateTime();
    updateTime = createTime;
}

Car::Car(const QString& plate, const QString& type, const QString& color)
    : plate(plate), type(type), color(color)
{
    createTime = QDateTime::currentDateTime();
    updateTime = createTime;
}

QJsonObject Car::toJson() const
{
    QJsonObject json;
    json["plate"] = plate;
    json["type"] = type;
    json["color"] = color;
    json["createTime"] = createTime.toString(Qt::ISODate);
    json["updateTime"] = updateTime.toString(Qt::ISODate);
    return json;
}

Car Car::fromJson(const QJsonObject& json)
{
    Car car;
    car.plate = json["plate"].toString();
    car.type = json["type"].toString();
    car.color = json["color"].toString();
    car.createTime = QDateTime::fromString(json["createTime"].toString(), Qt::ISODate);
    car.updateTime = QDateTime::fromString(json["updateTime"].toString(), Qt::ISODate);
    return car;
}

bool Car::isValid() const
{
    return !plate.isEmpty() && plate.length() >= 5 && plate.length() <= 10;
}

QString Car::validationError() const
{
    if (plate.isEmpty()) {
        return "车牌号不能为空";
    }
    if (plate.length() < 5 || plate.length() > 10) {
        return "车牌号长度必须在5-10位之间";
    }
    return QString();
}