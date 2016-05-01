/**
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010-2011 Andy Bridges

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#include <QDebug>
#include "cf_rule.h"

// =================================================================================
// ----- cf_rule_condition ---------------------------------------------------------
// =================================================================================

cf_rule_condition::cf_rule_condition(QDomNode condNode, QObject* parent): QObject(parent)
{
    test_ = condNode.toElement().attribute("test");

    QDomElement srcNode = condNode.firstChildElement("source");
    source_ = srcNode.text();

    if(srcNode.hasAttribute("mode"))
        mode_ = srcNode.attribute("mode");
    else
        mode_ = "Value";

    for (QDomElement valNode = condNode.firstChildElement("value"); !valNode.isNull(); valNode = valNode.nextSiblingElement("value"))
        values_.append(valNode.text());
}

cf_rule_condition::cf_rule_condition(LH_QtCFInstance *sender, QObject* parent): QObject(parent)
{
    test_ = sender->setup_cf_test_->valueText();
    source_ = sender->setup_cf_source_->valueText();
    mode_ = sender->setup_cf_source_mode_->valueText();
    values_.append(sender->setup_cf_testValue1_->value());
    if(!sender->setup_cf_testValue2_->hasFlag(LH_FLAG_HIDDEN))
         values_.append(sender->setup_cf_testValue2_->value());
}

QString cf_rule_condition::description()
{
    QString cf_cond_desc = "{modeA}[{source}]{modeB} {test} {values}";

    cf_cond_desc.replace("{test}", test_);
    cf_cond_desc.replace("{source}", source_);
    if(mode_!="Value")
    {
        cf_cond_desc.replace("{modeA}", mode_ + "(");
        cf_cond_desc.replace("{modeB}", ")");
    } else {
        cf_cond_desc.replace("{modeA}", "");
        cf_cond_desc.replace("{modeB}", "");
    }

    for (int i=0; i<values_.count(); i++)
    {
        cf_cond_desc.replace("{and_val}", QString(" AND \"%1\"").arg(values_[i]));
        cf_cond_desc.replace("{values}", QString("\"%1\"{and_val}").arg(values_[i]));
    }
    cf_cond_desc.replace("{and_val}", "");

    return cf_cond_desc;
}

bool cf_rule_condition::evaluate(cf_source_list sources, bool *ok)
{
    bool verboseTesting = false;
    if(source_=="")
        return false;

    if(!sources.contains(source_))
        return false;

    QString sourceVal = sources[source_]->value(mode_);

    bool allNumeric = true;

    QRegExp rx("^\\s*(-?[0-9]*(?:,?[0-9]{3})*(?:\\.[0-9]*)?)");
    float sourceValF = 0; float value1F = 0; float value2F = 0;

    if (rx.indexIn(sourceVal)!= -1)
        sourceValF = rx.cap(1).toFloat(&allNumeric);
    else
        sourceValF = sourceVal.toFloat(&allNumeric);

    if(allNumeric) {
        if (rx.indexIn(values_[0])!= -1)
            value1F = rx.cap(1).toFloat(&allNumeric);
        else
            value1F = values_[0].toFloat(&allNumeric);
    }

    if(ok!=NULL) *ok = false;

    if(test_=="Equals") {
        if(ok!=NULL) *ok = true;
        return (allNumeric? sourceValF == value1F : sourceVal == values_[0]);
    }
    if(test_=="Does not equal") {
        if(ok!=NULL) *ok = true;
        return (allNumeric? sourceValF != value1F : sourceVal != values_[0]);
    }
    if(test_=="Less than") {
        if(ok!=NULL) *ok = true;
        if(!allNumeric)
            {if(verboseTesting) qDebug()<<"Test requires numerical values: " << test_;}
        else
            return (sourceValF < value1F);
        return false;
    }
    if(test_=="Less than or equal") {
        if(ok!=NULL) *ok = true;
        if(!allNumeric)
            {if(verboseTesting) qDebug()<<"Test requires numerical values: " << test_;}
        else
            return (sourceValF <= value1F);
        return false;
    }
    if(test_=="Greater than") {
        if(ok!=NULL) *ok = true;
        if(!allNumeric)
            {if(verboseTesting) qDebug()<<"Test requires numerical values: " << test_;}
        else
            return (sourceValF > value1F);
        return false;
    }
    if(test_=="Greater than or equal") {
        if(ok!=NULL) *ok = true;
        if(!allNumeric)
            {if(verboseTesting) qDebug()<<"Test requires numerical values: " << test_;}
        else
            return (sourceValF >= value1F);
        return false;
    }
    if(test_=="Between") {
        if(ok!=NULL) *ok = true;
        if(allNumeric) {
            if (rx.indexIn(values(1))!= -1)
                value2F = rx.cap(1).toFloat(&allNumeric);
            else
                value2F = values(1).toFloat(&allNumeric);
        }
        if(!allNumeric)
            {if(verboseTesting) qDebug()<<"Test requires numerical values: " << test_;}
        else
            return (sourceValF >= value1F) && (sourceValF <= value2F);
        return false;
    }
    if(test_=="Is not between") {
        if(ok!=NULL) *ok = true;
        if(allNumeric) {
            if (rx.indexIn(values(1))!= -1)
                value2F = rx.cap(1).toFloat(&allNumeric);
            else
                value2F = values(1).toFloat(&allNumeric);
        }
        if(!allNumeric)
            {if(verboseTesting) qDebug()<<"Test requires numerical values: " << test_;}
        else
            return !((sourceValF >= value1F) && (sourceValF <= value2F));
        return false;
    }
    if(test_=="Begins with") {
        if(ok!=NULL) *ok = true;
        return sourceVal.startsWith(values_[0], Qt::CaseInsensitive);
    }
    if(test_=="Does not begin with") {
        if(ok!=NULL) *ok = true;
        return !sourceVal.startsWith(values_[0], Qt::CaseInsensitive);
    }
    if(test_=="Ends with") {
        if(ok!=NULL) *ok = true;
        return sourceVal.endsWith(values_[0], Qt::CaseInsensitive);
    }
    if(test_=="Does not end with") {
        if(ok!=NULL) *ok = true;
        return !sourceVal.endsWith(values_[0], Qt::CaseInsensitive);
    }
    if(test_=="Contains") {
        if(ok!=NULL) *ok = true;
        return sourceVal.contains(values_[0], Qt::CaseInsensitive);
    }
    if(test_=="Does not contain") {
        if(ok!=NULL) *ok = true;
        return !sourceVal.contains(values_[0], Qt::CaseInsensitive);
    }

    qWarning() << "Unhandled cf test type: " << test_;
    return false;
}

QDomNode cf_rule_condition::toXmlNode(QDomDocument doc)
{
    // Condition Node
    QDomElement condNode = doc.createElement("condition");
    condNode.setAttribute("test", test_);

    QDomElement srcNode = doc.createElement("source");
    srcNode.appendChild(doc.createTextNode(source_));
    if(mode_!="Value")
        srcNode.setAttribute("mode", mode_);

    condNode.appendChild(srcNode);

    for(int i=0; i<values_.length(); i++)
    {
        QDomElement valNode = doc.createElement("value");
        valNode.appendChild(doc.createTextNode(values_[i]));
        valNode.setAttribute("id", QString::number(i+1));
        condNode.appendChild(valNode);
    }

    return condNode;
}


// =================================================================================
// ----- cf_rule_action ------------------------------------------------------------
// =================================================================================

cf_rule_action::cf_rule_action(QDomNode actNode, QObject* parent): QObject(parent)
{
    type_ = actNode.toElement().attribute("type");
}

cf_rule_action::cf_rule_action(QString type, QObject* parent): QObject(parent)
{
    type_ = type;
}

QDomNode cf_rule_action::toXmlNode(QDomDocument doc)
{
    QDomElement actNode = doc.createElement("action");
    actNode.setAttribute("type", type_);
    return actNode;
}

// =================================================================================
// ----- cf_rule_action_property ---------------------------------------------------
// =================================================================================

cf_rule_action_property::cf_rule_action_property(QDomNode actNode, QObject* parent): cf_rule_action(actNode, parent)
{
    target_ = actNode.firstChildElement("target").text();
    value_ = actNode.firstChildElement("value").text();
}

cf_rule_action_property::cf_rule_action_property(LH_QtCFInstance *sender, QObject* parent): cf_rule_action("property", parent)
{
    target_ = sender->setup_cf_target_->valueText();
    switch(sender->targets()[target_]->type())
    {
    case lh_type_integer_color:
        value_ = QString::number(sender->setup_cf_newValue_Color_->value().rgba(), 16).toUpper();
        break;
    case lh_type_string_font:
        value_ = sender->setup_cf_newValue_Font_->value().toString();
        break;
    case lh_type_string:
        value_ = sender->setup_cf_newValue_String_->value();
        break;
    case lh_type_integer_boolean:
        value_ = (sender->setup_cf_newValue_Bool_->value()?"true":"false");
        break;
    case lh_type_string_filename:
        value_ = getRelativeFilePath(sender->setup_cf_newValue_File_->value(), sender->state()->dir_layout);
        break;
    case lh_type_integer:
        value_ = QString::number(sender->setup_cf_newValue_Int_->value());
        break;
    case lh_type_fraction:
        value_ = QString::number(sender->setup_cf_newValue_Float_->value());
        break;
    default:
        qWarning() << "Unable to acquire value for target: unrecognised type (" << sender->targets()[target_]->type() << ")";
        value_ = "";
    }
}

QString cf_rule_action_property::description()
{
    QString cf_act_desc = "Set [{target}] to {new_value}";
    cf_act_desc.replace("{target}", target_);
    cf_act_desc.replace("{new_value}", value_);
    return cf_act_desc;
}

void cf_rule_action_property::execute(LH_QtCFInstance* sender, cf_target_list targets)
{
    setTargetValue(sender, targets, false);
}

void cf_rule_action_property::edit(LH_QtCFInstance* sender, cf_target_list targets)
{
    sender->setup_cf_target_->setValue(target_);
    setTargetValue(sender, targets, true);
}

bool cf_rule_action_property::setTargetValue(LH_QtCFInstance* sender, cf_target_list targets, bool setPlaceholder)
{
    QFont font;
    QColor color;
    bool b;
    int i;
    float f;
    QFileInfo file;
    LH_QtSetupItem* target = targets[target_];
    switch(target->type())
    {
    case lh_type_integer_color:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_Color_;
        color = QColor::fromRgba(value_.toUInt(NULL, 16));
        if(((LH_Qt_QColor*)target)->value() != color )
        {
            ((LH_Qt_QColor*)target)->setValue(color);
            if(!setPlaceholder) return true;
        }
        break;
    case lh_type_string_font:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_Font_;
        if(font.fromString(value_))
            if(((LH_Qt_QFont*)target)->value() != font )
            {
                ((LH_Qt_QFont*)target)->setValue(font);
                if(!setPlaceholder) return true;
            }
        break;
    case lh_type_string:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_String_;
        if(((LH_Qt_QString*)target)->value() != value_ )
        {
            ((LH_Qt_QString*)target)->setValue(value_);
            if(!setPlaceholder) return true;
        }
        break;
    case lh_type_integer_boolean:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_Bool_;
        b = (value_=="true");
        if(((LH_Qt_bool*)target)->value() != b )
        {
            ((LH_Qt_bool*)target)->setValue(b);
            if(!setPlaceholder) return true;
        }
        break;
    case lh_type_string_filename:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_File_;
        file = QFileInfo(value_);
        if(!file.isFile())
            file = QFileInfo(QString("%1%2").arg(sender->state()->dir_layout).arg(value_));

        if(file.isFile())
            if(((LH_Qt_QFileInfo*)target)->value() != file )
            {
                ((LH_Qt_QFileInfo*)target)->setValue(file);
                if(!setPlaceholder) return true;
            }
        break;
    case lh_type_integer:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_Int_;
        i = value_.toInt();
        if(((LH_Qt_int*)target)->value() != i )
        {
            ((LH_Qt_int*)target)->setValue(i);
            if(!setPlaceholder) return true;
        }
        break;
    case lh_type_fraction:
        if(setPlaceholder)
            target = sender->setup_cf_newValue_Float_;
        f = value_.toFloat();
        if(((LH_Qt_float*)target)->value() != f )
        {
            ((LH_Qt_float*)target)->setValue(f);
            if(!setPlaceholder) return true;
        }
        break;
    default:
        qWarning() << "Unhandled cf target type: " << target->type();
    }
    return false;
}

QDomNode cf_rule_action_property::toXmlNode(QDomDocument doc)
{
    QDomNode actNode = cf_rule_action::toXmlNode(doc);
    QDomElement targNode = doc.createElement("target");
    targNode.appendChild(doc.createTextNode(target_));
    actNode.appendChild(targNode);
    QDomElement valNode = doc.createElement("value");
    valNode.appendChild(doc.createTextNode(value_));
    actNode.appendChild(valNode);
    return actNode;
}

// =================================================================================
// ----- cf_rule -------------------------------------------------------------------
// =================================================================================

cf_rule::cf_rule( QDomNode ruleNode, QObject *parent ) : QObject(parent)
{
    QDomNode conditionsNode = ruleNode.firstChildElement("conditions");
    Q_ASSERT(conditionsNode.childNodes().size()>0);
    for(int i=0; i<conditionsNode.childNodes().size(); i++)
        conditions.append(new cf_rule_condition(conditionsNode.childNodes().at(i), this));
    Q_ASSERT(conditions.length()>0);

    QDomNode actionsNode = ruleNode.firstChildElement("actions");
    Q_ASSERT(actionsNode.childNodes().size()>0);
    for(int i=0; i<actionsNode.childNodes().size(); i++)
    {
        QDomNode actNode = actionsNode.childNodes().at(i);
        if(actNode.toElement().attribute("type")=="property")
            actions.append(new cf_rule_action_property(actNode, this));
        else
            qWarning() << QString("Unknown Action: \"%1").arg(actNode.toElement().attribute("type"));
    }
    Q_ASSERT(actions.length()>0);
}

cf_rule::cf_rule( LH_QtCFInstance *sender, QObject *parent ) : QObject(parent)
{
    conditions.append(new cf_rule_condition(sender, this));
    actions.append(new cf_rule_action_property(sender, this));
}

QString cf_rule::description()
{
    return QString("%1: %2").arg(conditions[0]->description()).arg(actions[0]->description());
}

bool cf_rule::apply(LH_QtCFInstance* sender, cf_source_list sources_, cf_target_list targets_)
{
    if(!actions[0]->isValid(sender))
        return false;

    if(!conditions[0]->evaluate(sources_))
        return false;

    actions[0]->execute(sender, targets_);
    return true;
}

QDomNode cf_rule::toXmlNode(QDomDocument doc)
{
    QDomElement ruleNode = doc.createElement("rule");

    QDomElement condsNode = doc.createElement("conditions");
    condsNode.appendChild(conditions[0]->toXmlNode(doc));
    ruleNode.appendChild(condsNode);


    QDomElement actsNode = doc.createElement("actions");
    actsNode.appendChild(actions[0]->toXmlNode(doc));
    ruleNode.appendChild(actsNode);

    return ruleNode;
}
