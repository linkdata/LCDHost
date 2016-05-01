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

#ifndef APPSETUPLISTBOX_H
#define APPSETUPLISTBOX_H

#include <QListWidget>

class AppSetupListBox : public QListWidget
{
    Q_OBJECT

public:
    explicit AppSetupListBox(QWidget *parent = 0) : QListWidget(parent) {}

public slots:
    void setItems(const QVariant texts, int index )
    {
        blockSignals(true);
        clear();
        addItems( texts.toStringList() );
        if( index < -1 ) index = -1;
        if( index >= count() ) index = count() - 1;
        blockSignals(false);
        setCurrentRow( index );
    }

    void setCurrentRow( int n ) { QListWidget::setCurrentRow(n); }
};

#endif // APPSETUPLISTBOX_H
