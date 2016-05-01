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

#ifndef LH_DATAVIEWERDATA_H
#define LH_DATAVIEWERDATA_H

#include "LH_Qt_QString.h"

#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QRegExp>

// #include <stdio.h>
// #include <windows.h>

#include "LH_DataViewerDataTypes.h"
extern dataNode* rootNode;
extern sharedCollection* sharedData;


class LH_DataViewerData
{
    LH_QtObject *parent_;

    QString lastupdate_;
    QString lasttemplate_;
    bool lastexpired_;

public:
    bool expired;

    LH_DataViewerData(LH_QtObject *parent)
    {
        parent_ = parent;
    }

    ~LH_DataViewerData(){
    }

    bool open()
    {
        if(sharedData)
        {
            if(sharedData->expiresAt.trimmed()!="N/A")
            {
                QDateTime expiryTime = QDateTime::fromString(sharedData->expiresAt, "yyyyMMddHHmmss.zzz");
                expired = (QDateTime::currentDateTime().secsTo(expiryTime)<=0);
            } else
                expired = false;
            return true;
        }
        else
        {
            expired = false;
            return false;
        }
    }

    bool valid(QString currTemplate)
    {
        if( sharedData->valid && (lastupdate_ != sharedData->lastUpdated || lasttemplate_ != currTemplate || lastexpired_ != expired))
        {
            lastupdate_ = sharedData->lastUpdated;
            lasttemplate_ = currTemplate;
            lastexpired_ = expired;
            return true;
        } else
            return false;
    }

    QString populateLookupCode(QString lookupTemplate, bool getNames = false, bool preventEmpty = false)
    {
        QString templateResult = QString(lookupTemplate);

        // look for the old index-style references, e.g. {0} {2} {34}
        QRegExp rx = QRegExp("\\{([0-9]*)\\}");
        bool blankValues = true;
        int hitCount = templateResult.count(rx);
        while (rx.indexIn(templateResult) != -1)
        {
            QString strVal = getValueText(rx.cap(1).toInt(), getNames);
            if (strVal!="") blankValues = false;
            templateResult.replace(rx.cap(0), strVal);
        }

        // look for new address-based references, e.g. {player.name}
        rx = QRegExp("\\{([a-zA-Z0-9.[\\]]*(?:@[a-zA-Z0-9]+)?)\\}");
        hitCount += templateResult.count(rx);
        while (rx.indexIn(templateResult) != -1)
        {
            QString strVal = getValueText(rx.cap(1), getNames);
            if (strVal!="") blankValues = false;
            templateResult.replace(rx.cap(0), strVal);
        }

        if(!getNames)
        {
            // look for formatting commands, e.g. {=<something>:%.2f}
            rx = QRegExp("\\{=([^}=:]*)(?:\\:(.*))?\\}");
            hitCount += templateResult.count(rx);
            while (rx.indexIn(templateResult) != -1)
            {
                QString strVal = parseMath(rx.cap(1),0,rx.cap(2));
                templateResult.replace(rx.cap(0), strVal);
            }
        }

        if (blankValues && hitCount!=0) templateResult = "";
        if (preventEmpty && templateResult == "") templateResult = " ";

        return templateResult;
    }

    QString getValueText(int valueIndex, bool getName = false)
    {
        if (valueIndex>=sharedData->count() && sharedData->count()!=0)
            valueIndex = sharedData->count()-1;

        QString resultText = "";
        if (valueIndex < sharedData->count())
        {
            if(getName)
                resultText = sharedData->item(valueIndex).name;
            else if(expired)
                resultText = "";
            else
                resultText = sharedData->item(valueIndex).value;
        }

        return resultText;
    }

    QString getValueText(QString valueAddress, bool getName = false)
    {
        QString resultText = "";

        if(getName)
            return valueAddress;
        else
        {
            QString attrName = "";
            QStringList path = valueAddress.split(".");
            dataNode* curNode = rootNode;
            QRegExp rx("^(.*)\\[([0-9]+)\\]$");
            while (path.length()!=0)
            {
                QString nodeName = path.first();
                path.removeFirst();
                if(!nodeName.contains("@"))
                    attrName = "";
                else
                {
                    attrName = nodeName.split('@')[1];
                    nodeName = nodeName.split('@')[0];
                }

                int nodeIndex = 0;
                if(rx.indexIn(nodeName) != -1)
                {
                    nodeName = rx.cap(1);
                    nodeIndex = rx.cap(2).toInt();
                }

                if(!curNode->contains(nodeName))
                    return valueAddress;
                if(nodeIndex>=curNode->child(nodeName).count())
                    return valueAddress;
                curNode = curNode->child(nodeName)[nodeIndex];
            }
            while(curNode->defaultItem()!="")
                curNode = curNode->child(curNode->defaultItem())[0];

            if(attrName == "")
                resultText = curNode->value();
            else
                resultText = curNode->attributes.value(attrName);
        }

        return resultText;
    }

    QString parseMath(QString mathString, int depth, QString formatting = "")
    {
        if (depth>=50)
        {
            qWarning() << "Parser Depth >50: Aborted parse. " << mathString;
            return "0";
        }
        //qDebug() << "Parse Math: in: " << mathString;

        QRegExp rx = QRegExp("\\(([^)=]*)\\)");
        qreal fltVal = 0;
        int loopCount = 0;
        while (rx.indexIn(mathString) != -1)
        {
            QString strVal = parseMath(rx.cap(1), depth + 1);
            mathString.replace(rx.cap(0), strVal);
            if (++loopCount >=100)
            {
                qWarning() << "Parser Loops >100: Aborted parse. " << mathString;
                return "0";
            }
        }

        loopCount = 0;
        rx = QRegExp("(-?[0-9]*(?:\\.[0-9]*)?)\\s*(\\*|/)\\s*(-?[0-9]*(?:\\.[0-9]*)?)");
        while (rx.indexIn(mathString) != -1)
        {
            //qDebug() << "Parse Math: action: [" << rx.cap(1) << "] [" << rx.cap(2) << "] [" << rx.cap(3) << "]";
            if (rx.cap(2)=="*")
                fltVal = rx.cap(1).toFloat() * rx.cap(3).toFloat(); else
            if (rx.cap(2)=="/" && rx.cap(3).toFloat()!=0)
                fltVal = rx.cap(1).toFloat() / rx.cap(3).toFloat(); else
            if (rx.cap(2)=="/" && rx.cap(3).toFloat()==0)
                fltVal = 0;

            mathString.replace(rx.cap(0), QString::number(fltVal));
            if (++loopCount >=100)
            {
                qWarning() << "Parser Loops >100: Aborted parse. " << mathString;
                return "0";
            }
        }

        loopCount = 0;
        rx = QRegExp("(-?(?:[0-9]*\\.)?[0-9]{1,})\\s*(\\+|\\-)\\s*(-?(?:[0-9]*\\.)?[0-9]{1,})");
        while (rx.indexIn(mathString) != -1)
        {
            //qDebug() << "Parse Math: action: [" << rx.cap(1) << "] [" << rx.cap(2) << "] [" << rx.cap(3) << "]";
            if (rx.cap(2)=="+")
                fltVal = rx.cap(1).toFloat() + rx.cap(3).toFloat();
            if (rx.cap(2)=="-")
                fltVal = rx.cap(1).toFloat() - rx.cap(3).toFloat();

            mathString.replace(rx.cap(0), QString::number(fltVal));
            if (++loopCount >=100)
            {
                qWarning() << "Parser Loops >100: Aborted parse. " << mathString;
                return "0";
            }
        }


        if (formatting!="")
        {
            QString strVal;
            strVal.sprintf(formatting.toLatin1().data(),mathString.toFloat());
            mathString = strVal;
        }

        //qDebug() << "Parse Math: out: " << mathString;

        return mathString;
    }
};



#endif // LH_DATAVIEWERDATA_H
