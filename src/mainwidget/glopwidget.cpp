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
#include "widgetpopup.h"

#ifdef USE_REPOWIDGET
#include "repowidget.h"
#endif

#include "../gamewidget.h"

#include "../computation/trace.h"
#include "../computation/nodestore.h"
#include "../computation/computationthread.h"
#include "../computation/tracewidget.h"
#include "../computation/database.h"

#include "../baseclass/basegame.h"
#include "../baseclass/basenode.h"
#include "../baseclass/interfacewidget.h"

#include <QtGui>

QString GlopWidget::glopVersionNumber("2.2");

//--------------------------------------------------------------------------
//Interface definition
//--------------------------------------------------------------------------

vector<InterfaceWidget *> GlopWidget::gameWidgets;
vector<InterfaceWidget *> GlopWidget::nodeWidgets;
vector<InterfaceWidget *> GlopWidget::traversalWidgets;
vector<InterfaceWidget *> GlopWidget::commonWidgets;
QTabWidget *GlopWidget::tabNodeWidget;
QTabWidget *GlopWidget::tabWidget;
QComboBox *GlopWidget::box_game_choice;
QComboBox *GlopWidget::box_traversal_choice;

GlopWidget::GlopWidget( QWidget *parent) : QMainWindow( parent ) {
	//icon on the upper left corner and in the taskbar
	setWindowIcon(QIcon(QString(":/images/glop-24x32.png") ));
	
	//window title
	setWindowTitle(QString("GLOP ") + glopVersionNumber);
	compThread = new ComputationThread; //used to launch a computation in a separate thread
	
	computeRunning = false;
	pauseEnabled = false;
	noMoreMessage = false;
	
	//timer
	impulse = new QTimer(this);
	connect( this, SIGNAL(isComputing(bool)), this, SLOT(activate_timer(bool)) );
	
	//on each impulse, check result state and refresh tables tracing computation
	connect( impulse, SIGNAL(timeout()), this, SLOT(wait_result()) );
	connect( impulse, SIGNAL(timeout()), this, SLOT(trace_refresh()) );
	
	//error handling (check error on each timer impulse)
	WidgetError *widget_error= new WidgetError(this);
	connect( impulse, SIGNAL(timeout()), widget_error, SLOT(receive_error()) );
	
	//connexion mapping
	readText_Mapper=new QSignalMapper(this);
	connect( readText_Mapper, SIGNAL(mapped(int)), this, SLOT(read_text_file(int)) );
	
	//layout
	QWidget *widget_central = new QWidget( this );
	setCentralWidget(widget_central);
	QVBoxLayout *central_Vlayout = new QVBoxLayout(widget_central);	//vertical layout of main window
	
	//-----upper box-------------------------
	QGridLayout *upper_box = new QGridLayout;	//horizontal layout of top part
	central_Vlayout->addLayout( upper_box );
	upperPartIsVisible = true;
	
	//save icon
	#ifdef USE_REPOWIDGET
	QPushButton *save_button = new QPushButton( "Save", widget_central );
	save_button->setFixedSize(save_button->sizeHint());
	upper_box->addWidget( save_button, 0, 0);
	connect( this, SIGNAL(isComputing(bool)), save_button, SLOT(setDisabled(bool)) );
	connect( this, SIGNAL(setVisibleUpperPart(bool)), save_button, SLOT(setVisible(bool)) );
	#endif
	
	//glop icon
	QLabel *labelIcon = new QLabel( widget_central );
	QPixmap *pixIcon = new QPixmap(":/images/glop-100x46.png");
	labelIcon->setPixmap(*pixIcon);
	labelIcon->setFixedSize( 100, 46 );
	upper_box->addWidget( labelIcon, 0, 0, 3, 1 );
	connect( this, SIGNAL(setVisibleUpperPart(bool)), labelIcon, SLOT(setVisible(bool)) );
	
	//upper tabs for the node kinds
	tabNodeWidget = new QTabWidget(widget_central);
	upper_box->addWidget( tabNodeWidget, 0, 1, 3, 1);
	connect( this, SIGNAL(isComputing(bool)), tabNodeWidget, SLOT(setDisabled(bool)) );
	connect( this, SIGNAL(setVisibleUpperPart(bool)), tabNodeWidget, SLOT(setVisible(bool)) );
	
	int i;
	int n = BaseNode::nodesNumber();
	for(i=0; i<n; i++) {
		//initialize the databases
		BaseNode(i, 0 /*useless value*/).createDataStorage();
		
		Interface paramI = BaseNode(i, 0 /*useless value*/).getParamDef();
		InterfaceWidget * paramW = new InterfaceWidget;
		paramW->init(paramI);
		tabNodeWidget->addTab(paramW, QString(paramI.name.c_str()));
		nodeWidgets.push_back(paramW);
		
		//refresh the values on each impulse : used to refresh the databaseButtons
		connect( impulse, SIGNAL(timeout()), paramW, SLOT(refreshValues()) );
		
		//a button can ask a global refresh, this will be transmitted to all the other buttons
		//(used when moving a base to another)
		connect( paramW, SIGNAL(globalRefreshNeeded()), this, SIGNAL(globalRefreshNeeded()) );
		connect( this, SIGNAL(globalRefreshNeeded()), paramW, SLOT(refreshValues()) );		
	}
	
	//Start button
	QPushButton *start_button = new QPushButton( "Start", widget_central );
	start_button->setFixedSize(start_button->sizeHint());
	upper_box->addWidget( start_button, 0, 2);
	connect( start_button, SIGNAL(clicked()), this, SLOT(startComputation()) );
	connect( this, SIGNAL(isComputing(bool)), start_button, SLOT(setDisabled(bool)) );
	connect( this, SIGNAL(setVisibleUpperPart(bool)), start_button, SLOT(setVisible(bool)) );
	
	//Pause button
	pause_button = new QPushButton( "Pause", widget_central );
	pause_button->setEnabled(false);
	pause_button->setFixedSize(pause_button->sizeHint());
	upper_box->addWidget( pause_button, 1, 2);
	connect( pause_button, SIGNAL(clicked()), this, SLOT(pause_computing()) );
	connect( this, SIGNAL(isComputing(bool)), pause_button, SLOT(setEnabled(bool)) );
	
	//Stop button
	QPushButton *stop_button = new QPushButton( "Stop", widget_central );
	stop_button->setEnabled(false);
	stop_button->setFixedSize(stop_button->sizeHint());
	upper_box->addWidget( stop_button, 2, 2);
	connect( stop_button, SIGNAL(clicked()), this, SLOT(stop_computing()) );
	connect( this, SIGNAL(isComputing(bool)), stop_button, SLOT(setEnabled(bool)) );
	connect( this, SIGNAL(setVisibleUpperPart(bool)), stop_button, SLOT(setVisible(bool)) );
	
	//Label to display the state of NodeStore
	nodeStoreState = new QLabel;
	nodeStoreState->setText(QString("AAAAAAAAAAAAAAAAAAAAAAAAAAA\nA\nA\nA\n")); //to set the size
	nodeStoreState->setFixedSize(nodeStoreState->sizeHint());
	nodeStore_refresh();
	upper_box->addWidget( nodeStoreState, 0, 3, 3, 1);
	connect( impulse, SIGNAL(timeout()), this, SLOT(nodeStore_refresh()) );
	connect( this, SIGNAL(setVisibleUpperPart(bool)), nodeStoreState, SLOT(setVisible(bool)) );
	
	//-----Tabs in the main window-----------
	tabWidget = new QTabWidget;
	central_Vlayout->addWidget( tabWidget );
	setup_parameters_tab();
	setup_compute_tab();
	setup_test_tab();
	setupFiletab();
	setup_info_tab();
	setupGameWidgetTab();
	connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabClicked(int)) );
	connect( this, SIGNAL(isComputing(bool)), this, SLOT(initChildrenAfterComputation(bool)) );
	
	//the connection of the save button can be done only after the initialization of repowidget
	#ifdef USE_REPOWIDGET
	connect( save_button, SIGNAL(clicked()), repowidget, SLOT(saveSession()) );
	#endif
}

//Parameters tab
void GlopWidget::setup_parameters_tab() {
  //Main widget, layout and connection of this tab
	QWidget *parameters_tab = new QWidget;
	//Tab disabled when computing
	connect( this, SIGNAL(isComputing(bool)), parameters_tab, SLOT(setDisabled(bool)) );
	//Vertical Layout
	QVBoxLayout *parameters_tab_Vlayout = new QVBoxLayout( parameters_tab );
	
	//------------- Game choice --------------------------
	box_game_choice = new QComboBox;
	parameters_tab_Vlayout->addWidget( box_game_choice );
	
	//create a new choice for each registered game
	int nbGames = BaseGame::gamesNumber();
	int i;
	int DefaultGameIndex=0;
	for(i=0; i<nbGames; i++) {
		Interface paramI = BaseGame(i).getParamDef();
		if(paramI.name==string("Dots and Boxes")) DefaultGameIndex=i;
		box_game_choice->insertItem(i, QString(paramI.name.c_str()));
		InterfaceWidget * paramW = new InterfaceWidget;
		paramW->init(paramI);
		paramW->setVisible(i==0);
		parameters_tab_Vlayout->addWidget(paramW);
		gameWidgets.push_back(paramW);
	}
	
	box_game_choice->setFixedSize(box_game_choice->sizeHint());
	connect( box_game_choice, SIGNAL(currentIndexChanged(int)), this, SLOT(gameChoice(int)) );
	box_game_choice->setCurrentIndex(DefaultGameIndex); //choice by default
	gameChoice(DefaultGameIndex);						//needed for the particular case where the default game was already the one displayed 
	
	//------------- Traversal choice --------------------------
	box_traversal_choice = new QComboBox;
	parameters_tab_Vlayout->addWidget( box_traversal_choice );
	
	//create a new choice for each registered traversal algorithm
	int nbTraversal = BaseTraversal::classesNumber();
	int DefaultTravIndex=0;
	for(i=0; i<nbTraversal; i++) {
		Interface paramI = BaseTraversal(i).getParamDef();
		if(paramI.name==string("Alpha-beta")) DefaultTravIndex=i;
		box_traversal_choice->insertItem(i, QString(paramI.name.c_str()));
		InterfaceWidget * paramW = new InterfaceWidget;
		paramW->init(paramI);
		paramW->setVisible(i==0 && nbTraversal>1);
		parameters_tab_Vlayout->addWidget(paramW);
		traversalWidgets.push_back(paramW);
	}
	
	box_traversal_choice->setFixedSize(box_traversal_choice->sizeHint());
	connect( box_traversal_choice, SIGNAL(currentIndexChanged(int)), this, SLOT(traversalChoice(int)) );
	box_traversal_choice->setCurrentIndex(DefaultTravIndex); //choice by default
	if(nbTraversal<=1) box_traversal_choice->setVisible(false);
	
	//--------- common interfaces ------------------------------------
	vector<Interface> commonInterfaces;
	
	//Options
	Interface interface2;
	interface2.title="General options";
	interface2.addLabel("Refresh time (x 100 ms) :", /*pos*/ 0, 0);
	interface2.addSpinBox(1, 10, 1, /*pos*/ 0, 1);
	interface2.link(refreshTime);
	//interface2.addCheckButton("Use graph :", false, /*pos*/ 1, 0);
	//interface2.link(game->check_graph.use_graph);
	#ifdef UNSUPPORTED_FEATURE
	interface2.addLabel("Debug level :", /*pos*/ 0, 2);
	interface2.addSpinBox(0, 10, 0, /*pos*/ 0, 3);
	interface2.link(NodeStore::debugLevel);
	#endif
	interface2.addCheckButton("Use the minimal recursive structure (no trace)", false, 2, 0);
	interface2.link(compThread->recursiveStructure);
	interface2.addCheckButton("Normal check after random cycles", false, 2, 1);
	interface2.link(Parameter::LastCycleIsNormal);
	interface2.addCheckButton("Use compression (Dots-and-Boxes only)", true, 2, 2);
	interface2.setToolTip("Positions are represented with a more compact string.\nThis feature is available only for Dots-and-Boxes.");
	interface2.link(Parameter::useCompression);
	interface2.addLabel("Positions limit (in millions) :", /*pos*/ 3, 0);
	interface2.addSpinBox(0, 1000, 0, /*pos*/ 3, 1);
	interface2.link(mDatabasePositionsLimit);
	interface2.addCheckButton("Trace with graphics", true, /*pos*/ 3, 2);
	interface2.link(WidgetTrace::withGraphics);
	commonInterfaces.push_back(interface2);
	
	//---- Traversal ---
	//Interface traversalInterface = PN_Search::getParamDef();
	//commonInterfaces.push_back(traversalInterface);
	
	//Graph options
	//Graph options are not available until a new Graph object is written
	/*
	Interface interface3;
	interface3.title="Graph Options";
	interface3.addCheckButton("Use colors :", false, 0, 0);
	interface3.link(game->check_graph.use_colors);
	interface3.addLabel("Detail level :", 1, 0);
	interface3.addSpinBox(1, 3, 2, 1, 1);
	interface3.link(game->check_graph.detail_level);
	interface3.addLabel("Minimal number of lives :", 2, 0);
	interface3.addSpinBox(0, 99, 0, 2, 1);
	interface3.link(game->check_graph.min_lives);
	commonInterfaces.push_back(interface3);
	*/
	
	//add the common interfaces to glopwidget
	vector<Interface>::iterator Li;
	for(Li=commonInterfaces.begin(); Li!=commonInterfaces.end(); Li++) {
		InterfaceWidget * paramW = new InterfaceWidget;
		paramW->init(*Li);
		parameters_tab_Vlayout->addWidget(paramW);
		commonWidgets.push_back(paramW);
	}
	
	//add Parameter tab to the tabWidget
	tabWidget->addTab(parameters_tab, "Parameters");
}

void GlopWidget::gameChoice(int index) {
	Parameter::gameChoice = index;
	int i;
	int nbGames = BaseGame::gamesNumber();
	for(i=0; i<nbGames; i++) {
		InterfaceWidget * paramW = gameWidgets[i];
		paramW->setVisible(i==index);
	}
	
	//display only the possible nodes for the game category
	selectPossibleNode(BaseGame(Parameter::gameChoice).gameCategory());
	
	//init the internal parameters
	BaseGame(Parameter::gameChoice).setParam();
}

void GlopWidget::traversalChoice(int index) {
	Parameter::traversalChoice = index;
	int i;
	int nbClasses = BaseTraversal::classesNumber();
	for(i=0; i<nbClasses; i++)  {
		InterfaceWidget * paramW = traversalWidgets[i];
		paramW->setVisible(i==index && nbClasses>1);
	}
}

void GlopWidget::selectPossibleNode(int gameCategory) {
	int i;
	int n = BaseNode::nodesNumber();
	bool currentIndexCorrect=false;
	int firstCorrectIndex=n-1;
	int nimberIndex=-1;	//We want NimberNode as the default Node for Impartial games
	
	for(i=0; i<n; i++) {
		GameType t = BaseNode(i, 0 /*useless value*/).gameType();
		
		Interface paramI = BaseNode(i, 0).getParamDef();//these 2 lines allow us to find the Nimber Index
		if(paramI.name==string("Nimber")) nimberIndex=i;
		
		bool possibleNode = t.isFor[gameCategory];
		tabNodeWidget->setTabEnabled(i, possibleNode);
		if(tabNodeWidget->currentIndex()==i) {
			currentIndexCorrect = possibleNode;
		}
		if(possibleNode && i< firstCorrectIndex) {
			firstCorrectIndex = i;
		}
	}
	
	//For an Impartial game, select "Nimber" tab
	if(gameCategory==GameType::ImpartialGame) {
		tabNodeWidget->setCurrentIndex(nimberIndex);
	} else
	//set the current tab to a valid tab (if needed)
	if(!currentIndexCorrect) {
		tabNodeWidget->setCurrentIndex(firstCorrectIndex);
	}
}

//Computation and Explore tab
void GlopWidget::setup_compute_tab() {
  //Layout for this tab
	QWidget *compute_tab = new QWidget;			//main widget of Computation tab
	//connect( this, SIGNAL(isChecking(bool)), compute_tab, SLOT(setDisabled(bool)) );
	QVBoxLayout *compute_tab_Vlayout = new QVBoxLayout( compute_tab );	//layout for this tab
	compute_tab_Vlayout->setSpacing(8);
	compute_tab_Vlayout->setMargin(5);
	
  //Table to trace computation
	widget_trace=new WidgetTrace(compute_tab);
	connect( this, SIGNAL(isComputing(bool)), widget_trace->table, SLOT(setEnabled(bool)) );
	connect( widget_trace, SIGNAL(requestExplorer()), this, SLOT(displayExplorer()) );
	compute_tab_Vlayout->addWidget( widget_trace );
	
	//Explore tab
	explore_tab = new QWidget;			//main widget of Explore tab
	//connect( this, SIGNAL(isChecking(bool)), compute_tab, SLOT(setDisabled(bool)) );
	QVBoxLayout *explore_tab_Vlayout = new QVBoxLayout( explore_tab );	//layout for this tab
	explore_tab_Vlayout->setSpacing(8);
	explore_tab_Vlayout->setMargin(5);
	connect( this, SIGNAL(isComputing(bool)), widget_trace->children_popup, SLOT(setEnabled(bool)) );
	explore_tab_Vlayout->addWidget( widget_trace->children_popup );
	
  //add Computation tab to tabWidget
	tabWidget->addTab( compute_tab, "Computing branch" );
	tabWidget->addTab( explore_tab, "Search tree" );
}

//display the explorer (called when we click on a position of the computation tab)
void GlopWidget::displayExplorer() {
	tabWidget->setCurrentWidget(explore_tab);
}

//--------------------------------------------------------------------------
//Starts/Stops computation
//--------------------------------------------------------------------------

void GlopWidget::readModuleParameters() {
	//read the parameters of this kind of node
	Parameter::nodeChoice = tabNodeWidget->currentIndex();
	nodeWidgets[Parameter::nodeChoice]->copyInTarget();
	Parameter::computation_kind = BaseNode(Parameter::nodeChoice, 0 /*useless value*/).startComputationKind();
	
	//read the parameters specific to the chosen game
	InterfaceWidget * paramW = gameWidgets[Parameter::gameChoice];
	paramW->copyInTarget();
	BaseGame(Parameter::gameChoice).setParam();
	
	//read the parameters of the chosen traversal
	traversalWidgets[Parameter::traversalChoice]->copyInTarget();
	
	//read the parameters of all the common widgets
	vector<InterfaceWidget *>::iterator Wi;
	for(Wi=commonWidgets.begin(); Wi!=commonWidgets.end(); Wi++) {
		(*Wi)->copyInTarget();
	}
}

void GlopWidget::startComputation() {
	//read the parameters of the different modules (game, node, traversal)
	readModuleParameters();
	
	Parameter::debugGames=false; //initialize the debug variable for games (no debug)
	
	//initialize the trace
	Trace::initialize();
	bool lockedComputation = BaseNode(Parameter::nodeChoice, 0 /*useless value*/).lockedComputation();
	widget_trace->initialize(lockedComputation);
	
	//signals that computation started
	Trace::set_stop_value(false);
	compThread->computationRunning=true;
	computeRunning=true;
	emit isComputing(TRUE);
	
	//call ComputationThread::run() as a separate thread with the special start() function
	compThread->start(QThread::NormalPriority);
}

//stops computation
void GlopWidget::stop_computing() {
	//launch the signal of end of computation
	Trace::set_stop_value(true);
	
	//end the pause if necessary
	if(pauseEnabled) {
		Trace::pauseEnd();
		pause_button->setText("Pause");
		pauseEnabled = false;
	}
}

void GlopWidget::pause_computing() {
	if(pauseEnabled) {
		Trace::pauseEnd();
		pause_button->setText("Pause");
	} else {
		Trace::pauseStart();
		pause_button->setText("Restart");
	}
	pauseEnabled = !pauseEnabled;
}

//--------------------------------------------------------------------------
//Functions to trace computation
//--------------------------------------------------------------------------

//timer is activated when we receive true
//activate_timer is connected to isComputing
void GlopWidget::activate_timer(bool val) {
	if(val) {
		impulse->start(refreshTime*100);
	} else {
		impulse->stop();
	}
}

//refresh widgets on each timer's impulse
void GlopWidget::trace_refresh() {
	widget_trace->update_display();
}

//refresh the label with the state of NodeStore on each timer's impulse
void GlopWidget::nodeStore_refresh() {
	nodeStoreState->setText(NodeStore::stateString());
}

//Print Computation result
void GlopWidget::wait_result() {
	bool isRunning;
	
	if(!compThread->compthread_mutex.tryLock()) return;		//return if lock failed
		isRunning=compThread->computationRunning;
	compThread->compthread_mutex.unlock();
	
	if (isRunning) {
		//computation is still running
		//stop the computation if the limit number of positions have been reached
		if(mDatabasePositionsLimit!=0) { //0 means "no limit"
			int currentPositionsNumber = db::totalSize();
			if(currentPositionsNumber >= mDatabasePositionsLimit*1000000) {
				Trace::set_stop_value(true);
			}
		}
		return;
	}
	
	emit isComputing(FALSE); //signals computation ended
	emit isChecking(FALSE);
	computeRunning=false;
	
	//at the end of the computation, shows back the upper part of the interface
	if(!upperPartIsVisible) {
		upperPartIsVisible = ! upperPartIsVisible;
		emit setVisibleUpperPart(upperPartIsVisible);
	}
	
	if(noMoreMessage) {
		//user has asked to quit the program as soon as computation ends
		qApp->exit(0);
		return;
	}
	
	QString positionName;
	positionName = BaseNode(Parameter::nodeChoice, 0 /*useless value*/).getPositionName().c_str();
	
	//display message
	QString message;
	QString message_title="Result";
	
	if(Trace::stop_signal()) {
		message+="Computation stopped.";
	} else {
		message += QString("Game     : %1\n").arg(box_game_choice->currentText());
		message += QString("Position : %1\n").arg(positionName);
		message += QString("Result    : %1\n").arg(BaseNode::computationResult.c_str());
	}
	
	QMessageBox::information (this, message_title, message, QMessageBox::Ok);
}

//--------------------------------------------------------------------------
//  Close Glop function
//--------------------------------------------------------------------------

//check that databases are saved when user closes the program
void GlopWidget::closeEvent(QCloseEvent *event) {
	if( !computeRunning && db::allSaved()) {
		//accept the close event (end of the program)
		event->accept();
		return;
	}
	
	//determine error message
	QString message;
	if(computeRunning) {
		message += "A computation is running.";
	} else {
		message += db::unsavedBaseName() + " base has not been saved.";
	}
	
	message += "\nDo you really want to quit?";
	
	//print message
	int cancel=QMessageBox::question(this, "Confirmation", message, "Yes", "No", QString(), 1, 1);
	
	if(cancel) {
		//user canceled : ignore the close event
		event->ignore();
	} else {
		if(computeRunning) {
			//user chose to quit : stop the computation with noMoreMessage set to true
			noMoreMessage = true;
			stop_computing();
			event->ignore(); //the program will quit as soon as computation ends
		} else {
			event->accept();
		}
	}
}

//Fullscreen Mode support
void GlopWidget::keyPressEvent(QKeyEvent *event) {
	//we reimplement only the F11 key
	//for all other keys, we call the base class for normal operations
	switch (event->key()) {
		case Qt::Key_F11:
			//switch between normal and fullscreen mode
			setWindowState(windowState() ^ Qt::WindowFullScreen);
			break;
		case Qt::Key_F10:
			//hide or show the upper part of the interface (icon, nodes, stop, start button)
			upperPartIsVisible = ! upperPartIsVisible;
			emit setVisibleUpperPart(upperPartIsVisible);
			break;
		default:
			QMainWindow::keyPressEvent(event);
	}
}

//--------------------------------------------------------------------------
//  Information tab / repo tab
//--------------------------------------------------------------------------

//Info tab
void GlopWidget::setup_info_tab() {
	QWidget *info_tab = new QWidget;
	QGridLayout *grid_info = new QGridLayout( info_tab );
	
  //Images
	QLabel *labelHelp = new QLabel( info_tab );
	QPixmap *pixHelp = new QPixmap(":/images/help64.png");
	labelHelp->setPixmap(*pixHelp);
	labelHelp->setFixedSize( 64, 64 );
	grid_info->addWidget( labelHelp, 0, 0 );
	
	QLabel *labelLicence = new QLabel( info_tab );
	QPixmap *pixLicence = new QPixmap(":/images/licence64.png");
	labelLicence->setPixmap(*pixLicence);
	labelLicence->setFixedSize( 64, 64 );
	grid_info->addWidget( labelLicence, 0, 1 );
	
	QLabel *labelAbout = new QLabel( info_tab );
	QPixmap *pixAbout = new QPixmap(":/images/about64.png");
	labelAbout->setPixmap(*pixAbout);
	labelAbout->setFixedSize( 64, 64 );
	grid_info->addWidget( labelAbout, 0, 2 );
	
  //Buttons
	QPushButton *help_button = new QPushButton( "Help", info_tab );
	help_button->setFixedSize(help_button->sizeHint());
	grid_info->addWidget( help_button, 1, 0 );
	connect( help_button, SIGNAL(clicked()), readText_Mapper, SLOT( map() ) );
	readText_Mapper->setMapping(help_button, 0);
	
	QPushButton *licence_button = new QPushButton( "Licence", info_tab );
	licence_button->setFixedSize(licence_button->sizeHint());
	grid_info->addWidget( licence_button, 1, 1 );
	connect( licence_button, SIGNAL(clicked()), readText_Mapper, SLOT( map() ) );
	readText_Mapper->setMapping(licence_button, 1);
	
	QPushButton *about_button = new QPushButton( "About", info_tab );
	about_button->setFixedSize(about_button->sizeHint());
	grid_info->addWidget( about_button, 1, 2 );
	connect( about_button, SIGNAL(clicked()), readText_Mapper, SLOT( map() ) );
	readText_Mapper->setMapping(about_button, 2);
	
	tabWidget->addTab( info_tab, "Information" );
}

//open the right popup
void GlopWidget::read_text_file(int num) {
	if(num==0) {
		WidgetPopup popup( this, QString(":/txt/help.html"), QString("Help"));
		popup.exec();
	} else if(num==1) {
		WidgetPopup popup( this, QString(":/txt/gpl.html"), QString("Licence"));
		popup.exec();
	} else if(num==2) {
		WidgetPopup popup( this, QString(":/txt/about.html"), QString("About"));
		popup.exec();
	}
}

//Tab for RepoWidget
void GlopWidget::setupFiletab() {
	#ifdef USE_REPOWIDGET
	QWidget *fileTab = new QWidget;						//main widget of File tab
	QGridLayout *fileGrid = new QGridLayout( fileTab );	//layout for this tab
	repowidget = new RepoWidget;
	fileGrid->addWidget( repowidget, 0, 0);
	tabWidget->addTab( fileTab, "Repository" );			//add File tab to tabWidget
	#endif
}

void GlopWidget::setupGameWidgetTab() {
	#ifdef UNSUPPORTED_FEATURE
	QWidget *gameTab = new QWidget;						//main widget of GameWidget tab
	QGridLayout *gameGrid = new QGridLayout(gameTab);	//layout for this tab
	GameWidget *gamewidget = new GameWidget;
	gameGrid->addWidget( gamewidget, 0, 0);
	tabWidget->addTab( gameTab, "Game Widget" );		//add GameWidget tab to tabWidget
	#endif
}
