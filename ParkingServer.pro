QT += core network sql
CONFIG += c++14 console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    ParkingServerApplication.cpp \
    core/HttpServer.cpp \
    core/HttpRequest.cpp \
    core/HttpResponse.cpp \
    core/Router.cpp \
    core/Middleware.cpp \
    core/JsonBodyParser.cpp \
    core/ErrorHandler.cpp \
    api/ApiRegister.cpp \
    api/ApiResponse.cpp \
    controllers/CarController.cpp \
    controllers/SpaceController.cpp \
    controllers/ReportController.cpp \
    services/CarService.cpp \
    services/SpaceService.cpp \
    services/BillingService.cpp \
    models/Car.cpp \
    models/ParkingRecord.cpp \
    models/ParkingSpace.cpp \
    dao/CarRepository.cpp \
    dao/SpaceRepository.cpp \
    dao/ParkingRecordRepository.cpp \
    utils/DateTimeUtil.cpp \
    utils/JsonUtil.cpp \
    utils/Logger.cpp

HEADERS += \
    ParkingServerApplication.h \
    core/HttpServer.h \
    core/HttpRequest.h \
    core/HttpResponse.h \
    core/Router.h \
    core/Middleware.h \
    core/JsonBodyParser.h \
    core/ErrorHandler.h \
    api/ApiRegister.h \
    api/ApiResponse.h \
    controllers/CarController.h \
    controllers/SpaceController.h \
    controllers/ReportController.h \
    services/CarService.h \
    services/SpaceService.h \
    services/BillingService.h \
    models/Car.h \
    models/ParkingRecord.h \
    models/ParkingSpace.h \
    dao/CarRepository.h \
    dao/SpaceRepository.h \
    dao/ParkingRecordRepository.h \
    utils/DateTimeUtil.h \
    utils/JsonUtil.h \
    utils/Logger.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
