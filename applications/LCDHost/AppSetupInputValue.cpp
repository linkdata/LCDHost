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
#include "AppRawInput.h"
#include "AppRawInputDialog.h"
#include "AppSetupInputValue.h"

AppSetupInputValue::AppSetupInputValue(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    le_ = new QLineEdit(this);
    tb_ = new QToolButton(this);
    tb_->setText("...");
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget( le_, 1 );
    layout->addWidget( tb_, 0 );
    le_->setEnabled(false);
    connect( tb_, SIGNAL(clicked()), this, SLOT(selectButton()) );
    setLayout(layout);
    return;
}

void AppSetupInputValue::setEnabled(bool b)
{
    tb_->setEnabled(b);
    return;
}

void AppSetupInputValue::setText(QString s)
{
    le_->setText(s);
    return;
}

void AppSetupInputValue::selectValue()
{
    AppRawInputDialog dlg(true);
    if( dlg.exec() == QDialog::Accepted )
    {
        le_->setText( dlg.description() );
        emit inputChosen(dlg.description(),dlg.control(),dlg.flags());
    }
}
