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

#ifndef APPSETUPITEMHELP_H
#define APPSETUPITEMHELP_H

#include <QLabel>
#include <QString>
#include <QIcon>
#include <QMouseEvent>

class AppSetupItemHelp : public QLabel
{
    Q_OBJECT

    QIcon icon_;
    QString help_;

public:
    explicit AppSetupItemHelp( QString help, QWidget *parent = 0 );
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *ev);

signals:
    void clicked( QVariant pt );

public slots:

};

#endif // APPSETUPITEMHELP_H
