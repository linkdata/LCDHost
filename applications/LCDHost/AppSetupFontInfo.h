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

#ifndef APPSETUPFONTINFO_H
#define APPSETUPFONTINFO_H

#include <QWidget>
#include <QComboBox>
#include <QFontComboBox>
#include <QToolButton>

class AppSetupFontInfo : public QWidget
{
    Q_OBJECT

public:
    AppSetupFontInfo( QWidget *parent = 0 );
    void fillSizeBox();
    void setCurrentFont(QFont font);

public slots:
    void setText(QString);
    void setEnabled(bool b);
    void setBold(bool b);
    void setItalic(bool b);
    void fontnameChanged();
    void fontsizeChanged();

signals:
    void textEdited(QString s);

private:
    QFont font_;
    QFontComboBox *fontname_;
    QComboBox *fontsize_;
    QToolButton *fontbold_;
    QToolButton *fontitalic_;
};

#endif // APPSETUPFONTINFO_H
