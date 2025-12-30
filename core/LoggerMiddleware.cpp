#include "LoggerMiddleware.h"
#include <QCoreApplication>

bool LoggerMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString log = formatLog(request.method, request.path, timestamp);
    
    qDebug() << "[HTTP]" << log;
    
    // 记录请求开始时间，用于后续性能分析
    request.setContext("request_start_time", timestamp);
    
    return true; // 继续处理
}

QString LoggerMiddleware::formatLog(const QString& method, const QString& path, const QString& timestamp)
{
    return QString("[%1] %2 %3").arg(timestamp, method, path);
}