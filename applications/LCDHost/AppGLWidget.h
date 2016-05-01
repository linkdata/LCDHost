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

#ifndef APPGLWIDGET_H
#define APPGLWIDGET_H

#ifdef QT_OPENGL_LIB
#include <QGLWidget>

class AppGLWidget : public QGLWidget
{
    Q_OBJECT
public:
    AppGLWidget( QGLContext * context, QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0 )
        : QGLWidget(context,parent,shareWidget,f) {}
    AppGLWidget( const QGLFormat & format, QWidget * parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0 )
        : QGLWidget(format,parent,shareWidget,f) {}

    void initializeGL()
    {
        // Set up the rendering context, define display lists etc.:
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_TEXTURE_2D );
    }

    void resizeGL(int w, int h)
    {
        // setup viewport, projection etc.:
        glViewport(0, 0, (GLint)w, (GLint)h);
        // glFrustum(...);
    }

    void paintGL()
    {
    }
};
#else
#include <QObject>
class AppGLWidget : public QObject
{
    Q_OBJECT
};
#endif

#endif // APPGLWIDGET_H
