#include "JsonBodyParser.h"

bool JsonBodyParser::handle(HttpRequest& request, HttpResponse& response)
{
    QString contentType = request.getHeader("content-type");
    
    if (!isJsonContentType(contentType)) {
        return true; // 不是JSON内容，继续处理
    }
    
    if (request.bodyRaw.isEmpty()) {
        return true; // 没有body，继续处理
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(request.bodyRaw, &error);
    
    if (error.error != QJsonParseError::NoError) {
        response.badRequest("Invalid JSON format");
        return false; // 中断处理
    }
    
    if (!doc.isObject()) {
        response.badRequest("JSON must be an object");
        return false; // 中断处理
    }
    
    request.jsonBody = doc.object();
    return true; // 继续处理
}

bool JsonBodyParser::isJsonContentType(const QString& contentType) const
{
    return contentType.contains("application/json", Qt::CaseInsensitive);
}