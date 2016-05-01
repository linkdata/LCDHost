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


#include <QNetworkInterface>
#include <QList>
#include <QDateTime>
#include "NetworkEngine_mac.h"

// #import <Foundation/Foundation.h>
#include <errno.h>
#include <sys/sysctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <net/if_types.h>

NetworkEngine_mac::NetworkEngine_mac()
{

}

void NetworkEngine_mac::sample( lh_netdata *data, QString filter )
{
    // TODO implement filtering
    static bool ignore_PPP = false;
    Q_UNUSED(filter);

    size_t len;
    int mib[] = {
        CTL_NET,
        PF_ROUTE,
        0,
        0,
        NET_RT_IFLIST2,
        0
    };

    if( sysctl(mib, 6, NULL, &len, NULL, 0) < 0 )
    {
        qDebug() << "NetworkEngine_mac::list(): sysctl().1" << strerror(errno);
        return;
    }

    char *buf = (char *)malloc(len);
    if( buf == NULL ) return ;

    if( sysctl(mib, 6, buf, &len, NULL, 0) < 0 )
    {
        free( buf );
        qDebug() << "NetworkEngine_mac::list(): sysctl().2" << strerror(errno);
        return ;
    }

    char *lim = buf + len;
    char *next = NULL;
    u_int64_t totalibytes = 0;
    u_int64_t totalobytes = 0;
    for( next = buf; next < lim; )
    {
        struct if_msghdr *ifm = (struct if_msghdr *)next;
        next += ifm->ifm_msglen;

        if (ifm->ifm_type == RTM_IFINFO2)
        {
            struct if_msghdr2 *if2m = (struct if_msghdr2 *)ifm;

            if(if2m->ifm_data.ifi_type==IFT_ETHER)  /* If we've seen any ethernet traffic, */
                    ignore_PPP=true; 		/* ignore any PPP traffic (PPPoE or VPN) */

            if( ((if2m->ifm_data.ifi_type!=IFT_LOOP)   /* do not count loopback traffic */
               && !(ignore_PPP && if2m->ifm_data.ifi_type==IFT_PPP))) /* or VPN/PPPoE */
            {
                totalibytes += if2m->ifm_data.ifi_ibytes;
                totalobytes += if2m->ifm_data.ifi_obytes;
            }
        }
    }

    free( buf );

    if( data )
    {
        data->when = QDateTime::currentDateTime().toMSecsSinceEpoch();
        data->device = 0;
        data->in = totalibytes;
        data->out = totalobytes;
    }
}

QStringList NetworkEngine_mac::list()
{
    QStringList list;
    foreach( QNetworkInterface i, QNetworkInterface::allInterfaces() )
    {
        list.append( i.name() );
    }

    return list;
}
