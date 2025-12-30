#ifndef CARCONTROLLER_H
#define CARCONTROLLER_H

#include <QObject>
#include "../core/HttpRequest.h"
#include "../core/HttpResponse.h"
#include "../services/CarService.h"

class CarController : public QObject
{
    Q_OBJECT

public:
    static CarController& instance();

    // 车辆注册
    void registerCar(const HttpRequest& request, HttpResponse& response);
    
    // 车辆信息查询
    void getCarInfo(const HttpRequest& request, HttpResponse& response);
    void getAllCars(const HttpRequest& request, HttpResponse& response);
    void getCarsByType(const HttpRequest& request, HttpResponse& response);
    
    // 车辆信息更新
    void updateCar(const HttpRequest& request, HttpResponse& response);
    
    // 车辆删除
    void deleteCar(const HttpRequest& request, HttpResponse& response);
    
    // 车辆存在性检查
    void checkCarExists(const HttpRequest& request, HttpResponse& response);
    
    // 统计信息
    void getStatistics(const HttpRequest& request, HttpResponse& response);

private:
    CarController() = default;
    CarController(const CarController&) = delete;
    CarController& operator=(const CarController&) = delete;
    
    QString extractPlateFromPath(const QString& path);
    QString extractTypeFromPath(const QString& path);
};

#endif // CARCONTROLLER_H