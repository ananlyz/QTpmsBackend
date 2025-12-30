#ifndef PARKINGRECORD_H
#define PARKINGRECORD_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QJsonObject>

class ParkingRecord
{
public:
    ParkingRecord();
    ParkingRecord(const QString& plate, int spaceId);
    
    // Getter和Setter
    int getId() const { return id; }
    void setId(int value) { id = value; }
    
    QString getPlate() const { return plate; }
    void setPlate(const QString& value) { plate = value; }
    
    int getSpaceId() const { return spaceId; }
    void setSpaceId(int value) { spaceId = value; }
    
    QDateTime getEnterTime() const { return enterTime; }
    void setEnterTime(const QDateTime& value) { enterTime = value; }
    
    QDateTime getExitTime() const { return exitTime; }
    void setExitTime(const QDateTime& value) { exitTime = value; }
    
    double getFee() const { return fee; }
    void setFee(double value) { fee = value; }
    
    bool getIsPaid() const { return isPaid; }
    void setIsPaid(bool value) { isPaid = value; }
    
    QDateTime getPayTime() const { return payTime; }
    void setPayTime(const QDateTime& value) { payTime = value; }
    
    QString getPayMethod() const { return payMethod; }
    void setPayMethod(const QString& value) { payMethod = value; }
    
    // 计算停车时长（分钟）
    qint64 getParkingDuration() const;
    
    // 计算停车费用
    double calculateFee(double hourlyRate = 5.0) const;
    
    // JSON转换
    QJsonObject toJson() const;
    static ParkingRecord fromJson(const QJsonObject& json);
    
    // 状态检查
    bool isActive() const { return exitTime.isNull(); }
    bool isCompleted() const { return !exitTime.isNull(); }
    
private:
    int id;               // 记录ID
    QString plate;        // 车牌号
    int spaceId;          // 停车位ID
    QDateTime enterTime;  // 进入时间
    QDateTime exitTime;   // 离开时间
    double fee;           // 费用
    bool isPaid;          // 是否已支付
    QDateTime payTime;    // 支付时间
    QString payMethod;    // 支付方式
};

#endif // PARKINGRECORD_H