TARGET = nemoutilities
PLUGIN_IMPORT_PATH = org/nemomobile/utilities

TEMPLATE = lib
CONFIG += qt plugin hide_symbols
QT += declarative

target.path = $$[QT_INSTALL_IMPORTS]/$$PLUGIN_IMPORT_PATH
INSTALLS += target

qmldir.files += $$_PRO_FILE_PWD_/qmldir
qmldir.path +=  $$[QT_INSTALL_IMPORTS]/$$$$PLUGIN_IMPORT_PATH
INSTALLS += qmldir

SOURCES += plugin.cpp \
    declarativewindowattributes.cpp \
    declarativescreenshots.cpp

HEADERS += declarativewindowattributes.h \
    declarativescreenshots.h
