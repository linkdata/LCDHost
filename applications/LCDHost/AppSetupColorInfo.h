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

#ifndef APPSETUPCOLORINFO_H
#define APPSETUPCOLORINFO_H

#include <QWidget>
#include <QColor>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>

class AppSetupColorInfo : public QWidget
{
    Q_OBJECT

public:
    AppSetupColorInfo( QWidget *parent = 0 );

public slots:
    void setEnabled( bool b );
    void setValue( int n );
    void setColorName( QString s );
    void setColorAlpha( int newalpha );
    void colorPicker();

signals:
    void valueChanged(int v);

private:
    QColor color_;
    QLineEdit *colorname_;
    QSpinBox *coloralpha_;
    QFrame *colorsample_;
    QToolButton *colorpick_;
};

#endif // APPSETUPCOLORINFO_H
