#include "JsonUtil.h"
#include <QJsonParseError>
#include <QDebug>

QJsonObject JsonUtil::parseJson(const QString& jsonStr)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return QJsonObject();
    }
    
    return doc.object();
}

QJsonArray JsonUtil::parseJsonArray(const QString& jsonStr)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return QJsonArray();
    }
    
    return doc.array();
}

bool JsonUtil::isValidJson(const QString& jsonStr)
{
    QJsonParseError error;
    QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    return error.error == QJsonParseError::NoError;
}

QString JsonUtil::toString(const QJsonObject& obj)
{
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

QString JsonUtil::toString(const QJsonArray& array)
{
    QJsonDocument doc(array);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray JsonUtil::toByteArray(const QJsonObject& obj)
{
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

QByteArray JsonUtil::toByteArray(const QJsonArray& array)
{
    QJsonDocument doc(array);
    return doc.toJson(QJsonDocument::Compact);
}

QJsonObject JsonUtil::variantMapToJsonObject(const QVariantMap& map)
{
    return QJsonObject::fromVariantMap(map);
}

QVariantMap JsonUtil::jsonObjectToVariantMap(const QJsonObject& obj)
{
    return obj.toVariantMap();
}

QJsonArray JsonUtil::stringListToJsonArray(const QStringList& list)
{
    QJsonArray array;
    for (const QString& str : list) {
        array.append(str);
    }
    return array;
}

QStringList JsonUtil::jsonArrayToStringList(const QJsonArray& array)
{
    QStringList list;
    for (const QJsonValue& value : array) {
        if (value.isString()) {
            list.append(value.toString());
        }
    }
    return list;
}

QString JsonUtil::getString(const QJsonObject& obj, const QString& key, const QString& defaultValue)
{
    return obj.value(key).toString(defaultValue);
}

int JsonUtil::getInt(const QJsonObject& obj, const QString& key, int defaultValue)
{
    return obj.value(key).toInt(defaultValue);
}

double JsonUtil::getDouble(const QJsonObject& obj, const QString& key, double defaultValue)
{
    return obj.value(key).toDouble(defaultValue);
}

bool JsonUtil::getBool(const QJsonObject& obj, const QString& key, bool defaultValue)
{
    return obj.value(key).toBool(defaultValue);
}

QJsonObject JsonUtil::getObject(const QJsonObject& obj, const QString& key)
{
    return obj.value(key).toObject();
}

QJsonArray JsonUtil::getArray(const QJsonObject& obj, const QString& key)
{
    return obj.value(key).toArray();
}