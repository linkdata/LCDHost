/**
  \file     LH_TextTime.cpp
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

#include <QtGlobal>
#include <QTime>
#include <QFont>
#include <QFontMetrics>

#include "LH_Text.h"
#include "LH_Qt_QString.h"

class LH_TextTime : public LH_Text
{
protected:
    LH_Qt_QString *setup_format_;

public:
    LH_TextTime()
        : LH_Text()
        , setup_format_(0)
    {
    }

    const char *userInit()
    {
        if (const char* msg = LH_Text::userInit()) return msg;
        setup_format_ = new LH_Qt_QString(this,"<a href=\"http://doc.trolltech.com/4.6/qtime.html#toString\">Format</a>","HH:mm:ss");
        setup_text_->setName("Current time");
        setup_text_->setFlag(LH_FLAG_READONLY,true);
        return 0;
    }

    /**
      This being a time class, we want to update the image every second. Therefore we
      listen to the LH_NOTE_SECOND notification. Since the notification might fire a
      few milliseconds before or after the second switchover, we'll select the closest
      second to show. Finally, unless the text actually changed (setText() returns true)
      we don't need to re-render at all.
      */
    int notify(int n,void* p)
    {
        Q_UNUSED(p);

        if( !n || n&LH_NOTE_SECOND )
        {
            QTime now = QTime::currentTime();
            if( now.msec() > 500 ) now = now.addSecs(1);
            // if( !(now.second() % 10) ) for(;;) ; // to test plugins that hang
            if( setText( now.toString( setup_format_->value() ) ) )
                callback(lh_cb_render,NULL);
        }
        return LH_NOTE_SECOND | LH_Text::notify(n,p);
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Date and time",
            "SystemTimeText",
            "Time (Text)",
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
            classInfo.width = fm.width("00:00:00");
            // *( (int*) 0 ) = 0; // test plugin segmentation violations
        }
#endif

        return &classInfo;
    }
};

LH_PLUGIN_CLASS(LH_TextTime)
