#ifndef QUEUEREPOSITORY_H
#define QUEUEREPOSITORY_H

#include <QList>
#include <QSqlQuery>
#include <QThreadStorage>
#include <QSqlDatabase>
#include <QDateTime>
#include <QVariantMap>

struct QueueItem {
    int id;
    QString plate;
    QDateTime queueTime;

    QueueItem() : id(0) {}
    QueueItem(const QString& p) : id(0), plate(p), queueTime(QDateTime::currentDateTime()) {}
};

class QueueRepository
{
public:
    static QueueRepository& instance();

    bool insert(const QueueItem& item);
    bool remove(const QString& plate);
    QueueItem findFirst();
    QList<QueueItem> findAll();
    bool exists(const QString& plate);
    int count();
    QueueItem findByPlate(const QString& plate);
    int getPosition(const QString& plate);

private:
    QueueRepository() = default;
    QueueRepository(const QueueRepository&) = delete;
    QueueRepository& operator=(const QueueRepository&) = delete;

    QueueItem mapToItem(const QSqlQuery& query);
    QueueItem mapToItem(const QVariantMap& map);
    QSqlDatabase getDatabase();
    bool executeQuery(const QString& queryStr, const QVariantMap& params);
    QList<QVariantMap> executeQueryWithResults(const QString& queryStr, const QVariantMap& params = QVariantMap());
};

#endif // QUEUEREPOSITORY_H