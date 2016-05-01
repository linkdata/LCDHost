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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

enum potionEffects
{
    POT_HEALTH = 1,
    POT_MANA = 2,
    POT_STR = 4,
    POT_DEX = 8,
    POT_WIL = 16,
    POT_MAG = 32,
    POT_INT = 64,
    POT_CUN = 128
};

struct potion
{
    QString name;
    potionEffects effects;
    int amount;
    bool isPercent;
    bool canBoost;
    int price;
};

struct intAttribute
{
    uint base;
    uint current;
};

struct fltAttribute
{
    float base;
    float current;
};

class character
{
    char* charName;
    char* charRace;
public:
    intAttribute strength;
    intAttribute dexterity;
    intAttribute willpower;
    intAttribute magic;
    intAttribute cunning;
    intAttribute intelligence;
    fltAttribute health;
    fltAttribute mana;
    qlonglong gold;


    character()
    {
        charName = 0;
        charRace = 0;
    }

    void setName( QString n)
    {
        int len = n.length();
        if(charName)
            delete[] charName;
        charName = new char[len+1];
        for(int i=0; i<len; i++)
            charName[i] = n.toUtf8().at(i);
        charName[len] = '\0';
    }
    QString name()
    {
        QString nameVal = QString::fromUtf8(charName);
        int p = nameVal.indexOf(QChar('\0'), Qt::CaseInsensitive);
        return nameVal.left(p-1);
    }

    void setRace( QString n)
    {
        int len = n.length();
        if(charRace)
            delete[] charRace;
        charRace = new char[len+1];
        for(int i=0; i<len; i++)
            charRace[i] = n.toAscii().at(i);
        charRace[len] = '\0';
    }
    QString race()
    {
        QString raceVal = QString::fromAscii(charRace);
        int p = raceVal.indexOf(QChar('\0'), Qt::CaseInsensitive);
        return raceVal.left(p-1);
    }

    void resetAllStats()
    {
        strength.current = strength.base;
        dexterity.current = dexterity.base;
        willpower.current = willpower.base;
        magic.current = magic.base;
        cunning.current = cunning.base;
        intelligence.current = intelligence.base;
        health.current = health.base;
        mana.current = mana.base;
    }

    void heal()
    {
        if(health.current < health.base)
            health.current = health.base;
    }

    void recharge()
    {
        if(mana.current < mana.base)
            mana.current = mana.base;
    }

    void drink(potion p)
    {
        if(this->gold >= p.price)
        {
            this->gold -= p.price;

            if((p.effects & POT_HEALTH)==POT_HEALTH)
            {
                float val = (p.isPercent? p.amount * this->health.base / 100.0: p.amount);
                this->health.current += val;
                if(!p.canBoost && this->health.current > this->health.base)
                    this->health.current = this->health.base;
            }

            if((p.effects & POT_MANA)==POT_MANA)
            {
                float val = (p.isPercent? p.amount * this->mana.base / 100.0: p.amount);
                this->mana.current += val;
                if(!p.canBoost && this->mana.current > this->mana.base)
                    this->mana.current = this->mana.base;
            }

            if((p.effects & POT_STR)==POT_STR)
            {
                int val = (p.isPercent? qRound(p.amount * this->strength.base / 100.0): p.amount);
                this->strength.current += val;
                if(!p.canBoost && this->strength.current > this->strength.base)
                    this->strength.current = this->strength.base;
            }

            if((p.effects & POT_DEX)==POT_DEX)
            {
                int val = (p.isPercent? qRound(p.amount * this->dexterity.base / 100.0): p.amount);
                this->dexterity.current += val;
                if(!p.canBoost && this->dexterity.current > this->dexterity.base)
                    this->dexterity.current = this->dexterity.base;
            }

            if((p.effects & POT_WIL)==POT_WIL)
            {
                int val = (p.isPercent? qRound(p.amount * this->willpower.base / 100.0): p.amount);
                this->willpower.current += val;
                if(!p.canBoost && this->willpower.current > this->willpower.base)
                    this->willpower.current = this->willpower.base;
            }

            if((p.effects & POT_MAG)==POT_MAG)
            {
                int val = (p.isPercent? qRound(p.amount * this->magic.base / 100.0): p.amount);
                this->magic.current += val;
                if(!p.canBoost && this->magic.current > this->magic.base)
                    this->magic.current = this->magic.base;
            }

            if((p.effects & POT_CUN)==POT_CUN)
            {
                int val = (p.isPercent? qRound(p.amount * this->cunning.base / 100.0): p.amount);
                this->cunning.current += val;
                if(!p.canBoost && this->cunning.current > this->cunning.base)
                    this->cunning.current = this->cunning.base;
            }
        }
            /*
     = 1,
    POT_MANA = 2,
    POT_STR = 4,
    POT_DEX = 8,
    POT_WIL = 16,
    POT_MAG = 32,
    POT_INT = 64,
    POT_CUN
              */
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    character *player_;
    QList<potion> potions;

public:
    explicit MainWindow(QWidget *parent = 0);
    void generateCharacter(character *c);
    void refreshSheet(character *c);

    void init();

    ~MainWindow();

private:
    Ui::MainWindow *ui;

public slots:
    void drinkSelectedPotion();
    void castSpell();
    void damagePlayer();
    void changeName();
    void changeRace();
};

#endif // MAINWINDOW_H
