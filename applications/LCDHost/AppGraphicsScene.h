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

#ifndef APPGRAPHICSSCENE_H
#define APPGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>

class MainWindow;

class AppGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit AppGraphicsScene(MainWindow* parent);

    MainWindow* mainWindow() const;

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes () const;

    bool markoutline() const { return markoutline_; }

    void drawBackground( QPainter * painter, const QRectF & rectf );
    void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
    void dropEvent( QGraphicsSceneDragDropEvent * event );

    void keyPressEvent( QKeyEvent * event );

    void render( QPainter * painter,
                 const QRectF & target = QRectF(),
                 const QRectF & source = QRectF(),
                 Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio );

private:
    bool monochrome_;
    bool markoutline_;

};

#endif // APPGRAPHICSSCENE_H
