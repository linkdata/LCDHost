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

#ifndef LH_QT_CF_RULE_H
#define LH_QT_CF_RULE_H

#include <QtGlobal>
#include <QList>
#include <QHash>
#include <QtXml>
#include <QObject>

#include "LH_Qt_bool.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QTextEdit.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_QFont.h"
#include "LH_Qt_QFileInfo.h"

#include "LH_QtCFInstance.h"

#ifndef EXPORT
# define EXPORT extern "C" Q_DECL_EXPORT
#endif

class cf_rule_condition: public QObject
{
protected:
    QString test_;
    QString source_;
    QString mode_;
    QStringList values_;

public:
    cf_rule_condition(QDomNode condNode, QObject* parent = 0);
    cf_rule_condition(LH_QtCFInstance *sender, QObject* parent = 0);

    QString description();
    QString test() {return test_;}
    QString source() {return source_;}
    QString mode() {return mode_;}
    QString values(int i) {
        if(i>=0 && i<values_.length())
            return values_[i];
        else
            return "";
    }

    void edit(LH_QtCFInstance* sender)
    {
        sender->setup_cf_test_->setValue( test_ );
        sender->setup_cf_source_->setValue( source_ );
        sender->setup_cf_source_mode_->setValue( mode_ );
        sender->setup_cf_testValue1_->setValue( values(0) );
        sender->setup_cf_testValue2_->setValue( values(1) );
    }

    bool evaluate(cf_source_list sources, bool *ok = NULL);
    QDomNode toXmlNode(QDomDocument doc);
};

class cf_rule_action: public QObject
{
protected:
    QString type_;
public:
    cf_rule_action(QDomNode actNode, QObject* parent = 0);
    cf_rule_action(QString type, QObject* parent = 0);

    QString type() {return type_;}

    virtual QString description() { Q_ASSERT(false); return ""; }
    virtual void execute(LH_QtCFInstance* sender, cf_target_list targets) { Q_UNUSED(sender); Q_UNUSED(targets); Q_ASSERT(false); }
    virtual void edit(LH_QtCFInstance* sender, cf_target_list targets) { Q_UNUSED(sender); Q_UNUSED(targets); Q_ASSERT(false); }

    virtual bool isValid(LH_QtCFInstance* sender){ Q_UNUSED(sender); Q_ASSERT(false); return false;}

    virtual QDomNode toXmlNode(QDomDocument doc);
};

class cf_rule_action_property: public cf_rule_action
{
    QString target_;
    QString value_;

    bool setTargetValue(LH_QtCFInstance* sender, cf_target_list targets_, bool setPlaceholder = false);

    QString getRelativeFilePath(QFileInfo file, QString relativeTo)
    {
        if(!file.isFile())
            return "";
        QStringList absoluteDirectories = file.absoluteFilePath().split( '/', QString::SkipEmptyParts );
        QStringList relativeDirectories = relativeTo.split( '/', QString::SkipEmptyParts );

        //Get the shortest of the two paths
        int length = relativeDirectories.count() < absoluteDirectories.count() ? relativeDirectories.count() : absoluteDirectories.count();

        //Use to determine where in the loop we exited
        int lastCommonRoot = -1;
        int index;

        //Find common root
        for (index = 0; index < length; index++)
        if (relativeDirectories[index] == absoluteDirectories[index])
        lastCommonRoot = index;
        else
        break;

        //If we didn't find a common prefix then throw
        if (lastCommonRoot == -1)
            Q_ASSERT(!"Paths do not have a common base");

        //Build up the relative path
        QString relativePath;

        //Add on the ..
        for (index = lastCommonRoot + 1; index < relativeDirectories.count() - (true?1:0); index++)
        if (relativeDirectories[index].length() > 0)
        relativePath.append("../");

        //Add on the folders
        for (index = lastCommonRoot + 1; index < absoluteDirectories.count() - 1; index++)
        relativePath.append( absoluteDirectories[index] ).append( "/" );
        relativePath.append(absoluteDirectories[absoluteDirectories.count() - 1]);

        return relativePath;
    }

public:
    cf_rule_action_property(QDomNode actNode, QObject* parent = 0);
    cf_rule_action_property(LH_QtCFInstance *sender, QObject* parent = 0);

    QString description();
    QString target() {return target_;}
    QString value() {return value_;}

    void execute(LH_QtCFInstance* sender, cf_target_list targets);
    void edit(LH_QtCFInstance* sender, cf_target_list targets);

    bool isValid(LH_QtCFInstance* sender)
    {
        if(target_=="")
            return false;

        if(sender->setup_cf_target_->list().indexOf( target_ ) == -1)
            return false;

        return true;
    }
    QDomNode toXmlNode(QDomDocument doc);
};

class cf_rule : public QObject
{
public:
    cf_rule( QDomNode ruleNode, QObject *parent = 0 );
    cf_rule( LH_QtCFInstance *sender, QObject *parent = 0 );

    QString description();
    bool apply(LH_QtCFInstance* sender, cf_source_list sources_, cf_target_list targets_);

    QList<cf_rule_condition*> conditions;
    QList<cf_rule_action*> actions;

    QDomNode toXmlNode(QDomDocument doc);
};

#endif //LH_QT_CF_RULE_H
