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

#ifndef ACTIONTYPE_H
#define ACTIONTYPE_H

#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QTextEdit.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_int.h"
#include "LH_Qt_bool.h"

#include "LH_CursorData.h"

#include <QDomElement>

class LH_CursorAction;

enum actionParameterType
{
    aptString,
    aptFile,
    aptInteger
};

enum actionLimitType
{
    altWait,
    altCursorX,
    altCursorY
};

class actionParameter
{
public:
    QString description;
    actionParameterType type;
    QString xmlTag;
    QString xmlAttribute;
    actionLimitType limitType;

    actionParameter( QString d,
                    actionParameterType t = aptString,
                    QString xt = QString(),
                    QString xa = QString(),
                    actionLimitType lt = altWait ) :
        description(d), type(t), xmlTag(xt), xmlAttribute(xa), limitType(lt) {}
};

class actionType
{
    LH_CursorAction *cursorAction_;

    void setIntMinMax(LH_Qt_int *int_, int minVal, int maxVal)
    {
        int val = int_->value();
        int_->setMinMax(minVal,maxVal);
        if(val<minVal)val = minVal;
        if(val>maxVal)val = maxVal;
        int_->setValue(val);
    }

public:
    QString typeCode;
    QString description;
    QList<actionParameter> parameters;

    actionType(
        LH_CursorAction *cursorAction,
        QString typeCode__ = QString(),
        QString description__ = QString(),
        QList<actionParameter> parameters__ = QList<actionParameter>()
        ) :
        cursorAction_(cursorAction),
        typeCode(typeCode__),
        description(description__),
        parameters(parameters__)
    {}

    QString getParameter(QDomElement e, int index);
    QString generateXML(bool enabled, QString desc, QStringList paramValues);
    void displayParameter(int id, LH_Qt_QString *desc_, LH_Qt_QString *str_, LH_Qt_int *int_, LH_Qt_QFileInfo *file_, cursorData cd, QDomElement e = QDomElement());
    QString getParameterValue(int id, LH_Qt_QString *str_, LH_Qt_int *int_, LH_Qt_QFileInfo *file_);
};

class actionTypes
{
    QHash<QString,actionType> actionTypes_;
    QStringList actionTypeIDs_;
    LH_CursorAction *cursorAction_;

    void add(actionType at)
    {
        actionTypes_.insert(at.typeCode, at);
        actionTypeIDs_.append(at.typeCode);
        // return actionTypes_[at.typeCode];
        return;
    }

public:
    actionTypes( LH_CursorAction *p ) : cursorAction_(p) {
        add( actionType(p,"open"      ,"Open Layout"               , QList<actionParameter>() << actionParameter("Layout File",aptFile) ) );
        add( actionType(p,"run"       ,"Run Application"           , QList<actionParameter>() << actionParameter("Application",aptFile,"path") << actionParameter("Parameters",aptString,"args") ) );
        add( actionType(p,"url"       ,"Open URL"                  , QList<actionParameter>() << actionParameter("URL",aptString) ) );
        add( actionType(p,"move"      ,"Move Cursor"               , QList<actionParameter>() << actionParameter("New X Coordinate",aptInteger,"","x",altCursorX) << actionParameter("New Y Coordinate",aptInteger,"","y",altCursorY)) );
        add( actionType(p,"select"    ,"Move Cursor & Select"      , QList<actionParameter>() << actionParameter("New X Coordinate",aptInteger,"","x",altCursorX) << actionParameter("New Y Coordinate",aptInteger,"","y",altCursorY)) );
        add( actionType(p,"wait"      ,"Wait"                      , QList<actionParameter>() << actionParameter("Delay (in ms)",aptInteger,"","",altWait)) );
        add( actionType(p,"deselect"  ,"Clear Selection") );
        add( actionType(p,"deactivate","Deactivate the Cursor") );
        add( actionType(p,"reselect"  ,"Reselect the previous item") );
    }
    actionType at(int index)
    {
        return at(actionTypeIDs_[index]);
    }
    actionType at(QString typeCode)
    {
        return actionTypes_.value(typeCode,actionType(this->cursorAction_));
    }
    QStringList list()
    {
        QStringList list_ = QStringList();
        for (int i=0; i<actionTypeIDs_.count(); i++)
            list_.append(at(i).description);
        return list_;
    }
    int indexOf(QString typeCode)
    {
        return actionTypeIDs_.indexOf(typeCode);
    }
};

#endif // ACTIONTYPE_H
