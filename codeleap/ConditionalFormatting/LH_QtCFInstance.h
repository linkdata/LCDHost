/**
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010-2011 Andy Bridges

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

#ifndef LH_QTCFINSTANCE_H
#define LH_QTCFINSTANCE_H

#include <QtGlobal>
#include <QImage>
// #include <QtXml>

#include "LH_QtPlugin.h"
#include "LH_QtObject.h"
#include "LH_QtInstance.h"

#include "LH_Qt_bool.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QTextEdit.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_QFont.h"
#include "LH_Qt_QFileInfo.h"

#include "cf_sources_targets.h"

#ifdef LH_CF_LIBRARY
# define LH_CF_EXPORT Q_DECL_EXPORT
#else
# define LH_CF_EXPORT Q_DECL_IMPORT
#endif

class cf_rule;

class LH_CF_EXPORT LH_QtCFInstance : public LH_QtInstance
{
    Q_OBJECT

    enum cf_rule_edit_mode
    {
        Default = -1,
        None = 0,
        Existing = 1,
        New = 2
    };

    cf_source_list sources_;
    cf_target_list targets_;

    //QDomElement createNode(QDomDocument doc, QString tagName, QString nodeValue = "", QString attributeName = "", QString attributeValue = "");

    bool cf_initialized_;
    bool cf_applying_rules_;
    void cf_initialize();

    void add_cf_source(QString name, LH_QtSetupItem *si);

    //QString getTargetValue(int targetId);

    cf_rule_edit_mode cf_rule_editing_;
    void cf_set_edit_controls_visibility(cf_rule_edit_mode editMode = Default);

    bool watching_non_setup_item_;

protected:
    LH_Qt_bool        *setup_cf_enabled_;

    LH_Qt_QString     *setup_cf_state_;
    LH_Qt_bool        *setup_cf_visibility_;

    LH_Qt_QTextEdit   *setup_cf_XML_;
    LH_Qt_QStringList *setup_cf_rules_;

    LH_Qt_QString     *setup_cf_new_;
    LH_Qt_QString     *setup_cf_save_;
    LH_Qt_QString     *setup_cf_cancel_;
    LH_Qt_QString     *setup_cf_delete_;
    LH_Qt_QString     *setup_cf_move_up_;
    LH_Qt_QString     *setup_cf_move_down_;
    LH_Qt_QString     *setup_cf_copy_;
    LH_Qt_QString     *setup_cf_paste_;

    void cf_source_notify(QString name, QString value, int index = 0, int count = 1);

    void add_cf_source(LH_QtSetupItem *si);
    void add_cf_source(QString name);
    void add_cf_target(LH_QtSetupItem *si);

    void cf_set_rules(QString rulesXML, bool enable_cf = true);

public:
    LH_Qt_QStringList *setup_cf_source_;
    LH_Qt_QStringList *setup_cf_source_mode_;
    LH_Qt_QStringList *setup_cf_test_;
    LH_Qt_QString     *setup_cf_testValue1_;
    LH_Qt_QString     *setup_cf_testValue2_;
    LH_Qt_QStringList *setup_cf_target_;
    LH_Qt_QColor      *setup_cf_newValue_Color_;
    LH_Qt_QFont       *setup_cf_newValue_Font_;
    LH_Qt_QString     *setup_cf_newValue_String_;
    LH_Qt_QFileInfo   *setup_cf_newValue_File_;
    LH_Qt_bool        *setup_cf_newValue_Bool_;
    LH_Qt_int         *setup_cf_newValue_Int_;
    LH_Qt_float       *setup_cf_newValue_Float_;

    LH_QtCFInstance(LH_QtObject* parent = 0);
    const char* userInit();
    int notify(int n,void* p);

    cf_source_list sources() { return sources_; }
    cf_target_list targets() { return targets_; }

protected slots:
    void cf_enabled_changed();
    void cf_source_changed();
    void cf_target_changed();
    void cf_condition_changed();
    void cf_save_rule();
    void cf_cancel_edit_rule();
    void cf_XML_changed();
    void cf_rules_changed();
    void cf_delete_rule();
    void cf_new_rule();
    void cf_state_value_updated();
    void cf_update_visibility();
    void cf_apply_rules(bool allowRender = true);

    void cf_copy_rules();
    void cf_paste_rules();
    void cf_move_rule_up();
    void cf_move_rule_down();
};

#endif // LH_QTCFINSTANCE_H
