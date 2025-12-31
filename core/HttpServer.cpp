#include "HttpServer.h"
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QHostAddress>
#include <QUrl>

HttpServer::HttpServer(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    m_router = nullptr;
    m_port = 8080;
    m_maxConnections = 100;
    m_requestTimeout = 30000;
    
    connect(server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
}

HttpServer::~HttpServer()
{
    if (server->isListening()) {
        server->close();
    }
}

bool HttpServer::start()
{
    return listen("127.0.0.1", m_port);
}

void HttpServer::stop()
{
    server->close();
}

bool HttpServer::isRunning() const
{
    return server->isListening();
}

quint16 HttpServer::port() const
{
    return server->serverPort();
}

void HttpServer::setPort(quint16 port)
{
    m_port = port;
}

void HttpServer::setMaxConnections(int max)
{
    m_maxConnections = max;
    server->setMaxPendingConnections(max);
}

void HttpServer::setRequestTimeout(int timeout)
{
    m_requestTimeout = timeout;
}

void HttpServer::setRouter(Router* router)
{
    this->m_router = router;
}

bool HttpServer::listen(const QString& address, quint16 port)
{
    return server->listen(QHostAddress(address), port);
}

Router* HttpServer::router() const
{
    return m_router;
}

void HttpServer::onNewConnection()
{
    while (server->hasPendingConnections()) {
        QTcpSocket* socket = server->nextPendingConnection();
        
        connect(socket, &QTcpSocket::readyRead, this, &HttpServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &HttpServer::onDisconnected);
    }
}

void HttpServer::onReadyRead()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    socketBuffers[socket].append(data);
    
    // 处理累积的缓冲区数据
    processBufferedData(socket);
}

void HttpServer::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socketBuffers.remove(socket);  // 清理缓冲区
        socket->deleteLater();
    }
}

void HttpServer::processBufferedData(QTcpSocket* socket)
{
    QByteArray& buffer = socketBuffers[socket];
    
    // 循环处理缓冲区中的所有完整请求
    while (tryParseCompleteRequest(socket, buffer)) {
        // 继续处理下一个请求
    }
}

bool HttpServer::tryParseCompleteRequest(QTcpSocket* socket, QByteArray& buffer)
{
    // 查找头部结束标记 \r\n\r\n
    int headerEndIndex = buffer.indexOf("\r\n\r\n");
    if (headerEndIndex == -1) {
        return false;  // 头部不完整
    }
    
    // 解析请求行
    QByteArray headerPart = buffer.left(headerEndIndex);
    QString headerStr = QString::fromUtf8(headerPart);
    QStringList lines = headerStr.split("\r\n");
    
    if (lines.isEmpty()) return false;
    
    QString requestLine = lines.takeFirst();
    QString method, path, httpVersion;
    QString fullPath = parseRequestLine(requestLine, method, path, httpVersion);
    
    if (fullPath.isEmpty()) {
        // 无效请求行，发送错误响应
        HttpResponse response;
        response.badRequest("Invalid request line");
        sendHttpResponse(socket, response);
        // 移除无效数据
        buffer.remove(0, headerEndIndex + 4);
        return true;  // 已处理（即使是错误）
    }
    
    // 解析头部
    QMap<QString, QString> headers;
    for (const QString& line : lines) {
        if (line.isEmpty()) continue;
        int colonIndex = line.indexOf(':');
        if (colonIndex > 0) {
            QString headerName = line.left(colonIndex).trimmed().toLower();
            QString headerValue = line.mid(colonIndex + 1).trimmed();
            headers[headerName] = headerValue;
        }
    }
    
    // 获取Content-Length
    int contentLength = 0;
    if (headers.contains("content-length")) {
        contentLength = headers["content-length"].toInt();
    }
    
    // 计算body开始位置
    int bodyStartIndex = headerEndIndex + 4;
    int totalRequestLength = bodyStartIndex + contentLength;
    
    // 检查是否有足够的body数据
    if (buffer.size() < totalRequestLength) {
        return false;  // body不完整，等待更多数据
    }
    
    // 提取body
    QByteArray bodyData;
    if (contentLength > 0) {
        bodyData = buffer.mid(bodyStartIndex, contentLength);
    }
    
    qDebug() << "HTTP Request:" << method << fullPath << httpVersion;
    qDebug() << "Content-Length:" << contentLength;
    qDebug() << "Body size:" << bodyData.size();
    
    // 创建请求对象
    HttpRequest request;
    request.method = method;
    request.headers = headers;
    request.bodyRaw = bodyData;
    
    // 使用 QUrl 解析路径和查询
    QUrl url(fullPath);
    request.path = url.path();
    request.parseQueryString(url.query());
    
    // 处理请求
    HttpResponse response;
    if (m_router) {
        m_router->handleRequest(request, response);
    } else {
        response.serverError("No router configured");
    }
    
    sendHttpResponse(socket, response);
    
    // 从缓冲区移除已处理的请求数据
    buffer.remove(0, totalRequestLength);
    
    return true;  // 成功处理了一个请求
}

void HttpServer::sendHttpResponse(QTcpSocket* socket, const HttpResponse& response)
{
    QString responseLine = QString("HTTP/1.1 %1 %2\r\n")
        .arg(response.statusCode)
        .arg(response.statusCode == 200 ? "OK" : 
             response.statusCode == 400 ? "Bad Request" :
             response.statusCode == 404 ? "Not Found" : "Internal Server Error");
    
    QString headers;
    for (auto it = response.headers.constBegin(); it != response.headers.constEnd(); ++it) {
        headers += QString("%1: %2\r\n").arg(it.key()).arg(it.value());
    }
    
    headers += QString("Content-Length: %1\r\n").arg(response.body.size());
    headers += "\r\n";
    
    QByteArray fullResponse;
    fullResponse.append(responseLine.toUtf8());
    fullResponse.append(headers.toUtf8());
    fullResponse.append(response.body);
    
    socket->write(fullResponse);
    socket->flush();
}

QString HttpServer::parseRequestLine(const QString& line, QString& method, QString& path, QString& httpVersion)
{
    QStringList parts = line.split(' ');
    if (parts.size() != 3) return "";
    
    method = parts[0];
    path = parts[1];
    httpVersion = parts[2];
    
    return path;
}