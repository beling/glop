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

#ifndef INTERFACE_H
#define INTERFACE_H

#include "../gameglobal.h"

class QSpinBox;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class DataBaseButton;
class QDomElement;
class QDomDocument;
class QDomNode;

//! Class to store the definition of one object of the interface 
class ParamDef {
  public:
	ParamDef();
	
	int kind;		//integer indicating the type of values stored in ParamDef
	string name;	//optional name (used in Xml files)
	
	//position in the grid
	int x;
	int y;
	int xWidth;
	int yWidth;
	void setPos(int x0, int y0, int xWidth0, int yWidth0);
	
	//possible values
	bool boolValue;
	int intValue;
	string stringValue;
	
	//min and max values for spinbox
	int minValue;
	int maxValue;
	
	//tooltip string 
	string toolTipString;
	
	//pointers to remember the associated widget
	//(used only in interfacewidget, there is no dependency to Qt here)
	QSpinBox *spinBox;
	QRadioButton *radioButton;
	QCheckBox *checkBox;
	QLineEdit *lineEdit;
	DataBaseButton *dataBaseButton;
	
	//pointers to remember the associated target variable
	bool *boolTarget;
	int *intTarget;
	string *stringTarget;
	void copyInTarget();
	void copyFromTarget();
	
	//xml files
	void setValueFrom(QDomElement& element);					//set value from a DomElement
	void putValueIn(QDomElement& element, bool newRepoFile);	//put the value in a DomElement
};

//! Class to define easily a Qt interface for a given game or node
class Interface {
  public :
	Interface();
	vector<ParamDef> parameters;	//!< Internal list of ParamDef, each defining a widget item
	string title;  					//!< Title of the box containing the interface
	string name;					//!< Name of the Game/Node/etc
	
	//! @name Possible objects
	//! Functions to add the different possible objects to the interface
	//@{
	void addLabel(string s, int x0, int y0, int xWidth0 = 1, int yWidth0 = 1);						//!< Add a label
	void addSpinBox(int minV, int maxV, int V, int x0, int y0, int xWidth0 = 1, int yWidth0 = 1);	//!< Add a SpinBox
	void addRadioButton(string s, bool c, int x0, int y0, int xWidth0 = 1, int yWidth0 = 1);		//!< Add a RadioButton
	void addCheckButton(string s, bool c, int x0, int y0, int xWidth0 = 1, int yWidth0 = 1);		//!< Add a CheckButton
	void addLineEdit(string s, int x0, int y0, int xWidth0 = 1, int yWidth0 = 1);					//!< Add a LineEdit
	void addDataBaseButton(int numBase, int x0, int y0, int xWidth0 = 1, int yWidth0 = 1);			//!< Add a DataBase Button
	void addHiddenString(string s);																	//!< Add a hidden string variable
	//@}
	
	//! @name Link with variables
	//! Functions to link the LAST  parameter to  a given variable in the program.
	//! WARNING : a pointer to the variable will be stored. The linked variable should never be deleted until the end of the programm
	void link(bool &boolTarget0, string name=string(""));		//!< Create a link to a boolean variable
	void link(int &intTarget0, string name=string(""));			//!< Create a link to an int variable
	void link(string &stringTarget0, string name=string(""));	//!< Create a link to a string variable
	void linkDataBase(string name);								//!< Name of a Database for xml files
	void copyInTarget();
	//@}
	
	//! Function to set the toolTip of an element
	void setToolTip(string tip);
	
	//! Function for Xml Files
	void writeXml(QDomDocument& doc, QDomElement& root, bool newRepoFile);		//!< write the content of the interface in xml
	void readXml(QDomNode& rootNode);											//!< read the content of the interface from xml
};

#endif
