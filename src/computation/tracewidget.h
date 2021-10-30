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

#ifndef TRACEWIDGET_H
#define TRACEWIDGET_H

#include <QWidget>

#include "nodestore.h"

class QGridLayout;
class QTableWidget;
class QTableWidgetItem;
class QLineEdit;

class WidgetChildren;

//TODO : QColorFrom is also used by childrenTabWidget, and should be placed in a different file
QColor colorFrom(QString colorName);		//define our prefered colors for red, green, blue, etc
QColor colorUnknown(int unknownNumber);		//indicate the background color depending on the number of unknown children

//! Widget to trace computing
class WidgetTrace: public QWidget {
	Q_OBJECT
  public:
	//main trace widget
	WidgetTrace( QWidget *parent=0);
	QTableWidget *table;		//table where positions are displayed
	//widget to print the children of a position
	WidgetChildren *children_popup;
	bool interactionLocked;
	static bool withGraphics;
	static bool gameHasGraphics;
	
  public slots :
	void update_display();		//!< refresh tracing table
	void initialize(bool lockedComputation);

  private slots:
	//function called when the trace table is clicked
	void click_on_position(int row, int column);
	
  signals:
	void requestExplorer();
};

//! Widget to display the children of a position
class WidgetChildren : public QWidget {
	Q_OBJECT
  public:
	//initialization of children's trace widget
	WidgetChildren( QWidget *parent=0);
	
	QTableWidget *parentTable, *currentNodeTable, *childrenTable;	//tables where data is displayed
	QGridLayout *gridLayout;
	storeId requestedId;	//main id of the widget
	bool updateScroll;		//indicate if we should update the scroll position
	int nb_col;				//number of columns
	
	//column indexes
	int optional_col;		//optional information (lives, cells, edges...)
	int traversal_col;		//traversal information
	int unknown_col;		//number of unknown children
	int id_col;				//nodestore ids
	int bcu_col;			//best child unknown value
	int extraPos_col;		//part of the node other than the position (nimber, contract...)
	int pos_col;			//game position
	
	void initTable(QTableWidget *tableW);
	void updateTable(QTableWidget *tableW, list<NodeTrace> ntrace);
	
  public slots :
	void initialize();
	void update_display();		//!< refresh display of the children table
	
  private slots:
	void click_on_currentNode(int row, int column);  //called when the main node is clicked
	void click_on_children(int row, int column); //called when a child is clicked
	void click_on_parent(int row, int column);  //called when a parent is clicked
};

#endif
