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

#ifndef APPINSTANCEPREVIEW_H
#define APPINSTANCEPREVIEW_H

#include <QFrame>
#include <QPoint>
#include <QMouseEvent>

#include "AppInstance.h"

class AppInstancePreview : public QFrame
{
    Q_OBJECT

public:
    AppInstancePreview( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~AppInstancePreview();

    QString classId() const { return app_inst ? app_inst->classId() : QString(); }
    AppInstance *appInstance() { return app_inst; }
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void paintEvent( QPaintEvent * );
    void clear();

public slots:
    void clearAppInst();
    void recalcPixmap();
    void previewClass( QString );

private:
    AppInstance *app_inst;
    QPixmap pixmap;
    QRect pixrect;
    QPoint dragStartPosition;
};

#endif // APPINSTANCEPREVIEW_H

