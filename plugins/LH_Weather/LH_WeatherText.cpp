/**
  \file     LH_WeatherText.cpp
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

#include <QFont>
#include <QFontMetrics>
#include <QTime>
#include <QRegExp>
#include <QSharedMemory>

#include <stdio.h>

#include "LH_WeatherText.h"

LH_PLUGIN_CLASS(LH_WeatherText)

lh_class *LH_WeatherText::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Weather",
        "WeatherText",
        "Weather Text",
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

LH_WeatherText::LH_WeatherText()
    : LH_Text()
    , setup_value_type_(0)
    , setup_append_units_(0)
    , setup_pre_text_(0)
    , setup_post_text_(0)
    , setup_json_weather_(0)
{
    return;
}

const char *LH_WeatherText::userInit()
{
    if (const char* msg = LH_Text::userInit()) return msg;
    setup_text_->setFlag( LH_FLAG_READONLY, true );
    setup_text_->setFlag( LH_FLAG_NOSAVE_DATA, true );
    setText("...");

    valueTypes = QStringList();
    valueTypes.append("Location: City");
    valueTypes.append("Location: Region");
    valueTypes.append("Location: Country");
    valueTypes.append("Current Conditions: Description");
    valueTypes.append("Current Conditions: Image Code");
    valueTypes.append("Current Conditions: Temperature");
    valueTypes.append("Current Conditions: Last Updated");
    valueTypes.append("Wind: Chill");
    valueTypes.append("Wind: Direction");
    valueTypes.append("Wind: Speed");
    valueTypes.append("Atmosphere: Humidity");
    valueTypes.append("Atmosphere: Visibility");
    valueTypes.append("Atmosphere: Pressure");
    valueTypes.append("Atmosphere: Barometric Reading");
    valueTypes.append("Astronomy: Sunrise");
    valueTypes.append("Astronomy: Sunset");

    for(int i=1; i<=5; i++)
    {
        QString caption = QString("Forecast Day %1%2: %3").arg(i).arg(i==1? " (Today)" : (i==2? " (Tomorrow)" : ""));

        valueTypes.append(caption.arg("Day"));
        if(i==1)
            valueTypes.append(caption.arg("\"Today\"/\"Tonight\""));
        if(i==2)
            valueTypes.append(caption.arg("\"Tomorrow\""));
        valueTypes.append(caption.arg("Date"));
        valueTypes.append(caption.arg("Low"));
        valueTypes.append(caption.arg("High"));
        valueTypes.append(caption.arg("Description"));
        //valueTypes.append(caption.arg("Image Code"));
    }

    setup_value_type_ = new LH_Qt_QStringList(this, "Value Type", valueTypes, 0);
    setup_value_type_->setHelp( "<p>The type of value to display.</p>");
    setup_value_type_->setOrder(-4);
    connect( setup_value_type_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_append_units_ = new LH_Qt_bool(this, "Append Units", true, 0);
    setup_append_units_->setHelp( "<p>Append the units to the text.</p>");
    setup_append_units_->setOrder(-3);
    connect( setup_append_units_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_pre_text_ = new LH_Qt_QString( this, "Pre-Text", "", LH_FLAG_AUTORENDER);
    setup_pre_text_->setHelp( "<p>Text to be displayed before the sensor value.</p>");
    setup_pre_text_->setOrder(-3);
    connect( setup_pre_text_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_post_text_ = new LH_Qt_QString(this, "Post-Text", "", LH_FLAG_AUTORENDER);
    setup_post_text_->setHelp( "<p>Text to be displayed after the sensor value.</p>");
    setup_post_text_->setOrder(-3);
    connect( setup_post_text_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_json_weather_ = new LH_Qt_QString(this, "JSON Data", "", LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_HIDDEN);
    setup_json_weather_->setSubscribePath("/JSON_Weather_Data");
    setup_json_weather_->setMimeType("application/x-weather");
    connect( setup_json_weather_, SIGNAL(changed()), this, SLOT(updateText()) );

    return 0;
}

void LH_WeatherText::updateText()
{
    if( setText( getSelectedValueText()  ) )
        callback(lh_cb_render,NULL); // only render if the text changed
}

QString LH_WeatherText::getSelectedValueText()
{
    bool ok;
    weatherData weather_data(setup_json_weather_->value(), ok);


    //return QString("%1:\r\n%2%5\r\n%3 (%4)").arg(weather_data.location.city, weather_data.condition.temp, weather_data.condition.text, weather_data.condition.code, weather_data.units.temperature);
    QString selValue = valueTypes.at( setup_value_type_->value() );
    QString valueText;

    const int unit_none = 0;
    const int unit_dist = 1;
    const int unit_press = 2;
    const int unit_speed = 3;
    const int unit_temp = 4;
    const int unit_perc = 5;
    int units;

    valueText = "<not implimented>";

    if(selValue == "Location: City")                    {valueText = weather_data.location.city; units = unit_none;}
    if(selValue == "Location: Region")                  {valueText = weather_data.location.region; units = unit_none;}
    if(selValue == "Location: Country")                 {valueText = weather_data.location.country; units = unit_none;}

    if(selValue == "Current Conditions: Description")   {valueText = weather_data.condition.text; units = unit_none;}
    if(selValue == "Current Conditions: Image Code")    {valueText = weather_data.condition.code; units = unit_none;}
    if(selValue == "Current Conditions: Temperature")   {valueText = weather_data.condition.temp; units = unit_temp;}
    if(selValue == "Current Conditions: Last Updated")  {valueText = weather_data.condition.date; units = unit_none;}

    if(selValue == "Wind: Chill")                       {valueText = weather_data.wind.chill; units = unit_temp;}
    if(selValue == "Wind: Direction")                   {valueText = weather_data.wind.direction; units = unit_none;}
    if(selValue == "Wind: Speed")                       {valueText = weather_data.wind.speed; units = unit_speed;}

    if(selValue == "Atmosphere: Humidity")              {valueText = weather_data.atmosphere.humidity; units = unit_perc;}
    if(selValue == "Atmosphere: Visibility")            {valueText = weather_data.atmosphere.visibility; units = unit_dist;}
    if(selValue == "Atmosphere: Pressure")              {valueText = weather_data.atmosphere.pressure; units = unit_press;}
    if(selValue == "Atmosphere: Barometric Reading")    {valueText = weather_data.atmosphere.barometricReading; units = unit_none;}

    if(selValue == "Astronomy: Sunrise")                {valueText = weather_data.astronomy.sunrise; units = unit_none;}
    if(selValue == "Astronomy: Sunset")                 {valueText = weather_data.astronomy.sunset; units = unit_none;}

    QRegExp rx("Forecast Day ([1-5])[^:]*: (.*)");
    if(rx.indexIn(selValue) != -1)
    {
        int i = rx.cap(1).toInt()-1;
        units = unit_none;
        if(rx.cap(2) == "Day")
            valueText = weather_data.forecast[i].day;
        if(rx.cap(2) == "\"Today\"/\"Tonight\"" || rx.cap(2) == "\"Tomorrow\"")
            valueText = weather_data.forecast[i].relativeDay;
        if(rx.cap(2) == "Date")
            valueText = weather_data.forecast[i].date;
        if(rx.cap(2) == "Low") {
            valueText = weather_data.forecast[i].low;
            units = unit_temp;
        }
        if(rx.cap(2) == "High") {
            valueText = weather_data.forecast[i].high;
            units = unit_temp;
        }
        if(rx.cap(2) == "Description")
            valueText = weather_data.forecast[i].text;
        if(rx.cap(2) == "Image Code")
            valueText = weather_data.forecast[i].code;
    }

    QString resultText = QString(valueText);
    if (resultText=="") {
        resultText = " ";
    } else
        if (setup_append_units_->value())
            switch(units)
            {
            case unit_dist:
                resultText = resultText + QString(weather_data.units.distance);
                break;
            case unit_press:
                resultText = resultText + QString(weather_data.units.pressure);
                break;
            case unit_speed:
                resultText = resultText + QString(weather_data.units.speed);
                break;
            case unit_temp:
                resultText = resultText + QString(weather_data.units.temperature);
                break;
            case unit_perc:
                resultText = resultText + "%";
                break;
            }
    resultText = setup_pre_text_->value() + resultText + setup_post_text_->value();
    return resultText;
}
