/***************************************************************************
 *   Copyright (C) 2006-2011 by Julien Lemoine, Simon Viennot              *
 *   lapinot@tuxfamily.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "gamewidget.h"
#include "displaypainter.h"
#include "baseclass/basegame.h"
#include "computation/database.h"
#include <QtGui>

GameWidget::GameWidget( QWidget* parent) : QWidget( parent) {
	QGridLayout *grid = new QGridLayout( this );
	
	QLabel *repr_2 = new QLabel;
	repr_2->setText("Position :");
	grid->addWidget( repr_2, 0, 0 );
	box_given_position_2 = new QLineEdit;
	grid->addWidget( box_given_position_2, 0, 1 );
	box_given_position_2->setText( QString("0GG00*0000G00GG00000000000E") );
	box_given_position_2->setMinimumWidth( 130 );
	connect( box_given_position_2, SIGNAL(returnPressed()), this, SLOT(positionConfirmed()) );
		
	displayWidget = new DisplayGameWidget;
	displayWidget->cellSize = 30;
	
	grid->addWidget( displayWidget, 1, 0, 1, 2 );
	
	//initialize the display widget with the current line content
	positionConfirmed();
}

void GameWidget::positionConfirmed() {
	displayWidget->changePaintPosition( box_given_position_2->text().toStdString() );
}

DisplayGameWidget::DisplayGameWidget( QWidget* parent) : QWidget( parent) {
	//default value, used for example in the trace
	cellSize = 10;
	displayIsActivated = true;
}

//change the position displayed
//grahics are updated only if the position has really changed
void DisplayGameWidget::changePaintPosition(const PositionString &newPaintPosition) {
	if(paintPosition != newPaintPosition) {
		paintPosition = newPaintPosition;
		update();	//function of QWidget to update the graphics
	}
}

void DisplayGameWidget::paintEvent(QPaintEvent * /* event */) {
	DisplayPainter displayPainter;
	displayPainter.cellSize = cellSize;
	displayPainter.computeHeightOnly = false;
	displayPainter.painter->begin(this);
	displayPainter.painter->setRenderHint(QPainter::Antialiasing);
	if(displayIsActivated) {
		BaseGame game(paintPosition);
		game.paint(displayPainter);
	}
	displayPainter.painter->end();
}

int DisplayGameWidget::neededHeight() {
	DisplayPainter displayPainter;
	displayPainter.cellSize = cellSize;
	displayPainter.computeHeightOnly = true;
	BaseGame game(paintPosition);
	game.paint(displayPainter);
	return displayPainter.neededHeight;
}

//grahics are updated only if the activated status has changed
void DisplayGameWidget::activate(bool newIsActivated) {
	if(displayIsActivated != newIsActivated) {
		displayIsActivated = newIsActivated;
		update();  //function of QWidget to update the graphics
	}
}

DisplayBaseWidget::DisplayBaseWidget( QWidget* parent, int numBase) : QDialog( parent) {
	//init the base index (indicating which base will be displayed)
	baseSelectionIndex = numBase;
	
	setMinimumWidth(750);
	setMinimumHeight(600);
	
	QGridLayout *grid = new QGridLayout( this );
	grid->setMargin(5);
	
	QLabel *repr_2 = new QLabel;
	repr_2->setText("Index :");
	grid->addWidget( repr_2, 0, 0, Qt::AlignRight );
	givenIndex = new QLineEdit;
	grid->addWidget( givenIndex, 0, 1, Qt::AlignLeft );
	givenIndex->setText( QString("0") );
	givenIndex->setFixedWidth( 100 );
	connect( givenIndex, SIGNAL(returnPressed()), this, SLOT(indexConfirmed()) );
	
	QPushButton * randomButton = new QPushButton( "Random");
	randomButton->setFixedSize(randomButton->sizeHint());
	randomButton->setFocusPolicy(Qt::NoFocus);
	grid->addWidget( randomButton, 0, 2, Qt::AlignLeft );
	connect( randomButton, SIGNAL(clicked()), this, SLOT(randomClicked()) );
	
	//Note : the fixed size of givenIndex prevents the grid to use all the available size of the dialog
	//we need to allow the columns to "grow" with the stretch factor
	grid->setColumnStretch(0, 10);
	grid->setColumnStretch(1, 10);
	grid->setColumnStretch(2, 10);
	
	baseDisplayTable=new QTableWidget(15, 4, this);
	baseDisplayTable->setMinimumWidth(700);
	QStringList titles;
	titles << "Index  " << "Graphic representation" << "String representation" << "Value";
	baseDisplayTable->setHorizontalHeaderLabels(titles);
	baseDisplayTable->verticalHeader()->hide();
	
	//initialization with empty items
	int i, j;
	for(i=0; i<baseDisplayTable->rowCount(); i++) {
		baseDisplayTable->setRowHeight(i,25);
		for(j=0; j<4; j++) {
			QTableWidgetItem *item = new QTableWidgetItem(tr("%1").arg( (i+1)*(j+1) ));
			item->setText("");
			baseDisplayTable->setItem(i, j, item);
		}
		
		//create and initialize the display widgets
		DisplayGameWidget* displayWidget = new DisplayGameWidget;
		displayWidget->activate(false);
		baseDisplayTable->setCellWidget(i, 1, displayWidget);
	}
	
	grid->addWidget( baseDisplayTable, 1, 0, 1, 3 );
	baseDisplayTable->resizeColumnToContents (3);
	baseDisplayTable->setColumnWidth (1,300);
	
	//initialize the display widget
	indexConfirmed();
}

void DisplayBaseWidget::indexConfirmed() {
	bool gameHasGraphics = BaseGame(Parameter::gameChoice).isPaintable();
	int index = givenIndex->text().toInt();
	int row;
	QTableWidgetItem * item;
	for(row=0; row < baseDisplayTable->rowCount(); row++) {
		item=baseDisplayTable->item(row,0);
		item->setData(Qt::DisplayRole, QString("%1").arg(index+row) );
		
		if(db::findIndex(index+row, baseSelectionIndex)) {
			if(gameHasGraphics) {
				DisplayGameWidget* displayWidget = (DisplayGameWidget *) baseDisplayTable->cellWidget(row, 1);
				displayWidget->activate(true);
				displayWidget->changePaintPosition( gameUncompress(db::foundIndexString()) );
				int neededHeight = displayWidget->neededHeight() + 5;
				if(baseDisplayTable->rowHeight(row) < neededHeight) baseDisplayTable->setRowHeight(row, neededHeight);
			}
			
			item=baseDisplayTable->item(row,2);
			item->setData(Qt::DisplayRole, QString(db::foundIndexString().c_str()) );
			
			item=baseDisplayTable->item(row,3);
			item->setData(Qt::DisplayRole, QString(db::foundIndexImage().c_str()) );
		} else {
			DisplayGameWidget* displayWidget = (DisplayGameWidget *) baseDisplayTable->cellWidget(row, 1);
			displayWidget->activate(false);
			
			item=baseDisplayTable->item(row,2);
			item->setData(Qt::DisplayRole, QString("") );
			
			item=baseDisplayTable->item(row,3);
			item->setData(Qt::DisplayRole, QString(""));
		}
	}
	baseDisplayTable->resizeColumnToContents (0);
	baseDisplayTable->resizeColumnToContents (2);
}

void DisplayBaseWidget::keyPressEvent(QKeyEvent *event) {
	//we reimplement only the up/down Key
	//for all other keys, we call the base class for normal operations
	int index;
	switch (event->key()) {
		case Qt::Key_Down:
			index = givenIndex->text().toInt();
			index += 15;
			if(index < db::storeVect[baseSelectionIndex].size()) {
				givenIndex->setText( QString("%1").arg(index) );
				indexConfirmed();
			}
			break;
		case Qt::Key_Up:
			index = givenIndex->text().toInt();
			index -= 15;
			if(index < 0 ) index = 0;
			givenIndex->setText( QString("%1").arg(index) );
			indexConfirmed();
			break;
		default:
			QDialog::keyPressEvent(event);
	}
}

//choose an index randomly
//rand() is usually smaller than size, so we cannot use rand()%size as a random function
//instead, we compute a random "path" to the index by choosing the first or the second half
void DisplayBaseWidget::randomClicked() {
	int size = db::storeVect[baseSelectionIndex].size();
	int randomIndex = 0;
	while(size!=0) {
		if(rand()%2==0) {
			randomIndex += size/2;
		}
		size /= 2;
	}
	givenIndex->setText( QString("%1").arg(randomIndex) );
	indexConfirmed();
}
