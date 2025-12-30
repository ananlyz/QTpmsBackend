#include "HttpServer.h"
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QHostAddress>

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
    parseHttpRequest(socket, data);
}

void HttpServer::onDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

void HttpServer::parseHttpRequest(QTcpSocket* socket, const QByteArray& data)
{
    QString requestData = QString::fromUtf8(data);
    QStringList lines = requestData.split("\r\n");

    if (lines.isEmpty()) return;
    
    // 解析请求行
    QString requestLine = lines.takeFirst();
    QString method, path, httpVersion;
    QString fullPath = parseRequestLine(requestLine, method, path, httpVersion);
    
    qDebug() << "HTTP Request:" << method << fullPath << httpVersion;
    
    if (fullPath.isEmpty()) {
        HttpResponse response;
        response.badRequest("Invalid request line");
        sendHttpResponse(socket, response);
        return;
    }
    
    // 解析头部
    QMap<QString, QString> headers;
    QString body;
    bool headersComplete = false;
    int bodyStartIndex = -1;
    
    for (int i = 0; i < lines.size(); ++i) {
        const QString& line = lines[i];
        if (line.isEmpty()) {
            headersComplete = true;
            bodyStartIndex = requestData.indexOf("\r\n\r\n") + 4;
            continue;
        }
        
        if (!headersComplete) {
            int colonIndex = line.indexOf(':');
            if (colonIndex > 0) {
                QString headerName = line.left(colonIndex).trimmed().toLower();
                QString headerValue = line.mid(colonIndex + 1).trimmed();
                headers[headerName] = headerValue;
            }
        } else {
            body += line + "\n";
        }
    }
    
    // 正确解析请求体
    QByteArray bodyData;
    if (bodyStartIndex > 0 && bodyStartIndex < data.size()) {
        bodyData = data.mid(bodyStartIndex);
    }
    QJsonDocument jsonDoc = QJsonDocument::fromJson(bodyData);
    qDebug() << "HTTP Request Body Raw:" << bodyData;
    qDebug() << "HTTP Request Body JSON:" << jsonDoc;
    
    // 创建请求对象
    HttpRequest request;
    request.method = method;
    request.path = fullPath;
    request.headers = headers;
    request.bodyRaw = bodyData;
    
    // 解析查询参数
    if (fullPath.contains('?')) {
        QStringList parts = fullPath.split('?');
        request.path = parts[0];
        request.parseQueryString(parts[1]);
    }
    
    // 处理请求
    HttpResponse response;
    if (m_router) {
        m_router->handleRequest(request, response);
    } else {
        response.serverError("No router configured");
    }
    
    sendHttpResponse(socket, response);
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