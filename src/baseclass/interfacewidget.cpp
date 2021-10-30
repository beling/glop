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

#include "interfacewidget.h"
#include "../computation/databasebutton.h"

#include <QtGui>

InterfaceWidget::InterfaceWidget( QWidget* parent) : QWidget( parent) {

}

void InterfaceWidget::init(Interface paramInit) {
	param = paramInit;
	
	QGridLayout *grid;
	
	if(!param.title.empty()) {
		QVBoxLayout *vlayout = new QVBoxLayout(this);
		parameterBox = new QGroupBox(this);
		vlayout->addWidget( parameterBox );
		parameterBox->setTitle(QString(param.title.c_str()));
		grid = new QGridLayout(parameterBox);
	} else {
		grid = new QGridLayout(this);
	}
	
	vector<ParamDef>::iterator Pi;
	for(Pi=param.parameters.begin(); Pi!=param.parameters.end(); Pi++) {
		if(Pi->kind == 0) {
			QLabel *label = new QLabel;
			label->setText(QString(Pi->stringValue.c_str()));
			grid->addWidget( label, Pi->x, Pi->y, Pi->xWidth, Pi->yWidth );
			if(!Pi->toolTipString.empty()) label->setToolTip(QString(Pi->toolTipString.c_str()));
		} else
		if(Pi->kind == 1) {
			QSpinBox *spinBox = new QSpinBox;
			spinBox->setMinimum(Pi->minValue);
			spinBox->setMaximum(Pi->maxValue);
			spinBox->setFixedSize(spinBox->sizeHint());
			spinBox->setValue(Pi->intValue);
			grid->addWidget( spinBox, Pi->x, Pi->y, Pi->xWidth, Pi->yWidth );
			if(!Pi->toolTipString.empty()) spinBox->setToolTip(QString(Pi->toolTipString.c_str()));
			Pi->spinBox = spinBox;
		} else
		if(Pi->kind == 2) {
			QRadioButton *radioButton = new QRadioButton(QString(Pi->stringValue.c_str()));
			grid->addWidget( radioButton, Pi->x, Pi->y, Pi->xWidth, Pi->yWidth );
			radioButton->setChecked(Pi->boolValue);
			if(!Pi->toolTipString.empty()) radioButton->setToolTip(QString(Pi->toolTipString.c_str()));
			Pi->radioButton = radioButton;
		} else
		if(Pi->kind == 3) {
			QCheckBox *checkBox = new QCheckBox(QString(Pi->stringValue.c_str()));
			grid->addWidget( checkBox, Pi->x, Pi->y, Pi->xWidth, Pi->yWidth);
			checkBox->setChecked(Pi->boolValue);
			if(!Pi->toolTipString.empty()) checkBox->setToolTip(QString(Pi->toolTipString.c_str()));
			Pi->checkBox = checkBox;
		} else
		if(Pi->kind == 4) {
			QLineEdit *lineEdit = new QLineEdit;
			grid->addWidget( lineEdit, Pi->x, Pi->y, Pi->xWidth, Pi->yWidth );
			lineEdit->setText( QString(Pi->stringValue.c_str()) );
			lineEdit->setMinimumWidth( 150 );
			if(!Pi->toolTipString.empty()) lineEdit->setToolTip(QString(Pi->toolTipString.c_str()));
			Pi->lineEdit = lineEdit;
		} else
		if(Pi->kind == 5) {
			DataBaseButton *dataBaseButton = new DataBaseButton(0, Pi->intValue);
			dataBaseButton->refresh_size();
			connect(dataBaseButton, SIGNAL(globalRefreshNeeded()), this, SIGNAL(globalRefreshNeeded()) );
			grid->addWidget( dataBaseButton, Pi->x, Pi->y, Pi->xWidth, Pi->yWidth );
			if(!Pi->toolTipString.empty()) dataBaseButton->setToolTip(QString(Pi->toolTipString.c_str()));
			Pi->dataBaseButton = dataBaseButton;
		}
		
		//note : Pi->kind == 6 is a hidden string and is not displayed
	}
}

//refresh the internal values
//in case of a DataBaseButton, refresh the displayed size
void InterfaceWidget::refreshValues() {
	vector<ParamDef>::iterator Pi;
	for(Pi=param.parameters.begin(); Pi!=param.parameters.end(); Pi++) {
		if(Pi->kind == 1) {
			Pi->intValue = Pi->spinBox->value();
		} else
		if(Pi->kind == 2) {
			Pi->boolValue = Pi->radioButton->isChecked();
		} else
		if(Pi->kind == 3) {
			Pi->boolValue = Pi->checkBox->isChecked();
		} else
		if(Pi->kind == 4) {
			Pi->stringValue = (Pi->lineEdit->text()).toStdString();
		} else
		if(Pi->kind == 5) {
			Pi->dataBaseButton->refresh_size();
		} else
		if(Pi->kind == 6) {
			//hidden string variable : we refresh the internal value of ParamDef from the target
			Pi->copyFromTarget();
		}
	}
}

//Refresh the values of the widget from the values of the linked variables
//This function is the reverse of refreshValues and is used when reading parameters from a file
//we check that each element is really linked to a variable
//   this is necessary in particular for radiobuttons, because some radiobuttons are not linked to a variable
void InterfaceWidget::readTargetValues() {
	vector<ParamDef>::iterator Pi;
	for(Pi=param.parameters.begin(); Pi!=param.parameters.end(); Pi++) {
		if(Pi->kind == 1 && Pi->intTarget!=0) {
			Pi->spinBox->setValue(Pi->intValue);
		} else
		if(Pi->kind == 2 && Pi->boolTarget!=0) {
			//there is sometimes a strange bug with the radio button and setChecked has no effect
			//to avoid this problem, we manages directly the button true/false value
			//we need to change directly the value of all the radioButtons
			vector<ParamDef>::iterator Pj;
			for(Pj=param.parameters.begin(); Pj!=param.parameters.end(); Pj++) {
				if(Pj->kind == 2) {
					Pj->radioButton->setAutoExclusive(false); //to be sure that the value will be changed
					if(Pj != Pi) {
						//all buttons different from the current one (Pi) are set to the opposed boolean value
						Pj->radioButton->setChecked( ! Pi->boolValue);
					} else {
						//this is the current button (Pi)
						Pj->radioButton->setChecked( Pi->boolValue);
					}
					Pj->radioButton->setAutoExclusive(true); //set back again the exclusive characteristic of the radiobuttons
				}
			}
		} else
		if(Pi->kind == 3 && Pi->boolTarget!=0) {
			Pi->checkBox->setChecked(Pi->boolValue);
		} else
		if(Pi->kind == 4 && Pi->stringTarget!=0) {
			Pi->lineEdit->setText( QString(Pi->stringValue.c_str()) );
		} else
		if(Pi->kind == 5) {
			//nothing to do
		}  else
		if(Pi->kind == 6) {
			//hidden string variable : we init the value of the target with the value of ParamDef
			Pi->copyInTarget();
		}
	}
}

Interface InterfaceWidget::readParameters() {
	refreshValues();
	return param;
}

void InterfaceWidget::copyInTarget() {
	//refresh the internal object with the values of the interface widget
	refreshValues();
	param.copyInTarget();
}
