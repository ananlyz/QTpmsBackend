#ifndef LOGGERMIDDLEWARE_H
#define LOGGERMIDDLEWARE_H

#include "Middleware.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>

class LoggerMiddleware : public Middleware
{
public:
    bool handle(HttpRequest& request, HttpResponse& response) override;
    
private:
    QString formatLog(const QString& method, const QString& path, const QString& timestamp);
};

#endif // LOGGERMIDDLEWARE_H