#ifndef ROUTER_H
#define ROUTER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QRegularExpression>
#include <functional>
#include "HttpRequest.h"
#include "HttpResponse.h"

class Middleware;

struct Route {
    QString method;
    QString path;
    QRegularExpression regex;
    QList<Middleware*> middlewares;
    std::function<void(HttpRequest&, HttpResponse&)> handler;
    
    Route(const QString& method, const QString& path, 
          const QList<Middleware*>& middlewares,
          std::function<void(HttpRequest&, HttpResponse&)> handler);
};

class Router : public QObject
{
    Q_OBJECT

public:
    explicit Router(QObject *parent = nullptr);
    
    // 路由注册方法
    void get(const QString& path, const QList<Middleware*>& middlewares,
             std::function<void(HttpRequest&, HttpResponse&)> handler);
    void post(const QString& path, const QList<Middleware*>& middlewares,
              std::function<void(HttpRequest&, HttpResponse&)> handler);
    void put(const QString& path, const QList<Middleware*>& middlewares,
             std::function<void(HttpRequest&, HttpResponse&)> handler);
    void del(const QString& path, const QList<Middleware*>& middlewares,
             std::function<void(HttpRequest&, HttpResponse&)> handler);
    
    // 路由处理方法
    bool handleRequest(HttpRequest& request, HttpResponse& response);

    static QRegularExpression pathToRegex(const QString& path);
    
private:
    QList<Route> routes;
    
    void addRoute(const QString& method, const QString& path, 
                  const QList<Middleware*>& middlewares,
                  std::function<void(HttpRequest&, HttpResponse&)> handler);
    
    bool matchRoute(const Route& route, const QString& method, const QString& path, HttpRequest& request);
};

#endif // ROUTER_H