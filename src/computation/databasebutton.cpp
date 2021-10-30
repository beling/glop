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

#include "database.h"
#include "databasebutton.h"
#include "../baseclass/basegame.h"
#include "computationthread.h"

//dependency to DisplayBaseWidget
#include "../gamewidget.h"

//dependency to repotable to have the path to the repository
#include "../mainwidget/repofile.h"

#include <QtGui>

DataBaseButton::DataBaseButton( QWidget* parent, int buttonNumBase) : QWidget( parent) {
	num_base = buttonNumBase;
	
	QVBoxLayout *vboxLayout = new QVBoxLayout(this);
	vboxLayout->setSpacing(0);
	vboxLayout->setMargin(0);

	button = new QPushButton;
	connect( button, SIGNAL(clicked()), this, SLOT(baseMenu()) );

	vboxLayout->addWidget(button);
}

void DataBaseButton::refresh_size() {
	unsigned long nb_pos=0;
	QString shortName;
	int status;
	
	if(!db::base_lock.tryLock()) return;	//don't refresh size if database is not available
		nb_pos = db::storeVect[num_base].size();
		shortName = db::storeVect[num_base].shortName;
		status = db::storeVect[num_base].status;
	db::base_lock.unlock();
	
	QString statusString;
	if(status==1) {
		statusString = QString(" N");
	} else
	if(status==2) {
		statusString = QString(" I");
	}
	
	QString	base_size = shortName + QString( " : ") + QString( "%1 " ).arg( nb_pos, 0, 10 ) + statusString;
	button->setText(base_size);
}

//indicate if the corresponding base is empty or not
bool DataBaseButton::baseIsEmpty() {
	return db::storeVect[num_base].empty();
}

//return the size of the base
int DataBaseButton::baseSize() {
	return db::storeVect[num_base].size();
}

//function to handle database buttons
void DataBaseButton::baseMenu() {
	//menu isn't available during computation
	//if(menuDisabled) return;
	repoFileIsDefined = false; //there is no file defined in the repository at the start
	
	//create a context menu
	QMenu *menu = new QMenu ( this );
	
	//"add a database" menu
	QAction *addBaseAction = new QAction( this );
	menu->addAction(addBaseAction);
	addBaseAction->setText("Add base");
	connect( addBaseAction, SIGNAL( triggered() ), this, SLOT(fileOpen()) );
	
	//"save a database" menu
	QAction *saveAction = new QAction( this );
	saveAction->setText(tr("Save"));
	menu->addAction(saveAction);
	connect( saveAction, SIGNAL( triggered() ), this, SLOT(fileSaveAs()) );
	
	//"display a database" menu
	QAction *displayAction = new QAction( this );
	displayAction->setText(tr("Display"));
	menu->addAction(displayAction);
	connect( displayAction, SIGNAL( triggered() ), this, SLOT(displayBase()) );
	
	//"purge a database" menu
	QAction *purgeAction = new QAction( this );
	menu->addAction(purgeAction);
	purgeAction->setText("Purge");
	connect( purgeAction, SIGNAL(triggered()), this, SLOT(confirmPurgeBase()) );
	
	//for check database only, possibility to move to the normal database
	if(num_base%2==1) {
		QAction *moveToNormalAction = new QAction( this );
		menu->addAction(moveToNormalAction);
		moveToNormalAction->setText("Move to normal base");
		connect( moveToNormalAction, SIGNAL(triggered()), this, SLOT(moveToNormal()) );
	}
	
	
	//--------------------Compatibility with old files---------------------
	#ifdef UNSUPPORTED_FEATURE
	menu->addSeparator();
	
	QAction *addBaseCanoAction = new QAction( this );
	menu->addAction(addBaseCanoAction);
	addBaseCanoAction->setText("Add base with cano");
	connect( addBaseCanoAction, SIGNAL( triggered() ), this, SLOT(fileOpenCano()) );
	
	//convert : a useful function used for conversion
	//file is read, and then written, with the convert flag set to true.
	//the old file is saved as old_...
	QAction *convertAction = new QAction( this );
	menu->addAction(convertAction);
	convertAction->setText("Convert files");
	connect( convertAction, SIGNAL(triggered()), this, SLOT(convertFiles()) );
	#endif
	
	menu->popup( button->mapToGlobal(QPoint(0,29)) );
}

//Purge Database
void DataBaseButton::confirmPurgeBase() {
	//confirm message
	QString msg_confirm = QString("Do you really want to purge the ") + db::storeVect[num_base].completeName + QString(" database ?");
	
	//ask confirmation
	//"No" button is default one
	int cancel=QMessageBox::question(this, "Confirmation", msg_confirm, "Yes", "No", QString(), 1, 1);
	
	//database is not deleted if not confirmed
	if(cancel) return;
	purgeBase();
}

void DataBaseButton::purgeBase() {
	//clear the right database and refresh size
	db::storeVect[num_base].clear();
	refresh_size();
}

//note : the normal database associated to a check database is always the previous index
//the current index is num_base (check base), so the previous index num_base-1 is the normal base
void DataBaseButton::moveToNormal() {
	//confirm message if the normal base has not been saved
	if(!db::storeVect[num_base-1].isSaved) {
		QString msg_confirm = QString("The normal base will be purged but has not been saved.\nDo you really want to move the ") + db::storeVect[num_base].completeName + QString(" database ?");
		
		//ask confirmation
		//"No" button is default one
		int cancel=QMessageBox::question(this, "Confirmation", msg_confirm, "Yes", "No", QString(), 1, 1);
		if(cancel) return;
	}
	//move the source (num_base) to the destination (num_base-1)
	db::move(num_base, num_base-1);
	
	//we need to refresh the size of both the current (check) button but also the normal button
	//for this we emit a signal (connected in the end to glopwidget)
	emit globalRefreshNeeded();
}

//Save File Dialog Box. num_base parameter : which database to save
void DataBaseButton::fileSaveAs() {
	//database should not be empty
	db::base_lock.lock();
	bool baseIsEmpty=db::storeVect[num_base].empty();
	db::base_lock.unlock();
	if (baseIsEmpty) {
		QMessageBox::critical( this, "Warning", "There is no position to save.");
		return;
	}
	
	//dialog box
	QString filename = QFileDialog::getSaveFileName( this, "Save database", "", "*.spr");
	if ( !filename.isEmpty() ) {
		//add extension if there isn't
		if (!filename.endsWith(".spr")) {
			filename += ".spr";
		}
		
		setCursor( QCursor(Qt::WaitCursor) );
		write_file(filename, num_base);
		unsetCursor();
	}
}

//Open file Dialog Box
void DataBaseButton::fileOpen() {
	if(!readIsPossible()) return;
	QString filename = QFileDialog::getOpenFileName(this, "Add a database", "", "*.spr");
	if ( !filename.isEmpty() ) {
		setCursor( QCursor(Qt::WaitCursor) );
		read_file(filename, num_base, /*withCano*/ false, /*convert*/ false);
		refresh_size();  //refresh database size widget
		unsetCursor();
	}
}

void DataBaseButton::fileOpenCano() {
	if(!readIsPossible()) return;
	QString filename = QFileDialog::getOpenFileName(this, "Add a database with canonization", "", "*.spr");
	if ( !filename.isEmpty() ) {
		setCursor( QCursor(Qt::WaitCursor) );
		read_file(filename, num_base, /*withCano*/  true, /*convert*/ false);
		refresh_size();  //refresh database size widget
		unsetCursor();
	}
}

//read is not possible for a base that is not empty without merge functionnality
bool DataBaseButton::readIsPossible() {
	if(!baseIsEmpty() && db::storeVect[num_base].nodeIndex == -1) {
		//-1 means that no node is associated to the base for the merge functionnality
		QMessageBox::critical( this, "Warning", "This database doesn't support merge.\nPurge is needed before adding a new file.");
		return false;
	}
	return true;
}

//Save check graph
/*void DataBaseWidget::fileSaveGraph() {
	QString filename = QFileDialog::getSaveFileName(this, "Save check graph", "", "*.dot" );
	if ( !filename.isEmpty() ) {
		//add extension if there isn't
		if (!filename.endsWith(".dot")) {
			filename += ".dot";
		}
		
		//TODO : this functionality was disabled when separating database and glopwidget
		//update graph parameters
		//read_graph_parameters();
		
		//write graph file
		setCursor( QCursor(Qt::WaitCursor) );
		Game::check_graph.write_file(filename);
		unsetCursor();
	}
}*/

//Write a base in the specified file
void DataBaseButton::write_file( const QString &filename, int num_base ) {
	QMutexLocker locker(&(db::base_lock));
	
	//open file
	QFile the_file( filename );
	if ( !the_file.open( QIODevice::WriteOnly ) ) return;
	
	//write database
	QTextStream text( &the_file );
	db::storeVect[num_base].write(text);
	
	//close file
	the_file.close();
}

//Add positions from the file in the given base
void DataBaseButton::read_file( const QString &filename, int num_base, bool withCano, bool convert) {
	//this cause a dead-lock with the new function BaseNode::treatBaseCollision
	//QMutexLocker locker(&(db::base_lock));
	
	//open file
	QFile the_file( filename );
	if ( !the_file.open( QIODevice::ReadOnly ) ) return;
	
	QTextStream text( &the_file );
	
	//read database id
	if (!text.atEnd() ) {
		//headers
		if (text.readLine()!=db::storeVect[num_base].headerName) {
			QMessageBox::critical( this, "Warning", "This file is for a different database.");
			the_file.close();
			return;
		}
	}
	
	//read database
	if(!db::storeVect[num_base].read(text, withCano, convert) ) {
		QMessageBox::critical( this, "Warning", "A position is different in the database and in the file.\nLoading interrupted.");
	}
	//close file
	the_file.close();
}

void DataBaseButton::convertFiles() {
	QStringList filenames = QFileDialog::getOpenFileNames(this, "Choose the files to convert", "", "*.spr");
	if(filenames.isEmpty()) return;
	setCursor( QCursor(Qt::WaitCursor) );
	
	Parameter::isConvertingFiles=true; //indicate to any concerned function that we are converting files
	int i;
	//loop on the list of files
	for(i=0; i<filenames.size(); i++) {
		//clear the database
		db::storeVect[num_base].clear();
		
		//read the file
		cout << "Reading " << filenames.at(i).toStdString() << " ..." << endl;
		read_file(filenames.at(i), num_base, /*withCano*/ false, /*convert*/ true);
		
		//change the name of the original file
		QString copyName = filenames.at(i);
		copyName.insert(copyName.size()-4, "_sav");
		cout << "   -> renaming in " << copyName.toStdString() << endl;
		QFile::rename(filenames.at(i), copyName);
		
		//save the database
		cout << "   -> writing in the new format ..." << flush;
		write_file(filenames.at(i), num_base);
		cout << " completed" << endl;
		
		//clear the database
		db::storeVect[num_base].clear();
	}
	
	Parameter::isConvertingFiles=false;
	unsetCursor();
}

//! save the database and return the name of the file (without the path)
//! newFile : indicate if we need to create a new file or use the already existing file
QString DataBaseButton::autoSave(bool newFile) {
	//create a new file if requested or if it is the first time that we save the database
	if(newFile || !repoFileIsDefined) {
		repoFileIsDefined = true;
		shortRepoFileName = RepoFile::getNewFileName(QString("db"));
		QString completeFileName = RepoFile::getAbsoluteFileName(shortRepoFileName);
		write_file(completeFileName, num_base);
		refresh_size();
		lastSavedSize = db::storeVect[num_base].size();
		return shortRepoFileName;
	} else {
		//the database has been already saved in shortRepoFileName
		//save again if the number of positions has changed
		if(lastSavedSize != db::storeVect[num_base].size()) {
			QString completeFileName = RepoFile::getAbsoluteFileName(shortRepoFileName);
			write_file(completeFileName, num_base);
			refresh_size();
			lastSavedSize = db::storeVect[num_base].size();
		}
		return shortRepoFileName;
	}
}

//! clear and read the database
void DataBaseButton::autoRead(QString fileName) {
	db::storeVect[num_base].clear();
	QString completeFileName = RepoFile::getAbsoluteFileName(fileName);
	read_file(completeFileName, num_base, /*withCano*/ false, /*convert*/ false);
	refresh_size();
	
	repoFileIsDefined = true;
	shortRepoFileName = fileName;
	lastSavedSize = db::storeVect[num_base].size();
}

//! clear the database and init the needed parameters
void DataBaseButton::autoPurgeBase() {
	purgeBase();
	repoFileIsDefined = false;
}

//======================= Display ==========================

void DataBaseButton::displayBase() {
	DisplayBaseWidget displayBase(this, num_base);
	displayBase.exec();
}
