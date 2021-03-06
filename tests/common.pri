include( ../common.pri )

QT       += testlib
QT       -= gui

TARGET = tst_tests sshsettings

CONFIG   += console testcase
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

# gcov support
linux {
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
	LIBS += -lgcov
}

INCLUDEPATH += $$SRCDIR
