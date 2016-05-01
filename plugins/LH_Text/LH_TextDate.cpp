/**
  \file     LH_TextDate.cpp
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

#include <QtGlobal>
#include <QDate>
#include <QFont>
#include <QFontMetrics>

#include "LH_Text.h"

class LH_TextDate : public LH_Text
{
protected:
    LH_Qt_QString *setup_format_;

public:
    LH_TextDate()
        : LH_Text()
        , setup_format_(0)
    {
        return;
    }

    const char *userInit()
    {
        if (const char* msg = LH_Text::userInit()) return msg;
        setup_format_ = new LH_Qt_QString(this,"<a href=\"http://doc.trolltech.com/4.6/qdate.html#toString\">Format</a>","yyyy-MM-dd dddd");
        setup_format_->setHelp("<p>Format to use. Common formats include:</p>"
                               "<ul>"
                               "<li>yyyy-MM-dd dddd</li>"
                               "<li>MM/dd/yyyy</li>"
                               "<li>ddd MMM d yyyy</li>"
                               "</ul>");
        setup_text_->setName("Current date");
        setup_text_->setFlag(LH_FLAG_READONLY,true);
        return 0;
    }

    /**
      We'll use the notify LH_NOTE_SECOND to make sure we're up-to-date,
      but we might also have elected to use polling() with a long interval:

        \code
        QTime::currentTime().msecsTo( QTime( 23, 59, 59, 999 ) ) + 10
        \endcode

      In any case, we only redo the rendering if the text actually changed.
      */
    int notify(int n,void* p)
    {
        Q_UNUSED(p);
        if( !n || n&LH_NOTE_SECOND )
        {
            QDateTime now = QDateTime::currentDateTime();
            if( now.time().msec() > 500 ) now = now.addSecs(1);
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
            "SystemDateText",
            "Date (Text)",
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
            classInfo.width = fm.width("9999-99-99 weekday");
        }
#endif
        return &classInfo;
    }
};

LH_PLUGIN_CLASS(LH_TextDate)
