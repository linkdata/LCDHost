/**
  \file     LH_QtPlugin.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#include "LH_QtPlugin.h"
#include "LH_QtInstance.h"

const lh_class **LH_QtPlugin::class_list()
{
    int count = 0;

    if(first_p_ == 0)
        return 0;

    // count and check classes
    for( LH_QtClassLoader *load=*first_p_; load; load=load->next_)
    {
        Q_ASSERT( load->info_ != NULL );
        lh_class *p = load->info_();
        if( p )
        {
            LH_QtObject::build_object_calltable( &p->objtable );
            LH_QtInstance::build_instance_calltable( &p->table, load->factory_ );
            ++ count;
        }
    }

    // free the old list, if any
    if( classlist_ )
    {
        free( classlist_ );
        classlist_ = 0;
    }

    // allocate list and fill it
    if( count > 0 )
    {
        int n = 0;
        classlist_ = (const lh_class**) malloc( sizeof(lh_class*) * (count+1) );
        for( LH_QtClassLoader *load=*first_p_; load; load=load->next_)
        {
            if( n<count && load->info_() )
                classlist_[n++] = load->info_();
        }
        Q_ASSERT( n==count );
        classlist_[n] = NULL;
    }

    return classlist_;
}

LH_QtPlugin::LH_QtPlugin() :
    LH_QtObject(0),
    first_p_(0),
    classlist_(0)
{
    LH_QtObject::build_object_calltable( & objtable_ );
}

LH_QtPlugin::~LH_QtPlugin()
{
    if( classlist_ )
    {
        free( classlist_ );
        classlist_ = 0;
    }
}

int LH_QtPlugin::notify( int code, void *param )
{
    // if( code & LH_NOTE_SECOND )
    //     qDebug("LH_QtPlugin::notify(): %s QObject count: %d", qPrintable(objectName()), (findChildren<QObject *>()).size());
    return LH_QtObject::notify(code, param);
}
