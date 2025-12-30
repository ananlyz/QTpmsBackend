#ifndef JSONBODYPARSER_H
#define JSONBODYPARSER_H

#include "Middleware.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

class JsonBodyParser : public Middleware
{
public:
    bool handle(HttpRequest& request, HttpResponse& response) override;
    
private:
    bool isJsonContentType(const QString& contentType) const;
};

#endif // JSONBODYPARSER_H