#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "HttpRequest.h"
#include "HttpResponse.h"

class Middleware
{
public:
    virtual ~Middleware() = default;
    
    // 处理请求，返回true继续执行，返回false中断执行
    virtual bool handle(HttpRequest& request, HttpResponse& response) = 0;
};

#endif // MIDDLEWARE_H