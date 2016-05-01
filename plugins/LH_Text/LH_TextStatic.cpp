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


#include <QImage>
#include <QFont>
#include <QFontMetrics>

#include "LH_Text.h"

class LH_TextStatic : public LH_Text
{
public:
    static lh_class *classInfo()
    {
        static lh_class classinfo =
        {
            sizeof(lh_class),
            "Static",
            "StaticText",
            "Text",
            -1, -1,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };
/*
        if( classinfo.width == -1 )
        {
            QImage tmpimage = makeImage();
            QFont font("Arial",10);
            QFontMetrics fm( font, &tmpimage );
            classinfo.height = fm.height();
            classinfo.width = fm.width("LCDHost");
        }
        */

        return &classinfo;
    }
};

LH_PLUGIN_CLASS(LH_TextStatic)
