/**
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  @legalese Copyright (c) 2010-2011 Andy Bridges

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

#ifndef SIMPLETRANSLATOR_H
#define SIMPLETRANSLATOR_H

#include <QObject>

#include <QList>
#include <QHash>
#include <QStringList>

#include "LH_QtObject.h"

#ifdef LH_SIMPLETRANSLATOR_LIBRARY
# define LH_SIMPLETRANSLATOR_EXPORT Q_DECL_EXPORT
#else
# define LH_SIMPLETRANSLATOR_EXPORT Q_DECL_IMPORT
#endif

enum TranslationType
{
    ttAll,
    ttMonthName,
    ttDayName,
    ttNoun
};

class LH_SIMPLETRANSLATOR_EXPORT IndexedPairList
{
    QStringList codes_;
    QStringList names_;

public:

    bool append(QString code, QString name)
    {
        if(codes_.contains(code))
            return false;
        if(names_.contains(name))
            return false;

        codes_.append(code);
        names_.append(name);
        return true;
    }

    QStringList codes()
    {
        return QStringList(codes_);
    }

    QStringList names()
    {
        return QStringList(names_);
    }

    void clear()
    {
        codes_.clear();
        names_.clear();
    }

    QString getName(QString code)
    {
        if(codes_.contains(code))
            return names_.at(codes_.indexOf(code));
        else
            return "";
    }

    QString getCode(QString name)
    {
        if(names_.contains(name))
            return codes_.at(names_.indexOf(name));
        else
            return "";
    }

    bool containsName(QString name)
    {
        return names_.contains(name);
    }

    bool containsCode(QString code)
    {
        return codes_.contains(code);
    }
};

class LH_SIMPLETRANSLATOR_EXPORT SimpleTranslator: public QObject
{
    Q_OBJECT
    bool loaded_;
    QString name_;
    QString targetLanguage_;
    QString sourceLanguage_;
    LH_QtObject *parent_;

    QHash<QString,QString> languageCache_;
    QHash<QString,QString> newCacheItems_;

    void apply(QString* item, QString translatedValue, TranslationType transType);

    void addToCache(QString sourceItem, QString translatedItem);

    QString getCacheFileName(bool externalPath = true, QString targetLanguage = "");
    void loadCache(bool languagesList = false, bool includeUntranslatedLanguages = false);

    QString fixCaps(QString src);

public:
    IndexedPairList languages;

    SimpleTranslator(QString name, LH_QtObject *parent = 0);
    void loadLanguages(bool includeUntranslatedLanguages = false) { loadCache(true, includeUntranslatedLanguages); }

    void saveCache();
    void addItem(QString *item, TranslationType transType = ttAll);

    QString fullDateName(QString shortName);

    bool setTargetLanguage(QString language){
        if(targetLanguage_ == language && loaded_)
            return false;
        else
        {
            loaded_ = true;
            targetLanguage_ = language;
            loadCache();
            return true;
        }
    }
    QString targetLanguage(){ return targetLanguage_; }
};

#endif // SIMPLETRANSLATOR_H
