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

#ifndef GLOPWIDGET_H
#define GLOPWIDGET_H

#include <QMainWindow>
#include <QWidget>
#include "../global.h"

//conditional compilation of Repowidget
//#define USE_REPOWIDGET

//graphical user interface classes
class QTimer;
class QLabel;
class QString;
class QTabWidget;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QPushButton;
class QSignalMapper;
class QLineEdit;
class QCloseEvent;
class QComboBox;
class QGroupBox;
class QTableWidget;

class WidgetTrace;
class InterfaceWidget;
class ComputationThread;

#ifdef USE_REPOWIDGET
class RepoWidget;
#endif

//class for the main window
class GlopWidget : public QMainWindow {
	Q_OBJECT
  public:
	GlopWidget( QWidget *parent=0 );
  
  //object used to launch a computation in a separate thread
	ComputationThread *compThread;
	
  //graphical interface
	//tabs
	static QTabWidget *tabNodeWidget;
	static QTabWidget *tabWidget;
	QWidget *explore_tab;
	void setup_parameters_tab();
	void setup_compute_tab();
	void setup_test_tab();
	void setup_info_tab();
	void setupFiletab();
	void setupGameWidgetTab();
	
	//computation parameters
	static QComboBox *box_game_choice, *box_traversal_choice;
	static vector<InterfaceWidget *> gameWidgets, nodeWidgets, traversalWidgets, commonWidgets;
	
	//general parameters
	int refreshTime;
	int mDatabasePositionsLimit; //limit on the total number of positions stored in the database (in millions)
	
	//connexion mapping
	QSignalMapper *readText_Mapper;
	
	//tables to trace computation
	WidgetTrace *widget_trace;
	bool upperPartIsVisible;
	
	QPushButton *pause_button;
	
	//label to display the state of nodestore
	QLabel *nodeStoreState;
	
	//to print children
	QLineEdit *box_given_position_2;
	list<QString> positionCache;
	QTableWidget * parentDisplayTable;
	QTableWidget * childrenDisplayTable;
	int childrenWidgetIndex;
	
	static QString glopVersionNumber;
	
	//repowidget
	#ifdef USE_REPOWIDGET
	RepoWidget *repowidget;
	#endif
	
  protected:
	//override QApplication virtual function closeEvent and keyPressEvent
	void closeEvent(QCloseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	
	bool computeRunning;
	bool pauseEnabled;
	bool noMoreMessage;	//used to avoid the message when closing during a computation
	
  public slots:
	//menu
	void read_text_file(int num);
	
	//start/stop computation
	void startComputation();
	void stop_computing();
	void pause_computing();
	void readModuleParameters();
	
	void gameChoice(int);  //choose a different game in the parameter tab
	void selectPossibleNode(int);  //enable/disable nodes depending on the game category
	void traversalChoice(int);  //choose a different traversal in the parameter tab
	
  //trace computation related functions
	void activate_timer(bool);
	void trace_refresh(); //called on each timer's impulse
	void nodeStore_refresh();
	void wait_result();
	void displayExplorer();
	
  //children tab functions
	void initChildrenAfterComputation(bool compute);
	void initChildrenTab();
	void printChildren();
	void nextChildren(int, int);
	void previousChildren();
	void tabClicked(int index);
	void parentPositionInfo();
	
  signals:
	void isComputing(bool);			//specifies that the computation starts/stops
	void isChecking(bool);			//check computing only
	void globalRefreshNeeded();		//refresh all the interface
	void setVisibleUpperPart(bool);	//hide or show the upper part of the interface
	
  private:
	QTimer * impulse; //timer object with regular impulsion to refresh the graphical interface
};

#endif
