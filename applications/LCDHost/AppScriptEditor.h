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

#ifndef APPSCRIPTEDITOR_H
#define APPSCRIPTEDITOR_H

#include <QObject>
#include <QPlainTextEdit>

class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QFocusEvent;

class AppScriptEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    AppScriptEditor(QWidget *parent = 0);

    // void lineNumberAreaPaintEvent(QPaintEvent *event);
    // int lineNumberAreaWidth();
    // void setReadOnly( bool ro ) { QPlainTextEdit::setReadOnly(ro); updateLineNumberAreaWidth(0); update(); }

signals:
    void textEdited(QString);
    void contentStable(bool);
    void contentUnstable(bool);

protected:
    void focusOutEvent( QFocusEvent * e );
    // void resizeEvent( QResizeEvent *event );

private slots:
    void setText(QString);
    void editingInProgress();
    // void updateLineNumberAreaWidth(int newBlockCount);
    // void highlightCurrentLine();
    // void updateLineNumberArea(const QRect &, int);

private:
    // QWidget *lineNumberArea;
};

#endif // APPSCRIPTEDITOR_H
