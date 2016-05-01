/**
  \file     LH_Background.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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

#ifndef LH_BACKGROUND_H
#define LH_BACKGROUND_H

#include <QColor>
#include "LH_QtPlugin.h"
#include "LH_QtInstance.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_QColor.h"
#include "LH_Decor.h"

class LH_DECOR_EXPORT LH_Background : public LH_QtInstance
{
    Q_OBJECT

protected:
    LH_Qt_QColor *setup_startcolor_;
    LH_Qt_QColor *setup_stopcolor_;
    LH_Qt_bool *setup_gradient_;
    LH_Qt_bool *setup_horizontal_;

public:
    LH_Background();

    const char *userInit();
    QImage *render_qimage( int w, int h );

    static lh_class *classInfo();

public slots:
    void enableGradient(bool);
};

#endif // LH_BACKGROUND_H
