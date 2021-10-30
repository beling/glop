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
#include "../stringconverter.h"

void StringConverterTest() {
	testTitle("StringConverter");
	string s;
	
	//-------------------------------- set<int> ----------------------------
	testBegin("set<int> -> string");
	testComment("Insert 5, 120, 7, 8");
	set<int> a;
	a.insert(5); a.insert(120); a.insert(7); a.insert(8);
	StringConverter conv;
	conv << a;
	testComment("Normal output");
	testOutput( conv.getString(), "5 7 8 120 " );
	
	testComment("Using * separator with CvCd(S*)");
	StringConverter conv2;
	conv2 << CvCd("S*") << a;
	testOutput( conv2.getString(), "5*7*8*120*" );
	
	testComment("Using * separator but no terminal separator with CvCd(LF)");
	StringConverter conv3;
	conv3 << CvCd("S*") << CvCd("LF") << a;
	printOutput( conv3.getString(), "5*7*8*120" );

	testBegin("string -> set<int>");
	testComment("Read the strings and compare to the initial set");
	set<int> readSet;
	s = conv.getString();
	conv >> readSet;
	printOutput( s, readSet==a);
	
	set<int> readSet2;
	s = conv2.getString();
	conv2 >> readSet2;
	printOutput( s, readSet2==a);
	
	set<int> readSet3;
	s = conv3.getString();
	conv3 >> readSet3;
	printOutput( s, readSet3==a);
	
	//-------------------------------- list<bool> ----------------------------
	testBegin("list<bool> -> string");
	testComment("Create list : true, false, true, true, false");
	list<bool> b;
	b.push_back(true); b.push_back(false); b.push_back(true); b.push_back(true); b.push_back(false);
	StringConverter conv4;
	conv4 << b;
	testComment("Normal output");
	testOutput( conv4.getString(), "T F T T F " );
	
	testComment("Using . separator with CvCd(S.)");
	StringConverter conv5;
	conv5 << CvCd("S.") << b;
	testOutput( conv5.getString(), "T.F.T.T.F." );
	
	testComment("Using . separator and 0/1 symbol with CvCd(B10)");
	StringConverter conv6;
	conv6 << CvCd("S.") << CvCd("B10") << b;
	testOutput( conv6.getString(), "1.0.1.1.0." );
	
	testComment("Using - separator, no terminal character, and t/f symbol");
	StringConverter conv7;
	conv7 << CvCd("S-") << CvCd("LF") << CvCd("Btf") << b;
	testOutput( conv7.getString(), "t-f-t-t-f" );
	
	testBegin("string -> list<bool>");
	testComment("Read the strings and compare to the initial list");
	list<bool> readSet4;
	s = conv4.getString();
	conv4 >> readSet4;
	printOutput( s, readSet4==b);
	
	list<bool> readSet5;
	s = conv5.getString();
	conv5 >> readSet5;
	printOutput( s, readSet5==b);
	
	list<bool> readSet6;
	s = conv6.getString();
	conv6 >> readSet6;
	printOutput( s, readSet6==b);
	
	list<bool> readSet7;
	s = conv7.getString();
	conv7 >> readSet7;
	printOutput( s, readSet7==b);
}
