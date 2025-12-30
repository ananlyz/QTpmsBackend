#ifndef JSONUTIL_H
#define JSONUTIL_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QString>
#include <QVariant>
#include <QMap>

class JsonUtil
{
public:
    // JSON字符串解析
    static QJsonObject parseJson(const QString& jsonStr);
    static QJsonArray parseJsonArray(const QString& jsonStr);
    static bool isValidJson(const QString& jsonStr);
    
    // JSON对象操作
    static QString toString(const QJsonObject& obj);
    static QString toString(const QJsonArray& array);
    static QByteArray toByteArray(const QJsonObject& obj);
    static QByteArray toByteArray(const QJsonArray& array);
    
    // 类型转换
    static QJsonObject variantMapToJsonObject(const QVariantMap& map);
    static QVariantMap jsonObjectToVariantMap(const QJsonObject& obj);
    static QJsonArray stringListToJsonArray(const QStringList& list);
    static QStringList jsonArrayToStringList(const QJsonArray& array);
    
    // 安全获取值
    static QString getString(const QJsonObject& obj, const QString& key, const QString& defaultValue = QString());
    static int getInt(const QJsonObject& obj, const QString& key, int defaultValue = 0);
    static double getDouble(const QJsonObject& obj, const QString& key, double defaultValue = 0.0);
    static bool getBool(const QJsonObject& obj, const QString& key, bool defaultValue = false);
    static QJsonObject getObject(const QJsonObject& obj, const QString& key);
    static QJsonArray getArray(const QJsonObject& obj, const QString& key);
    
private:
    JsonUtil() = default;
};

#endif // JSONUTIL_H