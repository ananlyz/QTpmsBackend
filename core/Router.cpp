#include "Router.h"
#include "Middleware.h"
#include <QDebug>

Route::Route(const QString& method, const QString& path, 
             const QList<Middleware*>& middlewares,
             std::function<void(HttpRequest&, HttpResponse&)> handler)
    : method(method), path(path), middlewares(middlewares), handler(handler)
{
    this->regex = Router::pathToRegex(path);
}

Router::Router(QObject *parent) : QObject(parent)
{
}

void Router::get(const QString& path, const QList<Middleware*>& middlewares,
                 std::function<void(HttpRequest&, HttpResponse&)> handler)
{
    addRoute("GET", path, middlewares, handler);
}

void Router::post(const QString& path, const QList<Middleware*>& middlewares,
                  std::function<void(HttpRequest&, HttpResponse&)> handler)
{
    addRoute("POST", path, middlewares, handler);
}

void Router::put(const QString& path, const QList<Middleware*>& middlewares,
                 std::function<void(HttpRequest&, HttpResponse&)> handler)
{
    addRoute("PUT", path, middlewares, handler);
}

void Router::del(const QString& path, const QList<Middleware*>& middlewares,
                 std::function<void(HttpRequest&, HttpResponse&)> handler)
{
    addRoute("DELETE", path, middlewares, handler);
}

void Router::addRoute(const QString& method, const QString& path, 
                      const QList<Middleware*>& middlewares,
                      std::function<void(HttpRequest&, HttpResponse&)> handler)
{
    routes.append(Route(method, path, middlewares, handler));
}

bool Router::handleRequest(HttpRequest& request, HttpResponse& response)
{
    qDebug() << "Router handling request:" << request.method << request.path;
    qDebug() << "Available routes count:" << routes.size();
    
    for (const auto& route : routes) {
        qDebug() << "Checking route:" << route.method << route.path;
        if (matchRoute(route, request.method, request.path, request)) {
            qDebug() << "Route matched!";
            qDebug() << "Path parameters:" << request.pathParams;
            // 执行中间件链
            for (auto* middleware : route.middlewares) {
                if (!middleware->handle(request, response)) {
                    return false; // 中间件中断请求
                }
            }
            
            // 执行路由处理器
            if (route.handler) {
                route.handler(request, response);
                return true;
            }
        }
    }
    
    // 未找到匹配的路由
    qDebug() << "No matching route found for:" << request.method << request.path;
    response.notFound("Route not found");
    return false;
}

QRegularExpression Router::pathToRegex(const QString& path)
{
    QString pattern = path;
    // 处理路径参数 :param 格式
    QRegularExpression paramRegex(":([a-zA-Z_][a-zA-Z0-9_]*)");
    pattern.replace(paramRegex, "([^/]+)");
    
    // 处理通配符
    pattern.replace("*", "[^/]*");
    pattern.replace("?", "\\?");
    
    return QRegularExpression("^" + pattern + "$");
}

bool Router::matchRoute(const Route& route, const QString& method, const QString& path, HttpRequest& request)
{
    if (route.method != method) {
        return false;
    }
    
    QRegularExpressionMatch match = route.regex.match(path);
    if (!match.hasMatch()) {
        return false;
    }
    
    // 提取路径参数
    QString routePath = route.path;
    QRegularExpression paramRegex(":([a-zA-Z_][a-zA-Z0-9_]*)");
    QRegularExpressionMatchIterator paramIterator = paramRegex.globalMatch(routePath);
    
    int captureGroup = 1; // 第一个捕获组对应第一个参数
    while (paramIterator.hasNext()) {
        QRegularExpressionMatch paramMatch = paramIterator.next();
        QString paramName = paramMatch.captured(1);
        QString paramValue = match.captured(captureGroup);
        request.pathParams[paramName] = paramValue;
        captureGroup++;
    }
    
    return true;
}
