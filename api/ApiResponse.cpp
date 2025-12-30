#include "ApiResponse.h"

QJsonObject ApiResponse::success(const QJsonObject& data)
{
    return createResponse(0, "success", data);
}

QJsonObject ApiResponse::success(const QJsonArray& data)
{
    return createResponse(0, "success", data);
}

QJsonObject ApiResponse::success(const QString& message)
{
    return createResponse(0, message);
}

QJsonObject ApiResponse::success(const QString& message, const QJsonObject& data)
{
    return createResponse(0, message, data);
}

QJsonObject ApiResponse::error(int code, const QString& message)
{
    return createResponse(code, message);
}

QJsonObject ApiResponse::error(const QString& message)
{
    return createResponse(1001, message);
}

QJsonObject ApiResponse::badRequest(const QString& message)
{
    return createResponse(4001, message);
}

QJsonObject ApiResponse::notFound(const QString& message)
{
    return createResponse(4041, message);
}

QJsonObject ApiResponse::unauthorized(const QString& message)
{
    return createResponse(4011, message);
}

QJsonObject ApiResponse::serverError(const QString& message)
{
    return createResponse(5001, message);
}

QJsonObject ApiResponse::invalidParameter(const QString& param)
{
    return createResponse(4002, QString("Invalid parameter: %1").arg(param));
}

QJsonObject ApiResponse::resourceNotFound(const QString& resource)
{
    return createResponse(4042, QString("Resource not found: %1").arg(resource));
}

QJsonObject ApiResponse::operationFailed(const QString& operation)
{
    return createResponse(5002, QString("Operation failed: %1").arg(operation));
}

QJsonObject ApiResponse::createResponse(int code, const QString& message, const QJsonValue& data)
{
    QJsonObject response;
    response["code"] = code;
    response["msg"] = message;
    if (!data.isUndefined()) {
        response["data"] = data;
    }
    return response;
}
