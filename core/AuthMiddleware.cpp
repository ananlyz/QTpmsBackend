#include "AuthMiddleware.h"
#include <QDebug>

AuthMiddleware::AuthMiddleware()
{
    // 添加一些默认的测试token
    addValidToken("test-token-123");
    addValidToken("admin-token-456");
}

bool AuthMiddleware::handle(HttpRequest& request, HttpResponse& response)
{
    // 检查是否为公开路径
    if (isPublicPath(request.path)) {
        return true; // 继续处理
    }
    
    QString authHeader = request.getHeader("authorization");
    if (authHeader.isEmpty()) {
        response.unauthorized("Missing authorization header");
        return false; // 中断处理
    }
    
    QString token = extractToken(authHeader);
    if (token.isEmpty() || !validTokens.contains(token)) {
        response.unauthorized("Invalid or expired token");
        return false; // 中断处理
    }
    
    // 将用户信息存入请求上下文
    request.setContext("user_token", token);
    request.setContext("authenticated", true);
    
    return true; // 继续处理
}

void AuthMiddleware::addValidToken(const QString& token)
{
    validTokens[token] = true;
}

QString AuthMiddleware::extractToken(const QString& authHeader) const
{
    if (authHeader.startsWith("Bearer ", Qt::CaseInsensitive)) {
        return authHeader.mid(7);
    }
    return authHeader;
}

bool AuthMiddleware::isPublicPath(const QString& path) const
{
    // 定义公开路径，不需要认证
    static QStringList publicPaths = {
        "/api/auth/login",
        "/api/auth/register",
        "/api/health",
        "/api/space/list"
    };
    
    return publicPaths.contains(path);
}