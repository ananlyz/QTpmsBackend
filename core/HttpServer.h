#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "Router.h"

class HttpServer : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer();
    
    bool start();
    void stop();
    bool isRunning() const;
    quint16 port() const;
    void setPort(quint16 port);
    void setMaxConnections(int max);
    void setRequestTimeout(int timeout);
    
    bool listen(const QString& address = "127.0.0.1", quint16 port = 8080);
    void setRouter(Router* router);
    Router* router() const;
    
signals:
    void requestReceived(HttpRequest& request, HttpResponse& response);
    
private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    
private:
    QTcpServer* server;
    Router* m_router;
    quint16 m_port;
    int m_maxConnections;
    int m_requestTimeout;
    
    void parseHttpRequest(QTcpSocket* socket, const QByteArray& data);
    void sendHttpResponse(QTcpSocket* socket, const HttpResponse& response);
    QString extractHeaderValue(const QString& headers, const QString& headerName);
    QString parseRequestLine(const QString& line, QString& method, QString& path, QString& httpVersion);
};

#endif // HTTPSERVER_H
