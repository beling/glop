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

#ifndef STRINGCONVERTER_H
#define STRINGCONVERTER_H

#include "global.h"

//CvCd (abbreviation of ConverterCommand) is a class to send a command to StringConverter
//the use of commands allows to construct easily a converter between a string and a complex c++ object
//a command is used by inserting it in the flow of objects/strings
//for example StringConverter << object1 << object2 << command1 << object3 << etc..

//list of supported commands : activate/unactivate = use T or F for the ?
// "E?"  : ? will be the new separator between string elements
// "U?"  : activate/unactivate the use of a separator between string elements
//         U can also be used when reading a string for stl containers.
//         In this case, it indicates if we should stop reading the string when the separator is found
// "S?"  : ? will be the new stlSeparator for stl containers
// "L?"  : activate/unactivate the use of the stlSeparator after the last element
// "B??" : the first ? is the letter for true booleans, and the second ? the letter for false booleans
// "C?"  : activate/unactivate the conversion of char in int

class CvCd {
  public :
	CvCd(const string &command0) {command = command0;};
	string command;
};


//StringConverter is a class to automatize the conversion between c++ objects and strings

//To add the support of an object, you need to redefine the following two functions
//StringConverter& operator<<(StringConverter& strConv, const Object& x);
//StringConverter& operator>>(StringConverter& strConv, Object& x);
//If you try to use StringConverter on an object not supported without defining these two functions,
//then the template for usual objects (int, bool, etc) will be used by default
//and a compiler error will occur : no match for ‘operator<<’ in ‘strConv->StringConverter::internalString << x’

class StringConverter {
  public :
	string internalString;
	unsigned char stlSeparator;	//separator between the elements of a stl container
	bool lastElementSeparator;		//indicate if the separator is also used after the last element of a container
	unsigned char boolFalse;		//letter for representing a false boolean
	unsigned char boolTrue;		//letter for representing a true boolean
	bool convertCharToInt;			//indicate if char and unsigned char should be converted to int
	unsigned char separator;		//separator after an element in the string
	bool useSeparator;				//use the separator after adding an element in the string
	bool readStlContainer;			//true when reading an stl container from a string
	bool endOfContainer;			//indicate if we have reached the end of the container
	
	StringConverter() {
		initVariables();
	}
	StringConverter(const string& s) { //init the stringconverter with a string previously obtained from a StringConverter
		initVariables();
		internalString = s;
	}
	
	inline void initVariables();
	inline void addToken(const string& x);
	inline void addToken(const char& x);
	inline void addToken(const unsigned char& x);
	inline bool hasToken();
	inline int tokenNumber();
	inline bool stlContainerEmpty(); //return true if the stl container is empty and delete the separator if needed
	inline string testFirstToken();
	inline string getFirstToken();
  private:
	inline string internalReadToken(bool deleteToken); //internal function used by firstToken and getFirstToken
  public:
	inline string getString();
	
	void setCommand(const string & command);
};

inline void StringConverter::initVariables() {
	separator = (char) '-';		//default separator after adding an element to the string
	useSeparator = false;			//by default, don't use the separator between elements of the string
	stlSeparator = (char) ' ';		//default stl separator
	lastElementSeparator = true;	//by default, use the separator also after the last element of a container
	boolFalse = (char) 'F';		//default letter for false boolean
	boolTrue  = (char) 'T';		//default letter for true boolean
	convertCharToInt = true;		//by default, char are converted to int
	readStlContainer = false;		//this parameter is activated only by stl templates
	endOfContainer = false;			//this parameter is used by stl templates
}

inline void StringConverter::addToken(const string& x) {
	internalString += x;
	if(useSeparator) internalString += separator;
}

inline void StringConverter::addToken(const char& x) {
	internalString += x;
	if(useSeparator) internalString += separator;
}

inline void StringConverter::addToken(const unsigned char& x) {
	internalString += x;
	if(useSeparator) internalString += separator;
}

inline bool StringConverter::hasToken() {
	return (!internalString.empty());
}

inline int StringConverter::tokenNumber() {
	if(!useSeparator) {
		if(hasToken()) {
			return 1;
		} else {
			return 0;
		}
	}
	int result=0;
	int i;
	int n=internalString.length();
	unsigned char c;
	for(i=0;i<n;i++) {
		c= (unsigned char) internalString[i];
		if( c == separator ) result++;
	}
	c = internalString[n-1];
	if(c!=separator) result++; //even without terminal separator, the last element counts as one token
	return result;
}

inline bool StringConverter::stlContainerEmpty() { //return true if the stl container is empty and delete the separator is needed
	if(internalString.empty()) return true;
	unsigned char c= (unsigned char) internalString[0];
	if(useSeparator && c==separator) {
		internalString.erase(0, 1);
		return true;
	}
	return false;
}

inline string StringConverter::testFirstToken() { //return the value of the first token but without deleting it
	return internalReadToken(false);
}

inline string StringConverter::getFirstToken() { //return the value of the first token and delete it from the converter
	return internalReadToken(true);
}

//internal function used by firstToken and getFirstToken
inline string StringConverter::internalReadToken(bool deleteToken) {
	if(internalString.empty())
		cout << "empty internalString in getFirstToken" << endl;
	
	//return the first part of the string or the complete string if separator is not found
	string tmp;
	int i;
	int n=internalString.length();
	unsigned char c;
	for(i=0;i<n;i++) {
		c= (unsigned char) internalString[i];
		if( (readStlContainer && c==stlSeparator ) || c == separator ){
			//test if it is the end of the stl container
			if(readStlContainer) {
				if(useSeparator) {
					if(c==separator) {
						endOfContainer = true; //container without terminal stlSeparator but with a terminal separator
					} else {
						//check the next character
						int j = i+1;
						if(j<n) { //security
							c= (unsigned char) internalString[j];
							if(c==separator) {
								endOfContainer = true; // container with both terminal stlSeparator and separator
								i = j;
							}
						} else {
							endOfContainer = true; //end of the string
						}
					}
				} else {
					//if we don't use the separator, the end of the container is the end of the string
					if(i==n-1) endOfContainer = true;
				}
			}
			if(deleteToken) internalString.erase(0, i+1); //erase the first token and separator from the string
			return tmp;
		} else{
			tmp += c;
		}
	}
	if(deleteToken) internalString.clear(); //all the string has been read
	if(readStlContainer) endOfContainer = true; //container without terminal stlSeparator and without terminal separator
	return tmp;
}

inline string StringConverter::getString() {
	return internalString;
}

StringConverter& operator<<(StringConverter& strConv, const CvCd& x);
StringConverter& operator>>(StringConverter& strConv, const CvCd& x);

//useful template to use a StringConverter easily
//An object T supported by StringConverter can be converted in a string by calling toStringZ(T)
//The conversion from a string to an object is done with fromStringZ(string, T);

/*
template<typename T>
string toStringZ(const T& x) {
	StringConverter conv;
	conv << x;
	return conv.getString();
}

template<class T>
void fromStringZ(const string& s, T& x) {
	StringConverter conv;
	conv.internalString = s;
	conv >> x;
}
*/

//usual objects (like string, bool, int, char) are automatically supported through the following template
template<typename T>
StringConverter& operator<<(StringConverter& strConv, const T& x) {
	stringstream s;
	s << x;
	string c;
	s >> c;
	strConv.addToken(c);
	return strConv;
}

template<typename T>
StringConverter& operator>>(StringConverter& strConv, T& x) {
	string a = strConv.getFirstToken();
	stringstream s(a);
	s >> x;
	return strConv;
}

//boolean are explicitly supported to allow the use of letters specified by the user
StringConverter& operator<<(StringConverter& strConv, const bool& x);
StringConverter& operator>>(StringConverter& strConv, bool& x);

//char and unsigned char are explicitly supported to allow the conversion to int or not 
StringConverter& operator<<(StringConverter& strConv, const char& x);
StringConverter& operator<<(StringConverter& strConv, const unsigned char& x);
StringConverter& operator>>(StringConverter& strConv, char& x);
StringConverter& operator>>(StringConverter& strConv, unsigned char& x);

//string is explicitly supported to avoid the loss of spaces
StringConverter& operator<<(StringConverter& strConv, const string& x);
StringConverter& operator>>(StringConverter& strConv, string& x);

//list, set, multiset, vector are automatically suported through the following templates
//the separator between string elements is not used. We use instead the stlSeparator.
//in the "for" loop :
// first  if case : when the stlSeparator is not added after the last element
// second if case : when the stlSeparator is also added after the last element
#define TO_STRING_STL_TEMPLATE(STL_CONTAINER) \
template<typename T> \
StringConverter& operator<<(StringConverter& strConv, const STL_CONTAINER<T>& x) { \
	typename STL_CONTAINER<T>::const_iterator Ti; \
	bool useSeparatorSav = strConv.useSeparator; \
	strConv.useSeparator = false; \
	for(Ti=x.begin(); Ti != x.end(); Ti++) { \
		if(!strConv.lastElementSeparator && Ti!=x.begin()) strConv.internalString += strConv.stlSeparator; \
		strConv << *Ti; \
		if(strConv.lastElementSeparator) strConv.internalString += strConv.stlSeparator; \
	} \
	strConv.useSeparator = useSeparatorSav; \
	if(strConv.useSeparator) strConv.internalString += strConv.separator; \
	return strConv; \
}

//read the elements of StringConverter until the end is reached or the separator is reached (use CvCd("U?") to configure)
//the elements of the stl container are separated by stlSeparator
#define FROM_STRING_STL_TEMPLATE(STL_CONTAINER, INSERT_KEYWORD) \
template<typename T> \
StringConverter& operator>>(StringConverter& strConv, STL_CONTAINER<T>& x) { \
	strConv.readStlContainer = true; \
	strConv.endOfContainer = strConv.stlContainerEmpty(); \
	while (!strConv.endOfContainer) { \
		T Tresult; \
		strConv >> Tresult; \
		x.INSERT_KEYWORD(Tresult); \
	} \
	strConv.readStlContainer = false; \
	return strConv; \
}

//STL Templates for list, vector, set and multiset
TO_STRING_STL_TEMPLATE(list)
TO_STRING_STL_TEMPLATE(vector)
TO_STRING_STL_TEMPLATE(set)
TO_STRING_STL_TEMPLATE(multiset)

FROM_STRING_STL_TEMPLATE(list, push_back)
FROM_STRING_STL_TEMPLATE(vector, push_back)
FROM_STRING_STL_TEMPLATE(set, insert)
FROM_STRING_STL_TEMPLATE(multiset, insert)

#endif
