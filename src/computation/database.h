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

#ifndef DATABASE_H
#define DATABASE_H

#include "../global.h"
#include "../gameglobal.h"

#include <QWidget>

class QLabel;
class QTextStream;

//object to store a database
class DataStorage {
  public:
	DataStorage() {kind=0; isSaved=true;};
	
	//different kind of databases
	map<string, nimber> mapStringNimber; //kind 0
	map<string, string> mapStringString; //kind 1
	set<string> setString; //kind 2
	int kind;
	bool isSaved;
	int status; //used in check computations : 0=normal, 1=new result, 2=inconsistent
	QString shortName, completeName, headerName;
	void init(int kindInit, QString shortNameInit, QString completeNameInit, QString headerNameInit, int nodeIndexInit, int baseIndexInit, bool isCheckBaseInit);
	
	//parameters used when calling a node to merge a base and a file
	int nodeIndex;		//used to remember which node is associated to this database
	int baseIndex;		//used to remember the index of this base (used only
	bool isCheckBase;	//used to remember if the base is a check base or not
	
	//usual functions
	int size();
	void clear();
	bool empty();
	
	//functions to add data in the database
	void add(const string &a);
	void add(const string &a, nimber b);
	void add(const string &a, const string &b);
	
	//functions to find data in the database
	bool find(const string &a);
	bool findInverse(const string &a);  //available only for <string, string> type of database
	nimber foundNimber();
	string foundString();
	string foundInverse();
	bool findIndex(int index);  //find the data at the given index
	string foundIndexString();
	string foundIndexImage();
	map<string, nimber>::const_iterator itStringNimber;
	map<string, string>::const_iterator itStringString;
	set<string>::const_iterator itSetString;
	
	//copy the content of the base from another base
	void copyFrom(const DataStorage & sourceStorage);
	
	//write-read the data in a QTextStream
	void write(QTextStream &text);
	bool read(QTextStream &text, bool withCano, bool convert);
};

//class with all the databases
class db {
  private :
	static int lastUsedBase;
	
  public:
	static vector<DataStorage> storeVect;
	static int create(int kindInit, QString shortNameInit, QString completeNameInit, QString headerNameInit, int nodeIndexInit);
	
	//functions to add and find  data in the correct database
	static void add(const string &a, int numBase, bool isCheck=false, bool withDefaultCheck=false);
	static void add(const string &a, nimber b, int numBase, bool isCheck=false, bool withDefaultCheck=false);
	static void add(const string &a, const string &b, int numBase, bool isCheck=false, bool withDefaultCheck=false);
	
	static bool find(const string &a, int numBase, bool isCheck=false);
	static bool findInverse(const string &a, int numBase, bool isCheck=false);
	static nimber foundNimber();
	static string foundString();
	static string foundInverse();
	
	static bool findIndex(int index, int numBase);  //find the data at the given index
	static string foundIndexString();
	static string foundIndexImage();
	
	//indicate if all based are saved or not
	static bool allSaved();
	static QString unsavedBaseName();
	
	//move a database to another one
	static void move(int numBase_source, int numBase_dest);
	
	//total number of positions stored
	static unsigned int totalSize();
	
	//multi-thread lock to secure access
	static QMutex base_lock;
};

#endif
