/**
  \file     LH_CursorAction.h
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andy Bridges
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#ifndef LH_CURSORACTION_H
#define LH_CURSORACTION_H

#include "LH_QtInstance.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QTextEdit.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_int.h"
#include "LH_Qt_bool.h"

#include "LH_CursorReceiver.h"
#include "actionType.h"

#include <QHash>
#include <QList>
#include <QtXml>

const int LH_FLAG_UI = LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK;

class LH_CursorAction : public LH_QtInstance
{
    Q_OBJECT
    actionTypes actionTypes_;

    bool fired;
    bool selected;
    bool waiting;
    int lastStep;
    int delay;

    int cludgeLock;

    LH_CursorReceiver* rcvr_;
protected:
    QString statusCode_;

    LH_Qt_InputState *setup_jump_to_;

    LH_Qt_QStringList *setup_actions_;
    LH_Qt_QString *setup_action_desc_;
    LH_Qt_QStringList *setup_action_type_;

    LH_Qt_QString *setup_action_parameter1_desc_;
    LH_Qt_QString *setup_action_parameter1_str_;
    LH_Qt_int *setup_action_parameter1_int_;
    LH_Qt_QFileInfo *setup_action_parameter1_file_;

    LH_Qt_QString *setup_action_parameter2_desc_;
    LH_Qt_QString *setup_action_parameter2_str_;
    LH_Qt_int *setup_action_parameter2_int_;
    LH_Qt_QFileInfo *setup_action_parameter2_file_;

    LH_Qt_int *setup_action_index_;
    LH_Qt_bool *setup_action_enabled_;

    LH_Qt_QString *setup_action_add_;
    LH_Qt_QString *setup_action_delete_;
    LH_Qt_QString *setup_action_delete_confirm_;

    LH_Qt_QTextEdit *setup_actions_xml_;

public:
    LH_CursorAction();
    const char *userInit();
    static lh_class *classInfo();

    void fire(int = 0);

public slots:
    void stateChangeAction(bool, bool);
    void doJumpTo(QString key,int flags,int value);
    void xmlChanged();
    void actionSelected();
    void actionEdited();
    void actionMoved();
    void newAction();
    void deleteAction();
    void deleteActionCheck();
};

#endif // LH_CURSORACTION_H
