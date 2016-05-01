/**
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010-2011 Andy Bridges

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#ifndef LH_QT_CF_SOURCE_TARGET_H
#define LH_QT_CF_SOURCE_TARGET_H

#include <QtGlobal>
#include <QDebug>
#include <QDateTime>
#include <QList>
#include <QHash>

#include "LH_QtSetupItem.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_int.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_float.h"
//#include "LH_Qt_QTextEdit.h"
//#include "LH_Qt_QColor.h"
//#include "LH_Qt_QFont.h"
//#include "LH_Qt_QFileInfo.h"

#ifndef EXPORT
# define EXPORT extern "C" Q_DECL_EXPORT
#endif

class LH_QtCFInstance;

class cf_source: public QObject
{
    LH_QtSetupItem* obj_;
    QString name_;
    QDateTime changed_;
    QHash<int,QString> values_;
public:
    cf_source( LH_QtCFInstance* parent, QString& name, LH_QtSetupItem*& obj ): QObject((QObject*)parent)
    {
        name_ = name;
        obj_ = obj;
        changed_ = QDateTime::currentDateTime().addDays(-1);
    }

    LH_QtSetupItem* obj() { return obj_; }
    QString name() { return name_; }

    QString value(QString mode, int i = 0)
    {
        bool gotData = false;
        bool numericError = false;
        if(mode=="" || mode=="Value")
        {
            gotData = true;
            if(values_.contains(i))
                return values_[i];
            else
                return QString("~err//##RAGE/%1").arg(values_.count()) ;
        } else
        if(mode=="Time Since Last Change (seconds)")
        {
            gotData = true;
            qreal diff = changed_.msecsTo( QDateTime::currentDateTime() ) / 1000.0;
            return QString::number( qRound(diff) );
        } else
        if(mode=="Count")
        {
            gotData = true;
            return QString::number(values_.count());
        } else
        if(mode=="Average" || mode=="Sum")
        {
            gotData = true;
            float val = 0;
            int c = 0;
            for(int j = 0; j<values_.count(); j++)
            {
                bool ok;
                float num = values_[j].toFloat(&ok);
                numericError |= !ok;
                if(ok) { val += num; c++; }
            }
            if(mode=="Average" && c!=0) val /= c;
            if(numericError) qWarning() << "Non numeric value ignored during \"" << mode << "\" aggregation.";
            return QString::number(val);
        } else
        if(mode=="Minimum" || mode=="Maximum")
        {
            gotData = true;
            float val = 0;
            bool hadFirst = false;
            for(int j = 0; j<values_.count(); j++)
            {
                bool ok;
                float num = values_[j].toFloat(&ok);
                numericError |= !ok;
                if(ok) {
                    if(!hadFirst)
                    {
                        hadFirst = true;
                        val = num;
                    }
                    else
                    {
                        if(mode=="Minimum" && num < val) val = num;
                        if(mode=="Maximum" && num > val) val = num;
                    }
                }
            }
            if(numericError) qWarning() << "Non numeric value ignored during \"" << mode << "\" aggregation.";
            return QString::number(val);
        } else
        if(!gotData)
        {
            qWarning() << "Unhandled data mode: " << mode;
            return "";
        }

        qWarning() << "Unknown CF error";
        return "";
    }

    int count() { return values_.count(); }

    bool setValue(QString value = "", int index = 0)
    {
        if(obj_!=NULL)
        {
            index = 0;
            switch(obj_->type())
            {
            case lh_type_string:
                value = ((LH_Qt_QString*)obj_)->value();
                break;
            case lh_type_integer_list:
                value = ((LH_Qt_QStringList*)obj_)->valueText();
                break;
            case lh_type_integer_boolean:
                value = QString("%1").arg(((LH_Qt_bool*)obj_)->value());
                break;
            case lh_type_integer:
                value = QString("%1").arg(((LH_Qt_int*)obj_)->value());
                break;
            case lh_type_fraction:
                value = QString("%1").arg(((LH_Qt_float*)obj_)->value());
                break;
            default:
                qWarning() << "Unhandled cf source type: " << obj_->type() << " (" << obj_->objectName() << ")";
                return "";
            }
        }

        bool result = false;
        if(values_.contains(index))
        {
            result = (value!=values_[index]);
            values_[index]=value;
        }
        else
        {
            result = (value!="");
            values_.insert(index,value);
        }

        if(result) changed_ = QDateTime::currentDateTime();
        return result && (obj_!=NULL);
    }

    QStringList getModes()
    {
        QStringList modesList;
        if(count()>1)
        {
            modesList.append("Average");
            modesList.append("Minimum");
            modesList.append("Maximum");
            modesList.append("Sum");
            modesList.append("Count");
        }
        else
        {
            modesList.append("Value");
        }
        modesList.append("Time Since Last Change (seconds)");

        return modesList;
    }
};

class cf_source_list
{
    QList<cf_source*> list_;

public:
    cf_source* operator[](QString name)
    {
        foreach(cf_source* source, list_)
        {
            if(source->name() == name)
                return source;
            else
                if(source->obj() != NULL && source->obj()->name() == name)
                    return source;
        }
        return NULL;
    }

    bool contains(QString name)
    {
        return (this->operator [](name)!=NULL);
    }

    int count()
    {
        return list_.count();
    }

    void insert(cf_source* source)
    {
        list_.append(source);
    }
};

class cf_target_list: public QList<LH_QtSetupItem*>
{
public:
    cf_target_list():QList<LH_QtSetupItem*>()
    {
        return;
    }
    LH_QtSetupItem* operator[](QString name)
    {
        for(int i=0; i<length(); i++)
        {
            if(((LH_QtSetupItem*)at(i))->name() == name)
                return at(i);
        }
        return NULL;
    }
    LH_QtSetupItem* operator[](int i)
    {
        return at(i);
    }
    bool contains(QString name)
    {
        return (this->operator [](name)!=NULL);
    }

};

#endif //LH_QT_CF_SOURCE_TARGET_H
