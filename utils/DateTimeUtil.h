#ifndef DATETIMEUTIL_H
#define DATETIMEUTIL_H

#include <QObject>
#include <QDateTime>
#include <QString>

class DateTimeUtil
{
public:
    // 获取当前时间戳（毫秒）
    static qint64 currentTimestamp();
    static qint64 currentTimestampSeconds();
    
    // 时间格式化
    static QString formatDateTime(const QDateTime& dateTime, const QString& format = "yyyy-MM-dd hh:mm:ss");
    static QString formatCurrentDateTime(const QString& format = "yyyy-MM-dd hh:mm:ss");
    
    // 字符串转时间
    static QDateTime parseDateTime(const QString& dateTimeStr, const QString& format = "yyyy-MM-dd hh:mm:ss");
    
    // 时间计算
    static qint64 getDurationMinutes(const QDateTime& start, const QDateTime& end);
    static qint64 getDurationHours(const QDateTime& start, const QDateTime& end);
    static qint64 getDurationDays(const QDateTime& start, const QDateTime& end);
    
    // 时间比较
    static bool isSameDay(const QDateTime& dateTime1, const QDateTime& dateTime2);
    static bool isToday(const QDateTime& dateTime);
    
private:
    DateTimeUtil() = default;
};

#endif // DATETIMEUTIL_H