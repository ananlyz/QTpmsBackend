#ifndef SPACECONTROLLER_H
#define SPACECONTROLLER_H

#include <QObject>
#include "../core/HttpRequest.h"
#include "../core/HttpResponse.h"
#include "../services/SpaceService.h"
#include "../services/BillingService.h"
#include "../dao/ParkingRecordRepository.h"

class SpaceController : public QObject
{
    Q_OBJECT

public:
    static SpaceController& instance();

    // 停车位管理
    void addSpace(const HttpRequest& request, HttpResponse& response);
    void updateSpace(const HttpRequest& request, HttpResponse& response);
    void deleteSpace(const HttpRequest& request, HttpResponse& response);
    
    // 停车位查询
    void getSpaceInfo(const HttpRequest& request, HttpResponse& response);
    void getAllSpaces(const HttpRequest& request, HttpResponse& response);
    void getSpacesByStatus(const HttpRequest& request, HttpResponse& response);
    void getAvailableSpaces(const HttpRequest& request, HttpResponse& response);
    void getOccupiedSpaces(const HttpRequest& request, HttpResponse& response);
    
    // 停车位状态管理
    void occupySpace(const HttpRequest& request, HttpResponse& response);
    void releaseSpace(const HttpRequest& request, HttpResponse& response);
    void updateSpaceStatus(const HttpRequest& request, HttpResponse& response);
    
    // 统计信息
    void getStatistics(const HttpRequest& request, HttpResponse& response);
    void getUsageStatistics(const HttpRequest& request, HttpResponse& response);
    
    // 计费相关
    void calculateParkingFee(const HttpRequest& request, HttpResponse& response);
    
    // 排队系统
    void joinQueue(const HttpRequest& request, HttpResponse& response);

private:
    SpaceController() = default;
    SpaceController(const SpaceController&) = delete;
    SpaceController& operator=(const SpaceController&) = delete;
    
    int extractSpaceIdFromPath(const QString& path);
    QString extractStatusFromPath(const QString& path);
};

#endif // SPACECONTROLLER_H