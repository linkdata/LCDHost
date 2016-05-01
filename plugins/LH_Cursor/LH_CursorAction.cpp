/**
  \file     LH_CursorAction.cpp
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

#include "LH_CursorAction.h"
#include <QProcess>
#include <QDesktopServices>

LH_PLUGIN_CLASS(LH_CursorAction)

lh_class *LH_CursorAction::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Cursor",
        "CursorAction",
        "Cursor Action",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };
    return &classInfo;
}

LH_CursorAction::LH_CursorAction() : actionTypes_(this)
{
    waiting = false;
    selected = false;
    fired = false;
    delay = 0;
}

const char *LH_CursorAction::userInit()
{
    if( const char *err = LH_QtInstance::userInit() ) return err;
    hide();
    rcvr_ = new LH_CursorReceiver(this, SLOT(stateChangeAction(bool,bool)));

    setup_jump_to_ = new LH_Qt_InputState(this, "Quick Select", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_jump_to_->setHelp("This optional field allows you to bind a specific key to this coordinate; when that key is pressed the cursor immediately jumps to and selects the coordintes.<br/>"
                               "<br/>"
                               "Note that this functionality is meant more for pages than for menus, but can be used on either."
                               );
    connect( setup_jump_to_, SIGNAL(input(QString,int,int)), this, SLOT(doJumpTo(QString,int,int)) );

    statusCode_ = "OFF";

    setup_actions_ = new LH_Qt_QStringList(this, "Actions", QStringList(),LH_FLAG_UI,lh_type_integer_listbox);
    setup_actions_->setHelp("This box contains the list of actions that will be perfromed when the coordinate is selected. Each action is done in the order shown here.<br/><br/>Select an action from this list to edit or delete it.");
    setup_action_add_ = new LH_Qt_QString(this, "^AddNewAction","Add New Action", LH_FLAG_UI, lh_type_string_button);

    setup_action_desc_ = new LH_Qt_QString(this, "Action Description", "",LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_index_ = new LH_Qt_int(this, "Action Index",0, LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_type_ = new LH_Qt_QStringList(this, "Action Type", actionTypes_.list(),LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);

    setup_action_parameter1_desc_ = new LH_Qt_QString(  this, "^p0_desc","",LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_parameter1_str_  = new LH_Qt_QString(  this, "^p0_str" ,"",LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_parameter1_int_  = new LH_Qt_int(      this, "^p0_int" ,0,LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_parameter1_file_ = new LH_Qt_QFileInfo(this, "^p0_file",QFileInfo(),LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);

    setup_action_parameter2_desc_ = new LH_Qt_QString(  this, "^p1_desc","",LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_parameter2_str_  = new LH_Qt_QString(  this, "^p1_str" ,"",LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_parameter2_int_  = new LH_Qt_int(      this, "^p1_int" ,0,LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);
    setup_action_parameter2_file_ = new LH_Qt_QFileInfo(this, "^p1_file",QFileInfo(),LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);

    setup_action_enabled_= new LH_Qt_bool(this, "^Action Enabled", true, LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI);

    setup_action_delete_ = new LH_Qt_QString(this, "^DeleteAction","Delete Action", LH_FLAG_HIDDEN|LH_FLAG_UI, lh_type_string_button);
    setup_action_delete_confirm_ = new LH_Qt_QString(this, "^DeleteActionConfirm", "Confirm Delete", LH_FLAG_READONLY|LH_FLAG_HIDDEN|LH_FLAG_UI, lh_type_string_button);


    setup_actions_xml_ = new LH_Qt_QTextEdit(this, tr("~"), "<actions>\n</actions>", LH_FLAG_HIDDEN);

    connect( setup_actions_xml_,            SIGNAL(changed()), this, SLOT(xmlChanged()));
    connect( setup_actions_,                SIGNAL(changed()), this, SLOT(actionSelected()));

    connect( setup_action_desc_,            SIGNAL(changed()), this, SLOT(actionEdited()));
    connect( setup_action_index_,           SIGNAL(changed()), this, SLOT(actionMoved()));
    connect( setup_action_type_,            SIGNAL(changed()), this, SLOT(actionEdited()));

    connect( setup_action_parameter1_str_ , SIGNAL(changed()), this, SLOT(actionEdited()));
    connect( setup_action_parameter1_int_ , SIGNAL(changed()), this, SLOT(actionEdited()));
    connect( setup_action_parameter1_file_, SIGNAL(changed()), this, SLOT(actionEdited()));

    connect( setup_action_parameter2_str_ , SIGNAL(changed()), this, SLOT(actionEdited()));
    connect( setup_action_parameter2_int_ , SIGNAL(changed()), this, SLOT(actionEdited()));
    connect( setup_action_parameter2_file_, SIGNAL(changed()), this, SLOT(actionEdited()));

    connect( setup_action_enabled_,         SIGNAL(changed()), this, SLOT(actionEdited()));

    connect( setup_action_delete_,          SIGNAL(changed()), this, SLOT(deleteActionCheck()));
    connect( setup_action_delete_confirm_,  SIGNAL(changed()), this, SLOT(deleteAction()));
    connect( setup_action_add_,             SIGNAL(changed()), this, SLOT(newAction()));

    return 0;
}

void LH_CursorAction::stateChangeAction(bool newSelected, bool newActive)
{
    QString newStatusCode = QString("%1%2").arg(newActive? "ON" : "OFF").arg(newSelected? "_SEL" : "");
    if(statusCode_ != newStatusCode)
    {
        statusCode_ = newStatusCode;
        if(selected!=newSelected)
        {
            selected = newSelected;
            if(!newSelected) fired = false; else
            if(!fired) fire();
        }
    }
}

void LH_CursorAction::fire(int startAt)
{
    fired = true;
    waiting = false;
    qDebug()<<"Fire!";
    QDomDocument actionsXML("actionsXML");
    if(actionsXML.setContent(setup_actions_xml_->value()))
    {
        cursorData cursor_data = rcvr_->data();
        bool cursor_data_dirty = false;
        QDomNode rootNode = actionsXML.firstChild();
        for(int i = startAt; i< rootNode.childNodes().size(); i++)
        {
            lastStep = i;
            QDomNode n = rootNode.childNodes().at(i);
            QDomElement e = n.toElement();
            if(e.attribute("enabled")=="false")
                continue;
            QString typeCode = e.attribute("type");
            actionType action = actionTypes_.at(typeCode);
            if(typeCode=="open")
            {
                QString layout = action.getParameter(e,0);
                if (!layout.contains(":"))
                    layout = QString("%1\\%2").arg(state()->dir_layout).arg(layout);
                static QByteArray ary;
                ary = layout.toUtf8();
                callback(lh_cb_load_layout, ary.data() );
            }else
            if(typeCode=="run")
            {
                QRegExp rx("((?:[^\\s\"]*(?:\"[^\"]*\")?)*)");
                QProcess process;
                QString path = action.getParameter(e,0);
                QFileInfo exe = QFileInfo(path);
                if(!exe.isFile())
                    exe = QFileInfo(QString("%1%2").arg(state()->dir_layout).arg(path));
                QString argsString = action.getParameter(e,1);
                QStringList argsList;
                if(rx.indexIn(argsString) != -1)
                    for(int i=1; i<=rx.captureCount(); i++)
                        if(rx.cap(i)!="")
                            argsList.append(rx.cap(i));
                process.startDetached(exe.absoluteFilePath(),argsList,state()->dir_layout);
            }else
            if(typeCode=="url")
            {
                QDesktopServices::openUrl( action.getParameter(e,0) );
            }else
            if(typeCode=="wait")
            {
                waiting = true;
                delay = action.getParameter(e,0).toInt();
                break;
            }else
            if(typeCode=="move")
            {
                cursor_data.x = action.getParameter(e,0).toInt();
                cursor_data.y = action.getParameter(e,1).toInt();
                cursor_data_dirty = true;
            }else
            if(typeCode=="select")
            {
                cursor_data.x = action.getParameter(e,0).toInt();
                cursor_data.y = action.getParameter(e,1).toInt();
                cursor_data.sendSelect = true;
                cursor_data_dirty = true;
            }else
            if(typeCode=="deselect")
            {
                cursor_data.selState = false;
                cursor_data_dirty = true;
            }else
            if(typeCode=="deactivate")
            {
                cursor_data.active = false;
                cursor_data_dirty = true;
            }else
            if(typeCode=="reselect")
            {
                cursor_data.x = cursor_data.lastSelX2;
                cursor_data.y = cursor_data.lastSelY2;
                cursor_data.sendSelect = true;
                cursor_data_dirty = true;
            } else
                Q_ASSERT(false);
            if (cursor_data_dirty)
                rcvr_->postback(cursor_data);
            cursor_data_dirty = false;
        }
    }
}

void LH_CursorAction::doJumpTo(QString key, int flags, int value)
{
    Q_UNUSED(key);
    Q_UNUSED(flags);
    Q_UNUSED(value);
    QString coord = rcvr_->setup_coordinate_->value().split(';')[0];
    cursorData cursor_data = rcvr_->data();
    cursor_data.x = coord.split(',')[0].toInt();
    cursor_data.y = coord.split(',')[1].toInt();
    cursor_data.sendSelect = true;
    rcvr_->postback(cursor_data);
}

void LH_CursorAction::xmlChanged()
{
    setup_actions_->list().clear();
    QDomDocument actionsXML("actionsXML");
    if(actionsXML.setContent(setup_actions_xml_->value()))
    {
        QDomNode rootNode = actionsXML.firstChild();
        for(int i = 0; i< rootNode.childNodes().size(); i++)
        {
            QDomNode n = rootNode.childNodes().at(i);
            QDomElement e = n.toElement();

            QString desc = (e.attribute("desc")==""? actionTypes_.at(e.attribute("type")).description : e.attribute("desc"));
            if(e.attribute("enabled")=="false") desc+=" [disabled]";

            setup_actions_->list().append(desc);
        }
    }
    setup_actions_->refreshList();
    actionSelected();
}

void LH_CursorAction::actionSelected()
{
    int offFlag = 0;
    if(setup_actions_->list().count()==0) offFlag = LH_FLAG_HIDDEN | LH_FLAG_READONLY;

    setup_action_desc_->setFlags(LH_FLAG_UI | offFlag);
    setup_action_index_->setFlags(LH_FLAG_UI | offFlag);
    setup_action_index_->setMinMax(0,setup_actions_->list().count()-1);

    setup_action_type_->setFlags(LH_FLAG_UI | offFlag);
    setup_action_parameter1_desc_->setFlags(LH_FLAG_READONLY|LH_FLAG_UI | offFlag);
    setup_action_parameter1_str_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_UI | offFlag);
    setup_action_parameter1_int_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_UI | offFlag);
    setup_action_parameter1_file_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_UI | offFlag);
    setup_action_parameter2_desc_->setFlags(LH_FLAG_READONLY|LH_FLAG_UI | offFlag);
    setup_action_parameter2_str_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_UI | offFlag);
    setup_action_parameter2_int_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_UI | offFlag);
    setup_action_parameter2_file_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_UI | offFlag);
    setup_action_delete_->setFlags(LH_FLAG_UI | offFlag);
    setup_action_delete_confirm_->setFlags(LH_FLAG_HIDDEN|LH_FLAG_READONLY|LH_FLAG_UI | offFlag);
    setup_action_enabled_->setFlags(LH_FLAG_UI | offFlag);

    if(setup_actions_->list().count()==0) return;

    QDomDocument actionsXML("actionsXML");
    if(actionsXML.setContent(setup_actions_xml_->value()) && actionsXML.firstChild().childNodes().count()!=0 && setup_actions_->value()!=-1)
    {
        QDomElement e = actionsXML.firstChild().childNodes().at(setup_actions_->value()).toElement();
        QString typeCode = e.attribute("type");

        setup_action_type_->setValue( actionTypes_.indexOf(typeCode) );
        setup_action_desc_->setValue( e.attribute("desc") );
        setup_action_index_->setValue( setup_actions_->value() );

        cursorData cursor_data = rcvr_->data();
        actionTypes_.at(typeCode).displayParameter(0,setup_action_parameter1_desc_,setup_action_parameter1_str_,setup_action_parameter1_int_,setup_action_parameter1_file_, cursor_data, e);
        actionTypes_.at(typeCode).displayParameter(1,setup_action_parameter2_desc_,setup_action_parameter2_str_,setup_action_parameter2_int_,setup_action_parameter2_file_, cursor_data, e);

        setup_action_enabled_->setValue( e.attribute("enabled")!="false" );
    }
}

void LH_CursorAction::actionEdited()
{
    if(setup_actions_->list().count()==0) return;
    actionType at = actionTypes_.at(setup_action_type_->value());

    //acquire valid parameter values (blank values return in parameter is invalid)
    QStringList paramValues = QStringList();
    paramValues.append( at.getParameterValue(0,setup_action_parameter1_str_,setup_action_parameter1_int_,setup_action_parameter1_file_));
    paramValues.append( at.getParameterValue(1,setup_action_parameter2_str_,setup_action_parameter2_int_,setup_action_parameter2_file_));

    //in case the type has changed, update the parameter visibility
    cursorData cursor_data = rcvr_->data();
    at.displayParameter(0,setup_action_parameter1_desc_,setup_action_parameter1_str_,setup_action_parameter1_int_,setup_action_parameter1_file_, cursor_data);
    at.displayParameter(1,setup_action_parameter2_desc_,setup_action_parameter2_str_,setup_action_parameter2_int_,setup_action_parameter2_file_, cursor_data);

    //Update the caption in case the description or enabled state has changed
    QString desc = (setup_action_desc_->value()==""? at.description : setup_action_desc_->value());
    if(!setup_action_enabled_->value()) desc+=" [disabled]";
    setup_actions_->list()[setup_actions_->value()] = desc;
    setup_actions_->refreshList();

    //Update the xml data
    QStringList xmlLines = setup_actions_xml_->value().split("\n");
    xmlLines[setup_actions_->value()+1] = at.generateXML(setup_action_enabled_->value(),setup_action_desc_->value(),paramValues);
    setup_actions_xml_->setValue(xmlLines.join("\n"));

    //reset the deletion stuff
    setup_action_delete_->setFlag(LH_FLAG_READONLY, false);
    setup_action_delete_confirm_->setFlag(LH_FLAG_READONLY, true);
    setup_action_delete_confirm_->setFlag(LH_FLAG_HIDDEN, true);
}

void LH_CursorAction::actionMoved()
{
    int curPos = setup_actions_->value();
    int newPos = setup_action_index_->value();

    QStringList xmlLines = setup_actions_xml_->value().split("\n");
    xmlLines.move(curPos+1,newPos+1);
    setup_actions_xml_->setValue(xmlLines.join("\n"));

    xmlChanged();
    setup_actions_->setValue(newPos);
    actionSelected();
}

void LH_CursorAction::newAction()
{
    actionType at = actionTypes_.at("deselect");
    QString strXML =  at.generateXML(false,"",QStringList());

    QStringList xmlLines = setup_actions_xml_->value().split("\n");
    xmlLines.insert(xmlLines.count()-1, strXML);
    setup_actions_xml_->setValue(xmlLines.join("\n"));

    xmlChanged();
    setup_actions_->setValue(setup_actions_->list().count()-1);
    actionSelected();
}

void LH_CursorAction::deleteActionCheck()
{
    setup_action_delete_->setFlag(LH_FLAG_READONLY, true);
    setup_action_delete_confirm_->setFlag(LH_FLAG_READONLY, false);
    setup_action_delete_confirm_->setFlag(LH_FLAG_HIDDEN, false);
}

void LH_CursorAction::deleteAction()
{
    int selIndex = setup_actions_->value();

    QStringList xmlLines = setup_actions_xml_->value().split("\n");
    xmlLines.removeAt(selIndex+1);
    setup_actions_xml_->setValue(xmlLines.join("\n"));

    xmlChanged();
    if(selIndex>=setup_actions_->list().length()) selIndex--;
    setup_actions_->setValue(selIndex);
    actionSelected();
}

