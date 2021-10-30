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

#ifndef DATABASEBUTTON_H
#define DATABASEBUTTON_H

#include "../global.h"
#include "../gameglobal.h"

#include <QWidget>

class QPushButton;
class QSignalMapper;
class QLabel;
class QTextStream;

class DataBaseButton : public QWidget {
  Q_OBJECT
	
  public:
	DataBaseButton( QWidget* parent = 0, int buttonNumBase=0);

	QPushButton *button;
	bool repoFileIsDefined;	//indicate if the database is already associated to a file of the repository
	QString shortRepoFileName;		//name of the file in the repository
	int lastSavedSize;  //size of the database when last saved
	int num_base;
	
	//files read/write functions
	void write_file( const QString &filename , int num_base);
	void read_file( const QString &filename, int num_base, bool withCano, bool convert );
	bool readIsPossible();
	QString autoSave(bool newFile);	//!< save the database and return the name of the file
	void autoRead(QString fileName);
	void autoPurgeBase();
	bool baseIsEmpty();		//indicate if the corresponding base is empty or not
	int baseSize();				//return the size of the base
	
  public slots :
	void refresh_size();
	void baseMenu();
	void confirmPurgeBase();
	void moveToNormal();
	void purgeBase();
	void fileSaveAs();
	void fileOpen();
	void fileOpenCano();
	void convertFiles();
	void displayBase();
	
  signals:
	void globalRefreshNeeded();
};

#endif
