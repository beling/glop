TEMPLATE = app
DEPENDPATH += . src
INCLUDEPATH += .
CONFIG += thread
RESOURCES += glop.qrc
QT += xml

#choose a compilation mode in release, debug or profil
#intermediate files .o or .moc will be stored in different directories
CONFIG += release

#compilation configuration
#console is useful when testing, but shoud not be used for the final release
win32 {
	DEFINES += WINDOWS
	RC_FILE = glopico.rc
	CONFIG += static
	CONFIG += console
}

unix {
	DEFINES += LINUX
	CONFIG += static
}

macx {
	DEFINES += MACX
	CONFIG += x86 ppc
	CONFIG += static
}

CONFIG(debug, debug|release) {
	OBJECTS_DIR += debugTmp
	MOC_DIR += debugTmp
	RCC_DIR += debugTmp
	TARGET = bin/glop_debug
} else {
	OBJECTS_DIR += releaseTmp
	MOC_DIR += releaseTmp
	RCC_DIR += releaseTmp
	TARGET = bin/glop
}

#this flag can be used to debug binaries in release mode
#QMAKE_CXXFLAGS_RELEASE += -g

profil {
	CONFIG += debug
	QMAKE_CXXFLAGS_DEBUG += -pg
	QMAKE_LFLAGS_DEBUG += -pg
	TARGET = bin/glop_profil
	OBJECTS_DIR = profilTmp
	MOC_DIR = profilTmp
	RCC_DIR = profilTmp
}

# Input
HEADERS += src/global.h \
           src/stringconverter.h \
           src/baseclass/interface.h \
           src/baseclass/interfacewidget.h \
           src/baseclass/basegame.h \
           src/baseclass/basenode.h \
           src/baseclass/traversal.h \
           src/moveordering.h \
           src/displaypainter.h \
           src/computation/computationthread.h \
           src/computation/nodestore.h \
           src/computation/classicpnsearch.h \
           src/computation/database.h \
           src/computation/databasebutton.h \
           src/computation/trace.h \
           src/computation/tracewidget.h \
           src/node/winlossnode.h \
           src/node/nimbernode.h \
           src/node/scorenode.h \
           src/node/fulltreeinfo.h \
           src/node/fulltreenode.h \
           src/node/rct.h \
           src/node/rctmiserenode.h \
           src/sprouts/sproutsglobal.h \
           src/sprouts/representation.h \
           src/sprouts/boundary.h \
           src/sprouts/regionspr.h \
           src/sprouts/land.h \
           src/sprouts/position.h \
           src/board/board.h \
           src/board/splitboard.h \
           src/board/crambase.h \
           src/board/cram.h \
           src/board/dotsboxes.h \
           src/gameglobal.h \
           src/mainwidget/widgetpopup.h \
           src/mainwidget/glopwidget.h \
           src/mainwidget/childrentabwidget.h \
           src/mainwidget/repofile.h \
           src/gamewidget.h \
           src/error.h \
           src/test/test.h
SOURCES += src/stringconverter.cpp \
           src/baseclass/interface.cpp \
           src/baseclass/interfacewidget.cpp \
           src/baseclass/basegame.cpp \
           src/baseclass/basenode.cpp \
           src/baseclass/traversal.cpp \
           src/moveordering.cpp \
           src/displaypainter.cpp \
           src/computation/computationthread.cpp \
           src/computation/nodestore.cpp \
           src/computation/mainloop.cpp \
           src/computation/classicpnsearch.cpp \
           src/computation/database.cpp \
           src/computation/databasebutton.cpp \
           src/computation/trace.cpp \
           src/computation/tracewidget.cpp \
           src/node/winlossnode.cpp \
           src/node/nimbernode.cpp \
           src/node/scorenode.cpp \
           src/node/fulltreeinfo.cpp \
           src/node/fulltreenode.cpp \
           src/node/rct.cpp \
           src/node/rctmiserenode.cpp \
           src/sprouts/sproutsglobal.cpp \
           src/sprouts/representation.cpp \
           src/sprouts/boundary.cpp \
           src/sprouts/regionspr.cpp \
           src/sprouts/land.cpp \
           src/sprouts/position.cpp \
           src/board/board.cpp \
           src/board/splitboard.cpp \
           src/board/crambase.cpp \
           src/board/cram.cpp \
           src/board/dotsboxes.cpp \
		   src/board/boardcompress.cpp \
           src/gameglobal.cpp \
           src/mainwidget/widgetpopup.cpp \
           src/mainwidget/glopwidget.cpp \
           src/mainwidget/childrentabwidget.cpp \
           src/mainwidget/repofile.cpp \
           src/gamewidget.cpp \
           src/glop.cpp \
           src/error.cpp \
           src/test/test.cpp \
           src/test/stringconverter_test.cpp \
           src/test/sprouts_test.cpp \
           src/test/board_test.cpp \
           src/test/dotsboxes_test.cpp \
           src/test/cram_test.cpp \
		   src/test/memory_test.cpp
