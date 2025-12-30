#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include "Middleware.h"
#include <QString>

class ErrorHandler : public Middleware
{
public:
    // 处理请求中的异常
    bool handle(HttpRequest& request, HttpResponse& response) override;
    
private:
    // 记录错误日志
    void logError(const QString& error, const QString& path);
};

#endif // ERRORHANDLER_H