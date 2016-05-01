/**
  \file     LH_QtPlugin_Weather.h
  @author   Andy "Triscopic" Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andrew Bridges

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

#ifndef LH_QTPLUGIN_WEATHER_H
#define LH_QTPLUGIN_WEATHER_H

#include "LH_QtPlugin.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QUrl>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QSharedMemory>

#include "LH_Qt_QString.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QTextEdit.h"
#include "LH_Qt_bool.h"

#include <stdio.h>

#include "LH_WeatherData.h"
#include "SimpleTranslator.h"

class LH_QtPlugin_Weather : public LH_QtPlugin
{
    Q_OBJECT

    typedef void (LH_QtPlugin_Weather::*xmlParserFunc)();

    weatherData weather_data;

    QDateTime lastrefresh_;
    static const bool get5Day = true;

    static const bool debugHTTP = false;
    static const bool debugMemory = false;
    static const bool debugForecast = false;
    static const bool debugSaveXML = false;

    QNetworkReply* connectionId_WOEID;
    QNetworkReply* connectionId_2Day;
    QNetworkReply* connectionId_5Day;

    QNetworkAccessManager nam2Day;
    QNetworkAccessManager nam5Day;
    QNetworkAccessManager namWOEID;
    QNetworkReply* fetchWeather(bool is5Day, QXmlStreamReader& xml_, QNetworkAccessManager& nam, QNetworkReply* currentReply);

    QXmlStreamReader xml2Day_;
    QXmlStreamReader xml5Day_;
    QXmlStreamReader xmlWOEID_;

    void parseXmlWeather(bool is5Day, QXmlStreamReader& xml_);
    void parseXml2Day();
    void parseXml5Day();
    void parseXmlWOEID();

    QDate toDate(QString str, bool isLong);

    QString getWeatherValue(QXmlStreamReader& xml_, QString attrName);
    QString getWeatherValue(QXmlStreamReader& xml_, QString attrName, QString preText);

    void setNoForecast(forecastData &forecast);
    void setForecast(QXmlStreamReader &xml_, forecastData& forecast, int relativeDay);

    void requestTranslation();
    QString fullDateName(QString shortName);

    void processResponse(QByteArray xmlData, QString name, QXmlStreamReader& xmlReader, xmlParserFunc xmlParser);

protected:
    LH_Qt_QString *setup_location_name_;
    LH_Qt_QString *setup_yahoo_woeid_;
    LH_Qt_QString *setup_longlat_;
    LH_Qt_QString *setup_yahoo_5dayid_;
    LH_Qt_QString *setup_city_;
    LH_Qt_QSlider *setup_delay_;
    LH_Qt_QStringList *setup_method_;
    LH_Qt_int *setup_refresh_;
    LH_Qt_QStringList *setup_units_type_;

    LH_Qt_QStringList *setup_languages_;
    LH_Qt_QString *setup_language_;
    LH_Qt_bool *setup_show_all_languages_;

    LH_Qt_QString *setup_json_weather_;

    SimpleTranslator translator;
public:
    LH_QtPlugin_Weather();

    const char *userInit();
    int notify(int code,void* param);

    bool checkNight();
    int toTime(QString time, bool isDateTime);

public slots:
    void fetch2Day();
    void fetch2DayU();
    void fetch5Day();
    void fetchWOEID();

    void finished2Day(QNetworkReply*);
    void finished5Day(QNetworkReply*);
    void finishedWOEID(QNetworkReply*);

    //void openBrowser(QString,int,int);
    void saveXMLResponse(QByteArray,QString);
    void updateLanguagesList();
    void selectLanguage();
    void setLanguage();

};

#endif // LH_QTPLUGIN_WEATHER_H
