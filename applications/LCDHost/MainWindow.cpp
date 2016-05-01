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


#include <QtGlobal>
#include <QDebug>
#include <QMatrix>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QScrollBar>
#include <QCloseEvent>
#include <QStackedWidget>
#include <QInputDialog>
#include <QLibrary>
#include <QStringList>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QUrl>
#include <QWindowStateChangeEvent>
#include <QSysInfo>
#include <QImageReader>
#include <QPluginLoader>
#include <QShortcut>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QProcess>
#include <QThread>

#include <QCryptographicHash>

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "../miniz/miniz.c"

#include "LCDHost.h"
#include "../lh_logger/LH_Logger.h"
#include "AppState.h"
#include "AppDevice.h"
#include "AppDebugDialog.h"
#include "AppClass.h"
#include "AppClassTree.h"
#include "AppClassTreeDelegate.h"
#include "AppInstanceTree.h"
#include "AppInstance.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "AppTreeView.h"
#include "EventNotify.h"
#include "EventDeviceChanged.h"
#include "EventClearTimeout.h"
#include "EventDestroyInstance.h"
#include "EventLogEntry.h"
#include "EventDeviceCreate.h"
#include "EventDeviceDestroy.h"
#include "EventSetupComplete.h"
#include "EventRefreshPlugins.h"
#include "EventLibraryStop.h"
#include "EventLibraryStart.h"
#include "AppRawInput.h"
#include "RawInputDevice.h"
#include "wow64.h"
#include "AppAboutDialog.h"
#include "AppWelcomeDialog.h"
#include "AppUpdateDialog.h"
#include "AppDownloadUpdateDialog.h"
#include "PluginInfo.h"
#include "AppRawInputDialog.h"
#include "AppGLContext.h"
#include "AppGLWidget.h"
#include "AppPluginItemDelegate.h"
#include "AppSendVersionInfo.h"
#include "EventAppStartup.h"

#include <QDesktopWidget>

#ifdef Q_OS_WIN
# include <windows.h>
#endif

#if 1
#ifndef QT_NO_DEBUG
# include "modeltest.h"
#endif
#endif

char lh_mainwindow_signature_marker[] = LH_SIGNATURE_MARKER;

bool lh_log_load = false;

MainWindow::MainWindow(QWidget * parent, Qt::WindowFlags flags)
  : QMainWindow(parent, flags)
  , ui(new Ui::MainWindowClass())
  , refListModel_(new QStandardItemModel(this))
  , refListSizeModel_(new QStandardItemModel(this))
  , classTree_(0)
  , instTree_(0)
  , scene_(0)
  , dontShowWelcome_(false)
  , netautoin_(0)
  , netautoout_(0)
  , netautodev_(-1)
  , sceneChanged_(0)
  , loglines_(0)
  , need_instance_refresh_(false)
  , rendermethod_(0)
  , webcheckinterval_(24)
  , webautoupdate_(true)
  , webautocheck_(true)
  , plugins_(0)
  , state_(kStateInit)
  , app_library_waited_(0)
  #ifdef QT_OPENGL_LIB
  , glwidget_(0)
  #endif
  #if LH_USE_HID
  , hid_thread_(0)
  , hid_thread_waited_(0)
  #endif
  , systray_disabled_(false)
  , systray_(0)
  , systraymenu_(0)
  , systrayWarning_(false)
  , app_state_(NULL)
  , currentLayoutZoom(1)
  , pps(0)
{
  ui->setupUi(this);
  instTree_ = new AppInstanceTree(this);
  classTree_ = new AppClassTree(this);
  scene_ = new AppGraphicsScene(this);
  connect(ui->layoutView, SIGNAL(zoom(int)),
          this, SLOT(zoom(int)));
  return;
}

MainWindow::~MainWindow()
{
  return;
}

bool MainWindow::systrayStartup()
{
  if (systrayDisabled() || !QSystemTrayIcon::isSystemTrayAvailable())
    return false;

  if (!systraymenu_) {
    systraymenu_ = new QMenu(this);
    // systraymenu_->addAction(QLatin1String("Show window"), this, SLOT(showNormal()));
    // systraymenu_->addAction(QLatin1String("Open layout"), this, SLOT(on_actionOpen_triggered()));
    // systraymenu_->addAction(QLatin1String("Exit LCDHost"), this, SLOT(on_actionExit_triggered()));
    // qDebug("systraymenu_ actions: %d\n", systraymenu_->actions().size());
    systraymenu_->addAction(ui->actionSystrayRestore);
    systraymenu_->addAction(ui->actionSystrayOpen);
    systraymenu_->addAction(ui->actionSystrayQuit);
  }

  if (!systray_) {
    QIcon systray_icon;
#if defined(Q_OS_MAC)
    systray_icon = QIcon(":/lcdhost/images/LCDHost-GB.ico");
    // systray_ = new QSystemTrayIcon(QIcon(":/lcdhost/images/LCDHost-GB.ico"));
#elif defined(Q_OS_WIN)
    systray_icon = QIcon(":/lcdhost/images/LCDHost-Gray.ico");
    // systray_ = new QSystemTrayIcon(QIcon(":/lcdhost/images/LCDHost-Gray.ico"));
#else
    systray_icon = QIcon(
          QPixmap(
            ":/lcdhost/images/lcdhost128.png",
              0, Qt::ColorOnly | Qt::DiffuseAlphaDither
            ).scaled(
              QSize(22, 22), Qt::IgnoreAspectRatio, Qt::SmoothTransformation
            )
          );
#endif
    systray_ = new QSystemTrayIcon(systray_icon, this);
    systray_->setToolTip(tr("LCDHost"));
    systray_->setContextMenu(systraymenu_);
    //#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    connect( systray_, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             this, SLOT(systrayActivated(QSystemTrayIcon::ActivationReason)) );
    //#endif
    systray_->show();
  }

  if (!hasSystray()) {
    if (systray_) {
      qDebug() << "System tray failed, geometry " << systray_->geometry();
      delete systray_;
      systray_ = 0;
    }
    if (systraymenu_) {
      delete systraymenu_;
      systraymenu_ = 0;
    }
    return false;
  }

  return true;
}

void MainWindow::loadSettings()
{
  QSettings settings;

  layoutPath = settings.value( "layoutPath", QString() ).toString();
  layoutName = settings.value( "layoutName", QString() ).toString();

  settings.beginGroup("windows");

  settings.beginGroup("splitters");
  ui->splitterVLeft->restoreState( settings.value("VLeft", QByteArray() ).toByteArray() );
  ui->splitterVRight->restoreState( settings.value("VRight", QByteArray() ).toByteArray() );
  ui->splitterHLeft->restoreState( settings.value("HLeft", QByteArray() ).toByteArray() );
  ui->splitterHMid->restoreState( settings.value("HMid", QByteArray() ).toByteArray() );
  ui->splitterHRight->restoreState( settings.value("HRight", QByteArray() ).toByteArray() );
  ui->pluginsTabSplitter->restoreState( settings.value( "Plugins", QByteArray() ).toByteArray() );
  ui->outputSplitter->restoreState( settings.value( "Output", QByteArray() ).toByteArray() );
  ui->pluginsSplitterDescSettings->restoreState( settings.value( "PluginsDescSettings", QByteArray() ).toByteArray() );
  settings.endGroup();

  settings.beginGroup("instanceTree");
  ui->instanceTreeView->header()->restoreState( settings.value("state", QByteArray() ).toByteArray() );
  settings.endGroup();

  settings.beginGroup("layout");
  ui->layoutZoomSlider->setValue( settings.value("zoom", 1 ).toInt() );
  ui->layoutView->horizontalScrollBar()->setValue( settings.value("hscroll", ui->layoutView->horizontalScrollBar()->value() ).toInt() );
  ui->layoutView->verticalScrollBar()->setValue( settings.value("vscroll", ui->layoutView->verticalScrollBar()->value() ).toInt() );
  settings.endGroup();

  settings.beginGroup("devices");
  ui->cpuRefreshRate->setValue( settings.value("cpurefresh",10).toInt() );
  ui->memRefreshRate->setValue( settings.value("memrefresh",1).toInt() );
  ui->netRefreshRate->setValue( settings.value("netrefresh",5).toInt() );
  ui->netInterface->setCurrentIndex( settings.value("netinterface",0).toInt( ));
  ui->netMaxRateInComboBox->setCurrentIndex( settings.value("netinselector",0).toInt( ));
  ui->netMaxRateIn->setEnabled( ui->netMaxRateInComboBox->currentIndex() == 7 );
  ui->netMaxRateOutComboBox->setCurrentIndex( settings.value("netoutselector",0).toInt( ));
  ui->netMaxRateOut->setEnabled( ui->netMaxRateOutComboBox->currentIndex() == 7 );
  ui->netMaxRateIn->setValue( settings.value("netinmax",ui->netMaxRateIn->value()).toInt() );
  ui->netMaxRateOut->setValue( settings.value("netoutmax",ui->netMaxRateOut->value()).toInt() );
  netautoin_ = settings.value( "netautoin", 0 ).toInt();
  netautoout_ = settings.value( "netautoout", 0 ).toInt();
  ui->fpsLimit->setValue( settings.value( "fpslimit", 30 ).toInt() );
  wantedDevId_ = settings.value("output",QString()).toString();
  if (AppRawInput* ri = AppRawInput::instance())
    ri->setCapturedList(settings.value("inputs",QStringList()).toStringList());
  settings.endGroup();

  settings.beginGroup("debug");
  lh_log_load = settings.value("logLoad",true).toBool();
  settings.endGroup();

  ui->tabWidget->setCurrentIndex(5); // start with Log tab, load last used in later state
  /*
  settings.beginGroup("tab");
  ui->tabWidget->setCurrentIndex( settings.value("index", 0 ).toInt() );
  ui->tabWidget->setEnabled(true);
  settings.endGroup();
  */

  settings.beginGroup("main");
  ui->renderMethod->setCurrentIndex( settings.value( "renderMethod", 1 ).toInt() );
  systrayWarning_ = settings.value( "systrayWarning", false ).toBool();
  if( settings.value("debug",true).toBool() )
    ui->menuTools->addAction("Debug",this,SLOT(openDebugDialog()) );
  restoreState( settings.value("state", QByteArray() ).toByteArray() );
  restoreGeometry( settings.value("geometry", QByteArray() ).toByteArray() );
  settings.endGroup();

  settings.endGroup(); // windows

  settings.beginGroup("settings");
  webautocheck_ = settings.value("webautocheck", true).toBool();
  webautoupdate_ = settings.value("webautoupdate", true).toBool();
  webcheckinterval_ = settings.value("webcheckinterval", 24).toInt();
  if( webcheckinterval_ < 6 ) webcheckinterval_ = 6;
  if( webcheckinterval_ > 99 ) webcheckinterval_ = 99;
  settings.endGroup();

  dontShowWelcome_ = settings.value("dontShowWelcome", false).toBool();
}

void MainWindow::saveSettings()
{
  QSettings settings;
  // layoutPath and layoutName are set on File/Open
  settings.beginGroup("windows");
  settings.beginGroup("main");
  settings.setValue( "renderMethod", rendermethod_ );
  settings.setValue( "systrayWarning", systrayWarning_ );
  settings.setValue( "geometry", saveGeometry() );
  settings.setValue( "state", saveState() );
  settings.endGroup();
  settings.beginGroup("tab");
  settings.setValue( "index", ui->tabWidget->currentIndex() );
  settings.endGroup();
  settings.beginGroup("splitters");
  settings.setValue( "VLeft", ui->splitterVLeft->saveState() );
  settings.setValue( "VRight", ui->splitterVRight->saveState() );
  settings.setValue( "HLeft", ui->splitterHLeft->saveState() );
  settings.setValue( "HMid", ui->splitterHMid->saveState() );
  settings.setValue( "HRight", ui->splitterHRight->saveState() );
  settings.setValue( "Plugins", ui->pluginsTabSplitter->saveState() );
  settings.setValue( "PluginsDescSettings", ui->pluginsSplitterDescSettings->saveState() );
  settings.setValue( "Output", ui->outputSplitter->saveState() );
  settings.endGroup();
  settings.beginGroup("instanceTree");
  settings.setValue( "state", ui->instanceTreeView->header()->saveState() );
  settings.endGroup();
  settings.beginGroup("layout");
  settings.setValue( "zoom", ui->layoutZoomSlider->value() );
  settings.setValue("hscroll", ui->layoutView->horizontalScrollBar()->value() );
  settings.setValue("vscroll", ui->layoutView->verticalScrollBar()->value() );
  settings.endGroup();
  settings.beginGroup("devices");
  settings.setValue( "cpurefresh", ui->cpuRefreshRate->value() );
  settings.setValue( "memrefresh", ui->memRefreshRate->value() );
  settings.setValue( "netrefresh", ui->netRefreshRate->value() );
  settings.setValue( "netinmax", ui->netMaxRateIn->value() );
  settings.setValue( "netoutmax", ui->netMaxRateOut->value() );
  settings.setValue( "netinterface", ui->netInterface->currentIndex() );
  settings.setValue( "netinselector", ui->netMaxRateInComboBox->currentIndex() );
  settings.setValue( "netoutselector", ui->netMaxRateOutComboBox->currentIndex() );
  settings.setValue( "netautoin", netautoin_ );
  settings.setValue( "netautoout", netautoout_ );
  settings.setValue( "output", wantedDevId_ );
  settings.setValue( "fpslimit", ui->fpsLimit->value() );
  if (AppRawInput* ri = AppRawInput::instance())
    settings.setValue( "inputs", ri->getCapturedList());
  settings.endGroup();

  settings.beginGroup("debug");
  settings.setValue( "logLoad", lh_log_load );
  settings.endGroup();

  settings.endGroup(); // windows

  settings.beginGroup("settings");
  settings.setValue("webautocheck",webautocheck_);
  settings.setValue("webautoupdate",webautoupdate_);
  settings.setValue("webcheckinterval",webcheckinterval_);
  settings.endGroup();

  settings.sync();

  if( settings.status() != QSettings::NoError )
    QMessageBox::warning(this,tr("Failed to save settings"),tr("LCDHost failed to store the settings.") );
}

void MainWindow::init()
{
  Q_ASSERT(!plugins_);

  app_state_ = new AppState(this);
  AppRawInput::CreateInstance();

  if (layoutPath.isEmpty())
    layoutPath = layoutPathDefault();
  if (layoutName.isEmpty())
    layoutName = layoutNameDefault();

  plugins_ = new AppObject(this);
  plugins_->setObjectName("plugins");

  programStart_ = QDateTime::currentDateTime();
  vercache_.add("LCDHost", AppPluginVersion::makeUrl("http://www.linkdata.se/lcdhost/version.php?arch=$ARCH") );

  // connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(term()));

  // installPath = settings.value( "installPath", QCoreApplication::applicationDirPath() ).toString();
#ifdef QT_OPENGL_LIB
  ui->renderMethod->addItem("OpenGL");
#endif

  QString version_string = QString("LCDHost version %1 (Qt %2, %3 bits)").arg(VERSION).arg(QT_VERSION_STR).arg(QSysInfo::WordSize);
  ui->myVersion->setText(version_string);
  qDebug() << ui->myVersion->text() << "changeset" << REVISION << CHANGESET;

  qDebug() << "image formats" << QImageReader::supportedImageFormats();
  qDebug() << "executable" << QCoreApplication::applicationFilePath();
  qDebug() << "currentDir" << QDir::current().path();
  qDebug() << "libraryPaths" << QCoreApplication::libraryPaths();
  qDebug() << "binaries" << AppState::instance()->dir_binaries();
  qDebug() << "plugins" << AppState::instance()->dir_plugins();
  qDebug() << "data" << AppState::instance()->dir_data();

#ifdef Q_OS_WIN
  const char *osname = "unknown";
  switch( QSysInfo::windowsVersion() )
  {
    case QSysInfo::WV_32s:      osname = "3.1 with Win 32s"; break;
    case QSysInfo::WV_95:	osname = "95"; break;
    case QSysInfo::WV_98:	osname = "98"; break;
    case QSysInfo::WV_Me:	osname = "Me"; break;
    case QSysInfo::WV_NT:	osname = "NT"; break;
    case QSysInfo::WV_2000:	osname = "2000"; break;
    case QSysInfo::WV_XP:	osname = "XP"; break;
    case QSysInfo::WV_2003:	osname = "Server 2003"; break;
    case QSysInfo::WV_VISTA:	osname = "Vista"; break;
    case QSysInfo::WV_WINDOWS7:	osname = "7"; break;
    case QSysInfo::WV_CE:	osname = "CE"; break;
    case QSysInfo::WV_CENET:	osname = "CE .NET"; break;
    case QSysInfo::WV_CE_5:	osname = "CE 5.x"; break;
    case QSysInfo::WV_CE_6:	osname = "CE 6.x"; break;
    default:                    break;
  }
  const char *osarch = "unknown";
  SYSTEM_INFO sysinfo;
  GetNativeSystemInfo(&sysinfo);
  switch (sysinfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: osarch = "amd64"; break;
    case PROCESSOR_ARCHITECTURE_ARM: osarch = "arm"; break;
    case PROCESSOR_ARCHITECTURE_IA64: osarch = "ia64"; break;
    case PROCESSOR_ARCHITECTURE_INTEL: osarch = "x86"; break;
  }
  ui->osVersion->setText(QString("Windows %1 (%2), Qt %3")
                         .arg(osname)
                         .arg(osarch)
                         .arg(qVersion()));
#endif

#ifdef Q_OS_MAC
  const char *osname = "unknown";
  switch( QSysInfo::MacintoshVersion )
  {
    case QSysInfo::MV_CHEETAH:      osname = "Cheetah"; break;
    case QSysInfo::MV_PUMA:         osname = "Puma"; break;
    case QSysInfo::MV_JAGUAR:       osname = "Jaguar"; break;
    case QSysInfo::MV_PANTHER:      osname = "Panther"; break;
    case QSysInfo::MV_TIGER:        osname = "Tiger"; break;
    case QSysInfo::MV_LEOPARD:      osname = "Leopard"; break;
    case QSysInfo::MV_SNOWLEOPARD:  osname = "Snow Leopard"; break;
    case QSysInfo::MV_LION:         osname = "Lion"; break;
    case QSysInfo::MV_MOUNTAINLION: osname = "Mountain Lion"; break;
    case QSysInfo::MV_MAVERICKS:    osname = "Mavericks"; break;
    case QSysInfo::MV_YOSEMITE:     osname = "Yosemite"; break;
    case QSysInfo::MV_ELCAPITAN:    osname = "El Capitan"; break;
    default:                        break;
  }
  ui->osVersion->setText( QString("OS/X %1, Qt %2").arg(osname).arg(qVersion()) );
#endif

#ifdef Q_OS_LINUX
  ui->osVersion->setText( QString("X11, Qt %1").arg(qVersion()) );
#endif

  qDebug() << ui->osVersion->text();

  QNetworkProxyFactory::setUseSystemConfiguration( true );

#ifdef Q_OS_MAC
  AppOSXInstall();
#endif

#ifdef QT_OPENGL_LIB
  if( QGLFormat::hasOpenGL() && QGLPixelBuffer::hasOpenGLPbuffers() ) qDebug() << "OpenGL pbuffers are available";
  if( QGLFramebufferObject::hasOpenGLFramebufferObjects() ) qDebug() << "OpenGL frame buffer objects are available";
#endif

  // networkaccessmanager_ = new QNetworkAccessManager(this);
  // connect( networkaccessmanager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(logNetworkReply(QNetworkReply*)) );

  tree()->createRoot();

  ui->cpuCount->setText( QString::number( app_state_->cpuEngine().count() ));

  ui->netInterface->clear();
  ui->netInterface->addItem( tr("All available") );
  ui->netInterface->addItems( app_state_->netEngine().list() );

  scene()->setSceneRect( 0, 0, 320, 240 );
  connect( scene(), SIGNAL(changed(const QList<QRectF> &)), this, SLOT(sceneChanged(const QList<QRectF>&)) );
  currentLayoutZoom = 1;

  QDir layoutDir( layoutPath );
  layoutDir.mkpath( layoutPath );

  setWindowTitle( "<new layout>[*] - LCDHost" );

  ui->pluginList->setItemDelegate( new AppPluginItemDelegate( ui->pluginList ) );
  connect( ui->pluginList, SIGNAL(itemSelectionChanged()), this, SLOT(refreshPluginDetails()) );

  if (AppRawInput* ri = AppRawInput::instance()) {
    ri->refresh();
    ui->inputDeviceList->setModel(ri);
    connect( ui->inputDeviceList, SIGNAL(currentIndexChanged(QModelIndex)),
             this, SLOT(refreshInputDeviceDetails(QModelIndex)) );
    connect( ri, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this, SLOT(refreshInputDeviceDetails(QModelIndex)) );
  }

  ui->classTreeView->setEnabled(true);
  ui->classTreeView->setDragEnabled(true);
  ui->classTreeView->setAcceptDrops(true);
  ui->classTreeView->setDropIndicatorShown(true);
  ui->classTreeView->setDragDropMode( QAbstractItemView::DragDrop );
  ui->classTreeView->setModel( classTree() );
  // ui->classTreeView->setItemDelegate( new AppClassTreeDelegate(ui->classTreeView) );

  connect( ui->classTreeView, SIGNAL(wantPreview(QString)), ui->instancePreview, SLOT(previewClass(QString)) );

  ui->instanceTreeView->setDragEnabled(true);
  ui->instanceTreeView->setAcceptDrops(true);
  ui->instanceTreeView->setDropIndicatorShown(true);
  ui->instanceTreeView->setDragDropMode( QAbstractItemView::DragDrop );
  ui->instanceTreeView->setModel( tree() );
  connect( ui->instanceTreeView->model(), SIGNAL(modelReset()), this, SLOT(refreshInstanceDetails()) );
  connect( ui->instanceTreeView, SIGNAL(selectionChange(QItemSelection,QItemSelection)), this, SLOT(doRefreshInstanceDetails()) );

  connect( ui->layoutZoomSlider, SIGNAL(valueChanged(int)), ui->instancePreview, SLOT(recalcPixmap()) );

  ui->instanceDepth->setLineEditReadOnly(true);

  ui->layoutView->setAcceptDrops(true);
  ui->layoutView->setScene( scene() );

  QSize layoutSize( 320 + ui->layoutView->frameWidth() * 2, 240 + ui->layoutView->frameWidth() * 2 );
  ui->layoutView->setMinimumSize( layoutSize );

  QPalette pal;
  pal = ui->splitterVLeft->palette();
  pal.setColor( QPalette::Window, ui->tabWidget->palette().color( QPalette::Base ) );
  ui->splitterVLeft->setPalette( pal );
  pal = ui->pluginsTabSplitter->palette();
  pal.setColor( QPalette::Window, ui->tabWidget->palette().color( QPalette::Base ) );
  ui->pluginsTabSplitter->setPalette( pal );
  pal = ui->outputSplitter->palette();
  pal.setColor( QPalette::Window, ui->tabWidget->palette().color( QPalette::Base ) );
  ui->outputSplitter->setPalette( pal );
  pal = ui->pluginsSplitterDescSettings->palette();
  pal.setColor( QPalette::Window, ui->tabWidget->palette().color( QPalette::Base ) );
  ui->pluginsSplitterDescSettings->setPalette( pal );

  connect( scene(), SIGNAL(selectionChanged()), this, SLOT(sceneSelectionChanged()) );
  connect( scene(), SIGNAL(changed(const QList<QRectF> &)), this, SLOT(sceneChanged(const QList<QRectF>&)) );

  ui->instancePositionBox->setEnabled(false);
  ui->instanceSizeBox->setEnabled(false);

  // watcher_.addPath( app_state_->dir_plugins() );
  // connect( &watcher_, SIGNAL(directoryChanged(QString)), this, SLOT(directoryChanged()) );
  connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(term()));
  // metaObject()->invokeMethod(this,"refreshDeviceList",Qt::QueuedConnection);

  return;
}

void MainWindow::term() {
  if (state_ == kStateQuit)
    return;
  // qDebug("MainWindow::term() cleaning up\n");
  timer_.stop();
  if (state_ < kStateTerm)
    state_ = kStateTerm;
  while (state_ != kStateQuit)
    run();
  return;
}

QString MainWindow::layoutPathDefault()
{
  if (!app_state_)
    return QString();
  return app_state_->dir_data()+"layouts/g19-default/";
}

QString MainWindow::layoutNameDefault()
{
  return "g19-default.xml";
}

void MainWindow::clearLayout()
{
  while( !tree()->isEmpty() )
  {
    AppInstance *app_inst = tree()->list().first();
    tree()->removeAppInstance( app_inst );
    app_inst->term();
    Q_ASSERT( app_inst->id().hasAppObject() == false );
    delete app_inst;
  }
  Q_ASSERT( tree()->list().size() == 0 );
  Q_ASSERT( scene_->items().size() == 1 ); // root must still be there
  return;
}

void MainWindow::saveLayout()
{
  QDir dir( layoutPath );
  QFileInfo info( dir, layoutName );
  setWindowTitle( info.baseName() + "[*] - LCDHost" );

  Q_ASSERT( !info.suffix().isEmpty() );

  QFile xmlfile(info.filePath() );
  if( xmlfile.open(QIODevice::WriteOnly) == true )
  {
    qDebug() << "saving layout" << info.filePath();
    QXmlStreamWriter writer(&xmlfile);
    writer.setAutoFormatting(true);
    tree()->save( writer );
    setWindowModified( false );
    emit layoutSaved(layoutPath,layoutName);
    return;
  }
  else
  {
    QMessageBox::warning(this,tr("Save failed"),tr("Failed to save %1:\n%2")
                         .arg(layoutName).arg(xmlfile.errorString()));
  }

  return;
}

bool MainWindow::loadLayout()
{
  bool load_ok = false;
  QDir dir( layoutPath );
  QFileInfo info( dir, layoutName );
  QFile file( info.filePath() );

  // If the given layout doesn't exist, try to load it from the default
  // layout path. This helps the case where the user had one of the provided
  // layouts selected, uninstalled and then reinstalls in another location.
  if( !file.exists() && dir.canonicalPath() != QDir(AppState::instance()->dir_layout()).canonicalPath() )
  {
    qDebug() << file.fileName() << "doesn't exist, trying default layout directory";
    dir.setPath( AppState::instance()->dir_data().append("layouts") );
    info.setFile( dir, layoutName );
    file.setFileName( info.filePath() );
    if( file.exists() )
      layoutPath = dir.path();
  }

  if( file.open(QIODevice::ReadOnly) == false )
  {
    qWarning() << "can't load from" << file.fileName() << file.errorString();
    QMessageBox::warning(this,tr("Error"),tr("Failed to open layout '%1'\n%2").arg(info.filePath()).arg(file.errorString()));
    return false;
  }

  qDebug() << "loading layout" << info.filePath();
  setWindowTitle( info.baseName() + "[*] - LCDHost" );
  clearLayout();

  app_state_->setLayout( layoutPath, layoutName );

  if( info.suffix() == "xml" )
  {
    QXmlStreamReader in(&file);
    load_ok = tree()->load( in );
  }

  if( load_ok )
  {
    setWindowModified( false );
  }
  else
  {
    QMessageBox::warning(this,tr("Error"),tr("Failed to load layout"));
    clearLayout();
  }

  emit layoutChanged( app_state_->dir_layout(), app_state_->layout_file() );

  return load_ok;
}

QString MainWindow::previewClassId() const
{
  if( ui && ui->instancePreview ) return ui->instancePreview->classId();
  return QString();
}

AppInstance *MainWindow::previewInstance()
{
  if( ui && ui->instancePreview ) return ui->instancePreview->appInstance();
  return NULL;
}

bool MainWindow::hasSystray() const
{
  return !systrayDisabled()
      && systray_
      && systray_->isVisible()
#ifdef Q_OS_LINUX
      && systray_->geometry().x()
      && systray_->geometry().y()
#endif
      ;
//      && QSystemTrayIcon::isSystemTrayAvailable();
}

void MainWindow::logged(uint msgtime, int msgtype, QString msgtext)
{
  log(QDateTime::fromTime_t(msgtime), (QtMsgType) msgtype, msgtext);
  return;
}

void MainWindow::customEvent( QEvent *event )
{
  if( event->type() == EventRawInput::type() )
  {
    EventRawInput *e = static_cast<EventRawInput *>(event);
    if (AppRawInput* ri = AppRawInput::instance())
      ui->inputDeviceTestResult->setText(ri->describeEvent(e));
    return;
  }

  if( event->type() == EventDeviceCreate::type() )
  {
    refreshDeviceList();
    return;
  }

  if( event->type() == EventDeviceDestroy::type() )
  {
    refreshDeviceList();
    return;
  }

  if( event->type() == EventClearTimeout::type() )
  {
    EventClearTimeout *e = static_cast<EventClearTimeout*>(event);
    AppLibrary *app_library = AppLibrary::fromName(e->id);
    if( app_library )
      app_library->clearTimeout();
    else
      qDebug() << "MainWindow::event(): Unhandled EventClearTimeout for" << e->id;
    return;
  }

  if( event->type() == EventDestroyInstance::type() )
  {
    AppInstance *app_inst;
    EventDestroyInstance *e = static_cast<EventDestroyInstance*>(event);
    app_inst = AppInstance::fromId( e->id );
    if( app_inst )
    {
      layoutModified();
      tree()->removeAppInstance( app_inst );
      app_inst->term();
      delete app_inst;
    }
    return;
  }

  if( event->type() == EventSetupComplete::type() )
  {
    EventSetupComplete *e = static_cast<EventSetupComplete*>(event);
    if( e->receiver == selectedinstance_ )
      refreshInstanceUI( e->receiver );
    return;
  }

  if( event->type() == EventRefreshPlugins::type() )
  {
    loadPlugin();
    return;
  }

  if( event->type() == EventLibraryStart::type() )
  {
    EventLibraryStart *e = static_cast<EventLibraryStart*>(event);
    AppLibrary *app_lib = AppLibrary::fromId(e->id);
    if( app_lib )
    {
      Q_ASSERT( app_lib->isUsable() );
      app_lib->pi().setEnabled(true);
      app_lib->start();
      refreshPluginList();
      return;
    }
  }

  if (event->type() == LH_LoggerEvent::type()) {
    LH_LoggerEvent* e = static_cast<LH_LoggerEvent*>(event);
    log(e->msgTime(), e->msgType(), e->msgText());
    return;
  }

  qWarning() << "MainWindow::event() unhandled user event" << EventBase::name(event->type());
}

void MainWindow::log( QDateTime msgtime, QtMsgType msgtype, QString msgtext )
{
  const char * msgtypetext = "DBG";
  const char * msgtypecolor = "green";

  switch( msgtype )
  {
    case QtInfoMsg: break;
    case QtDebugMsg: break;
    case QtWarningMsg: msgtypetext = "WRN"; msgtypecolor = "orange"; break;
    case QtCriticalMsg: msgtypetext = "ERR"; msgtypecolor = "red"; break;
    case QtFatalMsg: msgtypetext = "FAT"; msgtypecolor = "red"; break;
  }

  while( msgtext.size() > 0 && msgtext.at( msgtext.size() - 1 ).isSpace() ) msgtext.chop( 1 );

  if( msgtext.endsWith( QLatin1String("FIXME!") ) )
  {
    msgtext.chop(6);
    if( fixmeLogs_.contains( msgtext ) ) return;
    fixmeLogs_.insert( msgtext );
    msgtypecolor = "blue";
    msgtypetext = "FIX";
  }

  if (ui && ui->logView) {
    QString logtext( msgtime.toString(QLatin1String("yyyy-MM-dd'&nbsp;'hh:mm:ss")) );
    logtext.append( "&nbsp;&nbsp;<tt style='color:" );
    logtext.append( msgtypecolor );
    logtext.append( "'>" );
    logtext.append( msgtypetext );
    logtext.append( "</tt>&nbsp;&nbsp;" );
    logtext.append( msgtext );
    ui->logView->appendHtml( logtext );
  }
}

static bool comparePluginName( const AppLibrary* a, const AppLibrary* b )
{
  if( a && b ) return a->name() < b->name();
  return 0;
}

QList<AppLibrary*> MainWindow::plugins() const
{
  QList<AppLibrary *> list(plugins_->findChildren<AppLibrary *>(QString(), Qt::FindDirectChildrenOnly));
  qStableSort( list.begin(), list.end(), comparePluginName );
  return list;
}

// look for unloaded plugins and try to load them
// returns true if a new plugin was loaded, false if no new ones found
bool MainWindow::loadPlugin()
{
  QString errmsg;
  QDir dir( app_state_->dir_plugins() );

  foreach( QFileInfo fileInfo, dir.entryInfoList(QDir::NoDotAndDotDot|QDir::Files|QDir::NoSymLinks) )
  {
    if(!QLibrary::isLibrary(fileInfo.fileName()))
      continue;

    AppLibrary* app_library = 0;
    foreach (app_library, plugins()) {
      if (app_library && app_library->fileInfo() == fileInfo)
        break;
      app_library = 0;
    }
    if (app_library)
      continue;

    PluginInfo pi;
    errmsg = pi.read( fileInfo );

    if(!pi.isAPI5Plugin())
      continue;

    if(!pi.isValid() || pi.api_major() != LH_API_MAJOR) {
      if( pi.isValid() )
        qWarning() << fileInfo.completeBaseName()
                   << "uses API" << pi.api_major()
                   << "while this LCDHost requires API" << LH_API_MAJOR;
      else
        qWarning() << fileInfo.completeBaseName()
                   << "has incorrectly formatted plugin data";
      continue;
    }

    app_library = new AppLibrary(pi, plugins_);
    app_library->init();

    if(app_library->isEnabled() && app_library->isUsable())
    {
      app_library->clearCrashes();
      app_library->start();
      refreshPluginList();
      refreshPluginDetails();
      // Only load one at a time
      return true;
    }

    // not enabled or not usable
    app_library->stop();
  }

  refreshPluginList();
  refreshPluginDetails();
  return false; // no new ones found
}

void MainWindow::stateChanged(AppId id)
{
  AppLibrary* app_library = qobject_cast<AppLibrary*>(id.appObject());
  if( app_library )
  {
    // Make sure a newly loaded library gets the initial state
    if( app_library->state() == AppLibrary::Loaded )
      app_library->layoutChanged( layoutPath, layoutName );
    refreshPluginList();
    emit libraryStateChanged(id);
  }
  return;
}

void MainWindow::refreshPluginList()
{
  QListWidgetItem *anItem;
  QModelIndex index;

  index = ui->pluginList->currentIndex();
  ui->pluginList->setUpdatesEnabled(false);
  ui->pluginList->clear();

  foreach(AppLibrary *app_library, plugins() )
  {
    QPixmap pm;

    switch( app_library->state() )
    {
      case AppLibrary::Invalid :
        app_library->term();
        delete app_library;
        continue;
      case AppLibrary::Orphanage :
        continue;
      case AppLibrary::Unloaded :
        pm = QPixmap(":/lcdhost/images/gray16.png");
        break;
      case AppLibrary::Loaded :
        pm = QPixmap(":/lcdhost/images/green16.png");
        break;
      case AppLibrary::Failed :
        pm = QPixmap(":/lcdhost/images/red16.png");
        break;
    }

    anItem = new QListWidgetItem();
    anItem->setData( Qt::DecorationRole, pm );
    anItem->setData( Qt::DisplayRole, app_library->name() );
    anItem->setData( Qt::UserRole, app_library->objectName() );
    anItem->setData( Qt::UserRole+1, app_library->description() );
    anItem->setData( Qt::UserRole+2, app_library->version() );
    anItem->setData( Qt::UserRole+3, app_library->revision() );

    AppPluginVersion apv = vercache_.get(app_library->objectName());
    if( apv.isUsable() ) anItem->setData( Qt::UserRole+4, apv.revision() );
    else anItem->setData( Qt::UserRole+4, 0 );

    anItem->setData( Qt::UserRole+5, app_library->error() );
    ui->pluginList->addItem( anItem );
  }

  if( index.isValid() ) ui->pluginList->setCurrentIndex(index);
  else ui->pluginList->setCurrentRow(0);
  ui->pluginList->setUpdatesEnabled(true);

  return;
}

void MainWindow::refreshPluginDetails()
{
  Q_ASSERT(ui);
  Q_ASSERT(ui->pluginList);
  Q_ASSERT(ui->pluginSettingsArea);

  QListWidgetItem *item = NULL;
  AppLibrary *app_library = NULL;
  QList<QListWidgetItem *> selitems;

  selitems = ui->pluginList->selectedItems();
  if( selitems.size() > 0 ) item = selitems.first();
  if( item != NULL ) app_library = AppLibrary::fromName( item->data(Qt::UserRole) );

  if( app_library == NULL )
  {
    ui->updatePluginButton->disconnect();
    ui->updatePluginButton->hide();
    ui->NameValue->clear();
    ui->VersionValue->clear();
    ui->FileValue->clear();
    ui->AuthorValue->clear();
    ui->HomepageValue->clear();
    ui->PluginComments->clear();
    ui->Logo->clear();
    ui->CrashCounter->clear();
    ui->LoadButton->setEnabled(false);
    if( ui->pluginSettingsArea && ui->pluginSettingsArea->widget() )
      delete ui->pluginSettingsArea->takeWidget();
    return;
  }

  AppPluginVersion apv = vercache_.get( app_library->objectName() );

  if( apv.isUsable() && apv.revision() != app_library->revision() )
  {
    if( apv.revision() > app_library->revision() ) ui->updatePluginButton->setText("Update");
    else ui->updatePluginButton->setText(tr("Downgrade to revision %1").arg(apv.revision()));
    ui->updatePluginButton->disconnect();
    ui->updatePluginButton->show();
    connect( ui->updatePluginButton, SIGNAL(clicked()), app_library, SLOT(requestWebUpdate()) );
  }
  else
  {
    ui->updatePluginButton->disconnect();
    ui->updatePluginButton->hide();
  }

  ui->pluginSettingsArea->setWidget( app_library->setupBuildUI( ui->pluginSettingsArea ) );

  ui->NameValue->setText( app_library->name() );
  ui->VersionValue->setText( app_library->version() );
  ui->FileValue->setText( app_library->fileInfo().fileName() );
  ui->AuthorValue->setText( app_library->author() );
  ui->HomepageValue->setText( app_library->homepage() );

  QRegExp hpurl("<a href=\\\"([^\\\"]*)\\\">");
  hpurl.setCaseSensitivity(Qt::CaseInsensitive);
  if( hpurl.indexIn(app_library->homepage()) != -1 )
    ui->HomepageValue->setToolTip( hpurl.cap(1) );

  ui->PluginComments->setText( app_library->comments() );
  ui->Logo->setPixmap( QPixmap::fromImage( app_library->logo() ) );

  if( app_library->crashes() < 1 ) ui->CrashCounter->setText( tr("None") );
  else ui->CrashCounter->setText( QString::number(app_library->crashes()) );

  ui->LoadButton->setEnabled( true );

  switch( app_library->state() )
  {
    case AppLibrary::Orphanage :
    case AppLibrary::Invalid :
      break;
    case AppLibrary::Unloaded :
      ui->LoadButton->setText(tr("Load"));
      break;
    case AppLibrary::Loaded :
      ui->LoadButton->setText(tr("Unload"));
      break;
    case AppLibrary::Failed :
      ui->LoadButton->setText(tr("Reload"));
      break;
  }

  return;
}

void MainWindow::refreshDeviceList()
{
  QListWidgetItem *item;
  QListWidgetItem *bestItem;
  int bestpixels, pixels;
  QFont font;

  // rebuild the device list
  ui->outputDeviceList->clear();
  foreach( AppLibrary *app_drv, plugins() )
  {
    foreach( QObject *obj, app_drv->children() )
    {
      AppDevice *app_device = qobject_cast<AppDevice *>(obj);
      if( app_device )
      {
        if( app_device->isValid() )
        {
          item = new QListWidgetItem( app_device->name() );
          item->setFlags( Qt::ItemIsSelectable|Qt::ItemIsEnabled );
          item->setCheckState( Qt::Unchecked );
          item->setData( Qt::UserRole, app_device->id() );
          item->setToolTip( app_device->id().toString() );
          ui->outputDeviceList->addItem( item );
        }
      }
    }
  }

  // locate the best device available
  bestItem = NULL;
  bestpixels = 0;
  for( int i=0; i<ui->outputDeviceList->count(); i++ )
  {
    item = ui->outputDeviceList->item(i);
    AppDevice *app_device = AppDevice::fromId( item->data(Qt::UserRole) );
    if( app_device && app_device->isValid() )
    {
      if( app_device->devId() == wantedDevId_ )
      {
        // preferred device is online, use that
        bestItem = item;
        bestpixels = -1;
      }
      if( bestpixels >= 0 )
      {
        pixels = app_device->size().width() * app_device->size().height();
        if( pixels > bestpixels )
        {
          bestItem = item;
          bestpixels = pixels;
        }
      }
    }
  }

  // Activate the best option
  if( bestItem &&
      (!AppDevice::current().isValid() ||
       AppDevice::current().id() != bestItem->data(Qt::UserRole))
      )
  {
    AppDevice *app_device = AppDevice::fromId( bestItem->data(Qt::UserRole) );
    if( app_device && app_device->isValid() && app_device->devId() != AppDevice::current().devId() )
    {
        AppDevice::setCurrent( app_device );

      font = bestItem->font();
      font.setBold(true);
      bestItem->setFont( font );
      if( app_device->devId() == wantedDevId_ )
        bestItem->setCheckState( Qt::Checked );


      if( scene() ) scene()->setSceneRect(0,0,app_device->size().width(),app_device->size().height());
      if( tree() ) tree()->deviceChanged( app_device->size().width(), app_device->size().height(), app_device->depth() );

      ui->layoutView->setMinimumSize(
            QSize( app_device->size().width() + ui->layoutView->frameWidth() * 2,
                   app_device->size().height() + ui->layoutView->frameWidth() * 2)
            );
      ui->layoutView->setSceneRect(
            QRectF( -app_device->size().width(), -app_device->size().height(),
                    app_device->size().width()*3, app_device->size().height()*3 )
            );
      ui->layoutView->centerOn( app_device->size().width()/2, app_device->size().height()/2 );
      ui->outputDeviceSettings->setWidget( app_device->setupBuildUI(ui->outputDeviceSettings) );

      qDebug() << "Device changed to" << app_device->id();
      foreach( AppLibrary *app_library, plugins() )
        app_library->id().postLibEvent( new EventDeviceChanged( app_device->size(), app_device->depth()) );
    }
  }
}

void MainWindow::on_actionExit_triggered()
{
  if (state_ < kStateTerm) {
    if (state_ == kStateRun && cancelIfModified())
        return;
    state_ = kStateTerm;
  }
  return;
}

void MainWindow::closeEvent( QCloseEvent * event )
{
  if(hasSystray())
  {
    if( systrayWarning_ || QMessageBox::information(this,tr("Notice"),
                                                    tr("LCDHost will keep running in the system notification area.\n"
                                                       "To show the main window again, click it's icon there.\n"
                                                       "To exit the application, select Exit from the File menu."),
                                                    QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
    {
      systrayWarning_ = true;
      hide();
    }
    event->ignore();
  }
  else
  {
    on_actionExit_triggered();
  }
  return;
}

void MainWindow::on_LoadButton_clicked()
{
  QListWidgetItem *item;
  AppLibrary *app_library;

  item = ui->pluginList->item(ui->pluginList->currentRow());
  if( item )
  {
    app_library = AppLibrary::fromName( item->data(Qt::UserRole) );
    if( app_library )
    {
      if( app_library->state() == AppLibrary::Loaded )
      {
        app_library->stop();
        app_library->pi().setEnabled(false);
      }
      else
      {
        if( !app_library->isUsable() )
          qWarning() << app_library->objectName() << "has incompatible API" << app_library->apiVersion();
        app_library->pi().setEnabled(true);
        app_library->start();
      }
      // refreshPluginList();
    }
  }
  return;
}

void MainWindow::on_instanceTreeView_selectionChange( const QItemSelection & selected, const QItemSelection & deselected )
{
  AppInstance *app_inst;

  foreach( QModelIndex index, deselected.indexes() )
  {
    app_inst = tree()->itemFromIndex( index );
    if( app_inst )
    {
      // app_inst->disconnect( this, 0 );
      app_inst->setSelected( false );
    }
  }
  foreach( QModelIndex index, selected.indexes() )
  {
    app_inst = tree()->itemFromIndex( index );
    if( app_inst ) app_inst->setSelected( true );
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::addParentItem( AppInstance *app_inst )
{
  if( app_inst->parentItem() ) addParentItem( app_inst->parentItem() );
  ui->instanceParent->addItem( app_inst->objectName(), QVariant::fromValue((void*)app_inst) );
  return;
}

void MainWindow::doRefreshInstanceDetails()
{
  QList<AppInstance*> list;
  AppInstance *app_inst;
  AppInstance *first_inst;

  need_instance_refresh_ = false;

#define UNIQUE_DECL(x) bool unique_##x = true;
#define UNIQUE_TEST(x) if( unique_##x && ( first_inst->x() != app_inst->x() ) ) unique_##x = false;

  UNIQUE_DECL(objectName);
  UNIQUE_DECL(xOffset);
  UNIQUE_DECL(yOffset);
  UNIQUE_DECL(parentItem);
  UNIQUE_DECL(xRef);
  UNIQUE_DECL(yRef);
  UNIQUE_DECL(xAlign);
  UNIQUE_DECL(yAlign);
  UNIQUE_DECL(xRefAlign);
  UNIQUE_DECL(yRefAlign);
  UNIQUE_DECL(wRef);
  UNIQUE_DECL(wMod);
  UNIQUE_DECL(wUnits);
  UNIQUE_DECL(hRef);
  UNIQUE_DECL(hMod);
  UNIQUE_DECL(hUnits);
  UNIQUE_DECL(isAlertEnabled);
  UNIQUE_DECL(isModifiable);
  UNIQUE_DECL(isSelectable);
  UNIQUE_DECL(classId);

  list = selectedInstances();

  // Disable and clear if nothing selected
  if( list.size() < 1 )
  {
    ui->instanceGeneralBox->setEnabled(false);
    ui->instanceName->clear();
    ui->instanceParent->clear();
    ui->instanceClass->clear();
    ui->instancePosSize->clear();
    ui->instanceDepth->clear();
    ui->instanceAlerts->setCheckState( Qt::PartiallyChecked );

    ui->instancePositionBox->setEnabled(false);
    ui->instanceXOffset->clear();
    ui->instanceXAlign->setCurrentIndex(-1);
    ui->instanceXSide->setCurrentIndex(-1);
    ui->instanceXRef->setCurrentIndex(-1);
    ui->instanceYOffset->clear();
    ui->instanceYAlign->setCurrentIndex(-1);
    ui->instanceYSide->setCurrentIndex(-1);
    ui->instanceYRef->setCurrentIndex(-1);

    ui->instanceSizeBox->setEnabled(false);
    ui->instanceWRef->setCurrentIndex(-1);
    ui->instanceWidthMod->clear();
    ui->instanceWidthUnits->setCurrentIndex(-1);
    ui->instanceHRef->setCurrentIndex(-1);
    ui->instanceHeightMod->clear();
    ui->instanceHeightUnits->setCurrentIndex(-1);

    setFocusInstance(NULL);
    return;
  }

  first_inst = list.first();

  // one or more selected
  ui->instanceGeneralBox->setEnabled(true);
  ui->instancePositionBox->setEnabled(true);
  ui->instanceSizeBox->setEnabled(true);

  // Only one selected, load configuration
  if( list.size() == 1 )
  {
    setFocusInstance( first_inst );

    ui->instancePosSize->setText(
          QString("%1,%2 %3x%4")
          .arg( (int) first_inst->scenePos().x() )
          .arg( (int) first_inst->scenePos().y() )
          .arg( (int) first_inst->size().width() )
          .arg( (int) first_inst->size().height() )
          );

    ui->instanceDepth->setValue( (int) first_inst->zValue() );
    ui->instanceDepth->setMaximum( first_inst->parentItem()->childItems().size()-1 );
  }
  else
  {
    setFocusInstance(NULL);
    ui->instancePosSize->clear();
    ui->instanceDepth->clear();
  }

  foreach( app_inst, list )
  {
    Q_ASSERT( app_inst != tree()->root() ); // should never be able to select root
    UNIQUE_TEST(objectName);
    UNIQUE_TEST(xOffset);
    UNIQUE_TEST(yOffset);
    UNIQUE_TEST(parentItem);
    UNIQUE_TEST(xRef);
    UNIQUE_TEST(yRef);
    UNIQUE_TEST(xAlign);
    UNIQUE_TEST(yAlign);
    UNIQUE_TEST(xRefAlign);
    UNIQUE_TEST(yRefAlign);
    UNIQUE_TEST(wRef);
    UNIQUE_TEST(wMod);
    UNIQUE_TEST(wUnits);
    UNIQUE_TEST(hRef);
    UNIQUE_TEST(hMod);
    UNIQUE_TEST(hUnits);
    UNIQUE_TEST(isAlertEnabled);
    UNIQUE_TEST(isModifiable);
    UNIQUE_TEST(isSelectable);
    UNIQUE_TEST(classId);
  }

  if( unique_objectName ) ui->instanceName->setText( first_inst->objectName() );
  else ui->instanceName->clear();

  // instanceKeypress here

  // comboboxes
  ui->instanceParent->clear();
  ui->instanceXRef->clear();
  ui->instanceYRef->clear();
  ui->instanceWRef->clear();
  ui->instanceHRef->clear();
  ui->instanceWRef->addItem( tr("<Plugin>") );
  ui->instanceHRef->addItem( tr("<Plugin>") );
  if( unique_parentItem )
  {
    int index_parent;
    AppInstance *parent_inst;
    parent_inst = first_inst->parentItem();
    addParentItem( parent_inst );
    index_parent = ui->instanceParent->count() - 1;

    ui->instanceXRef->addItem( parent_inst->objectName(), parent_inst->id() );
    ui->instanceYRef->addItem( parent_inst->objectName(), parent_inst->id() );
    ui->instanceWRef->addItem( parent_inst->objectName(), parent_inst->id() );
    ui->instanceHRef->addItem( parent_inst->objectName(), parent_inst->id() );
    foreach( app_inst, parent_inst->childItemsByZ() )
    {
      ui->instanceParent->addItem( app_inst->objectName(), app_inst->id() );
      ui->instanceXRef->addItem( app_inst->objectName(), app_inst->id() );
      ui->instanceYRef->addItem( app_inst->objectName(), app_inst->id() );
      ui->instanceWRef->addItem( app_inst->objectName(), app_inst->id() );
      ui->instanceHRef->addItem( app_inst->objectName(), app_inst->id() );
    }
    ui->instanceParent->setCurrentIndex( index_parent );

    int class_index = -1;
    ui->instanceClass->clear();
    foreach( AppLibrary *app_lib, plugins() )
    {
      foreach( AppClass *app_class, app_lib->classes() )
      {
        ui->instanceClass->addItem( app_class->id() );
        if( app_class->id() == first_inst->classId() )
          class_index = ui->instanceClass->count()-1;
      }
    }
    ui->instanceClass->setCurrentIndex(class_index);
  }
  else
  {
    ui->instanceParent->addItem( tree()->root()->objectName(), tree()->root()->id() );
    foreach( app_inst, list )
      ui->instanceParent->addItem( app_inst->objectName(), app_inst->id() );
    ui->instanceParent->setCurrentIndex(-1);
    ui->instanceClass->setCurrentIndex(-1);
  }

  // X
  if( unique_xRef )
  {
    if( first_inst->xRef() == NULL ) app_inst = first_inst->parentItem();
    else app_inst = first_inst->xRef();
    if( app_inst )
    {
      int i;
      for( i=0; i<ui->instanceXRef->count(); i++ )
      {
        if( ui->instanceXRef->itemData(i).value<AppId>() == app_inst->id() )
        {
          ui->instanceXRef->setCurrentIndex(i);
          break;
        }
      }
      if( i>=ui->instanceXRef->count() )
      {
        ui->instanceXRef->addItem( app_inst->objectName(), app_inst->id() );
        ui->instanceXRef->setCurrentIndex( ui->instanceXRef->count()-1 );
      }
    }
  }
  else ui->instanceXRef->setCurrentIndex(-1);

  if( unique_xOffset ) ui->instanceXOffset->setValue( (int) first_inst->xOffset() );
  else ui->instanceXOffset->clear();

  if( unique_xAlign ) ui->instanceXAlign->setCurrentIndex( first_inst->xAlign() );
  else ui->instanceXAlign->setCurrentIndex(-1);

  if( unique_xRefAlign ) ui->instanceXSide->setCurrentIndex( first_inst->xRefAlign() );
  else ui->instanceXSide->setCurrentIndex(-1);


  // Y
  if( unique_yRef )
  {
    if( first_inst->yRef() == NULL ) app_inst = first_inst->parentItem();
    else app_inst = first_inst->yRef();
    if( app_inst )
    {
      int i;
      for( i=0; i<ui->instanceYRef->count(); i++ )
      {
        if( app_inst->id() ==  ui->instanceYRef->itemData(i) )
        {
          ui->instanceYRef->setCurrentIndex(i);
          break;
        }
      }
      if( i>=ui->instanceYRef->count() )
      {
        ui->instanceYRef->addItem( app_inst->objectName(), app_inst->id() );
        ui->instanceYRef->setCurrentIndex( ui->instanceYRef->count()-1 );
      }
    }
  }
  else ui->instanceYRef->setCurrentIndex(-1);

  if( unique_yOffset ) ui->instanceYOffset->setValue( (int) first_inst->yOffset() );
  else ui->instanceYOffset->clear();

  if( unique_yAlign ) ui->instanceYAlign->setCurrentIndex( first_inst->yAlign() );
  else ui->instanceYAlign->setCurrentIndex(-1);

  if( unique_yRefAlign ) ui->instanceYSide->setCurrentIndex( first_inst->yRefAlign() );
  else ui->instanceYSide->setCurrentIndex(-1);


  // W
  if( unique_wRef )
  {
    if( first_inst->wRef() == NULL ) ui->instanceWRef->setCurrentIndex(0);
    else
    {
      app_inst = first_inst->wRef();
      int i;
      for( i=0; i<ui->instanceWRef->count(); i++ )
      {
        if( app_inst->id() == ui->instanceWRef->itemData(i)  )
        {
          ui->instanceWRef->setCurrentIndex(i);
          break;
        }
      }
      if( i>=ui->instanceWRef->count() )
      {
        ui->instanceWRef->addItem( app_inst->objectName(), app_inst->id() );
        ui->instanceWRef->setCurrentIndex( ui->instanceWRef->count()-1 );
      }
    }
  }
  else ui->instanceWRef->setCurrentIndex(-1);

  ui->instanceWidthMod->setEnabled( true );
  if( unique_wMod ) ui->instanceWidthMod->setValue( first_inst->wMod() );
  else ui->instanceWidthMod->clear();

  ui->instanceWidthUnits->setEnabled( true );
  if( unique_wUnits ) ui->instanceWidthUnits->setCurrentIndex( first_inst->wUnits() );
  else ui->instanceWidthUnits->setCurrentIndex(-1);

  // H
  if( unique_hRef )
  {
    if( first_inst->hRef() == NULL ) ui->instanceHRef->setCurrentIndex(0);
    else
    {
      app_inst = first_inst->hRef();
      int i;
      for( i=0; i<ui->instanceHRef->count(); i++ )
      {
        if( ui->instanceHRef->itemData(i).value<AppId>() == app_inst->id() )
        {
          ui->instanceHRef->setCurrentIndex(i);
          break;
        }
      }
      if( i>=ui->instanceHRef->count() )
      {
        ui->instanceHRef->addItem( app_inst->objectName(), app_inst->id() );
        ui->instanceHRef->setCurrentIndex( ui->instanceHRef->count()-1 );
      }
    }
  }
  else ui->instanceHRef->setCurrentIndex(-1);

  ui->instanceHeightMod->setEnabled( true );
  if( unique_hMod ) ui->instanceHeightMod->setValue( first_inst->hMod() );
  else ui->instanceHeightMod->clear();

  ui->instanceHeightUnits->setEnabled( true );
  if( unique_hUnits ) ui->instanceHeightUnits->setCurrentIndex( first_inst->hUnits() );
  else ui->instanceHeightUnits->setCurrentIndex(-1);

  // parent
  if( unique_parentItem )
  {
    ui->instanceXRef->setEnabled(true);
    ui->instanceYRef->setEnabled(true);
    ui->instanceWRef->setEnabled(true);
    ui->instanceHRef->setEnabled(true);
  }
  else
  {
    ui->instanceXRef->setEnabled(false);
    ui->instanceYRef->setEnabled(false);
    ui->instanceWRef->setEnabled(false);
    ui->instanceHRef->setEnabled(false);
  }

  if( unique_isAlertEnabled )
  {
    ui->instanceAlerts->setTristate( false );
    ui->instanceAlerts->setCheckState( first_inst->isAlertEnabled() ? Qt::Checked : Qt::Unchecked );
  }
  else
  {
    ui->instanceAlerts->setTristate( true );
    ui->instanceAlerts->setCheckState( Qt::PartiallyChecked );
  }

  if( !first_inst->isModifiable() || !unique_isModifiable )
  {
    ui->instanceGeneralBox->setEnabled( false );
    ui->instancePositionBox->setEnabled( false );
    ui->instanceSizeBox->setEnabled( false );
    ui->instancePositionBox->setEnabled( false );
    ui->instanceSizeBox->setEnabled( false );
  }
  else
  {
    ui->instanceGeneralBox->setEnabled( true );
    ui->instancePositionBox->setEnabled( true );
    ui->instanceSizeBox->setEnabled( true );
  }

  return;
}

void MainWindow::refreshInstanceUI( AppId id )
{
  if( id.isEmpty() || (selectedinstance_ == id) )
  {
    AppInstance *app_inst = AppInstance::fromId(id);
    if( app_inst )
    {
      ui->instanceSettingsArea->setWindowTitle(tr("Settings - %1").arg(app_inst->classId()) );
      ui->instanceSettingsArea->setWidget( app_inst->setupBuildUI(ui->instanceSettingsArea) );
    }
  }
  return;
}

void MainWindow::setFocusInstance( AppInstance *app_inst )
{
  if( app_inst && (app_inst->id() == selectedinstance_) ) return;

  if( !selectedinstance_.isEmpty() )
  {
    if( ui->instanceSettingsArea->widget() ) delete ui->instanceSettingsArea->takeWidget();
    ui->instanceSettingsArea->setWindowTitle(tr("Settings - (No instance selected)"));
  }

  selectedinstance_.clear();

  if( app_inst )
  {
    selectedinstance_ = app_inst->id();
    refreshInstanceUI( selectedinstance_ );
    connect( app_inst, SIGNAL(destroyed()), this, SLOT(setFocusInstance()) );
  }

  return;
}

AppClass* MainWindow::selectedClass()
{
  return NULL;
}

QList<AppInstance*> MainWindow::selectedInstances()
{
  QItemSelectionModel *selmodel;
  QList<AppInstance*> list;

  selmodel = ui->instanceTreeView->selectionModel();
  foreach( QModelIndex index, selmodel->selectedIndexes() )
  {
    if( index.column() == 0 )
    {
      Q_ASSERT( tree()->indexFromItem( tree()->itemFromIndex(index) ) == index );
      list.append( tree()->itemFromIndex(index) );
    }
  }

  return list;
}

// Changes the list view to match the scene
void MainWindow::sceneSelectionChanged()
{
  if( ui )
  {
    QItemSelectionModel *selmodel;
    QItemSelection itemsel;
    ui->instancePositionBox->setEnabled(false);
    selmodel = ui->instanceTreeView->selectionModel();
    if( selmodel )
    {
      foreach( QGraphicsItem *item, scene()->selectedItems() )
      {
        QModelIndex idx = tree()->indexFromItem( static_cast<AppInstance*>(item) );
        itemsel.select( idx, idx );
      }
      // Add in any hidden selected items
      foreach( QModelIndex idx, selmodel->selectedIndexes() )
      {
        AppInstance *app_inst = tree()->itemFromIndex( idx );
        if( app_inst && !app_inst->isVisible() ) itemsel.select( idx, idx );
      }

      selmodel->select( itemsel, QItemSelectionModel::ClearAndSelect );
    }
  }

  return;
}

void MainWindow::zoom(int steps)
{
  int newZoom = currentLayoutZoom + steps;
  if (newZoom < 1)
    newZoom = 1;
  if (newZoom > 10)
    newZoom = 10;
  if (newZoom != currentLayoutZoom)
    ui->layoutZoomSlider->setValue(newZoom);
}

void MainWindow::on_layoutZoomSlider_valueChanged( int value )
{
  QRectF selRect;
  QMatrix matrix;
  matrix.scale( value, value );
  currentLayoutZoom = value;
  ui->layoutView->setMatrix( matrix );
  foreach( QGraphicsItem* item, scene()->selectedItems() )
    selRect = selRect.united( item->sceneBoundingRect() );
  if( selRect.isEmpty() ) ui->layoutView->centerOn( AppDevice::current().size().width()/2, AppDevice::current().size().height()/2 );
  else ui->layoutView->centerOn( selRect.center() );
}

void MainWindow::on_actionSave_triggered()
{
  if( layoutName.isEmpty() )
  {
    on_actionSave_As_triggered();
    return;
  }
  saveLayout();
  return;
}

void MainWindow::on_actionSave_As_triggered()
{
  QString str;
  str = QFileDialog::getSaveFileName( this, "Save layout as...", layoutPath, "LCDHost XML layout (*.xml)" );
  if( !str.isEmpty() )
  {
    QFileInfo fi(str);
    layoutName = fi.fileName();
    if( fi.suffix().isEmpty() ) layoutName.append(".xml");
    layoutPath = fi.path();
    on_actionSave_triggered();
  }
  return;
}

bool MainWindow::cancelIfModified()
{
  if( isWindowModified() )
  {
    int retv;
    QMessageBox box(this);
    box.setText( tr("The layout has been modified.") );
    box.setInformativeText( tr("Do you want to save your changes?") );
    box.setStandardButtons( QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel );
    box.setDefaultButton( QMessageBox::Save );
    retv = box.exec();
    if( retv == QMessageBox::Cancel ) return true;
    if( retv == QMessageBox::Save ) saveLayout();
  }
  return false;
}

void MainWindow::on_actionOpen_triggered()
{
  QString str;

  if( cancelIfModified() ) return;
  str = QFileDialog::getOpenFileName( this, tr("Open layout..."), layoutPath, "LCDHost Layout (*.xml)" );
  if( !str.isEmpty() ) openLayout( str );
  return;
}

void MainWindow::openLayout( QString str )
{
  QFileInfo fi(str);
  layoutName = fi.fileName();
  layoutPath = fi.path();
  if( loadLayout() )
  {
    QSettings settings;
    if( layoutPath == layoutPathDefault() ) settings.remove( "layoutPath" );
    else settings.setValue( "layoutPath", layoutPath );
    if( layoutName == layoutNameDefault() ) settings.remove( "layoutName" );
    else settings.setValue( "layoutName", layoutName );
  }
}

void MainWindow::on_instanceAlerts_stateChanged(int newState)
{
  if( newState == Qt::PartiallyChecked ) return;
  ui->instanceAlerts->setTristate( false );
  foreach( AppInstance *app_inst, selectedInstances() )
    app_inst->setAlertEnabled( (newState == Qt::Checked) ? true : false );
}

void MainWindow::timerEvent(QTimerEvent* event) {
  if (event && event->timerId() == timer_.timerId())
    run();
  return;
}

void MainWindow::run() {
  switch (state_) {
    case kStateInit:
      init();
      state_ = kStateStartLog;
      break;
    case kStateStartLog: {
      QString logFileName(lh_log_dir());
      ui->logView->setMaximumBlockCount(1000);
      ui->logView->setTabStopWidth(20);
      if(LH_Logger *logger = LH_Logger::lock()) {
        logFileName = logger->fileName();
        logger->unlock();
      }
      ui->logFileName->setText(logFileName);
    }
      state_ = kStateStartAppState;
      break;
    case kStateStartAppState:
      if (AppState *app_state = AppState::instance()) {
        app_state->updateCPU();
        app_state->updateMemory();
        app_state->updateNetwork();
      }
      state_ = kStateStartHid;
      break;
    case kStateStartHid:
#if LH_USE_HID
      hid_thread_ = new LH_HidThread(this);
      hid_thread_->start();
#endif
      state_ = kStateStartSystray;
      break;
    case kStateStartSystray:
      if (!systrayDisabled()) {
        if (!systrayStartup())
          show();
        qDebug("System tray icon is %savailable", hasSystray() ? "" : "not ");
      }
      state_ = kStateStartPlugins;
      break;
    case kStateStartPlugins:
      if (loadPlugin())
        break;
      qDebug("All plugins loaded");
      state_ = kStateLoadLayout;
      break;
    case kStateLoadLayout:
      loadLayout();
      state_ = kStateShowWelcome;
      break;
    case kStateShowWelcome:
      {
        QSettings settings;
        settings.beginGroup("windows");
        settings.beginGroup("tab");
        ui->tabWidget->setCurrentIndex( settings.value("index", 0 ).toInt() );
        ui->tabWidget->setEnabled(true);
        settings.endGroup();
        settings.endGroup();
      }
      if(!dontShowWelcome_)
        new AppWelcomeDialog(this);
      state_ = kStateRun;
      break;
    case kStateRun:
      setNextTimer();
      return;

    case kStateTerm:
      saveSettings();
      clearLayout();
      tree()->destroyRoot();
      ui->instancePreview->clear();
      state_ = kStateStopPluginsStop;
      break;
    case kStateStopPluginsStop:
      foreach(AppLibrary *app_library, plugins_->findChildren<AppLibrary *>())
        if (app_library->isStarted())
          app_library->stop();
      state_ = kStateStopPluginsAbort;
      break;
    case kStateStopPluginsAbort:
      foreach(AppLibrary *app_library, plugins_->findChildren<AppLibrary *>())
        if (app_library->isStarted())
          app_library->abort();
      app_library_waited_ = 0;
      state_ = kStateStopPluginsTerm;
      break;
    case kStateStopPluginsTerm:
      if (app_library_waited_ < 10) {
        bool still_alive = false;
        foreach(AppLibrary *app_library, plugins_->findChildren<AppLibrary *>()) {
          if (app_library->isStarted()) {
            qDebug() << "Waiting for" << app_library->id();
            still_alive = true;
            ++app_library_waited_;
          } else {
            app_library->term();
          }
        }
        if (still_alive)
          break;
      }
      state_ = kStateStopPluginsDelete;
      break;
    case kStateStopPluginsDelete:
      foreach(AppLibrary *app_library, plugins_->findChildren<AppLibrary *>())
        if (!app_library->isStarted())
          delete app_library;
      state_ = kStateStopPluginsStopped;
      break;
    case kStateStopPluginsStopped: {
      QList<AppLibrary*> plugin_list(plugins_->findChildren<AppLibrary *>());
      if (!plugin_list.isEmpty()) {
        qWarning("%d plugins still loaded:\n", plugin_list.size());
        foreach(AppLibrary* app_library, plugin_list) {
          qWarning() << app_library->id() << "still loaded";
          app_library->term();
        }
      } else {
        qDebug("All plugins unloaded\n");
      }
      state_ = kStateStopSystray;
      break;
    }
    case kStateStopSystray:
      if (systray_) {
        delete systray_;
        systray_ = 0;
      }
      if (systraymenu_) {
        delete systraymenu_;
        systraymenu_ = NULL;
      }
      state_ = kStateStopHid;
      break;
    case kStateStopHid:
#if LH_USE_HID
      if (hid_thread_) {
        if (hid_thread_waited_ < 20) {
          if (!hid_thread_waited_)
            hid_thread_->quit();
          ++hid_thread_waited_;
          if (!hid_thread_->wait(100))
            break;
          delete hid_thread_;
          // hid_thread_->deleteLater();
          hid_thread_ = 0;
        } else {
          qWarning("Timed out waiting for HID thread to die\n");
        }
      }
#endif
      state_ = kStateStopRawInput;
      break;
    case kStateStopRawInput:
      AppDevice::setCurrent(NULL);
      if (AppRawInput* ri = AppRawInput::instance())
        ri->clear();
      AppRawInput::DestroyInstance();
      state_ = kStateStopAppState;
      break;
    case kStateStopAppState:
      if (app_state_) {
        delete app_state_;
        app_state_ = NULL;
      }
      state_ = kStateStopped;
      break;
    case kStateStopped:{
#ifndef QT_NODEBUG
      QTime now;
      now.start();
      while(now.elapsed() < 1000 && AppId::countAppObjects())
        QThread::yieldCurrentThread();
      foreach( AppObject *app_obj, AppId::appObjects() )
      {
        Q_ASSERT( app_obj );
        Q_ASSERT( app_obj->metaObject() );
        qWarning() << app_obj->metaObject()->className() << app_obj->id() << "still alive"
                   << (app_obj->parent() ? app_obj->parent()->metaObject()->className() : "(no parent)");
        Q_ASSERT( app_obj->inherits("AppObject") );
      }
      now.start();
      while(now.elapsed() < 1000 && AppId::countLibObjects())
        QThread::yieldCurrentThread();
      foreach( LibObject *lib_obj, AppId::libObjects() )
      {
        Q_ASSERT( lib_obj );
        Q_ASSERT( lib_obj->metaObject() );
        qWarning() << lib_obj->metaObject()->className() << lib_obj->id() << "still alive"
                   << (lib_obj->parent() ? lib_obj->parent()->metaObject()->className() : "(no parent)");
        Q_ASSERT( lib_obj->inherits("LibObject") );
      }
#endif
      state_ = kStateQuit;
      break;
    }
    case kStateQuit:
      timer_.stop();
      qApp->quit();
      return;
  }

  if (timer_.isActive())
    timer_.start(0, this);
  else
    QThread::yieldCurrentThread();
  return;
}

static bool checkTime( int rate, QDateTime& now, QDateTime& next )
{
  int diff;
  bool retv = false;

  if( rate < 1 || rate > 1000 )
  {
    next = now.addSecs(1);
    return false;
  }

  diff = (1000/rate) - now.time().msec() % (1000/rate);

  if( next.isValid() )
  {
    if( now >= next ) retv = true;
    else if( msecDiff( now, next ) < 11 )
    {
      retv = true;
      diff += 1000/rate;
    }
  }

  Q_ASSERT( diff > 0 && diff < 1100 );

  next = now.addMSecs( diff );
  return retv;
}

void MainWindow::setNextTimer()
{
  QDateTime now;
  int nextTimer;
  int note = 0;

  if( ui == NULL )
    return;

  now = QDateTime::currentDateTime();
  if (!now.isValid()) {
    qDebug("QDateTime::currentDateTime() was invalid\n");
    return;
  }

  if( need_instance_refresh_ ) doRefreshInstanceDetails();

  if( checkTime( ui->cpuRefreshRate->value(), now, cpuNext_ ) )
  {
    note |= LH_NOTE_CPU;
    app_state_->updateCPU();
  }

  if( checkTime( ui->memRefreshRate->value(), now, memNext_ ) )
  {
    note |= LH_NOTE_MEM;
    app_state_->updateMemory();
    ui->memPhysFree->setText( QString("%1 MB").arg(app_state_->mem().free_phys/(1024*1024)) );
    ui->memPhysTotal->setText( QString("%1 MB").arg(app_state_->mem().tot_phys/(1024*1024)) );
    ui->memVirtFree->setText( QString("%1 MB").arg(app_state_->mem().free_virt/(1024*1024)) );
    ui->memVirtTotal->setText( QString("%1 MB").arg(app_state_->mem().tot_virt/(1024*1024)) );
  }

  if( checkTime( ui->netRefreshRate->value(), now, netNext_ ) )
  {
    note |= LH_NOTE_NET;
    app_state_->updateNetwork( ui->netInterface->currentIndex() ? ui->netInterface->currentText() : QString() );
    app_state_->setNetMax( qMax( ui->netMaxRateIn->value() / 8 * 1024, 1024),
                         qMax( ui->netMaxRateOut->value() / 8 * 1024, 1024) );
  }

  if( checkTime( 1, now, secondNext_ ) )
  {
    int in, out;

    // executed each second
    note |= LH_NOTE_SECOND;

    app_state_->updateSecond();

    ui->cpuCount->setText( QString::number( app_state_->cpuEngine().count() ));
    ui->cpuAverageLoad->setText(
          QString("%1.%2%").arg(app_state_->cpu_load()/100).arg(app_state_->cpu_load()%100,2,10,QLatin1Char('0'))
          );
    ui->cpuSelfUsage->setText(
          tr("%1.%2% (%3.%4% avg)")
          .arg(app_state_->cpu_self_now()/100).arg(app_state_->cpu_self_now()%100,2,10,QLatin1Char('0'))
          .arg(app_state_->cpu_self_avg()/100).arg(app_state_->cpu_self_avg()%100,2,10,QLatin1Char('0'))
          );

    in = app_state_->net_cur_in() * 80 / (1024*1024);
    out = app_state_->net_cur_out() * 80 / (1024*1024);
    ui->netInbound->setText( QString("%1.%2 Mbit/s").arg( in/10 ).arg( in%10 ) );
    ui->netOutbound->setText( QString("%1.%2 Mbit/s").arg( out/10 ).arg( out%10 ) );

    in = app_state_->net_cur_in() * 8 / 1024;
    out = app_state_->net_cur_out() * 8 / 1024;
    if( in > netautoin_ ) netautoin_ = in;
    if( out > netautoout_ ) netautoout_ = out;
    if( app_state_->net().device != netautodev_ )
    {
      if( netautodev_ != -1 ) netautoout_ = netautoin_ = 0;
      netautodev_ = app_state_->net().device;
    }

    if( ui->netMaxRateOutComboBox->currentIndex() == 0 )
      ui->netMaxRateOut->setValue(netautoout_);
    if( ui->netMaxRateInComboBox->currentIndex() == 0 )
      ui->netMaxRateIn->setValue(netautoin_);

    ui->rendersPerSecond->setText( QString::number( app_state_->layout_rps() ) );
    if( !AppDevice::current().id().isEmpty() ) ui->framesPerSecond->setText( QString::number( app_state_->dev_fps() ) );
    else ui->framesPerSecond->setText( tr("No output device selected") );

    ui->labelRPS->setText( QString::number( app_state_->layout_rps() )+tr(" r/s") );
    ui->labelRPS->setToolTip( tr("Renders per second") );
    pps = 0;

    if( webautocheck_ && (
          (lastwebcheck_.isNull() || (lastwebcheck_.addSecs(webcheckinterval_*60*60) < QDateTime::currentDateTime())) )
        )
    {
      lastwebcheck_ = QDateTime::currentDateTime();
      webUpdateCheck();
    }

    emit onceASecond();
  }

  if( note )
  {
    AppId::notify(note,0);
    foreach( AppLibrary *app_library, plugins() )
    {
      if( app_library->state() == AppLibrary::Loaded )//&& (app_library->notifyFilter() & note) )
      {
        if( (note & LH_NOTE_SECOND) && (app_library->timeout() > 5) )
        {
          qWarning() << app_library->objectName() << "unresponsive";
          app_library->abort();
          app_library->addCrash();
        }
      }
    }
  }

  // update device
  if( checkTime( ui->fpsLimit->value(), now, fpsNext_ ) )
  {
    if( sceneChanged_ && AppDevice::current().isValid() && !tree()->isEmpty() ) //&& !tree()->isLoading() )
    {
      switch( renderMethod() )
      {
        case 0:
        {
          QImage image( AppDevice::current().size(), AppDevice::current().isMonochrome() ? QImage::Format_Mono : QImage::Format_ARGB32_Premultiplied );
          QPainter painter( &image );
          scene()->render( &painter ); //, QRectF(), QRectF(), Qt::IgnoreAspectRatio );
          app_state_->dev_fps_add();
          AppDevice::current().render(image);
        }
          break;
        case 1:
        {
          QPixmap pixmap( AppDevice::current().size() );
          QPainter painter( &pixmap );
          scene()->render( &painter );
          app_state_->dev_fps_add();
          AppDevice::current().render(pixmap.toImage());
        }
          break;
#ifdef QT_OPENGL_LIB
        case 2:
        {
          glWidget()->makeCurrent();
          QGLFramebufferObject fbo( AppDevice::current().size() );
          QPainter painter( &fbo );
          scene()->render( &painter );
          app_state_->dev_fps_add();
          AppDevice::current().render( fbo.toImage() );
        }
          break;
#endif
      }
      sceneChanged_ = 0;
    }
  }

  // find next timer
  nextTimer = qMin( msecDiff( now, cpuNext_ ), msecDiff( now, memNext_ ) );
  nextTimer = qMin( nextTimer, msecDiff( now, netNext_ ) );
  nextTimer = qMin( nextTimer, msecDiff( now, secondNext_ ) );
  nextTimer = qMin( nextTimer, msecDiff( now, fpsNext_ ) );
  if( nextTimer > 2000 ) nextTimer = 2000;
  if( nextTimer < 10 ) nextTimer = 10;
  timer_.start( nextTimer, this );

  return;
}

void MainWindow::screenShot()
{
  if( AppDevice::current().isValid() )
  {
    QImage image( AppDevice::current().size(), AppDevice::current().isMonochrome() ? QImage::Format_Mono : QImage::Format_ARGB32_Premultiplied );
    QPainter painter( &image );
    scene()->render( &painter, QRectF(), QRectF(), Qt::IgnoreAspectRatio );
    int i = 0;
    while( i <= 99 && QFile::exists( QString("%1/screenshot-%2.png").arg(layoutPath).arg(i,2,10,QLatin1Char('0')) ) ) i ++;
    image.save( QString("%1/screenshot-%2.png").arg(layoutPath).arg(i,2,10,QLatin1Char('0')) );
  }
  return;
}

void MainWindow::on_actionNew_triggered()
{
  if( cancelIfModified() ) return;
  clearLayout();
  setWindowTitle( "unnamed[*] - LCDHost" );
  layoutName = "unnamed.xml";
  return;
}

void MainWindow::sceneChanged( const QList<QRectF> & )
{
  sceneChanged_ ++;
}

void MainWindow::on_instanceName_editingFinished()
{
  QList<AppInstance*> selList = selectedInstances();

  if( selList.size() == 1 )
  {
    AppInstance *app_inst = selList.first();
    if( ui->instanceName->text().isEmpty() ) app_inst->setObjectName(  AppLibrary::getClass(app_inst->classId())->generateName() );
    else app_inst->setObjectName( ui->instanceName->text() );
  }
  else
  {
    int counter = 1;
    foreach( AppInstance *app_inst, selList )
    {
      if( ui->instanceName->text().isEmpty() ) app_inst->setObjectName(  AppLibrary::getClass(app_inst->classId())->generateName() );
      else app_inst->setObjectName( QString("%1 (%2)").arg(ui->instanceName->text(),QString::number(counter++)) );
    }
  }
  layoutModified();
  // refreshInstanceDetails();
}

void MainWindow::on_instanceParent_activated(int index)
{
  AppInstance *parent_inst;

  if( index == -1 ) return;

  parent_inst = static_cast<AppInstance*>( ui->instanceParent->itemData(index).value<void*>() );
  if( parent_inst )
  {
    foreach( AppInstance *app_inst, selectedInstances() )
    {
      if( app_inst->parentItem() != parent_inst )
      {
        QPointF old_pos = app_inst->scenePos();
        bool old_selected = app_inst->isSelected();
        layoutModified();
        tree()->removeAppInstance( app_inst );
        if( !tree()->insertAppInstance( app_inst, parent_inst ) )
          tree()->insertAppInstance( app_inst);
        app_inst->setScenePos( old_pos );
        app_inst->setSelected( old_selected );
      }
    }
  }
}

void MainWindow::on_instanceXOffset_editingFinished()
{
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->xOffset() != ui->instanceXOffset->value() )
    {
      app_inst->xOffsetSet( ui->instanceXOffset->value() );
      layoutModified();
    }
  }
  refreshInstanceDetails();
}

void MainWindow::on_instanceXOffset_stepping(int steps)
{
  foreach( AppInstance *app_inst, selectedInstances() )
    app_inst->moveBy( steps, 0 );
  layoutModified();
  refreshInstanceDetails();
}

void MainWindow::on_instanceYOffset_editingFinished()
{
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->yOffset() != ui->instanceYOffset->value() )
    {
      app_inst->yOffsetSet( ui->instanceYOffset->value() );
      layoutModified();
    }
  }
  refreshInstanceDetails();
}

void MainWindow::on_instanceYOffset_stepping(int steps)
{
  foreach( AppInstance *app_inst, selectedInstances() )
    app_inst->moveBy( 0, steps );
  layoutModified();
  refreshInstanceDetails();
}


void MainWindow::on_instanceXAlign_activated(int index)
{
  if( index < 0 ) return;
  Q_ASSERT( index <= 2 );
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->xAlign() != index )
    {
      app_inst->xAlignSet( index );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceXSide_activated(int index)
{
  if( index < 0 ) return;
  Q_ASSERT( index <= 2 );
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->xRefAlign() != index )
    {
      app_inst->xRefAlignSet( index );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceYAlign_activated(int index)
{
  if( index < 0 ) return;
  Q_ASSERT( index <= 2 );
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->yAlign() != index )
    {
      app_inst->yAlignSet( index );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceYSide_activated(int index)
{
  if( index < 0 ) return;
  Q_ASSERT( index <= 2 );
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->yRefAlign() != index )
    {
      app_inst->yRefAlignSet( index );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceXRef_activated(int index)
{
  if( index < 0 ) return;
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    app_inst->xRefSet( AppInstance::fromId( ui->instanceXRef->itemData( index ) ) );
  }
  layoutModified();
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceYRef_activated(int index)
{
  if( index < 0 ) return;
  foreach( AppInstance *app_inst, selectedInstances() )
    app_inst->yRefSet( AppInstance::fromId( ui->instanceYRef->itemData( index ) ) );
  layoutModified();
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceWRef_activated(int index)
{
  AppId ref_id;
  AppInstance *ref_inst = 0;
  if( index < 0 ) return;
  ref_id = ui->instanceWRef->itemData( index );
  ref_inst = AppInstance::fromId( ref_id );
  if( ref_inst == NULL )
  {
    qDebug() << "Unknown width reference" << ref_id;
    return;
  }
  foreach( AppInstance *app_inst, selectedInstances() )
    app_inst->wRefSet( ref_inst );
  layoutModified();
  refreshInstanceDetails();
}

void MainWindow::on_instanceHRef_activated(int index)
{
  if( index < 0 ) return;
  foreach( AppInstance *app_inst, selectedInstances() )
    app_inst->hRefSet( AppInstance::fromId( ui->instanceHRef->itemData( index ) ) );
  layoutModified();
  refreshInstanceDetails();
}

void MainWindow::on_instanceWidthMod_valueChanged(int n)
{
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->wMod() != n )
    {
      app_inst->wModSet( n );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceHeightMod_valueChanged(int n)
{
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->hMod() != n )
    {
      app_inst->hModSet( n );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceWidthUnits_activated(int index)
{
  if( index < 0 ) return;
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->wUnits() != index )
    {
      app_inst->wUnitsSet( index );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::on_instanceHeightUnits_activated(int index)
{
  if( index < 0 ) return;
  foreach( AppInstance *app_inst, selectedInstances() )
  {
    if( app_inst->hUnits() != index )
    {
      app_inst->hUnitsSet( index );
      layoutModified();
    }
  }
  refreshInstanceDetails();
  return;
}

void MainWindow::refreshInputDeviceDetails( QModelIndex index )
{
  if( !index.isValid() ) index = ui->inputDeviceList->currentIndex();
  if (AppRawInput* ri = AppRawInput::instance()) {
    RawInputDevice dev = ri->device( index.row() );
    ui->inputDeviceName->setText( dev.name() );
    ui->inputDeviceType->setText( dev.typeName() );
    ui->inputDeviceId->setText( dev.devid() );
    ui->inputDeviceError->setText( dev.error().isEmpty() ? tr("None") : dev.error() );
    ui->inputDeviceButtonsCount->setText( QString::number(dev.buttonCount()) );
    ui->inputDeviceValuesCount->setText( QString::number(dev.valueCount()) );
  }
}

void MainWindow::on_inputDeviceRefreshButton_clicked()
{
  if (AppRawInput* ri = AppRawInput::instance())
    ri->refresh();
  return;
}

void MainWindow::on_inputDeviceShowEvents_clicked(bool checked)
{
  if (AppRawInput* ri = AppRawInput::instance()) {
    if( checked )
    {
      connect( ri, SIGNAL(rawInput(QByteArray,QString,int,int,int)),
               this, SLOT(rawInput(QByteArray,QString,int,int,int)) );
      ui->inputDeviceTestResult->setText( tr("Waiting for an event") );
    }
    else
    {
      disconnect( ri, SIGNAL(rawInput(QByteArray,QString,int,int,int)),
                  this, SLOT(rawInput(QByteArray,QString,int,int,int)) );
      ui->inputDeviceTestResult->clear();
    }
  }
}

void MainWindow::rawInput( QByteArray devid, QString control, int item, int value, int flags )
{
  if (AppRawInput* ri = AppRawInput::instance())
    ui->inputDeviceTestResult->setText( ri->describeEvent( devid, control, item, value, flags ) );
}

void MainWindow::on_instanceDepth_stepping(int steps)
{
  if( !steps ) return;
  QList<AppInstance*> list = selectedInstances();
  foreach( AppInstance* app_inst, list )
  {
    while( steps < 0 )
    {
      tree()->zDown( app_inst );
      steps ++;
    }
    while( steps > 0 )
    {
      tree()->zUp( app_inst );
      steps --;
    }
  }
  layoutModified();
  refreshInstanceDetails();
  return;
}

void MainWindow::on_actionAbout_triggered()
{
  new AppAboutDialog(this);

  return;
}

void MainWindow::systrayActivated(QSystemTrayIcon::ActivationReason reason)
{
  switch( reason )
  {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
      ui->actionSystrayRestore->trigger();
      break;
    case QSystemTrayIcon::Context:
    case QSystemTrayIcon::MiddleClick:
      break;
    default:
      break;
  }

  return;
}

void MainWindow::on_netMaxRateInComboBox_activated(int index)
{
  ui->netMaxRateIn->setEnabled( index == 7 );

  switch(index)
  {
    case 0: ui->netMaxRateIn->setValue( netautoin_ ); break;
    case 1: ui->netMaxRateIn->setValue( 1024 * 1024 ); break;
    case 2: ui->netMaxRateIn->setValue( 100 * 1024 ); break;
    case 3: ui->netMaxRateIn->setValue( 10 * 1024 ); break;
    case 4: ui->netMaxRateIn->setValue( 2 * 1024 ); break;
    case 5: ui->netMaxRateIn->setValue( 768 ); break;
    case 6: ui->netMaxRateIn->setValue( 56 ); break;
  }
  return;
}

void MainWindow::on_netMaxRateOutComboBox_activated(int index)
{
  ui->netMaxRateOut->setEnabled( index == 7 );

  switch(index)
  {
    case 0: ui->netMaxRateOut->setValue( netautoout_ ); break;
    case 1: ui->netMaxRateOut->setValue( 1024 * 1024 ); break;
    case 2: ui->netMaxRateOut->setValue( 100 * 1024 ); break;
    case 3: ui->netMaxRateOut->setValue( 10 * 1024 ); break;
    case 4: ui->netMaxRateOut->setValue( 2 * 1024 ); break;
    case 5: ui->netMaxRateOut->setValue( 768 ); break;
    case 6: ui->netMaxRateOut->setValue( 56 ); break;
  }
  return;
}

void MainWindow::on_cpuRefreshRate_valueChanged(int )
{
  foreach( AppLibrary *app_library, plugins() )
    app_library->clearTimeout();
  return;
}

void MainWindow::on_actionWelcome_triggered()
{
  new AppWelcomeDialog(this);
  return;
}

void MainWindow::on_outputDeviceList_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
  Q_UNUSED(previous);

  AppDevice *app_device = NULL;
  if( current != NULL )
  {
    AppId dev_id = current->data(Qt::UserRole);
    app_device = AppDevice::fromId( dev_id );
  }
  if( app_device && app_device->isValid() )
  {
    ui->outputDeviceName->setText( app_device->name() );
    ui->outputDeviceResolution->setText(
          QString("%1x%2, %3 bpp")
          .arg(app_device->size().width())
          .arg(app_device->size().height())
          .arg(app_device->depth()) );
    ui->outputDeviceButtons->setText("?");
    ui->outputDeviceLogo->setPixmap( QPixmap::fromImage(app_device->logo()) );

    ui->outputDriverName->setText( app_device->appLibrary()->name() );
    ui->outputDriverFile->setText( app_device->appLibrary()->objectName() );
    ui->outputDriverDescription->setText( app_device->appLibrary()->description() );
    ui->outputDriverHomepage->setText( app_device->appLibrary()->homepage() );
    ui->outputDriverAuthor->setText( app_device->appLibrary()->author() );
    // ui->outputDriverVerifyURL->setText( app_device->appLibrary()->verifyURL() );
    if( app_device->appLibrary()->crashes() < 1 ) ui->outputDriverCrashes->setText( tr("None") );
    else ui->outputDriverCrashes->setText( QString::number(app_device->appLibrary()->crashes()) );
    ui->outputDriverDescription->setText( app_device->appLibrary()->description() );
    ui->outputDriverLogo->setPixmap( QPixmap::fromImage(app_device->appLibrary()->logo()) );

    ui->outputDeviceSettings->setWidget( app_device->setupBuildUI(ui->outputDeviceSettings) );
  }
  else
  {
    ui->outputDeviceName->setText("-");
    ui->outputDeviceResolution->setText("-");
    ui->outputDeviceButtons->setText("-");
    ui->outputDeviceLogo->clear();

    ui->outputDriverName->setText( "-" );
    ui->outputDriverFile->setText( "-" );
    ui->outputDriverDescription->setText( "-" );
    ui->outputDriverHomepage->setText( "-" );
    ui->outputDriverAuthor->setText( "-" );
    ui->outputDriverVerifyURL->setText( "-" );
    ui->outputDriverCrashes->setText( tr("None") );
    ui->outputDriverDescription->setText( "-" );
    ui->outputDriverLogo->clear();

    if( ui->outputDeviceSettings->widget() )
      delete ui->outputDeviceSettings->takeWidget();
  }
}

void MainWindow::on_outputDeviceList_itemActivated(QListWidgetItem* item)
{
  if( item )
  {
    AppDevice *app_device = AppDevice::fromId( item->data(Qt::UserRole) );
    if( app_device )
    {
      wantedDevId_ = app_device->devId();
      refreshDeviceList();
    }
  }
}

void MainWindow::on_outputRefresh_clicked()
{
  ui->outputRefresh->setEnabled(false);
  foreach( AppLibrary *app_library, plugins() )
  {
    if( app_library->running() && app_library->isDriver() )
    {
      app_library->stop();
      QApplication::postEvent( this, new EventLibraryStart( app_library->id() ) );
    }
  }
  ui->outputRefresh->setEnabled(true);
}

void MainWindow::on_pluginList_activated(QModelIndex)
{
  on_LoadButton_clicked();
}

void MainWindow::on_instanceClass_activated(QString classname)
{
  if( classname.isEmpty() ) return;

  foreach( AppInstance *app_inst, selectedInstances() )
  {
    layoutModified();
    app_inst->setClassId(classname);
  }
}

void MainWindow::openDebugDialog()
{
  new AppDebugDialog(this);
}

void MainWindow::on_actionScreenshot_triggered()
{
  screenShot();
}

void MainWindow::on_inputDeviceMouseKeyButton_clicked()
{
  AppRawInputDialog dlg(true);
  if( dlg.exec() == QDialog::Accepted )
  {
    ui->inputDeviceMouseKey->setText( dlg.description() );
    mousekey_ = dlg.control();

    // emit inputChosen(le_->text(),dlg.evt().device_id,dlg.evt().item);
  }
}

void MainWindow::on_actionReload_triggered()
{
  // reload existing layout
  if( cancelIfModified() ) return;
  loadLayout();
}

void MainWindow::on_renderMethod_currentIndexChanged(int index)
{
  if( index >= 0 && index <= 2 && index != rendermethod_ )
  {
    rendermethod_ = index;
    switch( rendermethod_ )
    {
      case 0:
#ifdef QT_OPENGL_LIB
        glwidget_ = 0;
#endif
        ui->layoutView->setViewport(0);
        break;
      case 1:
#ifdef QT_OPENGL_LIB
        glwidget_ = 0;
#endif
        ui->layoutView->setViewport(0);
        break;
#ifdef QT_OPENGL_LIB
      case 2:
        if( QGLFormat::hasOpenGL() )
        {
          AppGLWidget *w = new AppGLWidget(new AppGLContext(QGLFormat(QGL::AlphaChannel)));
          ui->layoutView->setViewport(w);
          glwidget_ = w;
        }
        else
          ui->renderMethod->setCurrentIndex(0);
        break;
#endif
    }
  }
}

void MainWindow::on_actionClean_layout_triggered()
{
  if( tree() && tree()->root() )
  {
    QMessageBox mb(this);
    int cleaned = tree()->root()->cleanUnusedSetupItems();
    if( cleaned )
    {
      mb.setText( tr("Cleanup done") );
      mb.setInformativeText( tr("%1 obsolete setup items removed.").arg(cleaned) );
      layoutModified();
    }
    else
    {
      mb.setText( tr("Nothing to do") );
      mb.setInformativeText( tr("There were no obsolete setup items to remove.") );
    }
    mb.exec();
  }
}

// Update or install a plugin from web
void MainWindow::webUpdatePlugin( QString id )
{    
  AppPluginVersion apv;
  apv = vercache_.get(id);
  if( apv.isValid() )
  {
    qDebug() << "Downloading" << apv.downloadurl().toString();
    QNetworkReply *r = nam_.get( QNetworkRequest( apv.downloadurl() ) );
    connect( r, SIGNAL(finished()), this, SLOT(netGotPlugin()) );
  }
}


void MainWindow::webUpdatePluginData( AppLibrary *app_lib, QNetworkReply *reply )
{
  QDir datadir( AppState::instance()->dir_data() );
  QString disp = QString::fromLatin1( reply->rawHeader("Content-Disposition") ).trimmed();
  qDebug() << "Updating" << app_lib->objectName() << "using" << disp;

  if( !disp.contains("filename=\"") )
  {
    qWarning() << "Download has invalid Content-Disposition:" << disp;
    return;
  }

  QString zipfilename = disp.mid( disp.indexOf("filename=\"")+10 );
  if( zipfilename.endsWith('\"') ) zipfilename.chop(1);
  if( !zipfilename.endsWith(".zip",Qt::CaseInsensitive) )
  {
    qWarning() << "Download is not a zipfile:" << zipfilename;
    return;
  }

  datadir.mkdir("downloads");

  // extract filename
  zipfilename.prepend(AppState::instance()->dir_data() + "downloads/");
  QByteArray zipname_array = zipfilename.toLocal8Bit();
  QFile zipfile( zipfilename );
  if( zipfile.open(QIODevice::WriteOnly|QIODevice::Truncate) )
  {
    zipfile.write( reply->readAll() );
    zipfile.close();
  }

  mz_zip_archive zip_archive;
  memset(&zip_archive, 0, sizeof(zip_archive));
  if(mz_zip_reader_init_file(
       &zip_archive,
       zipname_array.constData(),
       MZ_ZIP_FLAG_CASE_SENSITIVE | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY))
  {
    mz_uint file_count = mz_zip_reader_get_num_files(&zip_archive);
    for(mz_uint file_index = 0; file_index < file_count; ++ file_index)
    {
      mz_zip_archive_file_stat file_stat;
      if(mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat))
      {
        QString outfilename(QDir::fromNativeSeparators(QString::fromLocal8Bit(file_stat.m_filename)));
        outfilename.prepend(AppState::instance()->dir_plugins());
        QFileInfo outfileinfo(outfilename);
        if(outfileinfo == app_lib->fileInfo())
        {
          QString backupfilename = outfilename + QLatin1String(".orig");
          if(outfileinfo.isFile() && outfileinfo.isWritable())
          {
            app_lib->abort();
            if(! QFile::exists(backupfilename) || QFile::remove(backupfilename))
            {
              if(! QFile::rename(outfilename, backupfilename))
                qWarning() << "Failed to rename" << outfilename << "to" << backupfilename;
            }
            else
              qWarning() << "Failed to remove old backup" << backupfilename;
            if(! mz_zip_reader_extract_to_file(
                 &zip_archive,
                 file_index,
                 outfilename.toLocal8Bit().constData(),
                 MZ_ZIP_FLAG_CASE_SENSITIVE))
            {
              qCritical() << "Failed to extract" << outfilename;
              if(QFile::exists(outfilename))
                QFile::remove(outfilename);
              if(QFile::exists(backupfilename) && QFile::rename(backupfilename, outfilename))
                qDebug() << "Restored backup of" << outfilename << "from" << backupfilename;
              else
                qCritical() << "Failed to restore backup of" << outfilename << "from" << backupfilename;
            }
            QString errmsg = app_lib->pi().read();
            if( errmsg.isEmpty() && app_lib->pi().isUsable() )
              QApplication::postEvent( this, new EventLibraryStart( app_lib->id() ) );
            else
              qWarning() << "Won't start new version:" << errmsg << (app_lib->isUsable()?"":"(unusable version)");
          }
          else
            qWarning() << outfilename << "is not a writable file";
        }
        else
          qWarning() << outfileinfo.filePath() << "does not refer to" << app_lib->fileInfo().filePath();
      }
    }
    mz_zip_reader_end(&zip_archive);
  }

  return;
}

void MainWindow::webUpdateCheck()
{
  foreach( QUrl url, vercache_.staleUrls() )
  {
    vercache_.startedQuery(url);
    QNetworkRequest req(url);
    req.setRawHeader(QString("User-Agent").toLatin1(),webUserAgent().toLatin1());
    connect( nam_.get(req), SIGNAL(finished()), this, SLOT(netGotVersion()) );
  }
}

void MainWindow::webUpdateCompare()
{
  AppPluginVersion apv;
  if( webautoupdate_ )
  {
    foreach( AppLibrary* app_lib, plugins() )
    {
      apv = vercache_.get( app_lib->objectName() );
      if( apv.isUsable() && apv.revision() > app_lib->revision() )
        webUpdatePlugin( app_lib->objectName() );
    }
  }
  apv = vercache_.get( "LCDHost" );
  if( apv.revision() > AppVersionToRevision(VERSION) && !apv.downloadurl().isEmpty() )
  {
    qDebug() << "LCDHost" << AppRevisionToVersion(apv.revision()) << AppPluginVersion::defaultArch() << "announced at" << apv.downloadurl().toString();
    connect( nam_.head( QNetworkRequest(apv.downloadurl())), SIGNAL(finished()), this, SLOT(netGotHeadReply()) );
  }
}

void MainWindow::on_actionUpdates_triggered()
{
  new AppUpdateDialog(this);
}

void MainWindow::netGotPubkey()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  if( reply )
  {
    if( reply->error() != QNetworkReply::NoError )
    {
      qWarning() << "Network error:" << reply->url().toString() << reply->errorString();
      return;
    }

    if( pubkeys_.contains(reply->url().toString()) )
    {
      QByteArray pubkey = reply->readAll();
      qDebug() << "Got public key from" << reply->url().toString() << "containing" << pubkey.size() << "bytes";
      pubkeys_.insert(reply->url().toString(),pubkey);
      QCoreApplication::postEvent(this,new EventRefreshPlugins());
    }
  }
}

void MainWindow::netGotPlugin()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  if( reply )
  {
    if( reply->error() != QNetworkReply::NoError )
    {
      qWarning() << "Network error:" << reply->url().toString() << reply->errorString();
      return;
    }

    foreach( AppLibrary *app_lib, plugins() )
    {
      AppPluginVersion apv = vercache_.get( app_lib->objectName() );
      if( apv.downloadurl() == reply->url() )
      {
        if( reply->hasRawHeader("Content-Disposition") && reply->hasRawHeader("Content-Type") )
        {
          webUpdatePluginData( app_lib, reply);
        }
        else
        {
          qWarning() << "Download failed, no content";
        }
      }
    }
  }
}

void MainWindow::netGotVersion()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  if( reply )
  {
    if( reply->error() != QNetworkReply::NoError )
    {
      qWarning() << "Network error:" << reply->url().toString() << reply->errorString();
      return;
    }

    if( vercache_.wantsUrl(reply->url()) )
    {
      qDebug() << "Version data from" << reply->url().toString() << "containing" << reply->size() << "bytes";
      QString s = vercache_.parseReply(reply);
      if( !s.isEmpty() )
        qWarning() << s;
      else
      {
        webUpdateCompare();
        QCoreApplication::postEvent(this,new EventRefreshPlugins());
      }
    }
  }
}

void MainWindow::netGotHeadReply()
{
  QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
  if( reply )
  {
    if( reply->error() != QNetworkReply::NoError )
    {
      qWarning() << "Network error:" << reply->url().toString() << reply->errorString();
      return;
    }

    // this is a check if an announced next version is available
    if( reply->error() == QNetworkReply::NoError && reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() < 400 )
    {
      qDebug() << "A new version of LCDHost is available at" << reply->url().toString();
      new AppDownloadUpdateDialog(this);
    }
  }
  return;
}

QString MainWindow::webUserAgent() const
{
  return QString("LCDHost-%1/%2").arg(VERSION).arg(AppPluginVersion::defaultArch());
}

void MainWindow::on_checkForPluginsButton_clicked()
{
  ui->checkForPluginsButton->setEnabled(false);
  loadPlugin();
  ui->checkForPluginsButton->setEnabled(true);
}

void MainWindow::on_actionSystrayRestore_triggered()
{
  show();
  raise();
  activateWindow();
}

void MainWindow::on_actionSystrayQuit_triggered()
{
  on_actionExit_triggered();
}

void MainWindow::on_actionSystrayOpen_triggered()
{
  on_actionOpen_triggered();
}
