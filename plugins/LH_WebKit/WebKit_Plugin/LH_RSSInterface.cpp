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

#include "LH_RSSInterface.h"

LH_RSSInterface::LH_RSSInterface(LH_QtObject* parent)
{
    // Hide inherited attributes we don't use
    //setup_text_->setFlag( LH_FLAG_HIDDEN, true );
    url_ = getDefaultURL();
    setup_url_ = new LH_Qt_QString(parent,"URL",url_);
    setup_url_->setOrder(-1);
    connect( setup_url_, SIGNAL(changed()), this, SLOT(changeURL()));

    setup_method_ = NULL;

    setup_delay_ = new LH_Qt_QSlider(parent,tr("Switch delay"),5,1,10);
    connect( setup_delay_, SIGNAL(changed()), this, SLOT(changeDelay()) );

    setup_browser_ = new LH_Qt_InputState(parent,tr("Open in browser"),QString(),LH_FLAG_AUTORENDER);
    connect( setup_browser_, SIGNAL(input(QString,int,int)), this, SLOT(openBrowser(QString,int,int)) );

    setup_prev_ = new LH_Qt_InputState(parent,tr("Previous Headline"),QString(),LH_FLAG_AUTORENDER);
    connect( setup_prev_, SIGNAL(input(QString,int,int)), this, SLOT(prevHeadline(QString,int,int)) );

    setup_next_ = new LH_Qt_InputState(parent,tr("Next Headline"),QString(),LH_FLAG_AUTORENDER);
    connect( setup_next_, SIGNAL(input(QString,int,int)), this, SLOT(nextHeadline(QString,int,int)) );

    setup_refresh_ = new LH_Qt_int(parent,tr("Refresh (minutes)"),5);
    connect( setup_refresh_, SIGNAL(changed()), this, SLOT(changeRefresh()) );

    setup_modify_ = new LH_Qt_int(parent,tr("Modify Visible"),0,-20,20);
    setup_modify_->setHelp("Use this to display a different item from the one that is currently active: set it to 1 for the \"next\" item, \"-1\" for the previous item, etc.");
    connect( setup_modify_, SIGNAL(changed()), this, SLOT(reemitChanged()) );

    connectRSS();
    emit changed();
}

LH_RSSInterface::~LH_RSSInterface()
{
    releaseRSS();
}

QString LH_RSSInterface::getDefaultURL()
{
    if( rssFeeds )
    {
        for(int i=0; i<rssFeeds->count(); i++)
            if( !rssFeeds->keys().at(i).isEmpty() ) return rssFeeds->keys().at(i);
    }
    return QString();
}

LH_RSSFeed* LH_RSSInterface::getFeed()
{
    if(rssFeeds==NULL)
        rssFeeds = new QHash<QString, LH_RSSFeed* >();

    if(rssFeeds->contains(url_))
        return rssFeeds->value(url_);
    else
    {
        LH_RSSFeed* newFeed =  new LH_RSSFeed(url_, setup_refresh_->value(), setup_delay_->value());
        rssFeeds->insert(url_, newFeed);
        return newFeed;
    }
}

void LH_RSSInterface::connectRSS()
{
    getFeed()->connect(this);

    connect( getFeed(), SIGNAL(delayChanged()), this, SLOT(updateDelay()) );
    connect( getFeed(), SIGNAL(refreshChanged()), this, SLOT(updateRefresh()) );
    connect( getFeed(), SIGNAL(begin()), this, SLOT(reemitBegin()) );
    connect( getFeed(), SIGNAL(finished()), this, SLOT(reemitFinished()) );
    connect( getFeed(), SIGNAL(changed()), this, SLOT(reemitChanged()) );

    updateDelay();
    updateRefresh();
}

void LH_RSSInterface::releaseRSS()
{
    Q_ASSERT(rssFeeds!=NULL);

    if(rssFeeds==NULL) return;

    LH_RSSFeed* feed = getFeed();

    if(feed->release(this))
    {
        disconnect( getFeed(), SIGNAL(delayChanged()), this, SLOT(updateDelay()) );
        disconnect( getFeed(), SIGNAL(refreshChanged()), this, SLOT(updateRefresh()) );
        disconnect( getFeed(), SIGNAL(begin()), this, SLOT(reemitBegin()) );
        disconnect( getFeed(), SIGNAL(finished()), this, SLOT(reemitFinished()) );
        disconnect( getFeed(), SIGNAL(changed()), this, SLOT(reemitChanged()) );
        if(feed->connections()==0)
        {
            rssFeeds->remove(url_);

            delete feed;
            if(rssFeeds->count() == 0)
            {
                delete rssFeeds;
                rssFeeds = NULL;
            }
        }
    }
}

int LH_RSSInterface::notify(int code,void* param)
{
    return getFeed()->notify(code, param);
}

RSSItem LH_RSSInterface::item()
{
    return getFeed()->item(setup_modify_->value());
}

void LH_RSSInterface::openBrowser(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    getFeed()->openItem(setup_modify_->value());
}

void LH_RSSInterface::prevHeadline(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    getFeed()->moveBack();
}

void LH_RSSInterface::nextHeadline(QString key,int flags,int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    getFeed()->moveNext();
}

void LH_RSSInterface::changeURL()
{
    releaseRSS();
    url_=setup_url_->value();
    connectRSS();
    emit changed();
}

void LH_RSSInterface::changeDelay()
{
    getFeed()->setDelay(setup_delay_->value());
}

void LH_RSSInterface::changeRefresh()
{
    getFeed()->setRefresh(setup_refresh_->value());
}

void LH_RSSInterface::updateDelay()
{
    //setup_delay_->setValue(getFeed()->delay());
}

void LH_RSSInterface::updateRefresh()
{
    setup_refresh_->setValue(getFeed()->refresh());
}

