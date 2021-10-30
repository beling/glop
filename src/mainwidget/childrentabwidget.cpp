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

#include "glopwidget.h"
#include "childrentabwidget.h"

//for the function QColorFrom
#include "../computation/tracewidget.h"

#include "../baseclass/basegame.h"
#include "../baseclass/basenode.h"
#include "../baseclass/interfacewidget.h"

#include <QtGui>

//------------------Children tab -------------------------------
//TODO : some redundancy of the code between parent and children table

void GlopWidget::setup_test_tab() {
	QWidget *test_tab = new QWidget;
	connect( this, SIGNAL(isComputing(bool)), test_tab, SLOT(setDisabled(bool)) );
	QGridLayout *grid_test = new QGridLayout( test_tab );
	
	QLabel *repr_2 = new QLabel;
	repr_2->setText("Parent position :");
	grid_test->addWidget( repr_2, 0, 0 );
	
	QPushButton *back_button = new QPushButton( "Back to previous");
	back_button->setFixedSize(back_button->sizeHint());
	grid_test->addWidget( back_button, 1, 0 );
	connect( back_button, SIGNAL(clicked()), this, SLOT(previousChildren()) );
	
	//parent table
	parentDisplayTable=new QTableWidget(1, 3, this);
	parentDisplayTable->setMinimumWidth(700);
	QStringList titles;
	titles << "Graphic representation" << "String representation" << "Value";
	parentDisplayTable->setHorizontalHeaderLabels(titles);
	parentDisplayTable->verticalHeader()->hide();
	
	grid_test->addWidget( parentDisplayTable, 0, 1);
	parentDisplayTable->setColumnWidth (0,300);
	parentDisplayTable->setColumnWidth (1,300);
	parentDisplayTable->resizeColumnToContents(2);
	parentDisplayTable->setFixedHeight( 55 );
	
	int j;
	for(j=0; j<3; j++) {
		QTableWidgetItem *item = new QTableWidgetItem(tr("%1").arg((j+1) ));
		item->setText("");
		parentDisplayTable->setItem(0, j, item);
	}
	
	DisplayGameWidget* displayWidget = new DisplayGameWidget;
	displayWidget->activate(false);
	parentDisplayTable->setCellWidget(0, 0, displayWidget);
	
	box_given_position_2 = new QLineEdit;
	parentDisplayTable->setCellWidget(0, 1, box_given_position_2);
	box_given_position_2->setMinimumWidth( 150 );
	connect( box_given_position_2, SIGNAL(returnPressed()), this, SLOT(printChildren()) );
	
	childrenDisplayTable=new QTableWidget(0, 3, this);
	childrenDisplayTable->setMinimumWidth(700);
	childrenDisplayTable->setHorizontalHeaderLabels(titles);
	childrenDisplayTable->verticalHeader()->hide();
	
	grid_test->addWidget( childrenDisplayTable, 1, 1, 3, 3);
	childrenDisplayTable->setColumnWidth (0,300);
	childrenDisplayTable->setColumnWidth (1,300);
	childrenDisplayTable->resizeColumnToContents(2);
	
	//connect the clicks on children to nextChildren
	connect( childrenDisplayTable, SIGNAL( cellPressed(int, int) ), this, SLOT( nextChildren(int, int) ) );

	childrenWidgetIndex = tabWidget->addTab( test_tab, "Children" );
}

//function called after a click on a tab in the main window
//we detect here that the childrenWidget has been chosen by the user
//we display automatically the position selected in the parameter tab and its children
void GlopWidget::tabClicked(int index) {
	//note : don't initialize during a computation
	if(index == childrenWidgetIndex && !computeRunning) {
		initChildrenTab();
	}
}

//function called when a computation starts/stops
void GlopWidget::initChildrenAfterComputation(bool compute) {
	//when a computation has ended
	if(compute == false) {
		//if the children tab is selected
		if(tabWidget->currentIndex() == childrenWidgetIndex) {
			initChildrenTab();
		}
	}
}

//initialize the children tab with the correct start position and compute its children
//this function is called when the children tab is selected by the user
//or when the tab is already selected and a computation ends
void GlopWidget::initChildrenTab() {
	//clear the list of previously displayed positions
	positionCache.clear();
	
	//remove all rows of the current table
	while(childrenDisplayTable->rowCount() > 0) childrenDisplayTable->removeRow(0);
	
	//initialize the parent position with the current selection of the parameter tab
	readModuleParameters();
	string gameString;
	if(Parameter::usePositionString) {
		BaseGame g(Parameter::given_positionString);
		gameString = g.exportToString();
	} else {
		BaseGame g(Parameter::given_position);
		gameString = g.exportToString();
	}
	QString initString(gameString.c_str());
	box_given_position_2->setText(initString);
	
	printChildren();	//print the data of the parent position and of the children
}

//display the information of the parent position
void GlopWidget::parentPositionInfo() {
	string parentString = (box_given_position_2->text()).toStdString();
	bool gameHasGraphics = BaseGame(Parameter::gameChoice).isPaintable();
	parentDisplayTable->setColumnHidden(0, !gameHasGraphics);
	
	DisplayGameWidget* displayWidget = (DisplayGameWidget *) parentDisplayTable->cellWidget(0, 0);
	displayWidget->activate(gameHasGraphics);
	if(gameHasGraphics) {
		displayWidget->changePaintPosition(parentString);
		int neededHeight = displayWidget->neededHeight() + 5;
		parentDisplayTable->setRowHeight(0, neededHeight);
		parentDisplayTable->setFixedHeight( neededHeight + 25);
	} else {
		parentDisplayTable->setRowHeight(0,25); //row height initialization
		parentDisplayTable->setFixedHeight(55);
	}
	
	//get the data corresponding to the position (only for the current type of node)
	BaseNode c(Parameter::nodeChoice, 0 /*useless value*/);
	QStringList info = c.infoAbout(parentString);
	QColor textColor = Qt::black;
	if(info.size()>=2) textColor = colorFrom(info[1]);
	
	//note : color of the string representation seems not possible with the QLineEdit
	
	//display the data corresponding to the position
	QTableWidgetItem *item=parentDisplayTable->item(0,2);
	item->setTextColor(textColor);
	if(!info.isEmpty()) {
		item->setData(Qt::DisplayRole, info[0] );
	} else {
		item->setData(Qt::DisplayRole, QString("") );
	}
}

//prints children of a given position
void GlopWidget::printChildren() {
	//display the data about the parent position
	parentPositionInfo();
	
	//add the position to the cache (used to go back to a previous position)
	positionCache.push_back(box_given_position_2->text()); 
	
	//read the parameters of all the modules (game, node, traversal, common widget)
	readModuleParameters();
	bool gameHasGraphics = BaseGame(Parameter::gameChoice).isPaintable();
	childrenDisplayTable->setColumnHidden(0, !gameHasGraphics);
	
	//init the possible internal variables of this kind of node
	//note : it is usually used to initialize the first position of a computation
	//but it is also used by some nodes to initialize static variables
	BaseNode tmpNode(Parameter::nodeChoice, Parameter::computation_kind);
	tmpNode.initRootNode();
	
	//compute the children
	BaseGame g((box_given_position_2->text()).toStdString());
	g.computeChildrenSet();
	
	set<Line>::iterator it;
	list<Line> result;
	for(it=g.childrenSet().begin(); it!=g.childrenSet().end(); it++) {
		BaseGame h(*it);
		h.canonize();
		result.push_back(h.exportToLine());
	}
	
	result.sort();		//needed for unique
	result.unique();	//removes all consecutive duplicate positions
	
	//------------ Initialize the table ----------------
	//update the size of the table if needed
	while(childrenDisplayTable->rowCount() > 0) childrenDisplayTable->removeRow(0); //remove all rows
	while(childrenDisplayTable->rowCount() < (int) result.size()) {
		childrenDisplayTable->insertRow(0); //add rows if needed
		childrenDisplayTable->setRowHeight(0,25); //row height initialization
	}
	
	//initialization with empty items
	int i, j;
	for(i=0; i<childrenDisplayTable->rowCount(); i++) {
		for(j=0; j<3; j++) {
			QTableWidgetItem *item = new QTableWidgetItem(tr("%1").arg( (i+1)*(j+1) ));
			item->setText("");
			childrenDisplayTable->setItem(i, j, item);
		}
		
		//create and initialize the display widgets
		DisplayGameWidget* displayWidget = new DisplayGameWidget;
		displayWidget->activate(false);
		childrenDisplayTable->setCellWidget(i, 0, displayWidget);
	}
	//----------------------------------------------------------
	
	//print children in the text box
	list<Line>::iterator Pi;
	int row;
	for(Pi=result.begin(), row=0;Pi!=result.end();Pi++, row++){
		BaseGame h(*Pi);
		string posString = h.exportToString();
		
		//display the graphics corresponding to the position if the game is paintable
		if(gameHasGraphics) {
			DisplayGameWidget* displayWidget = (DisplayGameWidget *) childrenDisplayTable->cellWidget(row, 0);
			displayWidget->activate(true);
			displayWidget->changePaintPosition( posString );
			int neededHeight = displayWidget->neededHeight() + 5;
			if(childrenDisplayTable->rowHeight(row) < neededHeight) childrenDisplayTable->setRowHeight(row, neededHeight);
		}
		
		//get the data corresponding to the position (only for the current type of node)
		BaseNode c(Parameter::nodeChoice, 0 /*useless value*/);
		QStringList info = c.infoAbout(h.exportToString());
		QColor textColor = Qt::black;
		if(info.size()>=2) textColor = colorFrom(info[1]);
		
		//display the string representation
		QTableWidgetItem *item=childrenDisplayTable->item(row,1);
		item->setData(Qt::DisplayRole, QString(posString.c_str()));
		item->setTextColor(textColor);
		
		//display the data corresponding to the position
		item=childrenDisplayTable->item(row,2);
		item->setTextColor(textColor);
		if(!info.isEmpty()) {
			item->setData(Qt::DisplayRole, info[0] );
		} else {
			item->setData(Qt::DisplayRole, QString("") );
		}
	}
}

//Function called after a click on a child
void GlopWidget::nextChildren(int row, int column) {
	QTableWidgetItem * item = childrenDisplayTable->item(row, column);
	if ( !item ) return;
	childrenDisplayTable->setItemSelected(item, FALSE );
	
	QString positionPart = childrenDisplayTable->item(row, 1)->data(Qt::DisplayRole).toString();
	
	//compute children (only if the positionPart is not empty)
	if(!positionPart.isEmpty()) {
		box_given_position_2->setText(positionPart);
		printChildren();		//display the data of the children and the parent
	}
}

//Go back to the previous position
void GlopWidget::previousChildren() {
	if(positionCache.size() >= 2){
		positionCache.pop_back(); //delete the current child
		QString previous = positionCache.back();
		positionCache.pop_back();
		box_given_position_2->setText(previous);
		printChildren();
	}
}
