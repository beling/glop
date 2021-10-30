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

#include "stringconverter.h"

//bool is explicitly supported to allow the use of letters specified by the user
StringConverter& operator<<(StringConverter& strConv, const bool& x) {
	if(x==true) {
		strConv.addToken(strConv.boolTrue);
	} else {
		strConv.addToken(strConv.boolFalse);
	}
	return strConv;
}

StringConverter& operator>>(StringConverter& strConv, bool& x) {
	string a = strConv.getFirstToken();
	if(a[0] ==strConv.boolTrue ) {
		x = true;
	} else {
		x = false;
	}
	return strConv;
}

//char and unsigned char are explicitly supported to allow the conversion to int or not
//char are added directly to the string (spaces are not lost)
StringConverter& operator<<(StringConverter& strConv, const char& x) {
	if(strConv.convertCharToInt) {
		strConv << (int) x;
	} else {
		strConv.addToken(x);
	}
	return strConv;
}

StringConverter& operator>>(StringConverter& strConv, char& x) {
	if(strConv.convertCharToInt) {
		//read an int and then convert it
		int a;
		strConv >> a;
		x = (char) a;
	} else {
		//read a char
		string a = strConv.getFirstToken();
		stringstream s(a);
		s >> x;
	}
	return strConv;
}

StringConverter& operator<<(StringConverter& strConv, const unsigned char& x) {
	if(strConv.convertCharToInt) {
		strConv << (unsigned int) x;
	} else {
		strConv.addToken(x);
	}
	return strConv;
}

StringConverter& operator>>(StringConverter& strConv, unsigned char& x) {
	if(strConv.convertCharToInt) {
		//read an unsigned int and then convert it
		unsigned int a;
		strConv >> a;
		x = (unsigned char) a;
	} else {
		//read an unsigned char
		string a = strConv.getFirstToken();
		stringstream s(a);
		s >> x;
	}
	return strConv;
}

//string are directly added. Spaces are not lost.
StringConverter& operator<<(StringConverter& strConv, const string& x) {
	strConv.addToken(x);
	return strConv;
}

StringConverter& operator>>(StringConverter& strConv, string& x) {
	x = strConv.getFirstToken();
	return strConv;
}

//--------------Commandes-----------------------------------------------------------------

StringConverter& operator<<(StringConverter& strConv, const CvCd& x) {
	strConv.setCommand(x.command);
	return strConv;
}

StringConverter& operator>>(StringConverter& strConv, const CvCd& x) {
	strConv.setCommand(x.command);
	return strConv;
}

void StringConverter::setCommand(const string & command) {
	unsigned int cmdSize = command.size();
	if(cmdSize==2) {
		if(command[0]=='S') {
			//the next character defines the stlSeparator between objects
			//it will be used by the template for the stl containers
			stlSeparator = (char) command[1];
		} else
		if(command[0]=='L') {
			//the next character defines if the stlSeparator should be used after the last element
			//'T' = true, any other letter = false
			lastElementSeparator = (bool) (command[1]=='T');
		} else
		if(command[0]=='C') {
			//the next character defines if char should be converted to int
			//'T' = true, any other letter = false
			convertCharToInt = (bool) (command[1]=='T');
		} else
		if(command[0]=='E') {
			//the next character defines the separator between string elements
			separator = (char) command[1];
		} else
		if(command[0]=='U') {
			//the next character defines if the separator should be used between string elements
			//'T' = true, any other letter = false
			useSeparator = (bool) (command[1]=='T');
		}
	} else
	if(cmdSize==3) {
		if(command[0]=='B') {
			//the next two characters define the representation of true and false booleans
			boolTrue  = (char) command[1];
			boolFalse = (char) command[2];
		}
	}
}
