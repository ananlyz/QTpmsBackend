#ifndef PARKINGSPACE_H
#define PARKINGSPACE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>

class ParkingSpace
{
public:
    enum Status {
        AVAILABLE = 0,    // 可用
        OCCUPIED = 1,     // 已占用
        RESERVED = 2,     // 预留
        DISABLED = 3      // 禁用
    };
    
    ParkingSpace();
    ParkingSpace(int id, const QString& location, Status status = AVAILABLE);
    
    // Getter和Setter
    int getId() const { return id; }
    void setId(int value) { id = value; }
    
    QString getLocation() const { return location; }
    void setLocation(const QString& value) { location = value; }
    
    Status getStatus() const { return status; }
    void setStatus(Status value) { status = value; }
    
    QString getCurrentPlate() const { return currentPlate; }
    void setCurrentPlate(const QString& value) { currentPlate = value; }
    
    QDateTime getOccupiedTime() const { return occupiedTime; }
    void setOccupiedTime(const QDateTime& value) { occupiedTime = value; }
    
    QString getType() const { return type; }
    void setType(const QString& value) { type = value; }
    
    double getHourlyRate() const { return hourlyRate; }
    void setHourlyRate(double value) { hourlyRate = value; }
    
    // 状态检查
    bool isAvailable() const { return status == AVAILABLE; }
    bool isOccupied() const { return status == OCCUPIED; }
    bool isReserved() const { return status == RESERVED; }
    bool isDisabled() const { return status == DISABLED; }
    
    // 状态转换
    void occupy(const QString& plate);
    void release();
    void reserve();
    void enable();
    void disable();
    
    // JSON转换
    QJsonObject toJson() const;
    static ParkingSpace fromJson(const QJsonObject& json);
    
    // 状态字符串转换
    static QString statusToString(Status status);
    static Status stringToStatus(const QString& statusStr);
    
private:
    int id;                     // 停车位ID
    QString location;           // 位置描述
    Status status;              // 状态
    QString currentPlate;       // 当前停放车辆车牌
    QDateTime occupiedTime;     // 占用时间
    QString type;               // 类型（普通、VIP等）
    double hourlyRate;          // 每小时费率
};

#endif // PARKINGSPACE_H