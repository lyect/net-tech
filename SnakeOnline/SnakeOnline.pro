QT += \
    core \
    gui \
    widgets \
    network

CONFIG += c++17

SOURCES += \
    channels/messagechannel.cpp \
    channels/network/networkmessagechannel.cpp \
    controllers/gamecontroller.cpp \
    controllers/network/networkgamecontroller.cpp \
    field/field.cpp \
    main.cpp \
    proto/protohelp.cpp \
    proto/snakes.pb.cc \
    view/graphic/announcedgameswidget.cpp \
    view/graphic/fieldwidget.cpp \
    view/graphic/gamehostingwidget.cpp \
    view/graphic/gameinfowidget.cpp \
    view/graphic/graphicview.cpp

HEADERS += \
    channels/messagebuffer.h \
    channels/messagechannel.h \
    channels/network/networkmessagechannel.h \
    controllers/gamecontroller.h \
    controllers/network/networkgamecontroller.h \
    field/field.h \
    proto/protohelp.h \
    proto/snakes.pb.h \
    view/graphic/announcedgameswidget.h \
    view/graphic/fieldwidget.h \
    view/graphic/gamehostingwidget.h \
    view/graphic/gameinfowidget.h \
    view/graphic/graphicview.h

RESOURCES += \
    resources.qrc \
    resources.qrc

DISTFILES += \
    proto/snakes.proto

LIBS += -lprotobuf
