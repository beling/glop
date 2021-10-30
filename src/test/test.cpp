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

#include "test.h"

void testTitle(string a) {
	cout << endl << "====== " << a << " ===================" << endl;
}

void testBegin(string a) {
	cout << "------ " << a << " ------" << endl;
}

void testComment(string a) {
	cout << "Comment : " << a << endl;
}

//check that the output string and check string are equal
void testOutput(string output, string check) {
	cout << "Output  : " << output;
	
	if(output == check) {
		cout << " ... [OK]" << endl;
	} else {
		cout << " != " << check << " ... [Failed]" << endl;
	}
}

//different kind of ourput : message + result
void printOutput(string message, bool result) {
	cout << "Output  : " << message;
	
	if(result) {
		cout << " ... [OK]" << endl;
	} else {
		cout << " ... [Failed]" << endl;
	}
}
