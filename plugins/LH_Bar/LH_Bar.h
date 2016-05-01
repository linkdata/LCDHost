/**
  \file     LH_Bar.h
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

#ifndef LH_BAR_H
#define LH_BAR_H

#include "LH_QtPlugin.h"
#include "LH_QtCFInstance.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_int.h"

#ifdef LH_BAR_LIBRARY
# define LH_BAR_EXPORT Q_DECL_EXPORT
#else
# define LH_BAR_EXPORT Q_DECL_IMPORT
#endif

class LH_BAR_EXPORT LH_Bar : public LH_QtCFInstance
{
    Q_OBJECT

    qreal min_;
    qreal max_;
    QImage bar_img_;
    QImage bar_img_bg_;
    QImage bar_img_endMask_;
    QImage bar_img_emptyMask_;

    void draw_bar( qreal value, int pos = 0, int total = 1 );
    qreal boundedValue(qreal value);

protected:
    LH_Qt_QStringList *setup_type_;
    LH_Qt_QFileInfo *setup_file_;
    LH_Qt_QFileInfo *setup_file_bg_;
    LH_Qt_QStringList *setup_masking_;
    LH_Qt_QFileInfo *setup_file_endMask_;
    LH_Qt_QColor *setup_pencolor1_;
    LH_Qt_QColor *setup_pencolor2_;
    LH_Qt_QColor *setup_bgcolor_;
    LH_Qt_QStringList *setup_direction_;
    LH_Qt_QSlider *setup_spacing_;
    LH_Qt_bool *setup_discrete_;
    LH_Qt_int *setup_discrete_count_;

public:
    LH_Bar(LH_QtObject *parent = 0);

    QImage *render_qimage( int w, int h );

    qreal min_val() const { return min_; }
    qreal max_val() const { return max_; }

    bool setMin( qreal r ); // return true if rendering needed
    bool setMax( qreal r ); // return true if rendering needed

    void drawSingle( qreal value )
    {
        cf_source_notify("Value", QString::number(boundedValue(value)));
        draw_bar(value);
    }
    void drawList( qreal *values, int total )
    {
        for( int i=0; i<total; ++i )
            cf_source_notify("Value", QString::number(boundedValue(values[i])), i, total);
        for( int i=0; i<total; ++i )
            draw_bar( values[i], i, total );
    }
    void drawList( const QVector<qreal> &values )
    {
        for( int i=0; i<values.size(); ++i )
            cf_source_notify("Value", QString::number(boundedValue(values.at(i))), i, values.size());
        for( int i=0; i<values.size(); ++i )
            draw_bar( values.at(i), i, values.size() );
    }


public slots:
    void changeType();
    void changeDiscrete();
    void changeFile();

};

#endif // LH_BAR_H
