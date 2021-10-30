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
#include "../baseclass/basegame.h"
#include "computationthread.h"
#include "../stringconverter.h"

#include <QtGui>

QMutex db::base_lock;

//--------------------------------------------------------------------------
// DataStorage
//--------------------------------------------------------------------------

void DataStorage::init(int kindInit, QString shortNameInit, QString completeNameInit, QString headerNameInit, int nodeIndexInit, int baseIndexInit, bool isCheckBaseInit) {
	kind = kindInit;
	isSaved=true;
	status = 0;
	shortName = shortNameInit;
	completeName = completeNameInit;
	headerName = headerNameInit;
	nodeIndex = nodeIndexInit;
	baseIndex = baseIndexInit;
	isCheckBase = isCheckBaseInit;
}

int DataStorage::size() {
	if(kind==0) {
		return mapStringNimber.size();
	} else
	if(kind==1) {
		return mapStringString.size();
	} else
	if(kind==2) {
		return setString.size();
	}

	return 0;
}

void DataStorage::clear() {
	if(kind==0) {
		mapStringNimber.clear();
	} else
	if(kind==1) {
		mapStringString.clear();
	} else
	if(kind==2) {
		setString.clear();
	}
	
	isSaved=true;
	status = 0;
}

bool DataStorage::empty() {
	if(kind==0) {
		return mapStringNimber.empty();
	} else
	if(kind==1) {
		return mapStringString.empty();
	} else
	if(kind==2) {
		return setString.empty();
	}
	
	return 0;
}

void DataStorage::add(const string &a, nimber b) {
	if(kind==0) { //to avoid silent errors
		string copy = a;
		copy.reserve(copy.size());	//ensure that capacity is not greater than size 
		mapStringNimber[copy]=b;
		isSaved=false;
	}
}

void DataStorage::add(const string &a, const string &b) {
	if(kind==1) { //to avoid silent errors
		string copy = a;
		copy.reserve(copy.size());	//ensure that capacity is not greater than size
		mapStringString[copy]=b;
		isSaved=false;
	}
}

void DataStorage::add(const string &a) {
	if(kind==2) { //to avoid silent errors
		string copy = a;
		copy.reserve(copy.size());	//ensure that capacity is not greater than size
		setString.insert(copy);
		isSaved=false;
	}
}

bool DataStorage::find(const string &a) {
	if(kind==0) {
		itStringNimber = mapStringNimber.find(a);
		return ( itStringNimber != mapStringNimber.end() );
	}
	if(kind==1) {
		itStringString = mapStringString.find(a);
		return ( itStringString != mapStringString.end() );
	}
	if(kind==2) {
		return ( setString.find(a) != setString.end() );
	}
	
	return false;
}

//find a given string in the image set of the database
//stop on the first found image
//WARNING : this function is very slow
bool DataStorage::findInverse(const string &a) {
	if(kind!=1) return false;
	
	for(itStringString=mapStringString.begin(); itStringString != mapStringString.end(); itStringString++) {
		if(itStringString->second == a) {
			return true;
		}
	}
	
	return false;
}

nimber DataStorage::foundNimber() {
	return itStringNimber->second;
}

string DataStorage::foundString() {
	return itStringString->second;
}

string DataStorage::foundInverse() {
	return itStringString->first;
}

//find the data at the given index
bool DataStorage::findIndex(int index) {
	if(kind==0) {
		int i=0;
		for(itStringNimber=mapStringNimber.begin(); i<index && itStringNimber!=mapStringNimber.end() ; itStringNimber++, i++) {
			//nothing to do
		}
		return ( itStringNimber != mapStringNimber.end() );
	}
	if(kind==1) {
		int i=0;
		for(itStringString=mapStringString.begin(); i<index && itStringString!=mapStringString.end() ; itStringString++, i++) {
			//nothing to do
		}
		return ( itStringString != mapStringString.end() );
	}
	if(kind==2) {
		int i=0;
		for(itSetString=setString.begin(); i<index && itSetString!=setString.end() ; itSetString++, i++) {
			//nothing to do
		}
		return ( itSetString != setString.end() );
	}
	
	return false;
}

string DataStorage::foundIndexString() {
	if(kind==0) {
		return itStringNimber->first;
	}
	if(kind==1) {
		return itStringString->first;
	}
	if(kind==2) {
		return *itSetString;
	}
	string emptyString;
	return emptyString;
}

string DataStorage::foundIndexImage() {
	if(kind==0) {
		StringConverter conv;
		conv << itStringNimber->second;
		return conv.getString();
	}
	if(kind==1) {
		return itStringString->second;
	}
	//kind == 2
	string emptyString;
	return emptyString;
}

void DataStorage::copyFrom(const DataStorage & sourceStorage) {
	if(kind==0) {
		mapStringNimber = sourceStorage.mapStringNimber;
	} else
	if(kind==1) {
		mapStringString = sourceStorage.mapStringString;
	} else
	if(kind==2) {
		setString = sourceStorage.setString;
	}
}

void DataStorage::write(QTextStream &text) {
	//header
	text << headerName << "\n";
	
	map<string, nimber>::const_iterator SNi;
	set<string>::const_iterator Si;
	map<string, string>::const_iterator SSi;
	
	//save databases
	if(kind==0) {
		for(SNi=mapStringNimber.begin(); SNi!=mapStringNimber.end(); SNi++) {
			text << QString::fromStdString(SNi->first) << " " << (int) (SNi->second) << "\n";
		}
	} else
	if(kind==1) {
		for(SSi=mapStringString.begin(); SSi!=mapStringString.end(); SSi++) {
			text << QString::fromStdString(SSi->first) << " " << QString::fromStdString(SSi->second) << "\n";
		}
	} else
	if(kind==2) {
		for(Si=setString.begin(); Si!=setString.end(); Si++) {
			text << QString::fromStdString(*Si) << "\n";
		}
	}
	
	isSaved = true;
}


bool DataStorage::read(QTextStream &text, bool withCano, bool convert) {
	//map<string, nimber> case
	if (kind==0) {
		//read data
		QString read_string;
		string str;
		int read_nim;
		nimber nim;
		
		map<string, nimber>::const_iterator SNi;
		
		text >> read_string;
		text >> read_nim;
		
		while ( !text.atEnd() ) {
			nim=read_nim;
			str=read_string.toStdString();
			
			//piece of code to recover old databases (as much as possible)
			//when for example canonization has changed
			if(withCano) {
				BaseGame g(str);
				g.canonize();
				str = g.exportToString();
			}
			
			if(convert) {
				BaseGame g(str);			//read with oldFromString
				str = g.exportToString();	//write with toString
			}
			
			//consistency check and fusion
			SNi = mapStringNimber.find(str);
			if(SNi!=mapStringNimber.end()) {
				//we need to ask to the node to treat this case directly
				bool success = BaseNode(nodeIndex, 0 /*useless value*/).treatBaseCollision(str, SNi->second, nim, baseIndex, isCheckBase);
				if(!success) {
					//TODO : print the bad position and the 2 nimbers
					return false;
				}
			} else {
				mapStringNimber[str]=nim; //add position to database
			}
			text >> read_string;
			text >> read_nim;
		}
	} else
	if (kind==1) {
		//read data
		QString qstring1;
		QString qstring2;
		string str;
		map<string, string>::const_iterator Si;
		while ( !text.atEnd() ) {
			text >> qstring1;
			str = qstring1.toStdString();
			if(convert) {
				BaseGame g(str);			//read with oldFromString
				str = g.exportToString();	//write with toString
			}
			
			qstring2=text.readLine();
			qstring2.remove(0,1);	//remove the leading whitespace
			string fileString = qstring2.toStdString();
			
			//consistency check and fusion
			Si = mapStringString.find(str);
			if(Si!=mapStringString.end()) {
				//we need to ask to the node to treat this case directly
				bool success = BaseNode(nodeIndex, 0 /*useless value*/).treatBaseCollision(str, Si->second, fileString, baseIndex, isCheckBase);
				if(!success) {
					//TODO : print the bad position and the 2 value strings
					return false;
				}
			} else {
				mapStringString[str]=fileString; //add position to database
			}
		}
	} else
	if (kind==2) {
		//read data
		QString qstring1;
		string str;
		while ( !text.atEnd() ) {
			if(convert) {
				//the first string is the position part
				text >> qstring1;
				BaseGame g(qstring1.toStdString() );	//read with oldFromString
				str = g.exportToString();				//write with toString
				str += text.readLine().toStdString();	//add the end of the line after the position part
			} else {
				str = text.readLine().toStdString();
			}
			
			setString.insert(str);
		}
	}
	
	return true;
}

//--------------------------------------------------------------------------
// Database
//--------------------------------------------------------------------------

vector<DataStorage> db::storeVect;
int db::lastUsedBase;

//create a new database with the given parameter and return the index for this database
int db::create(int kindInit, QString shortNameInit, QString completeNameInit, QString headerNameInit, int nodeIndexInit) {
	//add a new datastorage object
	DataStorage a;
	storeVect.push_back(a);
	
	//init the datastorage object
	int lastIndex = storeVect.size() - 1;
	storeVect[lastIndex].init(kindInit, shortNameInit, completeNameInit, headerNameInit, nodeIndexInit, lastIndex, false);
	
	//add also a new datastorage object for check databases
	DataStorage b;
	storeVect.push_back(b);
	int checkIndex = storeVect.size() - 1;
	storeVect[checkIndex].init(kindInit, shortNameInit + QString(" Check"), completeNameInit + QString(" check"), headerNameInit, nodeIndexInit, checkIndex, true);
	
	return lastIndex;
}

//Add a position to the appropriate database
void db::add(const string &a, int numBase, bool isCheck, bool withDefaultCheck) {
	QMutexLocker locker(&(db::base_lock));
	//in case of a check computation, check the consistency with the previous database
	if(isCheck && withDefaultCheck) {
		bool exist = storeVect[numBase].find(a);
		//if it is a new result, change the status of the check database
		if(!exist) {
			storeVect[numBase+1].status = 1;
			cout << "New: " << a << endl;
		}
	}
	
	//add the result to the correct database
	if(isCheck) numBase++;	//go to the check database
	storeVect[numBase].add(a);
}

void db::add(const string &a, nimber b, int numBase, bool isCheck, bool withDefaultCheck) {
	QMutexLocker locker(&(db::base_lock));
	//in case of a check computation, check the consistency with the previous database
	if(isCheck && withDefaultCheck) {
		bool exist = storeVect[numBase].find(a);
		if(exist) {
			//if the result is not consistent with the previous database, change the status of check database
			if(storeVect[numBase].foundNimber() != b) {
				storeVect[numBase+1].status = 2;
				cout << "Inconsistent: " << a << " " << (int) b << endl;
			}
		} else {
			//it is a new result, change the status of the check database
			storeVect[numBase+1].status = 1;
			cout << "New: " << a << " " << (int) b << endl;
		}
	}

	if(isCheck) numBase++;	//go to the check database
	storeVect[numBase].add(a, b);
}

void db::add(const string &a, const string &b, int numBase, bool isCheck, bool withDefaultCheck) {
	QMutexLocker locker(&(db::base_lock));
	//in case of a check computation, check the consistency with the previous database
	if(isCheck && withDefaultCheck) {
		bool exist = storeVect[numBase].find(a);
		if(exist) {
			//if the result is not consistent with the previous database, change the status of check database
			if(storeVect[numBase].foundString() != b) {
				storeVect[numBase+1].status = 2;
				cout << "Inconsistent: " << a << " " << b << endl;
			}
		} else {
			//it is a new result, change the status of the check database
			storeVect[numBase+1].status = 1;
			cout << "New: " << a << " " << b << endl;
		}
	}

	if(isCheck) numBase++;	//go to the check database	
	storeVect[numBase].add(a, b);
}

bool db::find(const string &a, int numBase, bool isCheck) {
	if(isCheck) numBase++;
	lastUsedBase = numBase;
	return storeVect[numBase].find(a);
}

bool db::findInverse(const string &a, int numBase, bool isCheck) {
	if(isCheck) numBase++;
	lastUsedBase = numBase;
	return storeVect[numBase].findInverse(a);
}

nimber db::foundNimber() {
	return storeVect[lastUsedBase].foundNimber();
}

string db::foundString() {
	return storeVect[lastUsedBase].foundString();
}

string db::foundInverse() {
	return storeVect[lastUsedBase].foundInverse();
}

//find the data at a given index
bool db::findIndex(int index, int numBase) {
	if(index<0 || index > (int) storeVect[numBase].size()) return false;
	lastUsedBase = numBase;
	return storeVect[numBase].findIndex(index);
}

string db::foundIndexString() {
	return storeVect[lastUsedBase].foundIndexString();
}

string db::foundIndexImage() {
	return storeVect[lastUsedBase].foundIndexImage();
}

//move a database to another one
void db::move(int numBase_source, int numBase_dest) {
	storeVect[numBase_dest].clear();									//purge the destination database
	storeVect[numBase_dest].copyFrom(db::storeVect[numBase_source]);	//move the source to the destination
	storeVect[numBase_source].clear();									//finally purge the source database
}

bool db::allSaved() {
	unsigned int i;
	for(i=0; i<storeVect.size(); i++) {
		if(!storeVect[i].isSaved) return false;
	}
	return true;
}

QString db::unsavedBaseName() {
	unsigned int i;
	for(i=0; i<storeVect.size(); i++) {
		if(!storeVect[i].isSaved) return storeVect[i].completeName;
	}
	
	return QString();
}

unsigned int db::totalSize() {
	unsigned int total=0;
	unsigned int i;
	for(i=0; i<storeVect.size(); i++) {
		total += storeVect[i].size();
	}
	return total;
}
