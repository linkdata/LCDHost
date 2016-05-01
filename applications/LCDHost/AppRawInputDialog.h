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

#ifndef APPRAWINPUTDIALOG_H
#define APPRAWINPUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include "EventRawInput.h"

class QLabel;

class AppRawInputDialog : public QDialog
{
    Q_OBJECT

    bool as_value_;
    QListWidget *list_;
    QString description_;
    QString control_;
    int flags_;


public:
    AppRawInputDialog(bool as_value = false, QWidget * parent = 0, Qt::WindowFlags f = 0);

    QString description() const { return description_; }
    QString control() const { return control_; }
    int flags() const { return flags_; }

public slots:
    int exec();
    void rawInput( QByteArray, QString, int, int, int );
};

#endif // APPRAWINPUTDIALOG_H
