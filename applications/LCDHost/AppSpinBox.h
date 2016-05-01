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

#ifndef APPSPINBOX_H
#define APPSPINBOX_H

#include <QSpinBox>
#include <QLineEdit>

class AppSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    AppSpinBox( QWidget * parent = 0 ) : QSpinBox( parent ) {}

    void setLineEditEnabled( bool b ) { lineEdit()->setEnabled( b ); }
    void setLineEditReadOnly( bool b ) { lineEdit()->setReadOnly( b ); }

    void stepBy( int steps )
    {
        emit stepping( steps );
    }

signals:
    void stepping( int steps );
};

#endif // APPSPINBOX_H
