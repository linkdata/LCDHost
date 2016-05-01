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


#include <QSettings>
#include <QFont>
#include <QVarLengthArray>
#include <QFormLayout>

#include "AppDebugDialog.h"
#include "ui_AppDebugDialog.h"

#include "AppState.h"
#include "AppObject.h"
#include "AppInstance.h"
#include "AppInstanceTree.h"
#include "LibObject.h"
#include "EventLibraryStop.h"
#include "EventLibraryStart.h"
#include "EventLayoutChanged.h"
#include "EventLayoutSaved.h"
#include "EventLogEntry.h"
#include "EventDebugStateRefresh.h"
#include "LibLibrary.h"
#include "MainWindow.h"

AppDebugDialog::AppDebugDialog(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::AppDebugDialog)
{
    QSettings settings;

    ui->setupUi(this);
    setModal(false);
    setWindowTitle( tr("Debug LCDHost") );
    setAttribute( Qt::WA_DeleteOnClose, true );
    show();

    ui->appStateTree->setUniformRowHeights(true);
    ui->appStateTree->setHeaderHidden(true);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    ui->appStateTree->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
#else
    ui->appStateTree->header()->setResizeMode( QHeaderView::ResizeToContents );
#endif

    connect( ui->appStateTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)) );

    connect( ui->stressLoadPlugins, SIGNAL(clicked()), this, SLOT(signalLibraryStates()) );
    connect( ui->stressUnloadPlugins, SIGNAL(clicked()), this, SLOT(signalLibraryStates()) );
    connect( ui->stressLoadDrivers, SIGNAL(clicked()), this, SLOT(signalLibraryStates()) );
    connect( ui->stressUnloadDrivers, SIGNAL(clicked()), this, SLOT(signalLibraryStates()) );

    connect( ui->stressConstantlyReloadLayout, SIGNAL(clicked()), this, SLOT(loadLayout()) );
    connect( ui->stressConstantlySaveLayout, SIGNAL(clicked()), this, SLOT(saveLayout()) );

    settings.beginGroup("windows");
    settings.beginGroup("debug");
    restoreGeometry( settings.value("geometry", QByteArray() ).toByteArray() );
    ui->splitter->restoreState( settings.value("splitter", QByteArray() ).toByteArray() );
    settings.endGroup();
    settings.endGroup();

    ui->logLoad->setChecked( lh_log_load );

    onceASecond();

    connect( parent, SIGNAL(libraryStateChanged(AppId)), this, SLOT(libraryStateChanged(AppId)) );
    connect( parent, SIGNAL(layoutChanged(QString,QString)), this, SLOT(layoutChanged(QString,QString)) );
    connect( parent, SIGNAL(onceASecond()), this, SLOT(onceASecond()) );
    connect( parent, SIGNAL(layoutSaved(QString,QString)), this, SLOT(layoutSaved(QString,QString)) );
}

AppDebugDialog::~AppDebugDialog()
{
    QSettings settings;
    settings.beginGroup("windows");
    settings.beginGroup("debug");
    settings.setValue( "geometry", saveGeometry() );
    settings.setValue( "splitter", ui->splitter->saveState() );
    settings.endGroup();
    settings.endGroup();
    delete ui;
}

MainWindow* AppDebugDialog::mainWindow() const
{
    return static_cast<MainWindow*>(parent());
}

void AppDebugDialog::signalLibraryStates()
{
    MainWindow* w = qobject_cast<MainWindow*>(parentWidget());
    foreach(AppLibrary *app_lib, w->plugins())
        libraryStateChanged( app_lib->id() );
    return;
}

void AppDebugDialog::loadLayout()
{
    if( ui->stressConstantlyReloadLayout->isChecked() )
        mainWindow()->loadLayout();
}

void AppDebugDialog::saveLayout()
{
    if( ui->stressConstantlySaveLayout->isChecked() )
    {
        mainWindow()->saveLayout();
    }
}

void AppDebugDialog::layoutChanged( QString dir, QString name )
{
    if( ui->stressConstantlyReloadLayout->isChecked() )
    {
        QCoreApplication::postEvent( this, new EventLayoutChanged(dir,name) );
    }
}

void AppDebugDialog::layoutSaved( QString dir, QString name )
{
    if( ui->stressConstantlySaveLayout->isChecked() )
    {
        QCoreApplication::postEvent( this, new EventLayoutSaved(dir,name) );
    }
}

QString AppDebugDialog::location(AppInstance *app_inst, QModelIndex idx)
{
    QString retv;
    if( idx.parent().isValid() ) retv = QString("%1.").arg(location( app_inst->parentItem(), idx.parent() ));
    Q_ASSERT( app_inst->zValue() == idx.row() );
    retv.append( QString::number(idx.row()) );
    return retv;
}

void AppDebugDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void AppDebugDialog::onceASecond()
{
    QCoreApplication::postEvent( this, new EventDebugStateRefresh() );
    return;
}

void AppDebugDialog::updateInfo( QTreeWidgetItem * item )
{
    QWidget * info = new QWidget();
    if( item )
    {
        QFormLayout * lo = new QFormLayout( info );
        AppObject * ao = 0;
        AppSetupItem * asi = 0;

        if( item->type() == 1 )
        {
            ao = AppId( item->data(0,Qt::UserRole).toInt() ).appObject();
        }
        else if( item->type() == 2 && item->parent() )
        {
            QString asi_name( item->data(0,Qt::ToolTipRole).toString() );
            ao = AppId( item->parent()->data(0,Qt::UserRole).toInt() ).appObject();
            if( ao )
            {
                foreach( AppSetupItem * asi2, ao->setupItems() )
                {
                    if( asi2->objectName() == asi_name )
                    {
                        asi = asi2;
                        break;
                    }
                }
            }
        }

        if( asi )
        {
            lo->addRow( tr("Class"), new QLabel( asi->metaObject()->className() ) );
            lo->addRow( tr("Name"), new QLabel( asi->objectName() ) );
            lo->addRow( tr("Owner"), new QLabel( ao->id().toString() ) );
            lo->addRow( tr("Data type"), new QLabel( AppSetupItem::typeName( asi->type() ) ) );
            lo->addRow( tr("Data value"), new QLabel( asi->value().toString() ) );
            lo->addRow( tr("Publish path"), new QLabel( asi->publishPath() ) );
            lo->addRow( tr("Subscribe path"), new QLabel( asi->subscribePath() ) );
        }
        else if( AppInstance * ai = qobject_cast<AppInstance *>(ao) )
        {
            lo->addRow( tr("Class"), new QLabel( ai->metaObject()->className() ) );
            lo->addRow( tr("Title"), new QLabel(ai->objectName() ) );
            lo->addRow( tr("Parent"), new QLabel(ai->parentItem() ? ai->parentItem()->id().toString() : "none" ) );
        }
        else if( ao )
        {
            lo->addRow( tr("Class"), new QLabel( ao->metaObject()->className() ) );
            lo->addRow( tr("Identity"), new QLabel( ao->id().toString() ) );
        }
    }
    ui->objectInfoArea->setWidget( info );
}

bool AppDebugDialog::updateAppObject( AppObject * appobj, QTreeWidgetItem * parentitem )
{
    bool modified = false;
    QVarLengthArray<QTreeWidgetItem *> to_remove;
    QTreeWidgetItem * childitem;
    AppSetupItem * asi;

    Q_ASSERT( appobj->id().toInt() == parentitem->data(0,Qt::UserRole).toInt() );

    // Add new setup items
    foreach( asi, appobj->setupItems() )
    {
        childitem = 0;
        for( int i = 0; i < parentitem->childCount(); ++ i )
        {
            if( asi->objectName() == parentitem->child(i)->data(0,Qt::ToolTipRole).toString() )
            {
                childitem = parentitem->child(i);
                break;
            }
        }

        if( !childitem )
        {
            childitem = new QTreeWidgetItem( parentitem, 2 );
            childitem->setData( 0, Qt::DisplayRole, asi->name().left(32) );
            childitem->setData( 0, Qt::ToolTipRole, asi->objectName() );
            QFont asifont = childitem->font(0);
            asifont.setFamily("Courier New");
            childitem->setFont( 0, asifont );
            childitem->setFont( 1, asifont );
            childitem->setTextAlignment( 1, Qt::AlignRight );
            modified = true;
        }
    }

    // update or mark for removal
    for( int i = 0; i < parentitem->childCount(); ++ i )
    {
        childitem = parentitem->child(i);
        if( childitem->type() == 2 )
        {
            bool removeit = true;
            const QString itemname( childitem->data(0,Qt::ToolTipRole).toString() );
            foreach( asi, appobj->setupItems() )
            {
                if( asi->objectName() == itemname )
                {
                    // found it, update
                    removeit = false;
                    childitem->setData( 1, Qt::DisplayRole, asi->value().toString() );
                    childitem->setData( 1, Qt::ToolTipRole, AppSetupItem::typeName( asi->type() ) );
                    break;
                }
            }
            if( removeit ) to_remove.append( childitem );
        }
    }

    // remove dead ones
    for( int i=0; i<to_remove.size(); ++i )
    {
        childitem = to_remove.at(i);
        parentitem->removeChild( childitem );
        delete childitem;
        modified = true;
    }

    return modified;
}

bool AppDebugDialog::updateTree( QObject * parentobj, QTreeWidgetItem * parentitem )
{
    bool modified = false;
    QVarLengthArray<QTreeWidgetItem *> to_remove;
    AppObject * childobj;
    QTreeWidgetItem * childitem;

    // add new ones
    foreach( QObject * qobj, parentobj->children() )
    {
        childobj = qobject_cast<AppObject*>(qobj);
        if( childobj )
        {
            childitem = 0;
            for( int i = 0; i < parentitem->childCount(); ++ i )
            {
                if( childobj->id().toInt() == parentitem->child(i)->data(0,Qt::UserRole).toInt() )
                {
                    childitem = parentitem->child(i);
                    break;
                }
            }
            if( !childitem )
            {
                childitem = new QTreeWidgetItem( parentitem, 1 );
                childitem->setData(0,Qt::DisplayRole,childobj->id().toString());
                childitem->setData(0,Qt::ToolTipRole,childobj->metaObject()->className());
                childitem->setData(0,Qt::UserRole,childobj->id().toInt());
                modified = true;
            }
        }
    }

    // update or mark for removal
    for( int i = 0; i < parentitem->childCount(); ++ i )
    {
        childitem = parentitem->child(i);
        if( childitem->type() == 1 )
        {
            bool removeit = true;
            int itemid = childitem->data(0,Qt::UserRole).toInt();
            foreach( QObject * qobj, parentobj->children() )
            {
                if( AppObject * ao = qobject_cast<AppObject*>(qobj) )
                {
                    if( ao->id().toInt() == itemid )
                    {
                        // found it, update
                        removeit = false;
                        if( updateAppObject( ao, childitem ) ) modified = true;
                        if( updateTree( ao, childitem ) ) modified = true;
                        break;
                    }
                }
            }
            if( removeit ) to_remove.append( childitem );
        }
    }

    // remove dead ones
    for( int i=0; i<to_remove.size(); ++i )
    {
        childitem = to_remove.at(i);
        parentitem->removeChild( childitem );
        delete childitem;
        modified = true;
    }
    return modified;
}

void AppDebugDialog::refreshState()
{
    ui->appStateTree->setUpdatesEnabled(false);
    updateTree(mainWindow()->pluginParent(), ui->appStateTree->invisibleRootItem());
    ui->appStateTree->setUpdatesEnabled(true);
    updateInfo( ui->appStateTree->currentItem() );
    return;
}

bool AppDebugDialog::event(QEvent *event)
{
    if( event->type() == EventLibraryStop::type() )
    {
        EventLibraryStop *e = static_cast<EventLibraryStop*>(event);
        AppLibrary *app_lib = AppLibrary::fromId( e->id );
        if( app_lib && app_lib->state() == AppLibrary::Loaded ) app_lib->stop();
        return true;
    }
    if( event->type() == EventLibraryStart::type() )
    {
        EventLibraryStart *e = static_cast<EventLibraryStart*>(event);
        AppLibrary *app_lib = AppLibrary::fromId( e->id );
        if( app_lib && app_lib->state() == AppLibrary::Unloaded ) app_lib->start();
        return true;
    }
    if( event->type() == EventLayoutChanged::type() )
    {
        QTimer::singleShot( ui->stressConstantlyReloadLayoutDelay->value(), this, SLOT(loadLayout()) );
       return true;
    }
    if( event->type() == EventLayoutSaved::type() )
    {
        QTimer::singleShot( ui->stressConstantlySaveLayoutDelay->value(), this, SLOT(saveLayout()) );
        return true;
    }
    if( event->type() == EventDebugStateRefresh::type() )
    {
        refreshState();
        return true;
    }
    return QDialog::event(event);
}


void AppDebugDialog::libraryStateChanged(AppId id)
{
    AppLibrary *app_lib = qobject_cast<AppLibrary *>(id.appObject());

    if( app_lib )
    {
        if( app_lib->state() == AppLibrary::Loaded )
        {
            if( (app_lib->isDriver() && ui->stressUnloadDrivers->isChecked()) ||
                (app_lib->isPlugin() && ui->stressUnloadPlugins->isChecked()) )
                QCoreApplication::postEvent( this, new EventLibraryStop(id) );
        }
        else if( app_lib->state() == AppLibrary::Unloaded )
        {
            if( (app_lib->isDriver() && ui->stressLoadDrivers->isChecked()) ||
                (app_lib->isPlugin() && ui->stressLoadPlugins->isChecked()) )
                QCoreApplication::postEvent( this, new EventLibraryStart(id) );
        }
    }
}

void AppDebugDialog::on_logLoad_clicked(bool checked)
{
    lh_log_load = checked;
}

void AppDebugDialog::currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous )
{
    Q_UNUSED( previous )
    updateInfo( current );
    return;
}
