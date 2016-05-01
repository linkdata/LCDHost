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

#ifndef LH_DATAVIEWERDIAL_H
#define LH_DATAVIEWERDIAL_H

#include "../LH_Dial/LH_Dial.h"

#include "LH_QtPlugin.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QString.h"

#include "LH_DataViewerData.h"

class LH_DataViewerDial : public LH_Dial
{
    Q_OBJECT
    LH_DataViewerData data_;

    qreal barValue_;
    qreal barMin_;
    qreal barMax_;

    qreal toReal(QString str, qreal defaultVal = 0);

protected:
    LH_Qt_QString *setup_lookup_code_;
    LH_Qt_QString *setup_item_name_;
    LH_Qt_QString *setup_value_min_;
    LH_Qt_QString *setup_item_name_min_;
    LH_Qt_QString *setup_value_max_;
    LH_Qt_QString *setup_item_name_max_;

public:
    LH_DataViewerDial();
    QImage *render_qimage( int w, int h );

    int polling();
    static lh_class *classInfo();

public slots:
    void updateDial(bool rerender = false);
};

#endif // LH_DATAVIEWERDIAL_H
