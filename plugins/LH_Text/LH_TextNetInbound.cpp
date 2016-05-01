/**
  \file     LH_TextNetInbound.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2014 Johan Lindh

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

#include <QDebug>
#include "LH_TextNumber.h"

class LH_TextNetInbound : public LH_TextNumber
{
public:
    const char *userInit()
    {
        if (const char* msg = LH_TextNumber::userInit()) return msg;
        setup_bits_->setFlag( LH_FLAG_HIDDEN, false );
        return 0;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Network/Inbound",
            "SystemNetworkInboundText",
            "Inbound Bandwidth Usage (Text)",
            20,10,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };
        return &classInfo;
    }

    void prerender()
    {
        LH_TextNumber::prerender();
        if( showUnits() )
        {
            if( setup_bits_->value() ) setText( text().append( "bit/s" ) );
            else setText( text().append( "B/s" ) );
        }
    }

    int notify(int code, void *param)
    {
        if( !code || code&LH_NOTE_NET )
        {
            bool needrender = setValue( state()->net_cur_in ) | setMax( state()->net_max_in ); // bitwise OR - we need both to execute!
            if( needrender ) callback(lh_cb_render,NULL);
        }
        return LH_TextNumber::notify(code,param) | LH_NOTE_SECOND;
    }

};

LH_PLUGIN_CLASS(LH_TextNetInbound)
