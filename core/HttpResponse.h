#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>

class HttpResponse
{
public:
    int statusCode;
    QMap<QString, QString> headers;
    QByteArray body;

    HttpResponse();
    
    // 快速响应方法
    void ok(const QJsonObject& data = QJsonObject());
    void ok(const QString& message);
    void badRequest(const QString& message = "Bad Request");
    void notFound(const QString& message = "Not Found");
    void serverError(const QString& message = "Internal Server Error");
    void unauthorized(const QString& message = "Unauthorized");
    
    // 设置响应数据
    void json(const QJsonObject& data);
    void json(const QJsonArray& data);
    void text(const QString& text);
    void html(const QString& html);
    
    // 工具方法
    void setHeader(const QString& name, const QString& value);
    QString getHeader(const QString& name) const;
    void setStatusCode(int code);
    
private:
    QJsonObject createApiResponse(int code, const QString& message, const QJsonValue& data = QJsonValue());
};

#endif // HTTPRESPONSE_H