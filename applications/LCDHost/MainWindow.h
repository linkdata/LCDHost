/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

  This file is part of LCDHost.

  LCDHost is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LCDHost is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LCDHost.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGlobal>

#include "LCDHost.h"
#include "AppId.h"
#include "AppLibrary.h"
#include "AppClassTree.h"
#include "AppGraphicsScene.h"
#include "EventRawInput.h"
#include "AppVersionCache.h"

#ifdef QT_OPENGL_LIB
#include "AppGLWidget.h"
#endif

#if LH_USE_HID
#include "LH_HidThread.h"
#include "LH_HidDevice.h"
#endif

#include <QSystemTrayIcon>
#include <QtDebug>
#include <QMainWindow>
#include <QTimer>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QDateTime>
#include <QBasicTimer>
#include <QSet>


class AppClass;
class AppInstance;
class AppInstanceTree;
class QStandardItemModel;
class AppRawInput;

namespace Ui
{
    class MainWindowClass;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindowClass *ui;
    QStandardItemModel *refListModel_;
    QStandardItemModel *refListSizeModel_;
    AppClassTree *classTree_;
    AppInstanceTree *instTree_;
    AppGraphicsScene *scene_;
    QString wantedDevId_;
    QBasicTimer timer_;
    QDateTime programStart_;
    QString settingsVersion_;
    bool dontShowWelcome_;
    QDateTime cpuNext_, memNext_, netNext_, secondNext_, fpsNext_;
    int netautoin_, netautoout_, netautodev_;
    int sceneChanged_;
    int loglines_;
    AppId selectedinstance_;
    QNetworkAccessManager nam_;
    bool need_instance_refresh_;
    QString mousekey_;
    int rendermethod_;
    QMap<QString,QByteArray> pubkeys_;
    AppVersionCache vercache_;
    QDateTime lastwebcheck_;
    int webcheckinterval_; // in hours
    bool webautoupdate_;
    bool webautocheck_;
    QSet<QString> fixmeLogs_;
    AppObject* plugins_;
    enum State {
      kStateInit,
      kStateStartLog,
      kStateStartAppState,
      kStateStartHid,
      kStateStartSystray,
      kStateStartPlugins,
      kStateLoadLayout,
      kStateShowWelcome,
      kStateRun,
      kStateTerm,
      kStateStopPluginsStop,
      kStateStopPluginsAbort,
      kStateStopPluginsTerm,
      kStateStopPluginsDelete,
      kStateStopPluginsStopped,
      kStateStopSystray,
      kStateStopHid,
      kStateStopRawInput,
      kStateStopAppState,
      kStateStopped,
      kStateQuit
    } state_;
    int app_library_waited_;

#ifdef QT_OPENGL_LIB
    AppGLWidget *glwidget_;
#endif

#if LH_USE_HID
    LH_HidThread *hid_thread_;
    int hid_thread_waited_;
#endif

    bool systray_disabled_;
    QSystemTrayIcon* systray_;
    QMenu* systraymenu_;
    bool systrayWarning_;

    AppState *app_state_;

    void addParentItem( AppInstance *app_inst );

    void run();
    void setNextTimer();

public:
    int currentLayoutZoom;
    unsigned pps; // paints per second, incremented by AppGraphicsPixmapItem::paint()

    int renderMethod() const { return rendermethod_; }
#ifdef QT_OPENGL_LIB
    AppGLWidget *glWidget() { return glwidget_; }
#endif

    QString layoutPath;
    QString layoutName;

    QString layoutPathDefault();
    QString layoutNameDefault();

    explicit MainWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~MainWindow();
    void loadSettings();
    void saveSettings();
    void init();

    bool systrayDisabled() const { return systray_disabled_; }
    void setSystrayDisabled(bool b) { systray_disabled_ = b; }
    bool systrayStartup();
    bool hasSystray() const;

    AppGraphicsScene* scene() { return scene_; }
    AppInstanceTree* tree() { return instTree_; }
    AppClassTree* classTree() { return classTree_; }
    QStandardItemModel *refListModel() { return refListModel_; }
    QStandardItemModel *refListSizeModel() { return refListSizeModel_; }
    QString previewClassId() const;
    AppInstance *previewInstance();
    QNetworkAccessManager& getNAM() { return nam_; }
    AppVersionCache& versionCache() { return vercache_; }

    void openLayout( QString str );
    void saveLayout();
    bool loadLayout();
    AppClass *getAppClass( QString aClassId );
    void clearLayout();

    QList<AppInstance*> selectedInstances();
    AppClass* selectedClass();
    bool cancelIfModified();

    void setZoomLevel( int z );

    void customEvent(QEvent *);
    void closeEvent( QCloseEvent *event );
    void timerEvent( QTimerEvent *event );

    void log(QDateTime, QtMsgType, QString);

    void webUpdateCompare();
    void webUpdatePlugin( QString simplename );
    void webUpdatePluginData( AppLibrary *app_lib, QNetworkReply *reply );

    bool webAutoCheck() const { return webautocheck_; }
    void setWebAutoCheck( bool b ) { webautocheck_ = b; }
    bool webAutoUpdatePlugins() const { return webautoupdate_; }
    void setWebAutoUpdatePlugins( bool b ) { webautoupdate_ = b; }
    int webCheckInterval() const { return webcheckinterval_; }
    void setWebCheckInterval( int hrs ) { webcheckinterval_ = ( (hrs > 0) ? hrs : 1 ); }
    QDateTime webLastCheckTime() const { return lastwebcheck_; }
    QString webUserAgent() const;

    AppObject* pluginParent() const { return plugins_; }
    QList<AppLibrary*> plugins() const;
    bool loadPlugin();

    const QBasicTimer& timer() const { return timer_; }
    QBasicTimer& timer() { return timer_; }

public slots:
    void logged(uint msgtime, int msgtype, QString msgtext);
    void setFocusInstance( AppInstance *inst = NULL );
    void refreshPluginList();
    void refreshPluginDetails();
    void refreshDeviceList();
    void refreshInputDeviceDetails(QModelIndex = QModelIndex());
    void sceneSelectionChanged();
    void refreshInstanceDetails() { need_instance_refresh_ = true; }
    void doRefreshInstanceDetails();
    void refreshInstanceUI( AppId id = AppId() );
    void sceneChanged( const QList<QRectF> & region );
    void zoom(int steps);
    void systrayActivated( QSystemTrayIcon::ActivationReason );
    void term();
    void screenShot();
    void stateChanged(AppId);
    void openDebugDialog();
    void rawInput( QByteArray, QString, int, int, int );
    void webUpdateCheck();
    void netGotPubkey();
    void netGotPlugin();
    void netGotVersion();
    void netGotHeadReply();

signals:
    void libraryStateChanged(AppId);
    void layoutChanged(QString,QString);
    void layoutSaved(QString,QString);
    void onceASecond();

private slots:
    void on_actionClean_layout_triggered();
    void on_renderMethod_currentIndexChanged(int index);
    void on_actionReload_triggered();
    void on_inputDeviceMouseKeyButton_clicked();
    void on_actionScreenshot_triggered();
    void on_instanceClass_activated(QString );
    void on_pluginList_activated(QModelIndex index);
    void on_outputRefresh_clicked();
    void on_outputDeviceList_itemActivated(QListWidgetItem* item);
    void on_outputDeviceList_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_actionWelcome_triggered();
    void on_cpuRefreshRate_valueChanged(int );
    void on_netMaxRateOutComboBox_activated(int index);
    void on_netMaxRateInComboBox_activated(int index);
    void on_actionAbout_triggered();
    void on_instanceDepth_stepping(int );
    void on_inputDeviceShowEvents_clicked(bool checked);
    void on_inputDeviceRefreshButton_clicked();
    void on_instanceHeightUnits_activated(int index);
    void on_instanceWidthUnits_activated(int index);
    void on_instanceHRef_activated(int index);
    void on_instanceWRef_activated(int index);
    void on_instanceHeightMod_valueChanged(int );
    void on_instanceWidthMod_valueChanged(int );
    void on_instanceYRef_activated(int index);
    void on_instanceXRef_activated(int index);
    void on_instanceYSide_activated(int index);
    void on_instanceYAlign_activated(int index);
    void on_instanceXSide_activated(int index);
    void on_instanceXAlign_activated(int index);
    void on_instanceYOffset_editingFinished();
    void on_instanceYOffset_stepping(int steps);
    void on_instanceXOffset_editingFinished();
    void on_instanceXOffset_stepping(int steps);
    void on_instanceParent_activated(int index);
    void on_instanceName_editingFinished();
    void on_actionNew_triggered();
    void on_instanceAlerts_stateChanged(int );
    void on_actionOpen_triggered();
    void on_actionSave_As_triggered();
    void on_actionSave_triggered();
    void on_layoutZoomSlider_valueChanged(int value);
    void on_instanceTreeView_selectionChange( const QItemSelection & selected, const QItemSelection & deselected );
    void on_LoadButton_clicked();
    void on_actionExit_triggered();
    void on_actionUpdates_triggered();
    void on_checkForPluginsButton_clicked();
    void on_actionSystrayRestore_triggered();
    void on_actionSystrayQuit_triggered();
    void on_actionSystrayOpen_triggered();
};

#endif // MAINWINDOW_H
