#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QMap>

class AppConfig
{
public:
    static AppConfig& instance();
    
    // 加载配置
    bool loadConfig(const QString& configFile = "config/app.ini");
    
    // 服务器配置
    QString getServerHost() const;
    quint16 getServerPort() const;
    int getMaxThreads() const;
    
    // 数据库配置
    QString getDbHost() const;
    int getDbPort() const;
    QString getDbName() const;
    QString getDbUser() const;
    QString getDbPassword() const;
    int getDbPoolSize() const;
    
    // 日志配置
    QString getLogLevel() const;
    QString getLogFile() const;
    bool isLogToFile() const;
    bool isLogToConsole() const;
    
    // 通用配置获取
    QString getValue(const QString& key, const QString& defaultValue = QString()) const;
    int getIntValue(const QString& key, int defaultValue = 0) const;
    bool getBoolValue(const QString& key, bool defaultValue = false) const;
    
private:
    AppConfig();
    ~AppConfig() = default;
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
    
    QSettings* settings;
    QMap<QString, QVariant> defaultValues;
    
    void setDefaultValues();
};

#endif // APPCONFIG_H