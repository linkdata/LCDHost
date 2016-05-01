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


#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include "AppRawInput.h"
#include "AppRawInputDialog.h"
#include "EventRawInput.h"

AppRawInputDialog::AppRawInputDialog(bool as_value, QWidget * parent, Qt::WindowFlags f) :
  QDialog(parent, f),
  as_value_(as_value),
  list_(0)
{
}

void AppRawInputDialog::rawInput( QByteArray devid, QString control, int item, int value, int flags )
{
  if (AppRawInput* ri = AppRawInput::instance()) {
    description_ = ri->describeEvent( devid, control, item, value, flags );
    if( list_->findItems(description_,Qt::MatchExactly).size() ) return;
    QListWidgetItem *listitem = new QListWidgetItem(description_);
    listitem->setData( Qt::ToolTipRole, control );
    listitem->setData( Qt::UserRole, flags );
    list_->addItem(listitem);
  }
}

int AppRawInputDialog::exec()
{
  int retv;
  QVBoxLayout *layout;
  QLabel *label;
  QDialogButtonBox *buttonBox;

  label = new QLabel( tr("Press a button or move an axis on a monitored device,\n"
                         "then select the desired event from the list."), this );
  label->setAlignment( Qt::AlignHCenter );

  list_ = new QListWidget(this);

  buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this );
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  layout = new QVBoxLayout( this );
  layout->addWidget( label );
  layout->addWidget( list_ );
  layout->addWidget( buttonBox );

  setLayout(layout);

  connect( AppRawInput::instance(), SIGNAL(rawInput(QByteArray,QString,int,int,int)),
           this, SLOT(rawInput(QByteArray,QString,int,int,int)) );
  retv = QDialog::exec();

  QListWidgetItem *listitem = list_->currentItem();
  if( listitem )
  {
    description_ = listitem->text();
    control_ = listitem->data( Qt::ToolTipRole ).toString();
    flags_ = listitem->data( Qt::UserRole ).toInt();
  }

  return retv;
}
