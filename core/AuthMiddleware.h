#ifndef AUTHMIDDLEWARE_H
#define AUTHMIDDLEWARE_H

#include "Middleware.h"
#include <QMap>
#include <QString>

class AuthMiddleware : public Middleware
{
public:
    AuthMiddleware();
    bool handle(HttpRequest& request, HttpResponse& response) override;
    
    // 添加允许的token（简单实现）
    void addValidToken(const QString& token);
    
private:
    QMap<QString, bool> validTokens;
    QString extractToken(const QString& authHeader) const;
    bool isPublicPath(const QString& path) const;
};

#endif // AUTHMIDDLEWARE_H