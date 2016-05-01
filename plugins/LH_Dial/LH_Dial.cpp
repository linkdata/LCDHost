/**
  \file     LH_Dial.cpp
  @author   Andy Bridges <andy@bridgesuk.com>
  Copyright (c) 2011 Andy Bridges

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#include <QtGlobal>
#include <QDebug>
#include <QtCore/qmath.h>
#include <QLinearGradient>
#include <QPainter>

#include "LH_Dial.h"

#define UNUSED_AREA -2

static inline uint PREMUL(uint x)
{
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

LH_Dial::LH_Dial(DialType dialType, QString unusedCapacityStyle)
{
    min_ = max_ = 0.0;
    polling_on_ = false;
    isClock = false;
    dialType_ = dialType;
    unusedCapacityStyleLock_ = unusedCapacityStyle;
}

const char *LH_Dial::userInit()
{
    faceImage_ = new QImage();
    unusedImage_ = new QImage();

    setup_type_ = new LH_Qt_QStringList(this, "Dial Type", QStringList()<<"Full Circle"<<"Semi-Circle"<<"Quarter Circle", LH_FLAG_AUTORENDER);
    setup_type_->setHelp( "<p>The dial's shape.</p>");

    setup_orientation_ = new LH_Qt_QStringList(this,"Orientation",QStringList()<<"N/A", LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_orientation_->setHelp( "<p>The orientation of the dial (does not apply to circular dials).</p>");

    setup_bgcolor_ = new LH_Qt_QColor(this,"Background color",Qt::transparent,LH_FLAG_AUTORENDER);
    setup_bgcolor_->setHelp( "<p>The color for the background (i.e. <i>behind</i> the face).</p>");

    setup_face_style_ = new LH_Qt_QStringList(this, "Face Style",QStringList()<<"None"<<"Line"<<"Gradient Fill"<<"Image", LH_FLAG_AUTORENDER);
    setup_face_style_->setHelp( "<p>How the face of the dial should be drawn - if at all.</p>");

    setup_face_pencolor_ = new LH_Qt_QColor(this,"Pen color",Qt::black,LH_FLAG_AUTORENDER);
    setup_face_pencolor_->setHelp( "<p>The colour used for the face's outline.</p>");

    setup_face_fillcolor1_ = new LH_Qt_QColor(this,"Fill color (start)",Qt::white,LH_FLAG_AUTORENDER);
    setup_face_fillcolor1_->setHelp( "<p>The color used to fill dial's face at the top</p>");

    setup_face_fillcolor2_ = new LH_Qt_QColor(this,"Fill color (end)",Qt::lightGray,LH_FLAG_AUTORENDER);
    setup_face_fillcolor2_->setHelp( "<p>The color used to fill dial's face at the bottom</p>");

    setup_face_image_ = new LH_Qt_QFileInfo( this, tr("Face Image"), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_face_image_->setHelp( "<p>Image file to load and use as the dial's face</p>");

    setup_face_ticks_ = new LH_Qt_bool(this,"Show Ticks",false,LH_FLAG_AUTORENDER);
    setup_face_ticks_->setHelp( "<p>Whether to overlay marks denoting significant points along the dial.</p>");
    connect( setup_face_ticks_, SIGNAL(changed()), this, SLOT(changeTicks()));

    setup_face_tickcolor_ = new LH_Qt_QColor(this,"Tick color",Qt::black,LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
    setup_face_tickcolor_->setHelp( "<p>The colour used for the tick marks.</p>");

    connect( setup_face_style_, SIGNAL(changed()), this, SLOT(changeFaceStyle()));

    QString nameText1 = (dialType_==DIALTYPE_DIAL? "Needle" : "Segment");
    QString nameText2 = (dialType_==DIALTYPE_DIAL? "needle" : "segment");

    setup_needles_reverse_ = new LH_Qt_bool(this, QString("Reverse %1s").arg(nameText1), false, LH_FLAG_AUTORENDER);
    setup_needles_reverse_->setHelp( QString("<p>By default all %1s move clockwise; this setting changes that to anti-clockwise.</p>"
                                             "<p>This makes particular sense for some dial orientations, e.g. half-cicle dials in the \"top\" orientation default to having 0% on the right and 100% on the left. Applying this setting reverses that, which for this example will look more natural.</p>").arg(nameText2));

    setup_needles_smooth_ = new LH_Qt_bool(this, QString("Smooth %1s").arg(nameText1), true, LH_FLAG_AUTORENDER);
    setup_needles_smooth_->setHelp( QString("<p>This setting makes %1s move more slowly by rendering %1 movement in 8 gradual steps instead of a single step. This looks much nicer but causes a small increase in CPU usage.</p>").arg(nameText2));

    setup_needle_selection_ = new LH_Qt_QStringList(this,"Selected Needle",QStringList(),LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_needle_selection_->setHelp( QString("<p>Select a %1 here and configure it below. Seperate settings are stored for each %1.</p>").arg(nameText2));

    if(dialType_==DIALTYPE_DIAL)
    {
    setup_needle_style_ = new LH_Qt_QStringList(this, "Needle Style",QStringList()<<"Line"<<"Image [Needle Only]"<<"Image [Full Face]", LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_needle_style_->setHelp( "<p>How the selected needle should be drawn.</p>"
                                  "<p>Needle images can be created in one of two ways:<ul>"
                                  "<li>\"Needle Only\": "
                                  "<br/>This type of image is only as wide and as long as the needle. The image will be stretched or compressed as required to match the needle's length; the width will be scaled so as to preserve the aspect ratio. "
                                  "<br/>The needle is assumed to be in the vertical position and will be rotated around the image's <i>base</i> as required.</li>"
                                  "<li>\"Full Face\": "
                                  "<br/>This type of image is the size of the entire dial. The image will be stretched or compressed as required to completely fill the dial (and may therefore not preserve the aspect ratio). "
                                  "<br/>The needle is assumed to be in the vertical position and will be rotated around the image's <i>center point</i> as required.</li>"
                                  "</ul></p>");
    } else {
        setup_needle_style_ = new LH_Qt_QStringList(this, "Segment Style",QStringList()<<"Fill"<<"Image (Minimum Aligned)"<<"Image (Maximum Aligned)", LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
        setup_needle_style_->setHelp( "<p>How the selected segment should be drawn.</p>");
    }
    setup_needle_color_ = new LH_Qt_QColor(this,QString("%1 Color").arg(nameText1),Qt::red,LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    if(dialType_==DIALTYPE_DIAL)
        setup_needle_color_->setHelp( "<p>The colour used do draw \"Line\" needles.</p>");
    else
        setup_needle_color_->setHelp( "<p>The colour used do fill \"Fill\" segments.</p>");

    setup_needle_gradient_ = new LH_Qt_bool(this,"Gradient Fill",false,LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_needle_color2_ = new LH_Qt_QColor(this,QString("%1 Color 2").arg(nameText1),Qt::yellow,LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);


    setup_needle_thickness_ = new LH_Qt_int(this,"Needle Thickness",3,1,20,LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA|(dialType_!=DIALTYPE_DIAL?LH_FLAG_HIDDEN:0));
    setup_needle_thickness_->setHelp( "<p>The width of a \"Line\" needle.</p>");

    setup_needle_length_ = new LH_Qt_int(this,QString("%1 Length (%)").arg(nameText1),90,0,100,LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_needle_length_->setHelp( QString("<p>The %1's length as a percentage of the dial's radius.</p>").arg(nameText2));

    setup_needle_gap_ = new LH_Qt_int(this,"Needle Gap (%)",0,0,100,LH_FLAG_AUTORENDER|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA|(dialType_!=DIALTYPE_DIAL?LH_FLAG_HIDDEN:0));
    setup_needle_gap_->setHelp( "<p>The gap between the center of the dial and the needle's start as a percentage of the dial's radius.</p>");

    setup_needle_image_ = new LH_Qt_QFileInfo( this, QString("%1 Image").arg(nameText1), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN |LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA);
    setup_needle_image_->setHelp( QString("<p>Image file to load and use for this %2 (see \"%1 Style\" for more information about how the image will be used).</p>").arg(nameText1).arg(nameText2));
    connect( setup_needle_style_, SIGNAL(changed()), this, SLOT(changeNeedleStyle()));

    if(dialType_==DIALTYPE_PIE)
    {
        setup_unused_style_ = new LH_Qt_QStringList(this, "Unused Capacity Style", QStringList() << "Empty" << "Hidden" << "Fill" << "Image (Minimum Aligned)" << "Image (Maximum Aligned)", LH_FLAG_AUTORENDER);

        setup_unused_color_ = new LH_Qt_QColor(this,QString("%1 Color").arg("Unused Capacity"),Qt::white,LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
        setup_unused_color_->setHelp( "<p>The colour used do fill the unused capacity area.</p>");
        setup_unused_gradient_ = new LH_Qt_bool(this,"Unused Capacity Gradient Fill",false,LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
        setup_unused_color2_ = new LH_Qt_QColor(this,QString("%1 Color2").arg("Unused Capacity"),Qt::white,LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);

        setup_unused_length_ = new LH_Qt_int(this,QString("%1 Length (%)").arg("Unused Capacity"),90,0,100,LH_FLAG_AUTORENDER|LH_FLAG_HIDDEN);
        setup_unused_length_->setHelp( QString("<p>The %1's length as a percentage of the dial's radius.</p>").arg(nameText2));

        setup_unused_image_ = new LH_Qt_QFileInfo( this, QString("%1 Image").arg("Unused Capacity"), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_HIDDEN);
        setup_unused_image_->setHelp( QString("<p>Image file to load and use for this %2 (see \"%1 Style\" for more information about how the image will be used).</p>").arg(nameText1).arg(nameText2));

        if(unusedCapacityStyleLock_!="")
        {
            setup_unused_style_->setValue(unusedCapacityStyleLock_);
            setup_unused_style_->setVisible(false);
            changeUnusedStyle();
        } else
            connect( setup_unused_style_, SIGNAL(changed()), this, SLOT(changeUnusedStyle()));

    }

    setup_needle_configs_ = new LH_Qt_QTextEdit(this, QString("%1 Configs").arg(nameText1),"", LH_FLAG_HIDDEN);
    setup_needle_configs_->setHelp( QString("<p>This text field stores the configuration data for each individual %2 and should not be edited manually.</p>").arg(nameText2));
    setup_needle_configs_->setOrder(100);

    addNeedle("Default");
    connect( setup_type_, SIGNAL(changed()), this, SLOT(changeType()));
    connect( setup_needle_configs_, SIGNAL(changed()), this, SLOT(changeSelectedNeedle()) );
    connect( setup_needle_selection_, SIGNAL(changed()), this, SLOT(changeSelectedNeedle()) );
    connect( setup_needle_style_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_color_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_gradient_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_color2_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_thickness_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_length_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_gap_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );
    connect( setup_needle_image_, SIGNAL(changed()), this, SLOT(updateSelectedNeedle()) );

    ticks.fullCircle.append(tickObject(20, 1, 0.05, 0.90));
    ticks.fullCircle.append(tickObject(10, 2, 0.15, 0.80));
    ticks.semiCircle.append(tickObject(21, 1, 0.05, 0.90));
    ticks.semiCircle.append(tickObject(11, 2, 0.15, 0.80));
    ticks.quarterCircle.append(tickObject(11, 1, 0.05, 0.90));
    ticks.quarterCircle.append(tickObject(3, 2, 0.15, 0.80));

    changeType();
    changeSelectedNeedle();
    changeFaceStyle();
    changeNeedleStyle();
    return 0;
}

LH_Dial::~LH_Dial()
{
    for(int i = 0; i<needleImage_.count(); i++)
        delete needleImage_[i];
    delete unusedImage_;
}

void LH_Dial::addNeedle(QString name, QColor defaultColor)
{
    setup_needle_selection_->list().append(name);
    setup_needle_selection_->refreshList();
    needle_pos_.append(0);
    needle_val_.append(0);
    needle_step_.append(1);
    needle_vis_.append(true);
    needleCode_.append("");
    needleImage_.append( new QImage() );

    syncNeedleConfigs(defaultColor);
}

void LH_Dial::syncNeedleConfigs(QColor defaultColor)
{
    QStringList configs = setup_needle_configs_->value().split('~',QString::SkipEmptyParts);
    if (configs.length()<needleCount())
    {
        QColor newColor = defaultColor;
        while(configs.length()<needleCount())
        {
            if (newColor==Qt::transparent)
                switch (configs.length() % 6)
                {
                case 0: newColor = QColor::fromRgb(192,0,0); break;   // red
                case 1: newColor = QColor::fromRgb(0,0,192); break;   // blue
                case 2: newColor = QColor::fromRgb(0,192,0); break;   // green
                case 3: newColor = QColor::fromRgb(192,192,0); break; // yellow
                case 4: newColor = QColor::fromRgb(0,192,192); break; // cyan
                case 5: newColor = QColor::fromRgb(192,0,192); break; // mageneta
                }
            QString configString = buildNeedleConfig(newColor);
            configs.append(configString);
        }
        setup_needle_configs_->setValue(configs.join("~"));
    }
    setup_needle_selection_->setFlag(LH_FLAG_HIDDEN, setup_needle_selection_->list().count()==1);
    setup_needle_selection_->setValue(0);
    changeSelectedNeedle();
}

void LH_Dial::clearNeedles()
{
    setup_needle_selection_->list().clear();
    setup_needle_selection_->refreshList();
    needle_pos_.clear();
    needle_val_.clear();
    needle_step_.clear();
    needleCode_.clear();
    needle_vis_.clear();
    for(int i = 0; i<needleImage_.count(); i++)
        delete needleImage_[i];
    needleImage_.clear();
    setup_needle_selection_->setFlag(LH_FLAG_HIDDEN, false);
}

int LH_Dial::needleCount()
{
    return setup_needle_selection_->list().count();
}

bool LH_Dial::setMin( qreal r )
{
    if( min_ == r ) return false;
    min_ = r;
    return true;
}

bool LH_Dial::setMax( qreal r )
{
    if( max_ == r ) return false;
    max_ = r;
    return true;
}

QString LH_Dial::colString(QColor col)
{
    QString str = "#";
    str += QString::number(col.alpha() , 16);
    str += QString::number(col.red() , 16);
    str += QString::number(col.green() , 16);
    str += QString::number(col.blue() , 16);
    return str;
}

QImage LH_Dial::getFace()
{
    QString faceCode = "";

    int w = image()->width();
    int h = image()->height();

    switch(setup_face_style_->value())
    {
    case 0: // None
        break;
    case 2: // Gradient Fill
        faceCode = QString("%1;%2;").arg(colString(setup_face_fillcolor1_->value())).arg(colString(setup_face_fillcolor2_->value()));
    case 1: // Line
        faceCode = QString("%1,%2;%3,%4;").arg(w).arg(h).arg(maxDegrees()).arg(startDegrees()) + faceCode;
        faceCode += QString("%1;%2;%3;%4;").arg(colString(setup_bgcolor_->value())).arg(colString(setup_face_pencolor_->value())).arg(setup_face_ticks_->value()).arg(colString(setup_face_tickcolor_->value()));
        break;
    case 3: //Image
        faceCode = QString("%1,%2;%3").arg(w).arg(h).arg(setup_face_image_->value().absoluteFilePath());
        break;
    }
    faceCode = QString::number(setup_face_style_->value()) + ":" + faceCode;


    if (faceCode_ != faceCode)
    {
        delete faceImage_;
        faceImage_ = new QImage(w,h,QImage::Format_ARGB32_Premultiplied);
        faceImage_->fill( PREMUL( setup_bgcolor_->value().rgba() ) );

        if (setup_face_style_->value()!=0)
        {
            QPainter painter;
            if( painter.begin( faceImage_ ) )
            {
                if(setup_face_style_->value()==3)//image
                {
                    if(setup_face_image_->value().isFile())
                    {
                        QImage face_img = QImage(setup_face_image_->value().absoluteFilePath());
                        painter.drawImage(QRectF( 0, 0, w, h ), face_img);
                    }
                } else {
                    QPen pen = QPen(setup_face_pencolor_->value());
                    if (setup_face_style_->value()==2)
                    {
                        QLinearGradient gradient;
                        gradient.setStart( QPointF(0, 0) );
                        gradient.setFinalStop( QPointF(0,h) );
                        gradient.setColorAt(0,setup_face_fillcolor1_->value());
                        gradient.setColorAt(1,setup_face_fillcolor2_->value());
                        painter.setBrush(QBrush(gradient));
                    } else
                        painter.setBrush(QBrush(Qt::transparent));

                    int x = 0;
                    int y = 0;
                    int xm = 1;
                    int ym = 1;
                    int xo = 2;
                    int yo = 2;
                    painter.setPen(pen);
                    switch(setup_type_->value())
                    {
                    case 0: //full circle
                        painter.drawEllipse(0,0,w-1,h-1);
                        break;
                    case 1: //semi-circle
                        switch(setup_orientation_->value())
                        {
                        case 0: //left / bottom left
                            x = -1;
                            xm = 2;
                            break;
                        case 1: //top / top left
                            y = -1;
                            ym = 2;
                            break;
                        case 2: //right / top right
                            xm = 2;
                            xo = 4;
                            break;
                        case 3: //bottom / bottom right
                            ym = 2;
                            yo = 4;
                            break;
                        }
                        painter.drawPie(x*w+1, y*h+1, w*xm-xo, h*ym-yo, startDegrees()*-16 + 90*-16,maxDegrees()*16);
                        break;
                    case 2: //quarter circle
                        xm = 2;
                        ym = 2;
                        switch(setup_orientation_->value())
                        {
                        case 0: //left / bottom left
                            x = -1;
                            yo = 4;
                            break;
                        case 1: //top / top left
                            x = -1;
                            y = -1;
                            break;
                        case 2: //right / top right
                            y = -1;
                            xo = 4;
                            break;
                        case 3: //bottom / bottom right
                            xo = 4;
                            yo = 4;
                            break;
                        }
                        painter.drawPie(x*w+1, y*h+1, w*xm-xo, h*ym-yo, startDegrees()*-16,maxDegrees()*16);
                        break;
                    }

                    if(setup_face_ticks_->value())
                    {
                        QList<tickObject> tickdef;
                        switch(setup_type_->value())
                        {
                        case 0:
                            tickdef = ticks.fullCircle;
                            break;
                        case 1:
                            tickdef = ticks.semiCircle;
                            break;
                        case 2:
                            tickdef = ticks.quarterCircle;
                            break;
                        }

                        foreach(tickObject tick, tickdef)
                        {
                            int m = (setup_type_->value() == 0? 1 : 0); // fix for full circle having an overlapping last/first tick
                            for(int i=0; i<tick.count+m; i++)
                            {
                                QPen pen = QPen(setup_face_tickcolor_->value());
                                pen.setWidth(tick.width);
                                paintLine(painter, pen, startDegrees(), maxDegrees() / (tick.count+m-1) * i, tick.length, tick.gap );
                            }
                        }
                    }

                }

                painter.end();
            }
        }

        faceCode_ = faceCode;
    }
    return *faceImage_;
}


void LH_Dial::getDimensions(qreal degrees, int& h, int& w, qreal& radH, qreal& radW, qreal& radians, qreal& drawLen)
{
    qreal radiansM;
    h = image()->height();
    w = image()->width();

    getRadii(radH, radW);
    radians = getRadians(degrees, radiansM);
    drawLen = getDrawLen(radH, radW, radiansM);
}

qreal LH_Dial::getRadians(qreal degrees, qreal& offsetRadians)
{
    qreal radians = degrees * M_PI/180;
    offsetRadians = radians;
    switch(setup_type_->value())
    {
    case 0: //full circle
        break;
    case 1: //semi-circle
        switch(setup_orientation_->value())
        {
        case 0: //left / bottom left
            break;
        case 1: //top / top left
            offsetRadians += M_PI/2.0;
            break;
        case 2: //right / top right
            offsetRadians += M_PI;
            break;
        case 3: //bottom / bottom right
            offsetRadians += -M_PI/2.0;
            break;
        }
        break;
    case 2: //quarter circle
        break;
    }
    return radians;
}

qreal LH_Dial::getDrawLen(qreal boxHeight, qreal boxWidth, qreal radians)
{
    qreal denom = qSqrt( qPow(boxHeight * qSin(radians),2) + qPow(boxWidth * qCos(radians),2) );    
    if(denom == 0) return 0;
    qreal drawLen = boxHeight * boxWidth / denom;
    if( drawLen != drawLen )
    {
        qDebug() << "UNABLE TO CALCULATE drawLen!";
        qDebug() << "boxHeight/Width: " << boxHeight << ", " << boxWidth;
        qDebug() << "denominator: " << qSqrt( qPow(boxHeight * qSin(radians),2) + qPow(boxWidth * qCos(radians),2) );
        qDebug() << "Radians:" << radians << "; Sin: " << qSin(radians) << "; Cos: " << qCos(radians);
        Q_ASSERT(drawLen == drawLen);
    }
    return drawLen;
}

void LH_Dial::getRadii(qreal& radH, qreal& radW)
{
    qreal w = image()->width();
    qreal h = image()->height();
    radH = h/2;
    radW = w/2;

    switch(setup_type_->value())
    {
    case 0: //full circle
        break;
    case 1: //semi-circle
        switch(setup_orientation_->value())
        {
        case 0: //left / bottom left
            radW = w;
            break;
        case 1: //top / top left
            radH = h;
            break;
        case 2: //right / top right
            radW = w;
            break;
        case 3: //bottom / bottom right
            radH = h;
            break;
        }
        break;
    case 2: //quarter circle
        radW = (setup_needles_reverse_->value()? h : w);
        radH = (setup_needles_reverse_->value()? w : h);
        break;
    }
}

void LH_Dial::getCenter(QPointF& center)
{
    qreal w = image()->width();
    qreal h = image()->height();

    //h*0.9, w*0.9
    center.setX(w/2-.5) ;
    center.setY(h/2-.5) ;

    switch(setup_type_->value())
    {
    case 0: //full circle
        break;
    case 1: //semi-circle
        switch(setup_orientation_->value())
        {
        case 0: //left / bottom left
            center.setX( 0 );
            break;
        case 1: //top / top left
            center.setY( 0 );
            break;
        case 2: //right / top right
            center.setX( w );
            break;
        case 3: //bottom / bottom right
            center.setY( h );
            break;
        }
        break;
    case 2: //quarter circle
        switch(setup_orientation_->value())
        {
        case 0: //left / bottom left
            center.setX( 0 );
            center.setY( h );
            break;
        case 1: //top / top left
            center.setX( 0 );
            center.setY( 0 );
            break;
        case 2: //right / top right
            center.setX( w );
            center.setY( 0 );
            break;
        case 3: //bottom / bottom right
            center.setX( w );
            center.setY( h );
            break;
        }
        break;
    }
}

void LH_Dial::getRotationData(qreal startAngle, qreal angle, qreal& centerX, qreal& centerY, qreal& radH, qreal& radW, qreal& radians)
{
    radians = (180+(angle+startAngle)) * M_PI/180;
    getRadii(radH, radW);
    getCenter(centerX, centerY);
}

QString LH_Dial::generateNeedleCode(qreal drawLen, QColor needleColor, int needleThick, int needleLength, int needleGap, int h, int w, QString needleImagePath, int needleStyle, bool sliceGradient, QColor sliceColor2)
{
    // Generates a description of the current needle settings used to check if the "cached" needle image is still correct
    QString needleCode = "";
    switch(needleStyle)
    {
    case 0: // Line
        needleCode = QString("%1;%2;%3;%4;%5;%6;%7").arg(drawLen).arg(colString(needleColor)).arg(needleThick).arg(needleLength).arg(needleGap).arg(sliceGradient).arg(colString(sliceColor2));
        break;
    case 1: // Image (needle only)
    case 2: // Image (full face)
        needleCode = QString("%1;%2;%3;%4;%5;%6").arg(drawLen).arg(needleImagePath).arg(h).arg(w).arg(dialType_==DIALTYPE_PIE? needleThick : 0).arg(dialType_==DIALTYPE_PIE? needleGap : 0);
        break;
    }
    return QString::number(needleStyle) + ":" + needleCode;
}

/**
    getNeedle returns the image of the needle in the vertical position (i.e. at 12 o'clock). Although the needle is vertical
    it still needs to know what angle it will be rotated to later on so that is can be sized accordingly - otherwise dials
    with non-constant radii (i.e. ovals) would not see their needle change size correctly.
**/
QImage LH_Dial::getNeedle(int needleID, qreal degrees, int& needleStyle)
{
    QColor needleColor = QColor();
    int needleThick;
    int needleLength;
    int needleGap;
    QString needleImagePath;
    bool needleGradient;
    QColor needleColor2 = QColor();

    loadNeedleConfig(needleID, needleStyle, needleColor, needleThick, needleLength, needleGap, needleImagePath, needleGradient, needleColor2);

    int h; int w; qreal radH; qreal radW; qreal radians; qreal drawLen;
    getDimensions(degrees, h, w, radH, radW, radians, drawLen);

    QString needleCode = generateNeedleCode(drawLen, needleColor, needleThick, needleLength, needleGap, h, w, needleImagePath, needleStyle, needleGradient, needleColor2);
    if (needleCode_[needleID] != needleCode)
    {
        delete needleImage_[needleID];

        QFileInfo f(needleImagePath);
        if(needleStyle == 1 && f.isFile())
        {
            QImage needle_img(f.absoluteFilePath());
            //rescale needle image width to match the required height
            int drawWid = qCeil(((qreal)(needle_img.width() * drawLen)) / needle_img.height());

            needleImage_[needleID] = new QImage(drawWid,drawLen,QImage::Format_ARGB32_Premultiplied);
            needleImage_[needleID]->fill( PREMUL( QColor(Qt::transparent).rgba() ) );

            //create the resized needle image
            QPainter painter;
            if( painter.begin( needleImage_[needleID] ) )
            {
                painter.drawImage(QRectF( 0, 0, drawWid, drawLen ), needle_img);
                painter.end();
            }
        }else
        if(needleStyle == 2 && f.isFile())
        {
            QImage needle_img(f.absoluteFilePath());
            //rescale needle image width to match the required height
            qreal drawWid = getDrawLen(radW, radH, radians);

            needleImage_[needleID] = new QImage(drawWid*2,drawLen*2,QImage::Format_ARGB32_Premultiplied);
            needleImage_[needleID]->fill( PREMUL( QColor(Qt::transparent).rgba() ) );

            //create the resized needle image
            QPainter painter;
            if( painter.begin( needleImage_[needleID] ) )
            {
                painter.drawImage(QRectF( 0, 0, drawWid*2, drawLen*2 ), needle_img);
                painter.end();
            }
        }
        else
        {
            int drawWid = needleThick;
            needleImage_[needleID] = new QImage(drawWid,drawLen,QImage::Format_ARGB32_Premultiplied);
            needleImage_[needleID]->fill( PREMUL( QColor(Qt::transparent).rgba() ) );

            QPainter painter;
            if( painter.begin( needleImage_[needleID] ) )
            {
                int y =  drawLen * (1.0 - (needleLength + needleGap)/100.0);
                QBrush brush = QBrush(needleColor);
                if (needleGradient)
                {
                    QLinearGradient gradient;
                    gradient.setStart( 0, y );
                    gradient.setFinalStop( 0, drawLen * needleLength/100.0 );
                    gradient.setColorAt(0,needleColor);
                    gradient.setColorAt(1,needleColor2);
                    brush = QBrush(gradient);
                }
                painter.fillRect(0,y,drawWid, drawLen * needleLength/100.0, brush);
                painter.end();
            }
        }

        needleCode_[needleID] = needleCode;
    }
    return *needleImage_[needleID];
}

QImage LH_Dial::getSlice(int sliceID, qreal degrees, qreal offsetAngle, int& sliceStyle)
{
    QColor sliceColor = QColor();
    int sliceLength;
    QString sliceImagePath;
    int sliceImageAlpha;
    bool sliceGradient;
    QColor sliceColor2 = QColor();
    loadSliceConfig(sliceID, sliceStyle, sliceColor, sliceLength, sliceImagePath, sliceImageAlpha, sliceGradient, sliceColor2);

    int h; int w; qreal radH; qreal radW; qreal radians; qreal drawLen;
    getDimensions(degrees, h, w, radH, radW, radians, drawLen);

    QPainter painter;
    QString needleCode = generateNeedleCode(drawLen, sliceColor, qRound(degrees*16), sliceLength, offsetAngle, h, w, QString("%1@%2").arg(sliceImagePath).arg(sliceImageAlpha), sliceStyle, sliceGradient, sliceColor2);

    if (needleCode != (sliceID==UNUSED_AREA? unusedAreaCode_ : needleCode_[sliceID]))
    {
        qreal circleW = radW * 2;
        qreal circleH = radH * 2;
        QImage* sliceImage;

        QFileInfo f(sliceImagePath);
        if((sliceStyle == 1 || sliceStyle == 2) && f.isFile())
        {
            if(!fgImgs_.contains(sliceImagePath) && f.isFile())
                 fgImgs_.insert(sliceImagePath,QImage(sliceImagePath));
            Q_ASSERT(fgImgs_.contains(sliceImagePath));

            //build mask
            QImage maskImg = QImage(circleW, circleH,QImage::Format_ARGB32_Premultiplied);
            maskImg.fill( PREMUL( QColor(Qt::transparent).rgba() ) );
            if( painter.begin( &maskImg ) )
            {
                painter.setRenderHint( QPainter::Antialiasing, true );
                QColor maskCol = QColor(0,0,0,sliceImageAlpha);
                painter.setPen(maskCol);
                painter.setBrush(QBrush(maskCol));
                painter.drawPie(circleW*(100-sliceLength)/200.0,circleH*(100-sliceLength)/200.0, circleW*sliceLength/100.0, circleH*sliceLength/100.0, (startDegrees()+offsetAngle-90)*-16,qRound(degrees*-16));
                painter.end();
            }

            QImage tempImg = QImage(fgImgs_.value(sliceImagePath).width(), fgImgs_.value(sliceImagePath).height(),QImage::Format_ARGB32_Premultiplied);
            tempImg.fill( PREMUL( QColor(Qt::transparent).rgba() ) );

            //Rotate the image
            if( painter.begin( &tempImg ) )
            {
                paintImage(painter, fgImgs_.value(sliceImagePath), ROT_CENTER, startDegrees(), offsetAngle+( sliceStyle == 2? degrees : 0));
                painter.end();
            }

            sliceImage = new QImage(circleW, circleH,QImage::Format_ARGB32_Premultiplied);
            sliceImage->fill( PREMUL( QColor(Qt::transparent).rgba() ) );

            //Rescale the image & apply mask to create slice image
            if( painter.begin( sliceImage ) )
            {
                painter.drawImage(QRectF( 0,0, circleW, circleH ), tempImg);
                painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                painter.drawImage(QPointF( 0,0 ), maskImg);
                painter.end();
            }

        }
        else
        {
            sliceImage = new QImage(circleW, circleH,QImage::Format_ARGB32_Premultiplied);
            sliceImage->fill( PREMUL( QColor(Qt::transparent).rgba() ) );

            if( painter.begin( sliceImage ) )
            {

                QBrush brush = QBrush(sliceColor);
                if (sliceGradient)
                {
                    QLinearGradient gradient;
                    gradient.setStart( 0, 0 );
                    gradient.setFinalStop( 0, circleH * sliceLength/100.0 );
                    gradient.setColorAt(0,sliceColor);
                    gradient.setColorAt(1,sliceColor2);
                    brush = QBrush(gradient);
                }

                painter.setBrush(brush);
                painter.setPen(QPen(sliceColor));
                painter.drawPie(radW*(100-sliceLength)/100.0,radH*(100-sliceLength)/100.0, circleW*sliceLength/100.0, circleH*sliceLength/100.0, (startDegrees()+offsetAngle-90)*-16,qRound(degrees*-16));
                painter.end();
            }
        }

        if(sliceID==UNUSED_AREA)
        {
            delete unusedImage_;
            unusedAreaCode_ = needleCode;
            unusedImage_ = sliceImage;
        }else{
            delete needleImage_[sliceID];
            needleCode_[sliceID] = needleCode;
            needleImage_[sliceID] = sliceImage;
        }
    }

    return (sliceID==UNUSED_AREA? *unusedImage_ : *needleImage_[sliceID]);

}

void LH_Dial::paintLine(QPainter& painter, QPen& pen, qreal startAngle, qreal angle, qreal relLength, qreal gap)
{
    qreal centerX; qreal centerY; qreal radW; qreal radH; qreal radians;
    getRotationData(startAngle, angle, centerX, centerY, radH, radW, radians);

    int x1 = centerX - (qSin(radians) * radW * gap);
    int x2 = centerX - (qSin(radians) * radW * (gap+relLength));

    int y1 = centerY + (qCos(radians) * radH * gap);
    int y2 = centerY + (qCos(radians) * radH * (gap+relLength));

    painter.setPen(pen);
    painter.drawLine(x1,y1,x2,y2);
}

void LH_Dial::paintImage(QPainter& painter, QImage srcImage, RotationType rotationType, qreal startAngle, qreal angle)
{
    qreal x; qreal y; qreal radW; qreal radH; qreal radians;

    getRotationData(startAngle, (rotationType == ROT_NONE? 0 : angle), x, y, radW, radH, radians);

    painter.save();

    switch(rotationType)
    {
    case ROT_NONE:
        x -= (srcImage.width()/2.0);
        y -= (srcImage.height()/2.0);
        break;
    case ROT_CENTER:
        //ROT_CENTER is a special case that doesn't use the dial's dimensions, but rather uses the dimensions of srcImage.
        x = (srcImage.width()/2.0);
        y = (srcImage.height()/2.0);
    case ROT_FACE:
    case ROT_NEEDLE:
        qreal A = (x!=0? qAtan(y/x) : M_PI/2);
        qreal Hyp = qSqrt( qPow(y,2) + qPow(x,2) );
        qreal drawLen = getDrawLen(y,x,radians);

        painter.rotate(startAngle + angle);
        x = Hyp * qCos(A - (radians-M_PI)) - (srcImage.width()/2.0);
        y = Hyp * qSin(A - (radians-M_PI)) - (srcImage.height());
        if(rotationType==ROT_CENTER)
            y +=drawLen;
        if(rotationType==ROT_FACE)
        {
            y +=drawLen;
            switch(setup_type_->value())
            {
            case 0: //full circle
                break;
            case 1: //semi-circle
                switch(setup_orientation_->value())
                {
                case 0: //left / bottom left
                case 1: //top / top left
                    y += (srcImage.height()/2);
                    break;
                case 2: //right / top right
                case 3: //bottom / bottom right
                    break;
                }
                break;
            case 2: //quarter circle
                switch(setup_orientation_->value())
                {
                case 0: //left / bottom left
                case 1: //top / top left
                case 2: //right / top right
                    y += (srcImage.height()/2);
                    break;
                case 3: //bottom / bottom right
                    break;
                }
                break;
            }
        }
        break;
    }

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.drawImage(QPointF(x,y),srcImage);

    painter.restore();
}



bool LH_Dial::setVal(qreal value, int i, bool repoll )
{
    if(i >= needle_val_.count()) return repoll;

    if(value < min_) value = min_;
    if(value > max_) value = max_;
    if(needle_val_[i] != value)
    {
        needle_val_[i] = value;
        if(isClock && (needle_pos_[i] != max_ && needle_pos_[i] != min_) && (needle_val_[i] == min_ || needle_val_[i] == max_) ) {
            needle_pos_[i] -= max_;
            needle_val_[i] = min_;
        }
        needle_step_[i] = (needle_val_[i] - needle_pos_[i])/8;
        // minimum step is 0.5 degrees, otherwise rendering is very inefficient
        if(needle_step_[i]!=0)
            if(qAbs(needle_step_[i])<(max_-min_)/360/2)
                needle_step_[i] = (max_-min_)/360/2 * (qAbs(needle_step_[i]) / needle_step_[i]);
        repoll = true;
    }
    if(!polling_on_ && repoll && i==needleCount()-1)
        callback(lh_cb_polling, NULL);
    return repoll;
}

qreal LH_Dial::maxDegrees()
{
    switch(setup_type_->value())
    {
    case 0: //full circle
        return 360;
        break;
    case 1: //semi-circle
        return 180;
        break;
    case 2: //quarter circle
        return 90;
        break;
    }
    return 360;
}

qreal LH_Dial::startDegrees()
{
    switch(setup_orientation_->value())
    {
    case 0: //left / bottom left
        return 0;
        break;
    case 1: //top / top left
        return 90;
        break;
    case 2: //right / top right
        return 180;
        break;
    case 3: //bottom / bottom right
        return 270;
        break;
    }
    return 0;
}

void LH_Dial::setNeedleVisibility(bool visible, int index)
{
    if(index>=0 && index<needle_vis_.count())
        needle_vis_[index] = visible;
}

void LH_Dial::drawDial()
{
    if(!hasImage())
        return;

    QPainter painter;

    if( painter.begin( image() ) )
    {
        //Draw Face
        painter.drawImage(0,0, getFace() );

        qreal usedCapacity = 0;
        for( int i=0; i<needleCount(); ++i )
        {
            if(!needle_vis_[i]) continue;

            //Apply smoothing
            if(!setup_needles_smooth_->value())
                needle_pos_[i] = needle_val_[i];
            else
            {
                needle_pos_[i] += needle_step_[i];
                if( ((needle_step_[i]<0) && (needle_pos_[i]<needle_val_[i])) ||
                    ((needle_step_[i]>0) && (needle_pos_[i]>needle_val_[i])) )
                {
                    needle_pos_[i] = needle_val_[i];
                    needle_step_[i] = 0;
                }
            }
            usedCapacity += needle_pos_[i];
        }

        qreal totalAngle = 0;
        //Draw Needle(s)
        for( int i=0; i<needleCount(); ++i )
        {
            qreal pos = needle_pos_[i];
            if(dialType_ == DIALTYPE_PIE)
                if(setup_unused_style_->valueText()=="Hidden" && usedCapacity!=0)
                    pos *= max_val() / usedCapacity;

            if(max_ == min_) max_ ++;
            qreal angle = maxDegrees() * (pos-min_) / (max_-min_);
            int needleStyle;
            bool reverse = setup_needles_reverse_->value();
            if(dialType_ == DIALTYPE_DIAL)
            {
                QImage needleImage = getNeedle(i, angle, needleStyle);
                paintImage(painter, needleImage, (needleStyle==2? ROT_FACE : ROT_NEEDLE), startDegrees()+(reverse? maxDegrees() : 0), angle*(reverse? -1 : 1));
            } else {
                QImage sliceImage = getSlice(i, angle, totalAngle, needleStyle);
                paintImage(painter, sliceImage, ROT_NONE, startDegrees()+(reverse? maxDegrees() : 0));
            }
            totalAngle += angle*(reverse? -1 : 1);
        }
        if(dialType_ == DIALTYPE_PIE)
        {
            if(setup_unused_style_->valueText()=="Fill" || setup_unused_style_->valueText().contains("Image"))
            {
                int needleStyle;
                bool reverse = setup_needles_reverse_->value();

                qreal pos = max_val()-usedCapacity;
                qreal angle = maxDegrees() * (pos-min_) / (max_-min_);
                QImage sliceImage = getSlice(UNUSED_AREA, angle, totalAngle, needleStyle);
                paintImage(painter, sliceImage, ROT_NONE, startDegrees()+(reverse? maxDegrees() : 0));
            }
        }

        painter.end();
    }
}

int LH_Dial::polling()
{
    callback(lh_cb_render, NULL);
    if(setup_needles_smooth_->value())
        for( int i=0; i<needleCount(); i++ )
            if (qAbs(needle_pos_[i]-needle_val_[i])>qAbs(needle_step_[i])) {
                polling_on_ = true;
                return 50;
            }
    polling_on_ = false;
    return 0;
}

QImage *LH_Dial::render_qimage( int w, int h )
{
    if(QImage *img = initImage(w, h))
    {
        img->fill(PREMUL(setup_bgcolor_->value().rgba()));
        drawDial();
        return img;
    }
    return 0;
}

void LH_Dial::changeType()
{
    int selVal = setup_orientation_->value();
    if(selVal<0)selVal=0;

    setup_orientation_->setFlag(LH_FLAG_HIDDEN, setup_type_->value()==0 );
    setup_orientation_->list().clear();
    switch(setup_type_->value())
    {
    case 0: //full circle
        setup_orientation_->list().append("N/A");
        break;
    case 1: //semi-circle
        setup_orientation_->list().append("Left");
        setup_orientation_->list().append("Top");
        setup_orientation_->list().append("Right");
        setup_orientation_->list().append("Bottom");
        break;
    case 2: //full circle
        setup_orientation_->list().append("Bottom Left");
        setup_orientation_->list().append("Top Left");
        setup_orientation_->list().append("Top Right");
        setup_orientation_->list().append("Bottom Right");
        break;
    }
    setup_orientation_->refreshList();
    setup_orientation_->refreshList();
    setup_orientation_->refreshList();
    setup_orientation_->refreshList();

    if(selVal>=setup_orientation_->list().count()) selVal = setup_orientation_->list().count()-1;
    setup_orientation_->setValue(selVal);
}

void LH_Dial::changeFaceStyle()
{
    setup_face_pencolor_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()==0 || setup_face_style_->value()==3);
    setup_face_fillcolor1_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()!=2);
    setup_face_fillcolor2_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()!=2);
    setup_face_ticks_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()==0 || setup_face_style_->value()==3);
    setup_face_tickcolor_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()==0 || setup_face_style_->value()==3 || !setup_face_ticks_->value());
    setup_face_image_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()!=3);
}

void LH_Dial::changeTicks()
{
    setup_face_tickcolor_->setFlag(LH_FLAG_HIDDEN, setup_face_style_->value()==0 || setup_face_style_->value()==3 || !setup_face_ticks_->value());
}

void LH_Dial::changeNeedleStyle()
{
    if(dialType_==DIALTYPE_DIAL)
    {
        setup_needle_color_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_gradient_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_color2_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_thickness_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_length_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_gap_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_image_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()==0);
    }
    else
    {
        setup_needle_color_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_gradient_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_color2_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()!=0);
        setup_needle_thickness_->setFlag(LH_FLAG_HIDDEN, true);
        setup_needle_length_->setFlag(LH_FLAG_HIDDEN, false);
        setup_needle_gap_->setFlag(LH_FLAG_HIDDEN, true);
        setup_needle_image_->setFlag(LH_FLAG_HIDDEN, setup_needle_style_->value()==0);
    }
}

void LH_Dial::changeUnusedStyle()
{
    setup_unused_color_->setFlag(LH_FLAG_HIDDEN, setup_unused_style_->value()!=2);
    setup_unused_gradient_->setFlag(LH_FLAG_HIDDEN, setup_unused_style_->value()!=2);
    setup_unused_color2_->setFlag(LH_FLAG_HIDDEN, setup_unused_style_->value()!=2);
    setup_unused_length_->setFlag(LH_FLAG_HIDDEN, setup_unused_style_->value()<2);
    setup_unused_image_->setFlag(LH_FLAG_HIDDEN, setup_unused_style_->value()<3);
}

QString LH_Dial::buildNeedleConfig(QColor forceColor)
{
    int needleStyle = setup_needle_style_->value();
    QColor needleColor = ( forceColor == Qt::transparent? setup_needle_color_->value() : forceColor );
    bool needleGradient = ( forceColor == Qt::transparent? setup_needle_gradient_->value() : false);
    QColor needleColor2 = ( forceColor == Qt::transparent? setup_needle_color2_->value() : forceColor );

    int needleThick = setup_needle_thickness_->value();
    int needleLength = setup_needle_length_->value();
    int needleGap = setup_needle_gap_->value();
    QString needleImg = "";
    if(setup_needle_image_->value().isFile())
        needleImg = setup_needle_image_->value().absoluteFilePath();
    if (needleImg.startsWith(state()->dir_layout))
        needleImg.remove(state()->dir_layout);

    QStringList config = QStringList();

    config.append(QString::number(needleStyle));
    config.append(QString::number(needleColor.red()));
    config.append(QString::number(needleColor.green()));
    config.append(QString::number(needleColor.blue()));
    config.append(QString::number(needleColor.alpha()));
    config.append(QString::number(needleThick));
    config.append(QString::number(needleLength));
    config.append(QString::number(needleGap));
    config.append(needleImg.replace(",","?"));
    config.append(QString::number(needleGradient));
    config.append(QString::number(needleColor2.red()));
    config.append(QString::number(needleColor2.green()));
    config.append(QString::number(needleColor2.blue()));
    config.append(QString::number(needleColor2.alpha()));

    return config.join(",");
}

void LH_Dial::loadSliceConfig(int sliceID, int& sliceStyle, QColor& sliceColor, int& sliceLength, QString& sliceImage, int& sliceImageAlpha, bool &sliceGradient, QColor &sliceColor2)
{
    sliceImageAlpha = 255;
    if(sliceID == UNUSED_AREA)
    {
        sliceStyle = setup_unused_style_->value() - 2;
        sliceColor = setup_unused_color_->value();
        sliceLength = setup_unused_length_->value();
        sliceImage = setup_unused_image_->value().absoluteFilePath();
        sliceGradient = setup_unused_gradient_->value();
        sliceColor2 = setup_unused_color2_->value();
    } else {
        int unused_gap;
        int unused_thick;
        loadNeedleConfig(sliceID, sliceStyle, sliceColor, unused_thick, sliceLength, unused_gap, sliceImage, sliceGradient, sliceColor2);
    }
}

void LH_Dial::loadNeedleConfig(int needleID, int& needleStyle, QColor& needleColor, int& needleThick, int& needleLength, int& needleGap, QString& needleImage, bool& needleGradient, QColor& needleColor2)
{
    if (isDebug) qDebug() << "dial: load needle config: begin " << needleID;

    QStringList configs = setup_needle_configs_->value().split('~',QString::SkipEmptyParts);

    if( needleID < 0 ) needleID = 0;
    if( needleID >= configs.length() )
    {
        qDebug() << "LH_Dial: Error in needle data request: requested " << needleID+1 << ", max = " << configs.length();
        syncNeedleConfigs();
        configs = setup_needle_configs_->value().split('~',QString::SkipEmptyParts);
        qDebug() << "LH_Dial: Resynced needle data: new max = " << configs.length();

        if( needleID >= configs.length() ) needleID = configs.length();
    }

    QString configString = configs.at(needleID);
    QStringList config = configString.split(',');

    needleStyle = (config.at(0)).toInt();

    needleColor.setRed(QString(config.at(1)).toInt());
    needleColor.setGreen(QString(config.at(2)).toInt());
    needleColor.setBlue(QString(config.at(3)).toInt());
    needleColor.setAlpha(QString(config.at(4)).toInt());

    needleThick = (config.at(5)).toInt();
    needleLength = (config.at(6)).toInt();
    needleGap = (config.at(7)).toInt();
    if(config.length()>8)
        needleImage = QString(config.at(8)).replace('?',',');
    if(needleImage!="") needleImage = state()->dir_layout + needleImage;
    if(config.length()>9)
        needleGradient = QString(config.at(9)) == "1";
    if(config.length()>10)
    {
        needleColor2.setRed(QString(config.at(10)).toInt());
        needleColor2.setGreen(QString(config.at(11)).toInt());
        needleColor2.setBlue(QString(config.at(12)).toInt());
        needleColor2.setAlpha(QString(config.at(13)).toInt());
    }

    if (isDebug) qDebug() << "dial: load needle config: end ";
}

void LH_Dial::changeSelectedNeedle()
{
    int needleStyle;
    QColor needleColor = QColor();
    bool needleGradient;
    QColor needleColor2 = QColor();
    int needleThick;
    int needleLength;
    int needleGap;
    QString needleImage;

    loadNeedleConfig(setup_needle_selection_->value(), needleStyle, needleColor, needleThick, needleLength, needleGap, needleImage, needleGradient, needleColor2);

    setup_needle_length_->setMaximum(100 - needleGap);
    setup_needle_gap_->setMaximum(100 - needleLength);

    setup_needle_style_->setValue(needleStyle);
    setup_needle_color_->setValue(needleColor);
    setup_needle_thickness_->setValue(needleThick);
    setup_needle_length_->setValue(needleLength);
    setup_needle_gap_->setValue(needleGap);
    setup_needle_image_->setValue(QFileInfo(needleImage));
    setup_needle_gradient_->setValue(needleGradient);
    setup_needle_color2_->setValue(needleColor2);

    changeNeedleStyle();
}

void LH_Dial::updateSelectedNeedle()
{
    QStringList configs = setup_needle_configs_->value().split('~',QString::SkipEmptyParts);

    int needleID = setup_needle_selection_->value();
    if( needleID < 0 ) needleID = 0;
    if( needleID >= configs.length() ) needleID = configs.length()-1;

    configs.replace(needleID, buildNeedleConfig());

    setup_needle_length_->setMaximum(100 - setup_needle_gap_->value());
    setup_needle_gap_->setMaximum(100 - setup_needle_length_->value());

    setup_needle_configs_->setValue(configs.join("~"));
}
