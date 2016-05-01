/**
  \file     LH_TextCPULoad.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

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

#include <QFont>
#include <QFontMetrics>
#include <QTime>

#include "LH_Text.h"

class LH_TextCPULoad : public LH_Text
{
public:
    const char *userInit()
    {
        if (const char* msg = LH_Text::userInit()) return msg;
        setup_text_->setName( "Average CPU load" );
        setup_text_->setFlag( LH_FLAG_READONLY, true );
        setText( "?%" );
        return 0;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/CPU",
            "SystemCPUText",
            "Average Load (Text)",
            -1, -1,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };
#if 0
        if( classInfo.width == -1 )
        {
            QFont font;
            QFontMetrics fm( font );
            classInfo.height = fm.height();
            classInfo.width = fm.width("100%");
        }
#endif
        return &classInfo;
    }

    /**
      In this class, we set the text to render in notify() instead of prerender().
      We do this because rendering can be done a lot more often than we ask for it,
      and we just need to update the load text when the LH_NOTE_SECOND notification
      fires. We use LH_NOTE_SECOND because the average load value lh_systemstate->cpu_load
      updates once a second. We'll also do the work if the value for 'n' is zero, since
      that's the initial call to notify right after the instance has been created.
      */
    int notify(int n,void* p)
    {
        if( !n || n&LH_NOTE_SECOND )
        {
            if( setText( QString::number(state()->cpu_load/100)+"%" ) )
                callback(lh_cb_render,NULL); // only render if the text changed
        }
        return LH_Text::notify(n,p) | LH_NOTE_SECOND;
    }

};

LH_PLUGIN_CLASS(LH_TextCPULoad)
