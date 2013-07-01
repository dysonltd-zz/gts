TARGET = directShowCamera

EXCLUDE_DIRS += build

include(../../../../../lib/misc/qmake_template/dynamiclib.pro)

INCLUDEPATH += $$PROJ_DIR/ $$PROJECT_SRC_DIRS

INCLUDEPATH += "C:/Program Files/Microsoft SDKs/Windows/v6.0/Include"
LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v6.0/Lib" -lstrmiids -lole32 -loleaut32 -luuid -lcomsuppw

CONFIG -= QT

