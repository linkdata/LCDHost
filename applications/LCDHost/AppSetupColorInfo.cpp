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
#include <QColorDialog>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppSetupColorInfo.h"

AppSetupColorInfo::AppSetupColorInfo( QWidget *parent ) : QWidget( parent )
{
    QHBoxLayout *layout = new QHBoxLayout();
    colorname_ = new QLineEdit(this);
    coloralpha_ = new QSpinBox(this);
    colorsample_ = new QFrame(this);
    colorpick_ = new QToolButton(this);
    coloralpha_->setMinimum(0);
    coloralpha_->setMaximum(255);
    coloralpha_->setSingleStep(32);
    colorsample_->setMinimumWidth( colorsample_->height() );
    colorsample_->setFrameStyle( QFrame::StyledPanel );
    colorsample_->setAutoFillBackground(true);
    colorpick_->setText("...");
    connect( colorname_, SIGNAL(textEdited(QString)), this, SLOT(setColorName(QString)) );
    connect( coloralpha_, SIGNAL(valueChanged(int)), this, SLOT(setColorAlpha(int)) );
    connect( colorpick_, SIGNAL(clicked()), this, SLOT(colorPicker()) );
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget( colorname_, 1 );
    layout->addWidget( coloralpha_, 0 );
    layout->addWidget( colorsample_, 0 );
    layout->addWidget( colorpick_, 0 );
    setLayout(layout);
}

void AppSetupColorInfo::setEnabled( bool b )
{
    colorname_->setEnabled(b);
    coloralpha_->setEnabled(b);
    colorpick_->setEnabled(b);
    return;
}

void AppSetupColorInfo::setValue( int n )
{
    color_.setRgba( n );
    if( color_.alpha() == 0 )
    {
        colorname_->setText( tr("transparent") );
    }
    else
    {
        colorname_->setText( QString() );
        foreach( QString s, QColor::colorNames() )
        {
            QColor c(s);
            if( c.alpha() && c.rgb() == color_.rgb() )
            {
                colorname_->setText( s );
                break;
            }
        }
        if( colorname_->text().isEmpty() )
            colorname_->setText( color_.name() );
    }
    coloralpha_->setValue( color_.alpha() );
    QPalette pal = colorsample_->palette();
    pal.setColor( QPalette::Window, color_ );
    colorsample_->setPalette( pal );
    return;
}

void AppSetupColorInfo::setColorName( QString s )
{
    color_.setNamedColor( s );
    if( color_.isValid() )
    {
        coloralpha_->setValue( color_.alpha() );
        QPalette pal = colorsample_->palette();
        pal.setColor( QPalette::Window, color_ );
        colorsample_->setPalette( pal );
    }
    else
        color_ = QColor::fromRgb(0,0,0,0);
    emit valueChanged( color_.rgba() );
    return;
}

void AppSetupColorInfo::setColorAlpha( int newalpha )
{
    if( color_.alpha() != newalpha )
    {
        color_.setAlpha( newalpha );
        QPalette pal = colorsample_->palette();
        pal.setColor( QPalette::Window, color_ );
        colorsample_->setPalette( pal );
        emit valueChanged( color_.rgba() );
    }
    return;
}

void AppSetupColorInfo::colorPicker()
{
    setValue( QColorDialog::getColor(
            color_,
            mainWindow,
            tr("Select a color"),
            QColorDialog::ShowAlphaChannel ).rgba() );
    emit valueChanged( color_.rgba() );
    return;
}

