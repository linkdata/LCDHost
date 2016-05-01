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

#ifndef LH_TS3COMBINEDSTATUSIMAGE_H
#define LH_TS3COMBINEDSTATUSIMAGE_H

#include <LH_Image/LH_QImage.h>
#include <LH_Qt_QString.h>
#include <LH_Qt_QStringList.h>
#include <QDebug>

class LH_TS3CombinedStatusImage : public LH_QImage
{
    Q_OBJECT

protected:
    LH_Qt_QStringList *setup_connection_status_;
    LH_Qt_QString *setup_talking_;
    LH_Qt_bool *setup_talking_me_;
    LH_Qt_QString *setup_nickname_;
    LH_Qt_QStringList *setup_combined_status_;
    LH_Qt_QStringList *setup_mute_status_spk_;
    LH_Qt_QStringList *setup_mute_status_mic_;

    bool loadPlaceholderImage(QImage *img);

public:
    const char *userInit();
    static lh_class *classInfo();

public slots:
    void updateCombinedStatus();
};

#endif // LH_TS3COMBINEDSTATUSIMAGE_H
