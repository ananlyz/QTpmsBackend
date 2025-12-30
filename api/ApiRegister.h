#ifndef APIREGISTER_H
#define APIREGISTER_H

#include <QObject>
#include "../core/Router.h"
#include "../core/HttpServer.h"
#include "../controllers/CarController.h"
#include "../controllers/SpaceController.h"
#include "../controllers/ReportController.h"

class ApiRegister : public QObject
{
    Q_OBJECT

public:
    static ApiRegister& instance();
    
    // 注册所有API路由
    void registerRoutes(Router& router);
    
    // 初始化API
    bool initialize(HttpServer& server);

private:
    ApiRegister() = default;
    ApiRegister(const ApiRegister&) = delete;
    ApiRegister& operator=(const ApiRegister&) = delete;
    
    // 车辆管理API
    void registerCarRoutes(Router& router);
    
    // 停车位管理API
    void registerSpaceRoutes(Router& router);
    
    // 报告统计API
    void registerReportRoutes(Router& router);
    
    // 系统API
    void registerSystemRoutes(Router& router);
    
    // 健康检查
    void handleHealthCheck(const HttpRequest& request, HttpResponse& response);
    
    // API信息
    void handleApiInfo(const HttpRequest& request, HttpResponse& response);
};

#endif // APIREGISTER_H