#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonObject>
#include <QByteArray>
#include <QVariant>

class HttpRequest
{
public:
    QString method;
    QString path;
    QMap<QString, QString> headers;
    QMap<QString, QString> queryParams;
    QMap<QString, QString> pathParams;  // 路径参数
    QByteArray bodyRaw;
    QJsonObject jsonBody;
    QVariantMap context;

    HttpRequest();
    
    void parseQueryString(const QString& queryString);
    QString getHeader(const QString& name) const;
    QString getQueryParam(const QString& name) const;
    QString getPathParam(const QString& name) const;
    QVariant getContext(const QString& key) const;
    void setContext(const QString& key, const QVariant& value);
    
private:
    void parseQueryParam(const QString& key, const QString& value);
};

#endif // HTTPREQUEST_H