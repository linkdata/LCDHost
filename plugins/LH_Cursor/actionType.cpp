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


#include "actionType.h"
#include "LH_CursorAction.h"

QString actionType::getParameter(QDomElement e, int index)
{
    if(index<parameters.count())
    {
        QDomElement e2;
        if(parameters[index].xmlTag!="")
            e2 = e.elementsByTagName(parameters[index].xmlTag).at(0).toElement();
        else
            e2 = e;

        if(parameters[index].xmlAttribute!="")
            return e2.attribute(parameters[index].xmlAttribute);
        else
            return e2.text();
    }
    else
        return "";
}

QString actionType::generateXML(bool enabled, QString desc, QStringList paramValues)
{
    QHash<QString,QString> nodesXML = QHash<QString,QString>();
    QHash<QString,QString> nodesVal = QHash<QString,QString>();
    nodesXML.insert("","");
    nodesVal.insert("","");
    nodesXML[""] += "<action ";
    nodesXML[""] += "type='" + typeCode + "' ";
    if(desc != "")
        nodesXML[""] += "desc='" + desc + "' ";
    if(!enabled)
        nodesXML[""] += "enabled='false' ";

    for(int i=0; i<parameters.length(); i++)
    {
        if(!nodesXML.contains(parameters[i].xmlTag))
        {
            nodesXML.insert(parameters[i].xmlTag,"<"+parameters[i].xmlTag+" ");
            nodesVal.insert(parameters[i].xmlTag,"");
        }
        if(parameters[i].xmlAttribute=="")
            nodesVal[parameters[i].xmlTag]+=paramValues[i];
        else
            nodesXML[parameters[i].xmlTag]+=parameters[i].xmlAttribute + "='" + paramValues[i] + "' ";
    }
    QString strXML = nodesXML[""];
    if(nodesVal[""]!="")
        strXML += ">" + nodesVal[""] + "</action>";
    else
    if(nodesXML.count()==1)
        strXML += "/>";
    else
    {
        strXML += ">";
        foreach(QString key, nodesXML.keys())
        {
            if(key=="") continue;
            strXML += nodesXML[key];
            if(nodesVal[key]!="")
                strXML += ">" + nodesVal[key] + "</"+key+">";
            else
                strXML += "/>";
        }
        strXML += "</action>";
    }

    return strXML;
}

void actionType::displayParameter(int id, LH_Qt_QString *desc_, LH_Qt_QString *str_, LH_Qt_int *int_, LH_Qt_QFileInfo *file_, cursorData cd, QDomElement e)
{
    desc_->setFlag(LH_FLAG_HIDDEN, (parameters.count()<id+1));
    str_->setFlag(LH_FLAG_HIDDEN,  (parameters.count()<id+1) || (parameters[id].type != aptString));
    int_->setFlag(LH_FLAG_HIDDEN,  (parameters.count()<id+1) || (parameters[id].type != aptInteger));
    file_->setFlag(LH_FLAG_HIDDEN, (parameters.count()<id+1) || (parameters[id].type != aptFile));

    if (parameters.count()>=id+1)
    {
        if(parameters[id].type == aptInteger)
        {
            switch (parameters[id].limitType)
            {
            case altWait:    setIntMinMax(int_,0,300000); break;
            case altCursorX: setIntMinMax(int_,cd.range.x.min,cd.range.x.max); break;
            case altCursorY: setIntMinMax(int_,cd.range.y.min,cd.range.y.max); break;
            }
        }

        desc_->setValue(parameters[id].description);
        if(e.tagName()!="")
        {
            QString val = getParameter(e,id);
            str_->setValue(parameters[id].type != aptString ? "" : val);
            int_->setValue(parameters[id].type != aptInteger? id : val.toInt());
            QString val2 = QString("%1%2").arg(cursorAction_->state()->dir_layout).arg(val);
            file_->setValue(parameters[id].type != aptFile? QFileInfo() : (QFileInfo(val2).exists()? QFileInfo(val2) : QFileInfo(val)));
        }
    }

}

QString actionType::getParameterValue(int id, LH_Qt_QString *str_, LH_Qt_int *int_, LH_Qt_QFileInfo *file_)
{
    if (id>=parameters.count())
        return "";
    switch(parameters[id].type)
    {
    case aptString:
        return str_->value();
    case aptInteger:
        return QString("%1").arg(int_->value());
    case aptFile:
        QString path = file_->value().absoluteFilePath();
        QString dir_layout = QString::fromUtf8(cursorAction_->state()->dir_layout);
        if (path.startsWith(dir_layout))
            path = path.replace(dir_layout,"",Qt::CaseInsensitive);
        return path;
    }
    return "";
}

