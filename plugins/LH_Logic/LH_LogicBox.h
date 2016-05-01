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

#ifndef LH_LOGICBOX_H
#define LH_LOGICBOX_H

#include <LH_QtCFInstance.h>
#include <QTimer>

class LH_LogicBox : public LH_QtCFInstance
{
    Q_OBJECT

protected:
    LH_Qt_QString *setup_input1_;
    LH_Qt_QString *setup_input2_;
    LH_Qt_QString *setup_output_;

public:
    LH_LogicBox(LH_QtObject* parent = 0);

    const char *userInit();

    static lh_class *classInfo();
    QImage *render_qimage( int w, int h );

public slots:
    void refreshOutput();
};

#endif // LH_LOGICBOX_H
