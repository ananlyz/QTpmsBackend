#include "DateTimeUtil.h"

qint64 DateTimeUtil::currentTimestamp()
{
    return QDateTime::currentMSecsSinceEpoch();
}

qint64 DateTimeUtil::currentTimestampSeconds()
{
    return QDateTime::currentSecsSinceEpoch();
}

QString DateTimeUtil::formatDateTime(const QDateTime& dateTime, const QString& format)
{
    return dateTime.toString(format);
}

QString DateTimeUtil::formatCurrentDateTime(const QString& format)
{
    return formatDateTime(QDateTime::currentDateTime(), format);
}

QDateTime DateTimeUtil::parseDateTime(const QString& dateTimeStr, const QString& format)
{
    return QDateTime::fromString(dateTimeStr, format);
}

qint64 DateTimeUtil::getDurationMinutes(const QDateTime& start, const QDateTime& end)
{
    return start.secsTo(end) / 60;
}

qint64 DateTimeUtil::getDurationHours(const QDateTime& start, const QDateTime& end)
{
    return start.secsTo(end) / 3600;
}

qint64 DateTimeUtil::getDurationDays(const QDateTime& start, const QDateTime& end)
{
    return start.daysTo(end);
}

bool DateTimeUtil::isSameDay(const QDateTime& dateTime1, const QDateTime& dateTime2)
{
    return dateTime1.date() == dateTime2.date();
}

bool DateTimeUtil::isToday(const QDateTime& dateTime)
{
    return isSameDay(dateTime, QDateTime::currentDateTime());
}