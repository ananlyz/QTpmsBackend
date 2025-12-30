#ifndef CAR_H
#define CAR_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonValue>

class Car
{
public:
    Car();
    Car(const QString& plate, const QString& type = QString(), const QString& color = QString());
    
    // Getter和Setter
    QString getPlate() const { return plate; }
    void setPlate(const QString& value) { plate = value; }
    
    QString getType() const { return type; }
    void setType(const QString& value) { type = value; }
    
    QString getColor() const { return color; }
    void setColor(const QString& value) { color = value; }
    
    QDateTime getCreateTime() const { return createTime; }
    void setCreateTime(const QDateTime& value) { createTime = value; }
    
    QDateTime getUpdateTime() const { return updateTime; }
    void setUpdateTime(const QDateTime& value) { updateTime = value; }
    
    // JSON转换
    QJsonObject toJson() const;
    static Car fromJson(const QJsonObject& json);
    
    // 验证
    bool isValid() const;
    QString validationError() const;
    
private:
    QString plate;        // 车牌号
    QString type;         // 车辆类型
    QString color;        // 车辆颜色
    QDateTime createTime; // 创建时间
    QDateTime updateTime; // 更新时间
};

#endif // CAR_H