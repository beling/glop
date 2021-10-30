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

#include "tracewidget.h"
#include "trace.h"
#include "../gamewidget.h"
#include "../baseclass/basegame.h"

#include <QtGui>

QColor colorFrom(QString colorName) {
	if(colorName == QString("transparent")){
		return QColor(255,255,255,0);	//alpha=0 : no color
	} else
	if(colorName == QString("lightred")){
		return QColor(255,127,127);	//red
	} else
	if (colorName == QString("green")) {
		return QColor(127,255,127);	//green
	} else
	if (colorName == QString("light green")) {
		return QColor(127,255,127,150);
	} else
	if(colorName == QString("gray")){
		return QColor(127,127,127);	//gray
	} else
	if(colorName == QString("blue")) {
		return QColor(127,127,255);	//blue
	} else
	if(colorName == QString("yellow")){
		return QColor(255,255,127);	//yellow
	}
	
	//default case : Qt will search the name in the predefined colors
	return QColor(colorName);
}

//indicate the background color depending on the number of unknown children
QColor colorUnknown(int unknownNumber) {
	switch(unknownNumber) {
		//red, green, blue, alpha (default =255)
		case 1  : return QColor(255,127,127,255); //red
		case 2  : return QColor(255,196,127,255); //orange
		case 3  : return QColor(255,255,127,255); //yellow
		case 4  : return QColor(255,255,127,255); //yellow
		case 5  : return QColor(127,255,127,255); //green
		case 6  : return QColor(127,255,127,255); //green
		case 7  : return QColor(127,255,127,200); //light green
		case 8  : return QColor(127,255,127,150); //light green
		case 9  : return QColor(127,255,127,100); //light green
		case 10 : return QColor(127,255,127, 50); //light green
		default : return colorFrom(QString("transparent")); //all other cases (including unknownNumber==0)
	}
}

//--------------------------------------------------------------------------
//Widget Trace : main trace to display the computing branch
//--------------------------------------------------------------------------

bool WidgetTrace::withGraphics=false;
bool WidgetTrace::gameHasGraphics=false;

//initialization of main trace table
WidgetTrace::WidgetTrace(QWidget *parent) : QWidget( parent) {
	QVBoxLayout *vboxLayout = new QVBoxLayout(this);
	vboxLayout->setMargin(0);
	table=new QTableWidget(global::Max_Level, 6, this);
	table->setEnabled(false);
	
	QStringList titles;
	titles << "Index  " << "Traversal" << "Alive" << "Cells" << "Nimber" << "Position";
	table->setHorizontalHeaderLabels(titles);
	
	int i, j;
	//initialization of columns width
	for(j=0; j<5; j++) {
		table->resizeColumnToContents(j);
	}
	table->setColumnWidth(5,500);
	//initialization with empty items
	for(i=0; i<global::Max_Level; i++) {
		table->setRowHeight(i,25);
		for(j=0; j<6; j++) {
			QTableWidgetItem *item = new QTableWidgetItem(tr("%1").arg( (i+1)*(j+1) ));
			item->setText("");
			table->setItem(i, j, item);
		}
		
		//create and initialize the display widgets
		DisplayGameWidget* displayWidget = new DisplayGameWidget;
		displayWidget->activate(false);
		table->setCellWidget(i, 5, displayWidget);		
	}
	
	table->setColumnHidden(1, !NodeStore::useTraversal); //hide the column if traversal is not used
	table->setEditTriggers ( QAbstractItemView::NoEditTriggers );  //no edit of the table allowed
	vboxLayout->addWidget(table);  //add the table to the layout
	
	//click on the table
	//for an unknown reason, the signal cellClicked is not always emitted on some platforms when using graphic trace
	//we use cellPressed instead
	connect( table, SIGNAL( cellPressed(int, int) ), this, SLOT( click_on_position(int, int) ) );
	
	children_popup=new WidgetChildren(this); //table to trace children
}

//refresh the trace of the computing branch
void WidgetTrace::update_display() {
	QMutexLocker locker(&(Trace::trace_lock));
	
	if(interactionLocked) {
		table->setEnabled(false);
		children_popup->setEnabled(false);
	}
	
	//----------------update children table if displayed----------------------
	if(children_popup->isVisible()) children_popup->update_display();
	
	table->setColumnHidden(1, !NodeStore::useTraversal); //hide the column if traversal is not used
	QTableWidgetItem * item;
	
	//----------------update main table if displayed--------------------------
	if(!table->isVisible()) return;
	
	list<NodeTrace> ntrace = NodeStore::getMainTrace();
	list<NodeTrace>::iterator NTi=ntrace.begin();
	int row;
	for(row=0; row<global::Max_Level; row++) {
		if(NTi!=ntrace.end()) {
			NodeTrace n = *NTi;
			item=table->item(row,0);
			item->setData(Qt::DisplayRole, n.index);
			
			//information about the traversal of the tree
			item=table->item(row,1);
			item->setData(Qt::DisplayRole, n.traversal_info);
			
			//number of still alive positions
			item=table->item(row,2);
			item->setData(Qt::DisplayRole, QString("%1 ").arg(n.alive, 1, 10 ));
			item->setBackgroundColor( colorUnknown(n.alive) );
			
			//lives
			item=table->item(row,3);
			item->setData(Qt::DisplayRole, n.nb_lives);

			//nimber part, or more generally anything else than the position part
			item=table->item(row,4);
			item->setData(Qt::DisplayRole, n.nodePos);
			item->setBackgroundColor( colorFrom(n.posColor));
			
			//position
			if(withGraphics && gameHasGraphics) {
				DisplayGameWidget* displayWidget = (DisplayGameWidget *) table->cellWidget(row, 5);
				displayWidget->activate(true);
				displayWidget->changePaintPosition(n.pos.toStdString());
				int neededHeight = displayWidget->neededHeight() + 8;
				if(table->rowHeight(row) < neededHeight) table->setRowHeight(row, neededHeight);
				
				//don't display anything in the item
				item=table->item(row,5);
				item->setData(Qt::DisplayRole, "");
				item->setBackgroundColor( colorFrom(n.posColor));
			} else {
				item=table->item(row,5);
				item->setData(Qt::DisplayRole, n.pos);
				item->setBackgroundColor( colorFrom(n.posColor));
			}
			
			NTi++;
		} else {
			//there's nothing to display (end of list or level not yet filled with positions)
			item=table->item(row,0);
			item->setData(Qt::DisplayRole, "");
			item=table->item(row,1);
			item->setData(Qt::DisplayRole, "");
			item=table->item(row,2);
			item->setBackgroundColor( colorFrom(QString("transparent")) );
			item->setData(Qt::DisplayRole, "");
			item=table->item(row,3);
			item->setData(Qt::DisplayRole, "");
			item=table->item(row,4);
			item->setData(Qt::DisplayRole, "");
			item->setBackgroundColor( colorFrom(QString("transparent")) );
			
			if(withGraphics && gameHasGraphics) {
				DisplayGameWidget* displayWidget = (DisplayGameWidget *) table->cellWidget(row, 5);
				displayWidget->activate(false);
			}
			
			//in any case, don't display anything in the item
			item=table->item(row,5);
			item->setData(Qt::DisplayRole, "");
			item->setBackgroundColor( colorFrom(QString("transparent")) );
		}
	}
}

//Function called after a click in the trace widget
//click on left column : zap child
//click on the position cell : display children
void WidgetTrace::click_on_position(int row, int column) {
	QTableWidgetItem * item = table->item(row, column);
	if ( !item ) return;
	table->setItemSelected(item, FALSE );
	
	//nothing to do if it is a click trying to change the first position
	if(row==0 && column<3) return;
	
	//-----click on a left column-----------------
	//go to the next or the previous branch of the level
	if (column<3) {
		Trace::request_level( row, column==2 );
		return;
	}
	
	//-----click on a right column----------------
	QMutexLocker locker(&(Trace::trace_lock));
	
	int parent_candidate_level=row;
	
	//we do nothing if the click was too low in the interface
	if(parent_candidate_level >= Trace::current_level -1) return;
	
	//a position cell was clicked, we display children
	//the clicked position becomes the main position of the children table
	//we deduce the storeID of the position from the clicked line
	
	//update children tab
	children_popup->requestedId = NodeStore::getLevelStoreId(parent_candidate_level);
	children_popup->updateScroll = true;
	children_popup->update_display();
	emit requestExplorer();
}

//initialization is done just before beginning a computation
void WidgetTrace::initialize(bool lockedComputation) {
	//init the displayed child id
	children_popup->requestedId = 0;	//rootId of a computation is always 0
	children_popup->updateScroll = true;
	children_popup->initialize();
	interactionLocked = lockedComputation;
	
	//init the gameHasGraphics variable
	gameHasGraphics = BaseGame(Parameter::gameChoice).isPaintable();
	
	int i;
	for(i=0; i<global::Max_Level; i++) {
		//initialize the row height (to revert changes of any previous computation)
		table->setRowHeight(i,25);
		
		//initialize the graphic object
		//note : initialization needed even for computation without graphics, to clear possible graphics of any previous computation
		DisplayGameWidget* displayWidget = (DisplayGameWidget *) table->cellWidget(i, 5);
		displayWidget->activate(false);
	}
	
	//initialize the title of the optional string (lives, cells, etc.)
	QString title( BaseGame(Parameter::gameChoice).optionalStringTitle().c_str() );
	table->horizontalHeaderItem(3)->setText(title);
	table->resizeColumnToContents(3);
	
	//initialize the title for the column which is not the position part (nimber part, contract, etc.)
	QString titleB( BaseNode(Parameter::nodeChoice, Parameter::computation_kind).extraPositionPartTitle().c_str() );
	table->setColumnHidden(4, titleB.isEmpty()); //hide the column iff the title is empty
	table->horizontalHeaderItem(4)->setText(titleB);
	table->resizeColumnToContents(4);
}


//--------------------------------------------------------------------------
//Widget Children : display all the available information about a given node
//--------------------------------------------------------------------------

//initialization of children trace table
WidgetChildren::WidgetChildren(QWidget *parent) : QWidget( parent) {
	setMinimumWidth( 750 );
	//setMinimumHeight( 600 );
	gridLayout = new QGridLayout(this);
	
	//table for the parents
	QLabel *title_parentTable = new QLabel;
	title_parentTable->setText("Parent nodes :");
	gridLayout->addWidget( title_parentTable, 1, 0, 1, 2 );
	
	nb_col=7;
	parentTable=new QTableWidget(1, nb_col, this); //1 row, nb_col columns
	parentTable->setFixedHeight( 100 );
	initTable(parentTable);
	gridLayout->addWidget(parentTable, 2, 0, 1, 4);	//the table uses all 4 columns of the layout
	
	//table for the current node
	QLabel *title_currentTable = new QLabel;
	title_currentTable->setText("Current node :");
	gridLayout->addWidget( title_currentTable, 3, 0, 1, 2 );
	
	currentNodeTable=new QTableWidget(1, nb_col, this);
	currentNodeTable->setFixedHeight( 50 );
	initTable(currentNodeTable);
	gridLayout->addWidget(currentNodeTable, 4, 0, 1, 4);	//the table uses all 4 columns of the layout
	
	//table for the children
	QLabel *title_childrenTable = new QLabel;
	title_childrenTable->setText("Children nodes :");
	gridLayout->addWidget( title_childrenTable, 5, 0, 1, 2 );
	
	childrenTable=new QTableWidget(1, nb_col, this);
	initTable(childrenTable);
	gridLayout->addWidget(childrenTable, 6, 0, 1, 4);	//the table uses all 4 columns of the layout
	
	//connect the clicks on children and on parents
	connect( currentNodeTable, SIGNAL( cellPressed(int, int) ), this, SLOT( click_on_currentNode(int, int) ) );
	connect( childrenTable, SIGNAL( cellPressed(int, int) ), this, SLOT( click_on_children(int, int) ) );
	connect( parentTable, SIGNAL( cellPressed(int, int) ), this, SLOT( click_on_parent(int, int) ) );
}

void WidgetChildren::initTable(QTableWidget *tableW) {
	QStringList titles;
	titles << "Cells" << "Traversal" << "Unknown" << "B.C.U." << "StoreId" << "Nimber" << "Position";
	tableW->setHorizontalHeaderLabels(titles);
	
	//index of some specific columns containing ids
	optional_col=0;		//optional information (lives, cells, edges...)
	traversal_col=1;	//traversal information
	unknown_col=2;		//number of unknown children
	bcu_col=3;			//best child unknown value
	id_col=4;			//nodestore ids
	extraPos_col=5;		//part of the node other than the position (nimber, contract...)
	pos_col=6;			//game position
	
	//row height initialization
	tableW->setRowHeight(0,25);
	
	//column width initialization
	int j;
	for(j=0; j<nb_col; j++) {
		tableW->resizeColumnToContents(j);
	}
	tableW->setColumnWidth(pos_col,500);
	tableW->setColumnWidth(traversal_col,150);
	
	tableW->setColumnHidden(traversal_col, !NodeStore::useTraversal); //hide the column if traversal is not used
	tableW->setEditTriggers ( QAbstractItemView::NoEditTriggers ); //table is not editable
}

void WidgetChildren::initialize() {
	//initialize the title of the optional string (lives, cells, etc.)
	QString title( BaseGame(Parameter::gameChoice).optionalStringTitle().c_str() );
	currentNodeTable->horizontalHeaderItem(0)->setText(title);
	currentNodeTable->resizeColumnToContents(0);
	childrenTable->horizontalHeaderItem(0)->setText(title);
	childrenTable->resizeColumnToContents(0);
	parentTable->horizontalHeaderItem(0)->setText(title);
	parentTable->resizeColumnToContents(0);
	
	//initialize the title for the column which is not the position part (nimber part, contract, etc.)
	QString titleB( BaseNode(Parameter::nodeChoice, Parameter::computation_kind).extraPositionPartTitle().c_str() );
	currentNodeTable->setColumnHidden(extraPos_col, titleB.isEmpty()); //hide the column iff the title is empty
	currentNodeTable->horizontalHeaderItem(extraPos_col)->setText(titleB);
	currentNodeTable->resizeColumnToContents(extraPos_col);
	childrenTable->setColumnHidden(extraPos_col, titleB.isEmpty()); //hide the column iff the title is empty
	childrenTable->horizontalHeaderItem(extraPos_col)->setText(titleB);
	childrenTable->resizeColumnToContents(extraPos_col);
	parentTable->setColumnHidden(extraPos_col, titleB.isEmpty()); //hide the column iff the title is empty
	parentTable->horizontalHeaderItem(extraPos_col)->setText(titleB);
	parentTable->resizeColumnToContents(extraPos_col);
}

//refresh displayed children table
void WidgetChildren::update_display() {
	//note : no Trace lock, this function is always called only from an already locked function
	
	//updateScroll is always done on the first call of updateTable (i.e the children table)
	list<NodeTrace> ntrace = NodeStore::getChildrenTrace(requestedId);
	updateTable(childrenTable, ntrace);
	
	ntrace = NodeStore::getParentsTrace(requestedId);
	updateTable(parentTable, ntrace);
	
	ntrace = NodeStore::getOneNodeTrace(requestedId);
	if(!ntrace.empty()) {
		updateTable(currentNodeTable, ntrace);
	} else {
		//the node doesn't exist in NodeStore
		
	}
}

//TODO : enable/disable sorting
void WidgetChildren::updateTable(QTableWidget *tableW, list<NodeTrace> ntrace) {
	tableW->setColumnHidden(traversal_col, !NodeStore::useTraversal); //hide the column if traversal is not used
	//tableW->setSortingEnabled(false); //disable the sort
	int nodesNumber = ntrace.size();
	
	//update the size of the table if needed
	while(nodesNumber < tableW->rowCount()) tableW->removeRow(0); //remove rows if needed
	while(nodesNumber > tableW->rowCount()) {
		tableW->insertRow(0); //add rows if needed
		tableW->setRowHeight(0,25); //row height initialization
	}
	
	//initialize the items that need it
	int i, j;
	for(i=0; i<nodesNumber; i++) {
		for(j=0; j<nb_col; j++) {
			if(tableW->item(i, j) == 0) {
				QTableWidgetItem *item = new QTableWidgetItem(QString(""));
				tableW->setItem(i, j, item);
				
				if(j==pos_col) {
					//create and initialize the display widget corresponding to this cell
					DisplayGameWidget* displayWidget = new DisplayGameWidget;
					displayWidget->activate(false);
					tableW->setCellWidget(i, j, displayWidget);		
				}
			} else {
				//set back the color to the transparent one
				tableW->item(i, j)->setBackgroundColor(colorFrom(QString("transparent")));
			}
		}
	}
	
	list<NodeTrace>::iterator NTi;
	
	QTableWidgetItem * item;
	int row=0;
	int nbRows = tableW->rowCount();
	int minUnknown = 10000;  //look for the node with the least number of unknown children
	QTableWidgetItem * minItem = 0;
	for(NTi=ntrace.begin(); NTi!=ntrace.end() && row < nbRows; NTi++) {
		NodeTrace n = *NTi;
		
		//lives
		item=tableW->item(row, optional_col);
		item->setData(Qt::DisplayRole, n.nb_lives);
		
		//traversal
		item=tableW->item(row, traversal_col);
		item->setData(Qt::DisplayRole, n.traversal_info);
		
		//number of unknown children
		item=tableW->item(row, unknown_col);
		item->setData(Qt::DisplayRole, QString("%1 ").arg(n.alive, 1, 10 ));
		item->setBackgroundColor( colorUnknown(n.alive) ); //color of the cell
		if(n.alive > 0 && n.alive < minUnknown) {
			minUnknown = n.alive;
			minItem=item;
		}
		
		//B.C.U. : number of unknown children of the best child
		item=tableW->item(row, bcu_col);
		if(n.bcu>0) {
			item->setData(Qt::DisplayRole, QString("%1 ").arg(n.bcu, 1, 10 ));
		} else {
			item->setData(Qt::DisplayRole, QString("?"));
		}
		item->setBackgroundColor( colorUnknown(n.bcu) ); //color of the cell
		
		//storeID
		item=tableW->item(row, id_col);
		item->setData(Qt::DisplayRole, n.id);
		
		//extra-position part of the node (nimber, contract, ...)
		item=tableW->item(row, extraPos_col);
		item->setData(Qt::DisplayRole, n.nodePos);
		
		//position
		if(WidgetTrace::withGraphics && WidgetTrace::gameHasGraphics) {
			DisplayGameWidget* displayWidget = (DisplayGameWidget *) tableW->cellWidget(row, pos_col);
			displayWidget->activate(true);
			displayWidget->changePaintPosition(n.pos.toStdString());
			int neededHeight = displayWidget->neededHeight() + 8;
			if(tableW->rowHeight(row) < neededHeight) tableW->setRowHeight(row, neededHeight);
			if(tableW == currentNodeTable && tableW->height() < neededHeight+25) tableW->setFixedHeight(neededHeight+25);
			
			//don't display anything in the item
			item=tableW->item(row, pos_col);
			item->setData(Qt::DisplayRole, "");
			item->setBackgroundColor( colorFrom(n.posColor));
		} else {
			item=tableW->item(row, pos_col);
			item->setData(Qt::DisplayRole, n.pos);
			item->setBackgroundColor( colorFrom(n.posColor));
			
			DisplayGameWidget* displayWidget = (DisplayGameWidget *) tableW->cellWidget(row, pos_col);
			displayWidget->activate(false);
		}
		
		//color dead nodes in gray
		if(!n.is_alive){
			for(j=0; j<nb_col; j++) {
				tableW->item(row, j)->setBackgroundColor(colorFrom(QString("gray")));
			}
		}
		
		row++;
	}
	
	//scroll to the child with the least number of unknown children
	if(updateScroll && minItem != 0) {
		//cout << minItem->data(Qt::DisplayRole).toString().toStdString() << endl;
		tableW->setCurrentItem(minItem, QItemSelectionModel::Deselect);
		tableW->scrollToItem(minItem, QAbstractItemView::PositionAtTop);
		updateScroll = false;
	}
	
	//tableW->sortItems(3);
	//tableW->setSortingEnabled(true); //enable the sort
}

//function called when the children's table is clicked
void WidgetChildren::click_on_children(int row, int column) {
	QTableWidgetItem * item = childrenTable->item(row, column);
	if ( !item ) return;
	childrenTable->setItemSelected(item, FALSE );
	
	//no click on an already dead node
	if( item->backgroundColor() == colorFrom(QString("gray")) ) return;
	
	//new requested id is in cell (row, id_col)
	int clickedId = childrenTable->item(row, id_col)->data(Qt::DisplayRole).toInt();
	
	if (column==id_col) {
		Trace::requestTraversalId( clickedId ); //click on the column of storeId = choose this node for traversal
	} else {
		requestedId =  clickedId ; //other columns = choose this node as the new current node of widgetchildren
		updateScroll = true;
	}
}

//function called when the parents table is clicked
void WidgetChildren::click_on_parent(int row, int column) {
	QTableWidgetItem * item = parentTable->item(row, column);
	if ( !item ) return;
	parentTable->setItemSelected(item, FALSE );
	int clickedId = parentTable->item(row, id_col)->data(Qt::DisplayRole).toInt();
	
	if (column==id_col) {
		Trace::requestTraversalId( clickedId ); //click on the column of storeId = choose this node for traversal
	} else {
		requestedId =  clickedId ; //other columns = choose this node as the new current node of widgetchildren
		updateScroll = true;
	}
}

//function called when the current node table is clicked
void WidgetChildren::click_on_currentNode(int row, int column) {
	QTableWidgetItem * item = currentNodeTable->item(row, column);
	if ( !item ) return;
	currentNodeTable->setItemSelected(item, FALSE );
	
	if (column==bcu_col) {
		//click on the bcu, update the focus of the children table to the corresponding child
		updateScroll = true;
	}
}
