/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

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

#include "LH_WeatherData.h"

weatherData::weatherData() :
    isNight(false),
    forecastDays(5)
{}

weatherData::weatherData(QString jsonData, bool& ok):
  isNight(false),
  forecastDays(5)
{
    QVariantMap jobject = Json::parse(jsonData, ok).toMap();
    if(ok)
    {
        QVariantMap locationMap = jobject["location"].toMap();
        QVariantMap unitsMap = jobject["units"].toMap();
        QVariantMap windMap = jobject["wind"].toMap();
        QVariantMap atmosphereMap = jobject["atmosphere"].toMap();
        QVariantMap astronomyMap = jobject["astronomy"].toMap();
        QVariantMap conditionMap = jobject["condition"].toMap();

        location.city =locationMap["city"].toString();
        location.region = locationMap["region"].toString();
        location.country = locationMap["country"].toString();

        units.temperature = QLatin1Char('\260') + unitsMap["temperature"].toString();
        units.distance = unitsMap["distance"].toString();
        units.pressure = unitsMap["pressure"].toString();
        units.speed = unitsMap["speed"].toString();

        wind.chill = windMap["chill"].toString();
        wind.direction = windMap["direction"].toString();
        wind.speed = windMap["speed"].toString();

        atmosphere.humidity = atmosphereMap["humidity"].toString();
        atmosphere.visibility = atmosphereMap["visibility"].toString();
        atmosphere.pressure = atmosphereMap["pressure"].toString();
        atmosphere.barometricReading = atmosphereMap["barometricReading"].toString();

        astronomy.sunrise = astronomyMap["sunrise"].toString();
        astronomy.sunset = astronomyMap["sunset"].toString();

        condition.text = conditionMap["text"].toString();
        condition.code = conditionMap["code"].toString();
        condition.temp = conditionMap["temp"].toString();
        condition.date = conditionMap["date"].toString();

        for(int i=0; i<5; i++)
        {
            QVariantMap forecastMap = jobject[QString("forecast_%1").arg(i)].toMap();
            forecast[i].day = forecastMap["day"].toString();
            forecast[i].relativeDay = forecastMap["relativeDay"].toString();
            forecast[i].date = forecastMap["date"].toString();
            forecast[i].low = forecastMap["low"].toString();
            forecast[i].high = forecastMap["high"].toString();
            forecast[i].text = forecastMap["text"].toString();
            forecast[i].code = forecastMap["code"].toString();
        }

        isNight = jobject["isNight"].toBool();

    }
}

QString weatherData::serialize()
{
    QVariantMap jobject;
    QVariantMap locationMap;
    QVariantMap unitsMap;
    QVariantMap windMap;
    QVariantMap atmosphereMap;
    QVariantMap astronomyMap;
    QVariantMap conditionMap;
    QVariantMap forecastMap[5];

    locationMap.insert("city",location.city);
    locationMap.insert("region",location.region);
    locationMap.insert("country",location.country);

    unitsMap.insert("temperature",units.temperature.remove(QLatin1Char('\260')));
    unitsMap.insert("distance",units.distance);
    unitsMap.insert("pressure",units.pressure);
    unitsMap.insert("speed",units.speed);

    windMap.insert("chill",wind.chill);
    windMap.insert("direction",wind.direction);
    windMap.insert("speed",wind.speed);

    atmosphereMap.insert("humidity",atmosphere.humidity);
    atmosphereMap.insert("visibility",atmosphere.visibility);
    atmosphereMap.insert("pressure",atmosphere.pressure);
    atmosphereMap.insert("barometricReading",atmosphere.barometricReading);

    astronomyMap.insert("sunrise",astronomy.sunrise);
    astronomyMap.insert("sunset",astronomy.sunset);

    conditionMap.insert("text",condition.text);
    conditionMap.insert("code",condition.code);
    conditionMap.insert("temp",condition.temp);
    conditionMap.insert("date",condition.date);

    for(int i=0; i<5; i++)
    {
        forecastMap[i].insert("day",forecast[i].day);
        forecastMap[i].insert("relativeDay",forecast[i].relativeDay);
        forecastMap[i].insert("date",forecast[i].date);
        forecastMap[i].insert("low",forecast[i].low);
        forecastMap[i].insert("high",forecast[i].high);
        forecastMap[i].insert("text",forecast[i].text);
        forecastMap[i].insert("code",forecast[i].code);
    }

    jobject.insert("isNight",isNight);

    jobject.insert("location",locationMap);
    jobject.insert("units",unitsMap);
    jobject.insert("wind",windMap);
    jobject.insert("atmosphere",atmosphereMap);
    jobject.insert("astronomy",astronomyMap);
    jobject.insert("condition",conditionMap);
    jobject.insert("forecastDays",forecastDays);
    for(int i=0; i<5; i++)
        jobject.insert(QString("forecast_%1").arg(i),forecastMap[i]);
    jobject.insert("url",url);

    return QString::fromUtf8(Json::serialize(jobject));
}
