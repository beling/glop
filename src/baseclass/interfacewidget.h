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

#ifndef INTERFACEWIDGET_H
#define INTERFACEWIDGET_H

#include "interface.h"

#include <QWidget>
class QGroupBox;

//! Class to construct a QT widget from a simple definition given as an "Interface" object
//! The Interface object contains a list of definitions for the main elements used in Glop
//! InterfaceWidget creates a widget displayed on the screen for each of these elements
//! This class is used in particular to define the interface of a node or a game
class InterfaceWidget : public QWidget {
  Q_OBJECT
	
  public:
	InterfaceWidget( QWidget* parent = 0);
	
	Interface param;					//!< "Interface" object with the list of parameters defining the widgets
	QGroupBox *parameterBox;			//!< Main box containing all the widgets of the interface
	
	void init(Interface paramInit);	//!< Create a gridWidget with the elements defined in Interface
	Interface readParameters();			//!< refresh the interface object and return its value
	void copyInTarget();				//!< Copy the values of the widget to the linked variables
	void readTargetValues();			//!< Copy the values of the linked variables to the widgets
	
  public slots:
	void refreshValues();					//!< Copy the values of the widget to the interface object
	
  signals:
	void globalRefreshNeeded();
};

#endif
