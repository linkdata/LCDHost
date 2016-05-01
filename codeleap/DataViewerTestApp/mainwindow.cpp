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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    player_ = new character;
    connect(ui->drinkButton, SIGNAL(clicked()), this, SLOT(drinkSelectedPotion()));
    connect(ui->castButton, SIGNAL(clicked()), this, SLOT(castSpell()));
    connect(ui->damageButton, SIGNAL(clicked()), this, SLOT(damagePlayer()));
    connect(ui->changeName, SIGNAL(clicked()), this, SLOT(changeName()));
    connect(ui->changeRace, SIGNAL(clicked()), this, SLOT(changeRace()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete player_;
}

void MainWindow::init()
{
    generateCharacter(player_);
    refreshSheet(player_);

    potions.append( (potion){"Health Potion (Small)", POT_HEALTH, 10, true, false, 10});
    potions.append( (potion){"Health Potion (Large)", POT_HEALTH, 35, true, false, 20});
    potions.append( (potion){"Mana Potion (Small)", POT_MANA, 10, true, false, 10});
    potions.append( (potion){"Mana Potion (Large)", POT_MANA, 35, true, false, 20});

    potions.append( (potion){"Boost Strength Potion", POT_STR, 5, false, true, 15});
    potions.append( (potion){"Boost Dexterity Potion", POT_DEX, 5, false, true, 15});
    potions.append( (potion){"Boost Willpower Potion", POT_WIL, 5, false, true, 15});
    potions.append( (potion){"Boost Magic Potion", POT_MAG, 5, false, true, 15});
    potions.append( (potion){"Boost Intelligence Potion", POT_INT, 5, false, true, 15});
    potions.append( (potion){"Boost Cunning Potion", POT_CUN, 5, false, true, 15});

    for(int i = 0; i<potions.count(); i++)
        ui->potionsList->addItem(QString("%1 (%2gp)").arg(potions.at(i).name).arg(potions.at(i).price));

    ui->racesList->addItem("Human");
    ui->racesList->addItem("Elf");
    ui->racesList->addItem("Dwarf");
    ui->racesList->addItem("Orc");
    ui->racesList->addItem("Half-Elf");
    ui->racesList->addItem("Half-Orc");
    ui->racesList->addItem("Gnome");
    ui->racesList->addItem("Troll");
    ui->racesList->addItem("Dragonkin");
}

void MainWindow::generateCharacter(character *c)
{
    QTime now = QTime::currentTime();
    srand((uint)now.msec());

    c->setName("Player");
    c->setRace("Human");
    c->gold = 100;

    c->dexterity.base = qrand() % 20;
    c->strength.base = qrand() % 20;
    c->willpower.base = qrand() % 20;
    c->magic.base = qrand() % 20;
    c->cunning.base = qrand() % 20;
    c->intelligence.base = qrand() % 20;
    c->health.base = c->strength.base * 10 + 40;
    c->mana.base = c->willpower.base * 4 + 20;

    c->resetAllStats();
}

void MainWindow::refreshSheet(character *c)
{
    ui->textEdit->clear();
    QString str = "";
    str += QString("<b>Name:</b> %1<hr />").arg(c->name());
    str += QString("<b>Race:</b> %1<hr />").arg(c->race());
    str += QString("<b>Strength:</b> %1 (%2)<br />").arg(c->strength.current).arg(c->strength.base);
    str += QString("<b>Dexterity:</b> %1 (%2)<br />").arg(c->dexterity.current).arg(c->dexterity.base);
    str += QString("<b>Intelligence:</b> %1 (%2)<br />").arg(c->intelligence.current).arg(c->intelligence.base);
    str += QString("<b>Cunning:</b> %1 (%2)<br />").arg(c->cunning.current).arg(c->cunning.base);
    str += QString("<b>Willpower:</b> %1 (%2)<br />").arg(c->willpower.current).arg(c->willpower.base);
    str += QString("<b>Magic:</b> %1 (%2)<hr />").arg(c->magic.current).arg(c->magic.base);
    str += QString("<b>Health:</b> %1 / %2<br />").arg(c->health.current).arg(c->health.base);
    str += QString("<b>Mana:</b> %1 / %2<hr />").arg(c->mana.current).arg(c->mana.base);
    str += QString("<b>Gold:</b> %1gp<br />").arg(c->gold);

    ui->textEdit->append(str);
}

void MainWindow::drinkSelectedPotion()
{
    player_->drink( potions.at(ui->potionsList->currentIndex()) );
    refreshSheet(player_);
}

void MainWindow::castSpell()
{
    player_->mana.current -= (qrand() % 100)/10.0;
    if(player_->mana.current < 0)
        player_->mana.current = 0;
    refreshSheet(player_);
}

void MainWindow::damagePlayer()
{
    player_->health.current -= (qrand() % 100)/10.0;
    if(player_->health.current < 0)
        player_->health.current = 0;
    refreshSheet(player_);
}

void MainWindow::changeName()
{
    player_->setName(ui->lineEdit->text());
    ui->lineEdit->setText("");
    refreshSheet(player_);
}

void MainWindow::changeRace()
{
    player_->setRace(ui->racesList->currentText());
    refreshSheet(player_);
}
