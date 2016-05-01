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

#ifndef LH_CURSORDATA_H
#define LH_CURSORDATA_H

#include <QString>
#include <QHash>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QDateTime>
#include "json.h"
#include "LH_Qt_QString.h"

extern QHash<QString, QString> postback_data;

struct minmax {
    int min;
    int max;
    minmax() : min(0), max(0) {}
    minmax(int min_value, int max_value) : min(min_value), max(max_value) {}
};

struct bounds {
    minmax x;
    minmax y;
    bounds() {}
    bounds(minmax x_values, minmax y_values) : x(x_values), y(y_values) {}
};

/*struct cursorData
{
    int x;
    int y;
    bool active;
    int selX;
    int selY;
    bool selState;
    int lastSelX;
    int lastSelY;
    bool lastSelSet;
    bool sendSelect;
    int lastSelX2;
    int lastSelY2;
    bounds range;
};*/

class cursorData
{
private:
    void reset()
    {
        x = 1;
        y = 1;
        active = false;
        selX = 0;
        selY = 0;
        selState = false;
        lastSelX = 0;
        lastSelY = 0;
        lastSelSet = false;
        sendSelect = false;
        lastSelX2 = 0;
        lastSelY2 = 0;
        range = bounds();
    }

public:
    int x;
    int y;
    bool active;
    int selX;
    int selY;
    bool selState;
    int lastSelX;
    int lastSelY;
    bool lastSelSet;
    bool sendSelect;
    int lastSelX2;
    int lastSelY2;
    bounds range;

    cursorData()
    {
        reset();
    }

    cursorData(QString jsonData)
    {
        if(!deserialize(jsonData))
            reset();
    }

    bool deserialize(QString jsonData)
    {
        bool ok;
        QVariantMap jobject = Json::parse(jsonData, ok).toMap();
        if(ok)
        {
            x = jobject["x"].toInt();
            y = jobject["y"].toInt();
            active = jobject["active"].toBool();
            selX = jobject["selX"].toInt();
            selY = jobject["selY"].toInt();
            selState = jobject["selState"].toBool();
            lastSelX = jobject["lastSelX"].toInt();
            lastSelY = jobject["lastSelY"].toInt();
            lastSelSet = jobject["lastSelSet"].toBool();;
            sendSelect = jobject["sendSelect"].toBool();;
            lastSelX2 = jobject["lastSelX2"].toInt();
            lastSelY2 = jobject["lastSelY2"].toInt();

            QVariantMap rangeMap = jobject["range"].toMap();
            QVariantMap rangeYMap = rangeMap["y"].toMap();
            QVariantMap rangeXMap = rangeMap["x"].toMap();

            range.x.min = rangeXMap["min"].toInt();
            range.x.max = rangeXMap["max"].toInt();
            range.y.min = rangeYMap["min"].toInt();
            range.y.max = rangeYMap["max"].toInt();
        }
        return ok;
    }

    QString serialize()
    {
        QVariantMap jobject;
        QVariantMap rangeMap;
        QVariantMap rangeYMap;
        QVariantMap rangeXMap;

        jobject.insert("x",x);
        jobject.insert("y",y);
        jobject.insert("active",active);
        jobject.insert("selX",selX);
        jobject.insert("selY",selY);
        jobject.insert("selState",selState);
        jobject.insert("lastSelX",lastSelX);
        jobject.insert("lastSelY",lastSelY);
        jobject.insert("lastSelSet",lastSelSet);
        jobject.insert("sendSelect",sendSelect);
        jobject.insert("lastSelX2",lastSelX2);
        jobject.insert("lastSelY2",lastSelY2);

        rangeYMap.insert("min", range.y.min);
        rangeYMap.insert("max", range.y.max);
        rangeMap.insert("y", rangeYMap);

        rangeXMap.insert("min", range.x.min);
        rangeXMap.insert("max", range.x.max);
        rangeMap.insert("x", rangeXMap);

        jobject.insert("range", rangeMap);


        return QString::fromUtf8(Json::serialize(jobject));
    }

    QString getState(QStringList mycoords, bool &newSelected, bool &newActive)
    {
        newSelected = false;
        newActive = false;

        foreach (QString mycoord_str, mycoords)
        {
            QStringList mycoord = mycoord_str.split(',');
            if(mycoord.length()==2)
            {
                int myX = mycoord.at(0).toInt();
                int myY = mycoord.at(1).toInt();

                newSelected |= ( selState && selX==myX && selY==myY );
                newActive |= ( active && x==myX && y==myY );
            }
        }

        return QString("%1%2").arg(newActive? "ON" : "OFF").arg(newSelected? "_SEL" : "");
    }
};

#endif // LH_CURSORDATA_H
