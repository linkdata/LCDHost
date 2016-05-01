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

#include <QPainter>
#include <QPen>
#include <QFont>
#include <QFontMetrics>
#include "AppPluginItemDelegate.h"
#include "AppLibrary.h"

AppPluginItemDelegate::AppPluginItemDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
}

void AppPluginItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QRect r = option.rect;
    QFont titleFont;
    QFont descFont;

    titleFont.setBold(true);
    descFont.setItalic(true);

    //Color: #C4C4C4
    QPen linePen(QColor::fromRgb(211,211,211), 1, Qt::SolidLine);

    //Color: #005A83
    QPen lineMarkedPen(QColor::fromRgb(0,90,131), 1, Qt::SolidLine);

    //Color: #333
    QPen fontPen(QColor::fromRgb(51,51,51), 1, Qt::SolidLine);

    //Color: #fff
    QPen fontMarkedPen(Qt::white, 1, Qt::SolidLine);

    if(option.state & QStyle::State_Selected)
    {
        QLinearGradient gradientSelected(r.left(),r.top(),r.left(),r.height()+r.top());
        gradientSelected.setColorAt(0.0, QColor::fromRgb(119,213,247));
        gradientSelected.setColorAt(0.9, QColor::fromRgb(27,134,183));
        gradientSelected.setColorAt(1.0, QColor::fromRgb(0,120,174));
        painter->setBrush(gradientSelected);
        painter->drawRect(r);

        //BORDER
        painter->setPen(lineMarkedPen);
        painter->drawLine(r.topLeft(),r.topRight());
        painter->drawLine(r.topRight(),r.bottomRight());
        painter->drawLine(r.bottomLeft(),r.bottomRight());
        painter->drawLine(r.topLeft(),r.bottomLeft());

        painter->setPen(fontMarkedPen);

    }
    else
    {
        //BACKGROUND
        //ALTERNATING COLORS
        painter->setBrush( (index.row() % 2) ? Qt::white : QColor(248,248,248) );
        painter->drawRect(r);

        //BORDER
        painter->setPen(linePen);
        painter->drawLine(r.topLeft(),r.topRight());
        painter->drawLine(r.topRight(),r.bottomRight());
        painter->drawLine(r.bottomLeft(),r.bottomRight());
        painter->drawLine(r.topLeft(),r.bottomLeft());

        painter->setPen(fontPen);
    }

    //GET TITLE, DESCRIPTION AND ICON

    QIcon ic = QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
    QString title = index.data(Qt::DisplayRole).toString();
    QString description = index.data(Qt::UserRole + 1).toString();
    QString version = index.data(Qt::UserRole + 2).toString();
    int my_rev = index.data(Qt::UserRole + 3 ).toInt();
    int web_rev = index.data(Qt::UserRole + 4).toInt();
    QString error = index.data(Qt::UserRole + 5).toString();

    if( !ic.isNull() )
    {
        // ICON
        r = option.rect.adjusted(5, 10, -10, -10);
        ic.paint(painter, r, Qt::AlignVCenter|Qt::AlignLeft);
    }

    // VERSION
    r = option.rect.adjusted(28, option.rect.height()/7, -10, 0);
    painter->setFont( QFont() );
    if( web_rev < 1 ) painter->setPen( Qt::lightGray );
    else
    {
        if( web_rev == my_rev ) painter->setPen( Qt::green );
        else if( web_rev > my_rev ) painter->setPen( Qt::red );
        else painter->setPen( Qt::blue );
    }
    QRect vr;
    painter->drawText(r.left(), r.top(), r.width(), r.height(),
                      Qt::AlignRight, version, &vr);

    // ERROR
    if( !error.isEmpty() )
    {
        r = option.rect.adjusted(28, option.rect.height()/7, -20-vr.width(), 0);
        painter->setFont( descFont );
        painter->setPen( Qt::red );
        painter->drawText(r.left(), r.top(), r.width(), r.height(),
                          Qt::AlignTop|Qt::AlignRight, error, &r);
    }

    // TITLE
    r = option.rect.adjusted(28, option.rect.height()/7, -20-vr.width(), 0);
    painter->setFont( titleFont );
    painter->setPen( Qt::black );
    painter->drawText(r.left(), r.top(), r.width(), r.height(),
                      Qt::AlignTop|Qt::AlignLeft, title, &r);

    // DESCRIPTION
    r = option.rect.adjusted(28, option.rect.height()/3 + option.rect.height()/7, -20-vr.width(), 0);
    painter->setFont( descFont );
    painter->drawText(r.left(), r.top(), r.width(), r.height(),
                      Qt::AlignTop|Qt::AlignLeft, description, &r);
}

QSize AppPluginItemDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    static int triple = -1;
    Q_UNUSED( option );
    Q_UNUSED( index );
    if( triple == -1 )
    {
        QFont f;
        QFontMetrics fm( f );
        triple = fm.lineSpacing() * 3;
    }
    return QSize(200, triple);
}
