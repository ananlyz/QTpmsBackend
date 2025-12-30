#include "HttpRequest.h"
#include <QUrlQuery>
#include <QUrl>

HttpRequest::HttpRequest()
{
}

void HttpRequest::parseQueryString(const QString& queryString)
{
    if (queryString.isEmpty()) return;
    
    QUrlQuery query(queryString);
    for (const auto& item : query.queryItems()) {
        queryParams[item.first] = item.second;
    }
}

QString HttpRequest::getHeader(const QString& name) const
{
    return headers.value(name.toLower());
}

QString HttpRequest::getQueryParam(const QString& name) const
{
    return queryParams.value(name);
}

QString HttpRequest::getPathParam(const QString& name) const
{
    return pathParams.value(name);
}

QVariant HttpRequest::getContext(const QString& key) const
{
    return context.value(key);
}

void HttpRequest::setContext(const QString& key, const QVariant& value)
{
    context[key] = value;
}