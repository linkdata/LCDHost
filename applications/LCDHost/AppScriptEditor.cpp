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


#include <QtGui>

#include "AppScriptEditor.h"

AppScriptEditor::AppScriptEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    // lineNumberArea = new LineNumberArea(this);

    // connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    // connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    // connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
    connect(this, SIGNAL(textChanged()), this, SLOT(editingInProgress()) );

    // updateLineNumberAreaWidth(0);
    // highlightCurrentLine();
}

void AppScriptEditor::focusOutEvent( QFocusEvent * e )
{
    emit contentStable(true);
    emit contentUnstable(false);
    emit textEdited( toPlainText() );
    return QPlainTextEdit::focusOutEvent( e );
}

void AppScriptEditor::setText(QString s)
{
    setPlainText(s);
    emit contentStable(true);
    emit contentUnstable(false);
}

void AppScriptEditor::editingInProgress()
{
    emit contentUnstable(true);
    emit contentStable(false);
}
