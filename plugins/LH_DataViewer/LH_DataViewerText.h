/**
  \file     LH_DataViewerConnector.cpp
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

#ifndef LH_DATAVIEWERTEXT_H
#define LH_DATAVIEWERTEXT_H

#include "LH_Text/LH_Text.h"
#include "LH_Qt_int.h"

#include "LH_DataViewerData.h"
#include <QTime>

class LH_DataViewerText : public LH_Text
{
    Q_OBJECT
    LH_DataViewerData data_;
    QTime updateTimer_;
    QTime scrollTimer_;
    int scroll_poll_;
protected:
    LH_Qt_QString *setup_lookup_code_;
    LH_Qt_QString *setup_item_name_;

public:
    LH_DataViewerText()
        : LH_Text()
        , data_(this)
        , scroll_poll_(0)
        , setup_lookup_code_(0)
        , setup_item_name_(0)
    {}
    const char *userInit();
    int polling();

    static lh_class *classInfo();

public slots:
    void updateText();
};

#endif // LH_DATAVIEWERTEXT_H
