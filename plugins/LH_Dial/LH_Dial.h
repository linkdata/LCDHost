/**
  \file     LH_Dial.h
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

#ifndef LH_DIAL_H
#define LH_DIAL_H

#include "LH_QtInstance.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QTextEdit.h"

#include <QList>
#include <QStringList>
#include <QHash>
#include <QString>

#ifdef LH_DIAL_LIBRARY
# define LH_DIAL_EXPORT Q_DECL_EXPORT
#else
# define LH_DIAL_EXPORT Q_DECL_IMPORT
#endif


class tickObject
{
public:
    tickObject(int _count, int _width, qreal _length, qreal _gap)//, QColor _color = Qt::black)
    {
        //color = _color;
        width = _width;
        count = _count;
        length = _length;
        gap = _gap;
    }

    int count;
    int width;
    //QColor color;
    qreal gap;
    qreal length;
};

struct tickSet
{
    QList<tickObject> fullCircle;
    QList<tickObject> semiCircle;
    QList<tickObject> quarterCircle;
};

enum DialType
{
    DIALTYPE_DIAL,
    DIALTYPE_PIE
};

enum RotationType
{
    ROT_NONE,
    ROT_FACE,
    ROT_NEEDLE,
    ROT_CENTER
};

class LH_DIAL_EXPORT LH_Dial : public LH_QtInstance
{
    Q_OBJECT

    DialType dialType_;

    qreal min_;
    qreal max_;

    QString faceCode_;
    QString unusedCapacityStyleLock_;
    QImage *faceImage_;
    //QSize img_size_;
    QHash<QString,QImage> fgImgs_;

    void getRadii(qreal& radH, qreal& radW);
    qreal getRadians(qreal degrees, qreal& offsetRadians);
    qreal getRadians(qreal degrees) {qreal offsetRadians; return getRadians(degrees, offsetRadians);}
    qreal getDrawLen(qreal boxHeight, qreal boxWidth, qreal radians);
    void getDimensions(qreal degrees, int& h, int& w, qreal& radH, qreal& radW, qreal& radians, qreal& drawLen);
    QString generateNeedleCode(qreal drawLen, QColor needleColor, int needleThick, int needleLength, int needleGap, int h, int w, QString needleImagePath, int needleStyle, bool sliceGradient, QColor sliceColor2);

    QImage getFace();
    QImage getNeedle(int needleID, qreal degrees, int& needleStyle);
    QImage getSlice(int needleID, qreal degrees, qreal offsetAngle, int& needleStyle);
    void getCenter(QPointF& center);
    void getCenter(qreal& centerX, qreal& centerY) { QPointF center; getCenter(center); centerX = center.x(); centerY = center.y(); }

    void getRotationData(qreal startAngle, qreal angle, qreal& centerX, qreal& centerY, qreal& radH, qreal& radW, qreal& radians);
    void paintLine(QPainter& painter, QPen& pen, qreal startAngle, qreal angle, qreal relLength, qreal gap = 0);
    void paintImage(QPainter& painter, QImage needleImage, RotationType rotationType, qreal startAngle, qreal angle = 0);
    QString colString(QColor col);

    QList<qreal> needle_pos_;
    QList<qreal> needle_val_;
    QList<qreal> needle_step_;
    QList<QString> needleCode_;
    QString unusedAreaCode_;
    QImage* unusedImage_;
    QList<QImage*> needleImage_;
    QList<bool> needle_vis_;

    void loadNeedleConfig(int lineID, int& needleStyle, QColor& needleColor, int& needleThick, int& needleLength, int& needleGap, QString& needleImage, bool& needleGradient, QColor& needleColor2);
    void loadSliceConfig(int sliceID, int& sliceStyle, QColor& sliceColor, int& sliceLength, QString& sliceImage, int& sliceImageAlpha, bool &sliceGradient, QColor &sliceColor2);

    static const bool isDebug = false;

    QString buildNeedleConfig(QColor forceColor = Qt::transparent);

    bool polling_on_;

    qreal maxDegrees();
    qreal startDegrees();

protected:
    bool isClock;

    LH_Qt_QStringList *setup_type_;
    LH_Qt_QStringList *setup_orientation_;
    LH_Qt_bool *setup_needles_reverse_;
    LH_Qt_bool *setup_needles_smooth_;


    LH_Qt_QColor *setup_bgcolor_;

    LH_Qt_QStringList *setup_face_style_;
    LH_Qt_QColor *setup_face_pencolor_;
    LH_Qt_QColor *setup_face_fillcolor1_;
    LH_Qt_QColor *setup_face_fillcolor2_;
    LH_Qt_QFileInfo* setup_face_image_;
    LH_Qt_bool *setup_face_ticks_;
    LH_Qt_QColor *setup_face_tickcolor_;

    LH_Qt_QStringList *setup_needle_selection_;

    LH_Qt_QStringList *setup_needle_style_;
    LH_Qt_QColor *setup_needle_color_;
    LH_Qt_bool *setup_needle_gradient_;
    LH_Qt_QColor *setup_needle_color2_;
    LH_Qt_int *setup_needle_thickness_;
    LH_Qt_int *setup_needle_length_;
    LH_Qt_int *setup_needle_gap_;
    LH_Qt_QFileInfo* setup_needle_image_;

    LH_Qt_QColor* setup_unused_color_;
    LH_Qt_bool* setup_unused_gradient_;
    LH_Qt_QColor* setup_unused_color2_;
    LH_Qt_int* setup_unused_length_;
    LH_Qt_QFileInfo* setup_unused_image_;

    LH_Qt_QTextEdit *setup_needle_configs_;
    LH_Qt_QStringList *setup_unused_style_;

    void setNeedleVisibility(bool visible, int index = 0);
    void syncNeedleConfigs(QColor defaultColor = Qt::transparent);

public:    
    LH_Dial(DialType dialType = DIALTYPE_DIAL, QString unusedCapacityStyle = "");
    ~LH_Dial();

    virtual const char *userInit();

    int polling();
    QImage *render_qimage( int w, int h );

    qreal min_val() const { return min_; }
    qreal max_val() const { return max_; }

    bool setMin( qreal r ); // return true if rendering needed
    bool setMax( qreal r ); // return true if rendering needed
    bool setVal(qreal value, int index = 0, bool repoll = false);
    void setVal( qreal *values, int total ) {
        bool repoll = false;
        for( int i=0; i<total; ++i )
            repoll = setVal( values[i], i, repoll );
    }
    void setVal( const QVector<qreal> &values ) {
        bool repoll = false;
        for( int i=0; i<values.size(); ++i )
            repoll = setVal( values.at(i), i, repoll );
    }

    void addNeedle(QString name, QColor defaultColor = Qt::transparent);
    int needleCount();
    void clearNeedles();
    void setNeedles(QStringList names) {
        bool matchingList = names.length() == setup_needle_selection_->list().length();
        if(matchingList)
            for(int i = 0; i<setup_needle_selection_->list().length(); i++)
            {
                matchingList = matchingList && (names[i]==setup_needle_selection_->list().at(i));
                if(!matchingList) break;
            }
        if(!matchingList)
        {
            clearNeedles();
            for(int i=0; i<names.length(); i++) addNeedle(names[i]);
        }
    }

    void drawDial();

    tickSet ticks;

public slots:
    void changeType();
    void changeFaceStyle();
    void changeTicks();
    void changeNeedleStyle();
    void changeUnusedStyle();
    void changeSelectedNeedle();
    void updateSelectedNeedle();

};

#endif // LH_DIAL_H
