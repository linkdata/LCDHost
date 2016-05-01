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

#ifndef LH_TS3_CHANNELLIST_H
#define LH_TS3_CHANNELLIST_H

#include <QHash>

struct channeldetail
{
    int cid;
    int pid;
    int order;
    QString name;
    int clientCount;
    bool neededSubscribePower;
};

class channellist: public QHash<int,channeldetail>
{
public:
    bool load(QString responseMsg)
    {
        if(!responseMsg.contains("channel_name=") || !responseMsg.contains("cid=") )
            return false;
        this->clear();
        QStringList list = responseMsg.split('|');
        QRegExp rx("(cid|pid|channel_order|channel_name|total_clients|channel_needed_subscribe_power)\\s*=\\s*(\\S+)");
        int pos = 0;
        foreach(QString item, list)
        {
            channeldetail chan;
            pos = rx.indexIn(item,0);
            while(pos != -1)
            {
                if(rx.cap(1)=="cid")
                    chan.cid = rx.cap(2).toInt();
                if(rx.cap(1)=="pid")
                    chan.pid = rx.cap(2).toInt();
                if(rx.cap(1)=="channel_order")
                    chan.order = rx.cap(2).toInt();
                if(rx.cap(1)=="channel_name")
                    chan.name = rx.cap(2).replace("\\s"," ");;
                if(rx.cap(1)=="total_clients")
                    chan.clientCount = rx.cap(2).toInt();
                if(rx.cap(1)=="channel_needed_subscribe_power")
                    chan.neededSubscribePower = (rx.cap(2).toInt()!=0);
                pos += rx.matchedLength();
                pos = rx.indexIn(item,pos);
            }
            insert(chan.cid, chan);
        }
        return true;
    }
};

#endif // LH_TS3_CHANNELLIST_H
