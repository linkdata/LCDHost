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


#include <QHBoxLayout>
#include <QDir>
#include <QFileDialog>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppSetupOpenFile.h"

AppSetupOpenFile::AppSetupOpenFile(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    le_ = new QLineEdit(this);
    tb_ = new QToolButton(this);
    tb_->setText("...");
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget( le_, 1 );
    layout->addWidget( tb_, 0 );
    connect( tb_, SIGNAL(clicked()), this, SLOT(openFile()) );
    connect( le_, SIGNAL(textEdited(QString)), this, SIGNAL(textEdited(QString)) );
    setLayout(layout);
    return;
}

void AppSetupOpenFile::setEnabled(bool b)
{
    le_->setEnabled(b);
    tb_->setEnabled(b);
    return;
}

void AppSetupOpenFile::setText(QString s)
{
    le_->setText(s);
    return;
}

void AppSetupOpenFile::openFile()
{
    if( mainWindow )
    {
        QDir dir( mainWindow->layoutPath );
        dir.cd( le_->text() );
        QString fileName = dir.absolutePath();
        fileName = QFileDialog::getOpenFileName( mainWindow, tr("Open"), fileName );
        if( !fileName.isNull() )
        {
            le_->setText( QDir( mainWindow->layoutPath ).relativeFilePath( fileName ) );
            emit textEdited(le_->text());
        }
    }
    return;
}
