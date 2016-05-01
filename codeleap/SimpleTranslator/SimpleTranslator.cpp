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

#include "SimpleTranslator.h"

#include <QDebug>
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QNetworkProxyQuery>
#include <QUrl>

SimpleTranslator::SimpleTranslator(QString name, LH_QtObject *parent) : QObject(parent)
{
    parent_ = parent;
    name_ = name;
    loaded_ = false;
    sourceLanguage_ = "en";
    targetLanguage_ = "en";
}

void SimpleTranslator::addItem(QString *item, TranslationType transType)
{
    if((*item).trimmed() == "" || (*item).trimmed() == "N/A")
        return;

    QString value = *item;
    QString rxString = "";
    switch(transType)
    {
    case ttMonthName:
        rxString = "(Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?)";
        break;
    case ttDayName:
        rxString = "(Mon(?:day)?|Tue(?:sday)?|Wed(?:nesday)?|Thu(?:rsday)?|Fri(?:day)?|Sat(?:urday)?|Sun(?:day)?)";
        break;
    default:
        break;
    }
    if(rxString!="")
    {
        QRegExp rx(rxString);
        if(rx.indexIn(value)!=-1)
            value = fullDateName(rx.cap(1));
        else
            value = "";
    }

    if (value=="") return;

    if(languageCache_.keys().contains(value.toLower()))
        apply(item, languageCache_[value.toLower()], transType);
    else
        apply(item, value, transType);
}

QString SimpleTranslator::fullDateName(QString shortName)
{
    QString longNames[19] = {"January","February","March","April","May","June","July","August","September","October","Novemeber","December",
                             "Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"
                            };
    for(int i=0; i<19; i++)
        if(shortName == longNames[i].left(3))
            return longNames[i];

    return shortName;
}

void SimpleTranslator::apply(QString* item, QString translatedValue, TranslationType transType)
{
    QString value = *item;
    QString rxString = "";
    switch(transType)
    {
    case ttMonthName:
        rxString = "(Jan(?:uary)?|Feb(?:ruary)?|Mar(?:ch)?|Apr(?:il)?|May|Jun(?:e)?|Jul(?:y)?|Aug(?:ust)?|Sep(?:tember)?|Oct(?:ober)?|Nov(?:ember)?|Dec(?:ember)?)";
        break;
    case ttDayName:
        rxString = "(Mon(?:day)?|Tue(?:sday)?|Wed(?:nesday)?|Thu(?:rsday)?|Fri(?:day)?|Sat(?:urday)?|Sun(?:day)?)";
        break;
    default:
        addToCache(value, translatedValue);
        value = fixCaps(translatedValue.trimmed());
        break;
    }
    if(rxString!="")
    {
        QRegExp rx(rxString);
        if(rx.indexIn(value)!=-1)
        {
            QString translation = translatedValue.trimmed();
            translation = fixCaps(translation);
            addToCache(fullDateName(rx.cap(0)), translation);
            if(transType == ttDayName)
                translation = translation.left(3);
            value = value.replace(rx, translation);
        }
    }

    *item = value;
}

QString SimpleTranslator::fixCaps(QString src)
{
    src = src.replace(QRegExp("(^|\\W)pm(\\W|$)"),"\\1PM\\2");
    src = src.replace(QRegExp("(^|\\W)am(\\W|$)"),"\\1AM\\2");

    QRegExp rx("(?:^|\\W)([a-z])");
    int pos = 0;
    while((pos = rx.indexIn(src, pos)+1)!=0)
    {
        QString capText = rx.cap(0);
        src = QString("%1%2%3")
                .arg(pos<=1? "" : src.left(pos-1))
                .arg(capText.toUpper())
                .arg(src.length()-pos-capText.length()+1 <= 0 ? "" : src.right(src.length()-pos-capText.length()+1));
    }
    return src;
}

void SimpleTranslator::addToCache(QString sourceItem, QString translatedItem)
{
    if( languageCache_.keys().contains(sourceItem.toLower()) )
        return; //already in cache;

    languageCache_.insert(sourceItem.toLower(), translatedItem.trimmed());

    newCacheItems_.insert(sourceItem, translatedItem.trimmed());
}

QString SimpleTranslator::getCacheFileName(bool externalPath, QString targetLanguage)
{
    if (targetLanguage.toLower() == "languages")
        return QString("%1languages.cache")
                .arg(externalPath? QString::fromUtf8(parent_->state()->dir_data) : ":/translations/");

    if (targetLanguage == "") targetLanguage = targetLanguage_;
    return QString("%1translations.%2.%3.%4.cache")
            .arg(externalPath? QString::fromUtf8(parent_->state()->dir_data) : ":/translations/")
            .arg(name_)
            .arg(sourceLanguage_)
            .arg(targetLanguage);
}

void SimpleTranslator::saveCache()
{
    if(newCacheItems_.count()==0) return;
    QFile file(getCacheFileName(true));
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        foreach(QString key, newCacheItems_.keys())
            out << key << "\t" << newCacheItems_[key] << "\n";
        file.close();
        newCacheItems_.clear();
    }
}

void SimpleTranslator::loadCache(bool languageList, bool includeUntranslatedLanguages)
{
    if(languageList)
        languages.clear();
    else
        languageCache_.clear();

    for (int i = 0; i<=1; i++)
    {
        QFile file(getCacheFileName(i==0,(languageList? "languages" : "")));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream out(&file);
            QStringList cachedItems = out.readAll().split("\n", QString::SkipEmptyParts);
            file.close();

            for(int i=0; i<cachedItems.length(); i++)
            {
                QString line = cachedItems.at(i).trimmed();

                QStringList itemPair = cachedItems[i].split("\t");
                if(itemPair.count()!=2) continue;
                QString key = itemPair[0].toLower();
                if(languageList)
                {
                    if (!languages.containsCode(key))
                        if (key=="en" || includeUntranslatedLanguages || QFileInfo(getCacheFileName(true, key)).exists() || QFileInfo(getCacheFileName(false, key)).exists())
                            languages.append(key,itemPair[1]);
                }
                else
                {
                    if (!languageCache_.contains(key))
                        languageCache_.insert(key,itemPair[1]);
                }
            }
        }
    }
}
