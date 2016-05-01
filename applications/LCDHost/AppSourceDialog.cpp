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

#include "AppSourceDialog.h"
#include "ui_AppSourceDialog.h"

#include "AppObject.h"
#include "AppState.h"
#include "AppSetupLink.h"
#include "MainWindow.h"

AppSourceDialog::AppSourceDialog(AppSetupItem *asi, MainWindow* parent) :
    QDialog(parent),
    ui(new Ui::AppSourceDialog)
{
    ui->setupUi(this);
    setAttribute( Qt::WA_DeleteOnClose, true );

    connect( asi, SIGNAL(subscribePathChanged(QString)), this, SLOT(setSubscribePath(QString)) );
    connect( this, SIGNAL(subscribePathChanged(QString)), asi, SLOT(setSubscribePath(QString)) );


    connect( ui->sourceEnabled, SIGNAL(clicked()),
             this, SLOT(evaluate()) );

    connect( ui->sourceTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
             this, SLOT(evaluate()) );

    connect( asi, SIGNAL(publishPathChanged(QString)),
             ui->publishPath, SLOT(setText(QString)) );
    connect( ui->publishPath, SIGNAL(textEdited(QString)),
             asi, SLOT(setPublishPath(QString)) );

    setWindowTitle( asi->objectName() );

    ui->sourceEnabled->setChecked( asi->isSink() );
    ui->sourceEnabled->setEnabled( !(asi->flags() & LH_FLAG_NOSINK) );
    ui->sourceName->setText( asi->subscribePath() );
    ui->sourceTree->setEnabled( ui->sourceEnabled->isChecked() );

    foreach( AppSetupItem * src, parent->pluginParent()->findChildren<AppSetupItem *>() )
    {
        if( src->canSource( asi ) )
        {
            QString path( src->publishPathUI() );
            if( !path.isEmpty() )
            {
                if(!src->publishPath().isEmpty())
                    ui->sourceTree->addItem(src->publishPathUI(), src->publishPath());
            }
        }
    }

    if( asi->subscribePath().isEmpty() )
        ui->sourceName->setStyleSheet("color:gray");
    else if( ui->sourceTree->setCurrentItemByLinkPath( asi->subscribePath() ) )
        ui->sourceName->setStyleSheet("color:green");
    else
        ui->sourceName->setStyleSheet("color:red");

    ui->publishPath->setText( asi->publishPath() );
}

AppSourceDialog::~AppSourceDialog()
{
    delete ui;
}

void AppSourceDialog::setSubscribePath( const QString & path )
{
    if( path.isEmpty() )
    {
        ui->sourceEnabled->setChecked( false );
        ui->sourceTree->setEnabled( false );
    }
    else
    {
        ui->sourceEnabled->setChecked( true );
        ui->sourceTree->setEnabled( true );
        ui->sourceTree->setCurrentItemByLinkPath( path );
    }
}

void AppSourceDialog::setPublishPath( const QString & path )
{
    ui->publishPath->setText( path );
}

void AppSourceDialog::evaluate()
{
    if( ui->sourceEnabled->isChecked() )
    {
        ui->sourceTree->setEnabled( true );
        if( ui->sourceTree->currentItem() && ui->sourceTree->currentItem()->data(0,Qt::ToolTipRole).isValid() )
            emit subscribePathChanged( ui->sourceTree->currentItem()->data(0,Qt::ToolTipRole).toString() );
    }
    else
    {
        ui->sourceTree->setEnabled( false );
        emit subscribePathChanged( QString() );
    }

    emit publishPathChanged( ui->publishPath->text() );
}
