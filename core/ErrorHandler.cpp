#include "ErrorHandler.h"
#include <exception>
#include <stdexcept>
#include <QDebug>

bool ErrorHandler::handle(HttpRequest& request, HttpResponse& response)
{
    try {
        // 这里可以添加异常捕获逻辑
        // 由于中间件链式调用，异常可能在其他地方抛出
        return true; // 继续处理
    } catch (const std::exception& e) {
        QString errorMsg = QString("Exception caught: %1").arg(e.what());
        logError(errorMsg, request.path);
        response.serverError("Internal server error occurred");
        return false;
    } catch (...) {
        QString errorMsg = "Unknown exception caught";
        logError(errorMsg, request.path);
        response.serverError("Internal server error occurred");
        return false;
    }
}

void ErrorHandler::logError(const QString& error, const QString& path)
{
    qWarning() << "[ErrorHandler]" << error << "Path:" << path;
}
