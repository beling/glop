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

#include <QApplication>
#include <time.h>
#include "mainwidget/glopwidget.h"

void main_help();
void main_test();
void main_testpart(string name);
void main_error();
void main_graphic(int argc, char **argv );

int main( int argc, char **argv ) {
	srand(time(NULL));	//initialisation so that rand() won't give always the same values
	string s, s2;
	switch(argc) {
	  case 3 : s=(string) argv[1]; s2=(string) argv[2];
		if(s=="-t" || s=="--test") main_testpart(s2);
		else main_error();
		break;
	  case 2 : s=(string) argv[1];
		if     (s=="-h" || s=="--help") main_help();
		else if(s=="-t" || s=="--test") main_test();
		#ifdef MACX
		//special hack for mac platform (there is a program id parameter when run on double-click)
		else if (s.substr(0,4)=="-psn")  main_graphic(argc, argv);
		#endif	
		else                             main_error();
		break;
	  case 1 :
		main_graphic(argc, argv);
		break;
	  default :
		main_error();
	}
	
	return EXIT_SUCCESS;
}

//Qt graphical interface
void main_graphic(int argc, char **argv ) {
	QApplication a( argc, argv );
	GlopWidget *hm = new GlopWidget();
	hm->show();
	a.exec();
}

//available options in command line
void main_help() {
	cout << "Available options for Glop :" << endl << endl
	     << "nothing               :" << "  " << "start graphical interface" << endl
	     << "-h or --help          :" << "  " << "print this help list" << endl
	     << "-t or --test          :" << "  " << "test suite for Glop" << endl
	     << "-t or --test [Target] :" << "  " << "test only a specific Target part of Glop" << endl << endl
		 << "Possible Target for test : StringConverter, Sprouts, Board, Cram, DotsBoxes, Memory" << endl << endl;
}

//unavailable parameter
void main_error(){
	cout << "This parameter is not valid." << endl
	     << "Use option \"-h\" or \"--help\" to print a list of available parameters." << endl;
}


//----------------------------------Test functions ---------------------
//we only declare the name of the functions here. It avoids to include .h files
void StringConverterTest();
void SproutsTest();
void BoardTest();
void CramTest();
void DotsBoxesTest();
void MemoryTest();

//test all the possible test functions
void main_test(){
	StringConverterTest();
	SproutsTest();
	BoardTest();
	CramTest();
	DotsBoxesTest();
	MemoryTest();
}

//Function used to test a particular part of Glop
void main_testpart(string name) {
	if(name=="StringConverter") {
		StringConverterTest();
	} else
	if(name=="Sprouts") {
		SproutsTest();
	} else
	if(name=="Board") {
		BoardTest();
	} else
	if(name=="Cram") {
		CramTest();
	} else
	if(name=="DotsBoxes") {
		DotsBoxesTest();
	} else
	if(name=="Memory") {
		MemoryTest();
	} else {
		main_error();
	}
}
