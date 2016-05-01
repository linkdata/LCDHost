TEMPLATE = app
QT += core network gui widgets
QT -= opengl
TARGET = LCDHost

VERSION=$$quote($${LH_VERSION})
DEFINES += VERSION=\"\\\"$${VERSION}\\\"\"

INCLUDEPATH += ../lh_hid
LIBS += -llh_hid

INCLUDEPATH += ../../linkdata/lh_api5plugin

FORMS += MainWindow.ui \
    AppWelcomeDialog.ui \
    AppDebugDialog.ui \
    AppUpdateDialog.ui \
    AppDownloadUpdateDialog.ui \
    AppSourceDialog.ui

SOURCES += ../lh_logger/LH_Logger.cpp
HEADERS += ../lh_logger/LH_Logger.h

SOURCES += \
    AppClass.cpp \
    AppGraphicsScene.cpp \
    AppGraphicsView.cpp \
    AppInstance.cpp \
    AppInstanceTree.cpp \
    AppTreeView.cpp \
    AppSpinBox.cpp \
    LibClass.cpp \
    LibInstance.cpp \
    main.cpp \
    MainWindow.cpp \
    AppInstancePreview.cpp \
    AppSetupItem.cpp \
    wow64.cpp \
    AppClassTree.cpp \
    AppRawInput.cpp \
    RawInputEngine.cpp \
    RawInputDevice.cpp \
    AppListView.cpp \
    AppRawInputDialog.cpp \
    AppLibraryThread.cpp \
    AppLibrary.cpp \
    CPUEngine.cpp \
    MemoryEngine.cpp \
    NetworkEngine.cpp \
    RemoteWindow.cpp \
    AppAboutDialog.cpp \
    LibLibrary.cpp \
    AppWelcomeDialog.cpp \
    AppDevice.cpp \
    LibDevice.cpp \
    AppObject.cpp \
    LibObject.cpp \
    AppSetupFontInfo.cpp \
    AppSetupColorInfo.cpp \
    AppSetupFilename.cpp \
    AppSetupInputState.cpp \
    AppSetupInputValue.cpp \
    AppScriptEditor.cpp \
    AppId.cpp \
    AppState.cpp \
    AppDebugDialog.cpp \
    AppSetupComboBox.cpp \
    AppSetupSpinBox.cpp \
    AppSetupSlider.cpp \
    AppSetupProgressBar.cpp \
    AppSetupDoubleSpinBox.cpp \
    AppSetupItemHelp.cpp \
    AppOSXInstall.cpp \
    AppGLContext.cpp \
    AppGLWidget.cpp \
    AppSetupListBox.cpp \
    AppSetupButton.cpp \
    AppVersionCache.cpp \
    AppPluginVersion.cpp \
    AppPluginItemDelegate.cpp \
    AppSendVersionInfo.cpp \
    AppUpdateDialog.cpp \
    AppDownloadUpdateDialog.cpp \
    EventBase.cpp \
    PluginInfo.cpp \
    AppSourceDialog.cpp \
    LibSetupItem.cpp \
    AppSourceLinkButton.cpp \
    AppTreeWidget.cpp \
    LibDevicePointer.cpp \
    AppSetupItemSubscription.cpp \
    AppSetupLink.cpp

HEADERS += \
    AppClass.h \
    AppGraphicsScene.h \
    AppGraphicsView.h \
    AppInstance.h \
    AppInstanceTree.h \
    AppTreeView.h \
    AppSpinBox.h \
    EventCreateInstance.h \
    EventDestroyInstance.h \
    LibClass.h \
    LibInstance.h \
    MainWindow.h \
    EventRequestRender.h \
    AppInstancePreview.h \
    AppSetupItem.h \
    wow64.h \
    LCDHost.h \
    AppClassTree.h \
    AppRawInput.h \
    RawInputEngine.h \
    EventRawInput.h \
    RawInputDevice.h \
    EventRawInputRefresh.h \
    AppListView.h \
    RawInputLogitech.h \
    AppRawInputDialog.h \
    AppLibraryThread.h \
    AppLibrary.h \
    CPUEngine.h \
    EventNotify.h \
    MemoryEngine.h \
    NetworkEngine.h \
    EventLayoutChanged.h \
    RemoteWindow.h \
    AppAboutDialog.h \
    LibLibrary.h \
    EventLibraryLoaded.h \
    AppWelcomeDialog.h \
    EventClearTimeout.h \
    EventClassLoaded.h \
    EventClassRemoved.h \
    EventDeviceChanged.h \
    EventCallback.h \
    AppDevice.h \
    LibDevice.h \
    EventCreateAppDevice.h \
    EventDestroyAppDevice.h \
    EventRender.h \
    EventDeviceOpen.h \
    EventDeviceClose.h \
    EventLogEntry.h \
    AppObject.h \
    EventSetupItem.h \
    LibObject.h \
    AppSetupFontInfo.h \
    AppSetupColorInfo.h \
    AppSetupFilename.h \
    AppSetupInputState.h \
    AppSetupInputValue.h \
    EventInstanceRendered.h \
    EventInstanceRender.h \
    EventSetupItemRefresh.h \
    EventObjectCreated.h \
    AppScriptEditor.h \
    AppId.h \
    AppState.h \
    AppDebugDialog.h \
    EventLibraryStop.h \
    EventLibraryStart.h \
    EventSetupComplete.h \
    EventDeviceCreate.h \
    EventDeviceDestroy.h \
    EventLayoutSaved.h \
    AppSetupComboBox.h \
    EventLayoutLoad.h \
    AppSetupSpinBox.h \
    AppSetupSlider.h \
    AppSetupProgressBar.h \
    AppSetupDoubleSpinBox.h \
    AppSetupItemHelp.h \
    AppGLContext.h \
    AppGLWidget.h \
    AppSetupListBox.h \
    AppSetupButton.h \
    EventRefreshPlugins.h \
    AppVersionCache.h \
    AppPluginVersion.h \
    AppPluginItemDelegate.h \
    AppSendVersionInfo.h \
    AppUpdateDialog.h \
    AppDownloadUpdateDialog.h \
    EventRequestVisibility.h \
    EventBase.h \
    EventDebugStateRefresh.h \
    PluginInfo.h \
    EventLoadClassList.h \
    EventRequestPolling.h \
    EventRequestUnload.h \
    AppSourceDialog.h \
    LibSetupItem.h \
    EventSetupItemEnum.h \
    AppSourceLinkButton.h \
    EventObjectDestroy.h \
    AppTreeWidget.h \
    ../miniz/miniz.c \
    LibDevicePointer.h \
    AppSetupItemSubscription.h \
    AppSetupLink.h \
    EventAppStartup.h

RESOURCES += LCDHostResources.qrc
OTHER_FILES += lcdhost.rc

CONFIG(debug, debug|release): include(../modeltest/modeltest.pri)

win32 {
    RC_FILE = lcdhost.rc
    LIBS *= -lkernel32 -luser32 -lws2_32 -lgdi32 -lhid -ladvapi32
    SOURCES += CPUEngine_win.cpp \
        MemoryEngine_win.cpp \
        NetworkEngine_win.cpp \
        NetworkEngine_NDIS.cpp \
        NetworkEngine_TDI.cpp \
        RawInputEngine_win.cpp \
        RawInputDevice_win.cpp
    HEADERS += CPUEngine_win.h \
        MemoryEngine_win.h \
        NetworkEngine_win.h \
        NetworkEngine_NDIS.h \
        NetworkEngine_TDI.h \
        RawInputEngine_win.h \
        RawInputDevice_win.h
}

macx {
    ICON = images/LCDHost.icns
    QMAKE_INFO_PLIST = Info.plist
    LIBS += -framework CoreFoundation
    SOURCES += CPUEngine_mac.cpp \
        MemoryEngine_mac.cpp \
        NetworkEngine_mac.cpp \
        RawInputEngine_mac.cpp
    HEADERS += CPUEngine_mac.h \
        MemoryEngine_mac.h \
        NetworkEngine_mac.h \
        RawInputEngine_mac.h
}

unix:!macx {
    SOURCES += RawInputEngine_x11.cpp \
        MemoryEngine_x11.cpp \
        NetworkEngine_x11.cpp \
        CPUEngine_x11.cpp
    HEADERS += RawInputEngine_x11.h \
        MemoryEngine_x11.h \
        NetworkEngine_x11.h \
        CPUEngine_x11.h
}

LH_DESTDIR=$$lh_destdir()
LH_DIR_BUNDLE=$$dirname(OUT_PWD)
LH_DIR_BUNDLE=$$dirname(LH_DIR_BUNDLE)
LH_DIR_BUNDLE=$$quote($${LH_DIR_BUNDLE}/LCDHost.app)
macx: DESTDIR=$$dirname(LH_DIR_BUNDLE)
else: DESTDIR=$$LH_DIR_BUNDLE/bin
