/**
  \file     LH_CursorImage.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andy Bridges
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#include "LH_CursorImage.h"

LH_PLUGIN_CLASS(LH_CursorImage)

lh_class *LH_CursorImage::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Cursor",
        "CursorImage",
        "Cursor Image",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

const char *LH_CursorImage::userInit()
{
    if( const char *err = LH_QImage::userInit() ) return err;
    rcvr_ = new LH_CursorReceiver(this);

    new LH_Qt_QString(this, "hr1", "<hr>", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_FIRST | LH_FLAG_HIDETITLE, lh_type_string_html );

    add_cf_target(setup_image_file_);
    add_cf_source(rcvr_->setup_cursor_state_);


    cf_set_rules("<rules>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>OFF</value></condition></conditions>"
                 "<actions><action type='property'><target>Image</target><value>cursor_off.png</value></action></actions>"
                 "</rule>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>OFF_SEL</value></condition></conditions>"
                 "<actions><action type='property'><target>Image</target><value>cursor_off_sel.png</value></action></actions>"
                 "</rule>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>ON</value></condition></conditions>"
                 "<actions><action type='property'><target>Image</target><value>cursor_on.png</value></action></actions>"
                 "</rule>"
                 "<rule>"
                 "<conditions><condition test='Equals'><source>Cursor State</source><value id='1'>ON_SEL</value></condition></conditions>"
                 "<actions><action type='property'><target>Image</target><value>cursor_on_sel.png</value></action></actions>"
                 "</rule>"
                 "</rules>");

    return 0;
}

