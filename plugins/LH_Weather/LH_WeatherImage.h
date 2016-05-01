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

#ifndef LH_WEATHERIMAGE_H
#define LH_WEATHERIMAGE_H

#include <QtGlobal>

#include "LH_QtInstance.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_QTextEdit.h"

#include "LH_WeatherData.h"

struct imageMapData
{
    QString code;
    QString dayImage;
    QString nightImage;
};

class LH_WeatherImage : public LH_QtInstance
{
    Q_OBJECT

    QHash<int, QStringList> imageDefinitions;
    QString weatherCode;
    bool isNight;

protected:
    LH_Qt_QStringList *setup_value_type_;
    LH_Qt_QFileInfo *setup_file_;
    LH_Qt_QString *setup_text_;
    LH_Qt_QTextEdit *setup_usage_;
    LH_Qt_QString *setup_json_weather_;

public:
    LH_WeatherImage(LH_QtObject *parent = 0);

    QImage *render_qimage( int w, int h );

    static lh_class *classInfo();

    QString getWeatherImageName();

public slots:
    void fileChanged();
    void updateImage(bool rerender = false);
};

#endif // LH_WEATHERIMAGE_H
