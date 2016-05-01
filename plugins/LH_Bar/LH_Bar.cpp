/**
  \file     LH_Bar.cpp
  @author   Johan Lindh <johan@linkdata.se>
  @author   Andy Bridges <andy@bridgesuk.com>
  Copyright (c) 2010 Johan Lindh, Andy Bridges

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

#include <QtGlobal>
#include <QDebug>
#include <QLinearGradient>
#include <QPainter>

#include "LH_Bar.h"

static inline uint PREMUL(uint x) {
    uint a = x >> 24;
    uint t = (x & 0xff00ff) * a;
    t = (t + ((t >> 8) & 0xff00ff) + 0x800080) >> 8;
    t &= 0xff00ff;

    x = ((x >> 8) & 0xff) * a;
    x = (x + ((x >> 8) & 0xff) + 0x80);
    x &= 0xff00;
    x |= t | (a << 24);
    return x;
}

LH_Bar::LH_Bar(LH_QtObject *parent) : LH_QtCFInstance(parent)
{
    uchar data[4] = {255,0,0,0};
    bar_img_emptyMask_ = QImage(data,1,1,QImage::Format_ARGB32);

    QStringList valueTypes = QStringList();
    valueTypes.append("Gradient Fill");
    valueTypes.append("Image Stretch");
    valueTypes.append("Image Crop");

    setup_type_ = new LH_Qt_QStringList(this, "Bar Style", valueTypes, LH_FLAG_AUTORENDER);
    connect( setup_type_, SIGNAL(changed()), this, SLOT(changeType()) );

    setup_file_ = new LH_Qt_QFileInfo( this, tr("Bar Image"), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN );
    connect( setup_file_, SIGNAL(changed()), this, SLOT(changeFile()) );

    valueTypes = QStringList();
    valueTypes.append("None");
    valueTypes.append("Bar Image Transparency");
    valueTypes.append("Custom Image Transparency");
    setup_masking_ = new LH_Qt_QStringList(this, "Masking", valueTypes, LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    connect( setup_masking_, SIGNAL(changed()), this, SLOT(changeFile()) );

    setup_file_endMask_ = new LH_Qt_QFileInfo( this, tr("Transparency Mask"), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN );
    connect( setup_file_endMask_, SIGNAL(changed()), this, SLOT(changeFile()) );

    setup_pencolor1_ = new LH_Qt_QColor(this,"Bar color (start)",Qt::green,LH_FLAG_AUTORENDER);
    setup_pencolor2_ = new LH_Qt_QColor(this,"Bar color (end)",Qt::red,LH_FLAG_AUTORENDER);

    setup_file_bg_ = new LH_Qt_QFileInfo( this, tr("Bar Background Image"), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN );
    connect( setup_file_bg_, SIGNAL(changed()), this, SLOT(changeFile()) );

    setup_bgcolor_ = new LH_Qt_QColor(this,"Background color",Qt::transparent,LH_FLAG_AUTORENDER);
    setup_direction_ = new LH_Qt_QStringList(this,"Direction",
                                             QStringList("Automatic")
                                             <<"Bottom to top"
                                             <<"Left to right"
                                             <<"Top to bottom"
                                             <<"Right to left",LH_FLAG_AUTORENDER);
    setup_spacing_ = new LH_Qt_QSlider(this,"Spacing",0,0,100,LH_FLAG_AUTORENDER);

    setup_discrete_ = new LH_Qt_bool(this,"Discrete Segments",false,LH_FLAG_AUTORENDER);
    connect( setup_discrete_, SIGNAL(changed()), this, SLOT(changeDiscrete()) );
    setup_discrete_count_ = new LH_Qt_int(this,"Number of Segments",20,LH_FLAG_AUTORENDER | LH_FLAG_READONLY);


    min_ = max_ = 0.0;

    changeType();


    add_cf_source("Value");
    add_cf_target(setup_pencolor1_);
    add_cf_target(setup_pencolor2_);
    add_cf_target(setup_file_);
    add_cf_target(setup_file_bg_);
    add_cf_target(setup_bgcolor_);
}

qreal LH_Bar::boundedValue(qreal value)
{
    if( max_ < min_ ) { qreal tmp = max_; max_ = min_; min_ = tmp; }

    if( value > max_ ) value = max_;
    else if( value < min_ ) value = min_;

    if( value <= min_ ) value = 0.0;
    else value = ( value - min_ ) / (max_ - min_);

    Q_ASSERT( value >= 0.0 );
    Q_ASSERT( value <= 1.0 );

    return value;
}

void LH_Bar::draw_bar( qreal value, int pos, int total )
{
    qreal x, y;
    qreal image_width, image_height, tot;  // saves a bunch of casts
    qreal bar_width, bar_height;
    int direction;
    qreal spacing;
    qreal spacesize;
    QPainter painter;
    QLinearGradient gradient;
    QRectF rect;
    QRectF rect_full;

    if(!hasImage()) return;
    if( max_ == min_ ) return;

    image_width = image()->width();
    image_height = image()->height();
    tot = total;

    Q_ASSERT( total > 0 );
    Q_ASSERT( pos >= 0 );
    Q_ASSERT( pos < total );

    value = boundedValue(value);

    direction = setup_direction_->value();
    if( direction < 1 )
    {
        if( image_width > image_height ) direction = 2;
        else direction = 1;
    }

    x = y = 0;
    bar_width = bar_height = 0;

    switch( direction )
    {
    case 1: // Bottom to top
    case 3: // Top to bottom
        if( total < 2 ) spacesize = 0.0;
        else spacesize = (image_width - tot) / (tot-1.0) / 100.0;
        spacing = setup_spacing_->value() * spacesize;
        bar_width = ( image_width - ( spacing * (tot-1.0) ) ) / tot;
        x = (bar_width + spacing) * (qreal) pos;
        y = 0;
        bar_height = (qreal) image_height * value;
        if(setup_discrete_->value())
        {
            qreal segWidth = image_height/setup_discrete_count_->value();
            bar_height =  qRound( bar_height / segWidth ) * segWidth;
        }
        break;

    case 2: // Left to right
    case 4: // Right to left
        if( total < 2 ) spacesize = 0.0;
        else spacesize = (image_height - tot) / (tot-1.0) / 100.0;
        spacing = setup_spacing_->value() * spacesize;
        bar_width = ( image_height - ( spacing * (tot-1.0) ) ) / tot;
        x = 0;
        y = (bar_width + spacing) * (qreal) pos;
        bar_height = (qreal) image_width * value;
        if(setup_discrete_->value())
        {
            qreal segWidth = image_width/setup_discrete_count_->value();
            bar_height =  qRound( bar_height / segWidth ) * segWidth;
        }
        break;
    }


    if( painter.begin(image()) )
    {
        gradient.setStart( QPointF(0, 0) );
        switch( direction )
        {
        case 1: // Bottom to top
            {
                gradient.setFinalStop( QPointF(0,image_height) );
                gradient.setColorAt(0,setup_pencolor2_->value());
                gradient.setColorAt(1,setup_pencolor1_->value());
                rect = QRectF( x,image_height-bar_height, bar_width, bar_height );
                rect_full = QRectF( x,0, bar_width, image_height );
            }
            break;

        case 2: // Left to right
            {
                gradient.setFinalStop( QPointF(image_width,0) );
                gradient.setColorAt(0,setup_pencolor1_->value());
                gradient.setColorAt(1,setup_pencolor2_->value());
                rect = QRectF( 0,y, bar_height,bar_width );
                rect_full = QRectF( 0,y, image_width,bar_width );
            }
            break;

        case 3: // Top to bottom
            {
                gradient.setFinalStop( QPointF(0,image_height) );
                gradient.setColorAt(0,setup_pencolor1_->value());
                gradient.setColorAt(1,setup_pencolor2_->value());
                rect = QRectF( x,0, bar_width,bar_height );
                rect_full = QRectF( x,0, bar_width,image_height );
            }
            break;

        case 4: // Right to left
            {
                gradient.setFinalStop( QPointF(image_width,0) );
                gradient.setColorAt(0,setup_pencolor2_->value());
                gradient.setColorAt(1,setup_pencolor1_->value());
                rect = QRectF( image_width-bar_height,y, bar_height,bar_width );
                rect_full = QRectF( 0,y, image_width,bar_width );
            }
            break;
        }

        switch( setup_type_->value() )
        {
        case 0: //Gradient fill
            painter.setRenderHint( QPainter::Antialiasing, true );
            // painter.fillRect( bgrect, setup_bgcolor_->value() );
            painter.fillRect( rect, QBrush(gradient) );
            break;
        case 1: //Image Stretch
            if(setup_file_bg_->value().isFile())
                painter.drawImage(rect_full, bar_img_bg_);
            if(setup_file_->value().isFile())
                painter.drawImage(rect, bar_img_);
            break;
        case 2:
            if(setup_file_bg_->value().isFile())
                painter.drawImage(rect_full, bar_img_bg_);
            if(setup_file_->value().isFile())
            {
                QRectF sourcePart;
                QRectF rect_endMask;
                QRectF rect_endTidy;
                switch( direction )
                {
                case 1: // Bottom to top
                    sourcePart = QRectF(0,bar_img_.height() * (1 - bar_height/image_height), bar_img_.width(),bar_img_.height()/image_height*bar_height);
                    rect_endMask = QRectF( sourcePart.left()  ,sourcePart.top() ,sourcePart.width(), bar_img_endMask_.height() );
                    rect_endTidy = QRectF( sourcePart.left()  ,0                ,sourcePart.width() ,sourcePart.top()          );
                    break;
                case 2: // Left to right
                    sourcePart = QRectF(0,0,bar_img_.width()/image_width*bar_height,bar_img_.height());
                    rect_endMask = QRectF( sourcePart.right()  -bar_img_endMask_.width(),sourcePart.top(), bar_img_endMask_.width()           ,sourcePart.height() );
                    rect_endTidy = QRectF( sourcePart.right()                            ,sourcePart.top(), bar_img_.width()-sourcePart.width(),sourcePart.height() );
                    break;
                case 3: // Top to bottom
                    sourcePart = QRectF(0,0,bar_img_.width(),bar_img_.height()/image_height*bar_height);
                    rect_endMask = QRectF( sourcePart.left(), sourcePart.bottom()  -bar_img_endMask_.height() ,sourcePart.width() ,bar_img_endMask_.height()           );
                    rect_endTidy = QRectF( sourcePart.left(), sourcePart.bottom()                              ,sourcePart.width() ,bar_img_.height()-sourcePart.height());
                    break;
                case 4:  // Right to left
                    sourcePart = QRectF(bar_img_.width()*(1 - bar_height/image_width),0,bar_img_.width()/image_width*bar_height,bar_img_.height());
                    rect_endMask = QRectF( sourcePart.left()  ,sourcePart.top(), bar_img_endMask_.width() ,sourcePart.height() );
                    rect_endTidy = QRectF( 0                  ,sourcePart.top(), sourcePart.left()         ,sourcePart.height() );
                    break;
                }

                if((setup_masking_->value()==1 &&setup_file_->value().isFile()) || (setup_masking_->value()==2 && setup_file_endMask_->value().isFile()))
                {
                    QImage tempImg = QImage(bar_img_);
                    QPainter tempPaint;
                    if( tempPaint.begin( &tempImg ) )
                    {
                        tempPaint.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                        tempPaint.drawImage(rect_endMask, bar_img_endMask_);
                        tempPaint.drawImage(rect_endTidy, bar_img_emptyMask_);
                        tempPaint.end();
                    }
                    painter.drawImage(rect, tempImg, sourcePart);
                }
                else
                    painter.drawImage(rect, bar_img_, sourcePart);
            }
        }
        painter.end();
    }
}

bool LH_Bar::setMin( qreal r )
{
    if( min_ == r ) return false;
    min_ = r;
    return true;
}

bool LH_Bar::setMax( qreal r )
{
    if( max_ == r ) return false;
    max_ = r;
    return true;
}

QImage *LH_Bar::render_qimage( int w, int h )
{
    if(QImage * img = initImage(w, h))
    {
        img->fill(PREMUL(setup_bgcolor_->value().rgba()));
        return img;
    }
    return 0;
}

void LH_Bar::changeType()
{
    setup_file_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()==0));
    setup_file_bg_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()==0));
    setup_masking_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()!=2));
    setup_file_endMask_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()!=2 || setup_masking_->value()!=2));

    setup_pencolor1_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()!=0));
    setup_pencolor2_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()!=0));
}

void LH_Bar::changeDiscrete()
{
    setup_discrete_count_->setFlag(LH_FLAG_READONLY, !(setup_discrete_->value()));
}

void LH_Bar::changeFile()
{
    bar_img_ = setup_file_->value().isFile() ? QImage(setup_file_->value().absoluteFilePath()) : QImage();
    bar_img_bg_ = setup_file_bg_->value().isFile() ? QImage(setup_file_bg_->value().absoluteFilePath()) : QImage();

    setup_file_endMask_->setFlag(LH_FLAG_HIDDEN, (setup_type_->value()!=2 || setup_masking_->value()!=2));

    QString maskFile;
    switch(setup_masking_->value())
    {
    case 0:
        maskFile = "";
        break;
    case 1:
        maskFile = setup_file_->value().absoluteFilePath();
        break;
    case 2:
        maskFile = setup_file_endMask_->value().absoluteFilePath();
        break;
    }
    bar_img_endMask_ = QFile::exists(maskFile) ? QImage(maskFile) : QImage();
}

