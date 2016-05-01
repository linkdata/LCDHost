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


#include "AppSetupItemHelp.h"
#include <QToolTip>
#include <QCursor>

AppSetupItemHelp::AppSetupItemHelp( QString s, QWidget *parent ) : QLabel(parent)
{
    icon_ = QIcon(":/lcdhost/images/information.png");
    help_ = s;
    setPixmap( icon_.pixmap(16,QIcon::Disabled,QIcon::On) );
    setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    setToolTip(help_);
    setFixedWidth(16);
}

void AppSetupItemHelp::enterEvent(QEvent *)
{
    setPixmap( icon_.pixmap(16,QIcon::Active,QIcon::On) );
}

void AppSetupItemHelp::leaveEvent(QEvent *)
{
    setPixmap( icon_.pixmap(16,QIcon::Disabled,QIcon::On) );
}

void AppSetupItemHelp::mousePressEvent(QMouseEvent *ev)
{
    if( ev->button() == Qt::LeftButton )
    {
        QToolTip::showText( ev->globalPos(), help_, this );
    }
}
