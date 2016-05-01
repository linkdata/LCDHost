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
#include <QHBoxLayout>
#include "AppSetupFontInfo.h"

AppSetupFontInfo::AppSetupFontInfo( QWidget *parent ) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout();
    fontname_ = new QFontComboBox(this);
    fontsize_ = new QComboBox(this);
    fontbold_ = new QToolButton(this);
    fontitalic_ = new QToolButton(this);
    fontsize_->setMinimumContentsLength(2);
    fontbold_->setText("B");
    fontbold_->setCheckable(true);
    fontitalic_->setText("i");
    fontitalic_->setCheckable(true);
    connect( fontname_, SIGNAL(activated(int)), this, SLOT(fontnameChanged()) );
    connect( fontsize_, SIGNAL(activated(int)), this, SLOT(fontsizeChanged()) );
    connect( fontbold_, SIGNAL(clicked(bool)), this, SLOT(setBold(bool)) );
    connect( fontitalic_, SIGNAL(clicked(bool)), this, SLOT(setItalic(bool)) );
    fillSizeBox();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget( fontname_, 1 );
    layout->addWidget( fontsize_, 0 );
    layout->addWidget( fontbold_, 0 );
    layout->addWidget( fontitalic_, 0 );
    setLayout(layout);
}

void AppSetupFontInfo::setBold(bool b)
{
    font_.setBold(b);
    emit textEdited( font_.toString() );
    return;
}

void AppSetupFontInfo::setItalic(bool b)
{
    font_.setItalic(b);
    emit textEdited( font_.toString() );
    return;
}

void AppSetupFontInfo::fontsizeChanged()
{
    int pointsize = fontsize_->currentText().toInt();
    if( pointsize > 0 )
    {
        font_.setPointSize(pointsize);
        emit textEdited( font_.toString() );
    }
    return;
}

void AppSetupFontInfo::fontnameChanged()
{
    int pointsize = fontsize_->currentText().toInt();
    if( pointsize < 1 ) pointsize = 10;
    font_ = fontname_->currentFont();
    font_.setPointSize( pointsize );
    font_.setBold( fontbold_->isChecked() );
    font_.setItalic( fontitalic_->isChecked() );
    fillSizeBox();
    emit textEdited( font_.toString() );
    return;
}

void AppSetupFontInfo::setText(QString s)
{
    font_.fromString(s);
    fontname_->setCurrentFont(font_);
    fontbold_->setChecked( font_.bold() );
    fontitalic_->setChecked( font_.italic() );
    fillSizeBox();
    return;
}

void AppSetupFontInfo::setEnabled(bool b)
{
    fontname_->setEnabled(b);
    fontsize_->setEnabled(b);
    fontbold_->setEnabled(b);
    fontitalic_->setEnabled(b);
    return;
}

void AppSetupFontInfo::fillSizeBox()
{
    QFontDatabase db;
    QList<int> sizelist;
    int index = 0;
    int pointsize = font_.pointSize();
    fontsize_->clear();
    sizelist = db.pointSizes( font_.family(), db.styleString(font_) );
    if( sizelist.isEmpty() ) sizelist = db.standardSizes();
    foreach( int i, sizelist )
    {
        fontsize_->addItem( QString::number(i) );
        if( i < pointsize ) index ++;
    }
    fontsize_->setCurrentIndex(index);
}

