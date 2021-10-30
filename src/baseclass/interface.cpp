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

#include "interface.h"
#include "../computation/databasebutton.h"
#include <QtXml>

//----------------------------------------------------------------------------------------
//                               ParamDef
//----------------------------------------------------------------------------------------

//init all the pointers to 0
ParamDef::ParamDef() {
	boolTarget=0;
	intTarget=0;
	stringTarget=0;
	dataBaseButton=0;
}

//copy the value stored in the ParamDef object to the variable that are linked to ParamDef
void ParamDef::copyInTarget() {
	if(kind==1) {
		//spinbox
		if(intTarget!=0) {
			*intTarget = intValue;
		}
	} else
	if(kind==2 || kind==3) {
		//radiobutton, checkbutton
		if(boolTarget!=0) {
			*boolTarget = boolValue;
		}
	} else
	if(kind==4 || kind==6) {
		//lineEdit, hidden string
		if(stringTarget!=0) {
			*stringTarget = stringValue;
		}
	}
}

//reverse process : copy the value from the linked variable
//used only for hidden string variables
void ParamDef::copyFromTarget() {
	if(kind==6) {
		//hidden string
		if(stringTarget!=0) {
			stringValue = *stringTarget;
		}
	}
}

//initialize the position of the object in the Qt grid
void ParamDef::setPos(int x0, int y0, int xWidth0, int yWidth0) {
	x=x0;
	y=y0;
	xWidth=xWidth0;
	yWidth=yWidth0;
}

//write the value of paramdef in an xml dom element
void ParamDef::putValueIn(QDomElement& element, bool newRepoFile) {
	if(kind==1) {
		element.setAttribute("Value", intValue);
	} else
	if(kind==2 || kind==3) {
		element.setAttribute("Value", boolValue);
	} else
	if(kind==4 || kind==6) {
		element.setAttribute("Value", QString::fromStdString(stringValue));
	} else
	if(kind==5) {
		if(dataBaseButton != 0) { //security
			if(! dataBaseButton->baseIsEmpty()) { //don't save empty databases
				QString shortFileName = dataBaseButton->autoSave(newRepoFile);  //save the database
				element.setAttribute("Value", shortFileName);              //write the filename in the xml file
				element.setAttribute("Size", dataBaseButton->baseSize()); //number of positions in the base
			}
		}
	}
}

//initialize the value of paramdef from an xml dom element
void ParamDef::setValueFrom(QDomElement& element) {
	if(kind==1) {
		intValue = element.attribute("Value").toInt();
	} else
	if(kind==2 || kind==3) {
		if(element.attribute("Value").toInt()==0) {
			boolValue = false;
		} else {
			boolValue = true;
		}
	} else
	if(kind==4 || kind==6) {
		stringValue = element.attribute("Value").toStdString();
	} else
	if(kind==5) {
		if(element.hasAttribute("Value")) {
			//a file exist for this database
			QString fileName = element.attribute("Value");
			if(dataBaseButton != 0) dataBaseButton->autoRead(fileName);
		} else {
			//the database for this button is empty, purge the corresponding database
			if(dataBaseButton != 0) dataBaseButton->autoPurgeBase();
		}
	}
}

//----------------------------------------------------------------------------------------
//                               Interface
//----------------------------------------------------------------------------------------

Interface::Interface() {
}

void Interface::addLabel(string s, int x0, int y0, int xWidth0, int yWidth0) {
	ParamDef d;
	d.kind = 0;
	d.stringValue=s;
	d.setPos(x0, y0, xWidth0, yWidth0);
	parameters.push_back(d);
}

void Interface::addSpinBox(int minV, int maxV, int V, int x0, int y0, int xWidth0, int yWidth0) {
	ParamDef d;
	d.kind = 1;
	d.minValue = minV; d.maxValue = maxV; d.intValue = V;
	d.setPos(x0, y0, xWidth0, yWidth0);
	parameters.push_back(d);
}

void Interface::addRadioButton(string s, bool c, int x0, int y0, int xWidth0, int yWidth0) {
	ParamDef d;
	d.kind = 2;
	d.stringValue = s; d.boolValue = c;
	d.setPos(x0, y0, xWidth0, yWidth0);
	parameters.push_back(d);
}

void Interface::addCheckButton(string s, bool c, int x0, int y0, int xWidth0, int yWidth0) {
	ParamDef d;
	d.kind = 3;
	d.stringValue = s; d.boolValue = c;
	d.setPos(x0, y0, xWidth0, yWidth0);
	parameters.push_back(d);
}

void Interface::addLineEdit(string s, int x0, int y0, int xWidth0, int yWidth0) {
	ParamDef d;
	d.kind = 4;
	d.stringValue = s;
	d.setPos(x0, y0, xWidth0, yWidth0);
	parameters.push_back(d);
}

void Interface::addDataBaseButton(int numBase, int x0, int y0, int xWidth0, int yWidth0) {
	ParamDef d;
	d.kind = 5;
	d.intValue = numBase;
	d.setPos(x0, y0, xWidth0, yWidth0);
	parameters.push_back(d);
}

//the hidden string will not be displayed in the Qt interface
//this is why the position in the grid is meaningless
void Interface::addHiddenString(string s) {
	ParamDef d;
	d.kind = 6;
	d.stringValue = s;
	d.setPos(-1, -1, 1, 1); //no special meaning, just to avoid non-initialized variables
	parameters.push_back(d);
}

//set the tooltip for the last element
void Interface::setToolTip(string tip) {
	int n = parameters.size();
	if(n>0) {
		parameters[n-1].toolTipString = tip;
	}
}

//----------------------------------------------------------------------------------------
//                               Link with variables
//----------------------------------------------------------------------------------------

void Interface::link(bool &boolTarget0, string name) {
	int n = parameters.size();
	if(n>0) {
		//security
		if(parameters[n-1].kind != 2 && parameters[n-1].kind != 3) {
			cout << "Error in Interface::link : type mismatch" << endl;
			return;
		}
		
		parameters[n-1].boolTarget=&boolTarget0;
		parameters[n-1].name = name;
	}
}

void Interface::link(int &intTarget0, string name) {
	int n = parameters.size();
	if(n>0) {
		//security
		if(parameters[n-1].kind != 1) {
			cout << "Error in Interface::link : type mismatch" << endl;
			return;
		}
		
		parameters[n-1].intTarget=&intTarget0;
		parameters[n-1].name = name;
	}
}

void Interface::link(string &stringTarget0, string name) {
	int n = parameters.size();
	if(n>0) {
		//security
		if(parameters[n-1].kind != 4 && parameters[n-1].kind != 6) {
			cout << "Error in Interface::link : type mismatch" << endl;
			return;
		}
		
		parameters[n-1].stringTarget=&stringTarget0;
		parameters[n-1].name = name;
	}
}

//!< Name of a Database for xml files
void Interface::linkDataBase(string name) {
	int n = parameters.size();
	if(n>0) {
		//security
		if(parameters[n-1].kind != 5) {
			cout << "Error in Interface::link : type mismatch" << endl;
			return;
		}
		
		parameters[n-1].name = name;
	}
}

void Interface::copyInTarget() {
	int i;
	int n=parameters.size();
	for(i=0; i<n; i++) {
		parameters[i].copyInTarget();
	}
}

//----------------------------------------------------------------------------------------
//                               Read/Write Xml files
//----------------------------------------------------------------------------------------

//Write an xml file with the content of the Interface
//newRepoFile : indicate if we are creating a new repository file
void Interface::writeXml(QDomDocument& doc, QDomElement& root, bool newRepoFile) {
	root.setAttribute("Name", QString::fromStdString(name));
	
	vector<ParamDef>::iterator Vi;
	for(Vi=parameters.begin(); Vi!=parameters.end(); Vi++) {
		if(!Vi->name.empty()) { //we write in the xml file only the values linked to a variable in Glop
			QDomElement element = doc.createElement("Parameter");
			element.setAttribute("Name", QString::fromStdString(Vi->name));
			Vi->putValueIn(element, newRepoFile);
			root.appendChild(element);
		}
	}
}

void Interface::readXml(QDomNode& rootNode) {
	QDomElement parameterElement = rootNode.firstChildElement("Parameter");
	while ( !parameterElement.isNull() ) {
		vector<ParamDef>::iterator Vi;
		for(Vi=parameters.begin(); Vi!=parameters.end(); Vi++) {
			if(Vi->name==parameterElement.attribute("Name").toStdString()) {
				Vi->setValueFrom(parameterElement);
			}
		}
		parameterElement = parameterElement.nextSiblingElement("Parameter");
	}
}
