#ifndef REPORTCONTROLLER_H
#define REPORTCONTROLLER_H

#include <QObject>
#include "../core/HttpRequest.h"
#include "../core/HttpResponse.h"
#include "../services/BillingService.h"
#include "../services/SpaceService.h"
#include "../services/CarService.h"
#include "../dao/ParkingRecordRepository.h"

class ReportController : public QObject
{
    Q_OBJECT

public:
    static ReportController& instance();

    // 收入统计
    void getRevenueReport(const HttpRequest& request, HttpResponse& response);
    void getParkingStatistics(const HttpRequest& request, HttpResponse& response);
    void getPaymentStatistics(const HttpRequest& request, HttpResponse& response);
    
    // 使用率统计
    void getSpaceUsageReport(const HttpRequest& request, HttpResponse& response);
    void getOccupancyRate(const HttpRequest& request, HttpResponse& response);
    
    // 车辆统计
    void getCarStatistics(const HttpRequest& request, HttpResponse& response);
    void getCarTypeDistribution(const HttpRequest& request, HttpResponse& response);
    
    // 欠费统计
    void getUnpaidReport(const HttpRequest& request, HttpResponse& response);
    void getOverdueReport(const HttpRequest& request, HttpResponse& response);
    
    // 综合报告
    void getDashboardSummary(const HttpRequest& request, HttpResponse& response);
    void getDetailedReport(const HttpRequest& request, HttpResponse& response);
    
    // 支付
    void processPayment(const HttpRequest& request, HttpResponse& response);

private:
    ReportController() = default;
    ReportController(const ReportController&) = delete;
    ReportController& operator=(const ReportController&) = delete;
    
    QPair<QDateTime, QDateTime> parseDateRange(const HttpRequest& request);
    QJsonObject generateDashboardData();
    QJsonObject generateDetailedData(const QDateTime& startTime, const QDateTime& endTime);
};

#endif // REPORTCONTROLLER_H