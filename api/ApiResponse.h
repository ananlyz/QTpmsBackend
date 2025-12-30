#ifndef APIRESPONSE_H
#define APIRESPONSE_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>

class ApiResponse
{
public:
    // 成功响应
    static QJsonObject success(const QJsonObject& data = QJsonObject());
    static QJsonObject success(const QJsonArray& data);
    static QJsonObject success(const QString& message);
    static QJsonObject success(const QString& message, const QJsonObject& data);
    
    // 错误响应
    static QJsonObject error(int code, const QString& message);
    static QJsonObject error(const QString& message);
    static QJsonObject badRequest(const QString& message = "Bad Request");
    static QJsonObject notFound(const QString& message = "Not Found");
    static QJsonObject unauthorized(const QString& message = "Unauthorized");
    static QJsonObject serverError(const QString& message = "Internal Server Error");
    
    // 业务错误
    static QJsonObject invalidParameter(const QString& param);
    static QJsonObject resourceNotFound(const QString& resource);
    static QJsonObject operationFailed(const QString& operation);
    
private:
    static QJsonObject createResponse(int code, const QString& message, const QJsonValue& data = QJsonValue());
};

#endif // APIRESPONSE_H
