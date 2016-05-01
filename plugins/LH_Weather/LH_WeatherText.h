/**
  \file     LH_WeatherConnector.cpp
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

#ifndef LH_WEATHERTEXT_H
#define LH_WEATHERTEXT_H

#include "../LH_Text/LH_Text.h"

#include "LH_WeatherData.h"

class LH_WeatherText : public LH_Text
{
    Q_OBJECT
    QStringList valueTypes;

protected:
    LH_Qt_QStringList *setup_value_type_;
    LH_Qt_bool *setup_append_units_;
    LH_Qt_QString *setup_pre_text_;
    LH_Qt_QString *setup_post_text_;
    LH_Qt_QString *setup_json_weather_;

public:
    LH_WeatherText();
    const char* userInit();

    QString getSelectedValueText();

    static lh_class *classInfo();

public slots:
    void updateText();
};

#endif // LH_WEATHERTEXT_H
