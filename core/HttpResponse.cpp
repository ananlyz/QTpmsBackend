#include "HttpResponse.h"
#include <QJsonDocument>

HttpResponse::HttpResponse() : statusCode(200)
{
    headers["Content-Type"] = "application/json";
    headers["Server"] = "ParkingServer/1.0";
}

void HttpResponse::ok(const QJsonObject& data)
{
    statusCode = 200;
    json(createApiResponse(0, "success", data));
}

void HttpResponse::ok(const QString& message)
{
    statusCode = 200;
    json(createApiResponse(0, message));
}

void HttpResponse::badRequest(const QString& message)
{
    statusCode = 400;
    json(createApiResponse(4001, message));
}

void HttpResponse::notFound(const QString& message)
{
    statusCode = 404;
    json(createApiResponse(4041, message));
}

void HttpResponse::serverError(const QString& message)
{
    statusCode = 500;
    json(createApiResponse(5001, message));
}

void HttpResponse::unauthorized(const QString& message)
{
    statusCode = 401;
    json(createApiResponse(4011, message));
}

void HttpResponse::json(const QJsonObject& data)
{
    headers["Content-Type"] = "application/json";
    QJsonDocument doc(data);
    body = doc.toJson(QJsonDocument::Compact);
}

void HttpResponse::json(const QJsonArray& data)
{
    headers["Content-Type"] = "application/json";
    QJsonDocument doc(data);
    body = doc.toJson(QJsonDocument::Compact);
}

void HttpResponse::text(const QString& text)
{
    headers["Content-Type"] = "text/plain; charset=utf-8";
    body = text.toUtf8();
}

void HttpResponse::html(const QString& html)
{
    headers["Content-Type"] = "text/html; charset=utf-8";
    body = html.toUtf8();
}

void HttpResponse::setHeader(const QString& name, const QString& value)
{
    headers[name] = value;
}

QString HttpResponse::getHeader(const QString& name) const
{
    return headers.value(name);
}

void HttpResponse::setStatusCode(int code)
{
    statusCode = code;
}

QJsonObject HttpResponse::createApiResponse(int code, const QString& message, const QJsonValue& data)
{
    QJsonObject response;
    response["code"] = code;
    response["msg"] = message;
    if (!data.isUndefined()) {
        response["data"] = data;
    }
    return response;
}