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
#include "../sprouts/position.h"

//the toString and fromString conversion functions are only available for Position
//we add here some useful functions to convert Boundary, Region or Lands to and from strings
Boundary BoundaryFromString(string init) {
	Line bndLine = fromString(init);
	//delete the end of region, land and position
	bndLine.pop_back();
	bndLine.pop_back();
	bndLine.pop_back();
	Boundary bnd(bndLine);
	return bnd;
}

string BoundaryToString(const Boundary &bnd) {
	Line bndLine = bnd.r_vtx;
	//add the end of region, land and position End_Region, End_Land, End_Position
	bndLine.push_back(sproutsGlobal::End_Region);
	bndLine.push_back(sproutsGlobal::End_Land);
	bndLine.push_back(sproutsGlobal::End_Position);
	return toString(bndLine);
}

void SproutsTest() {
	testTitle("Sprouts");
	
	//----------------------Line and string Representation ---------------------
	//note : "0*4" -> "0*2.1a1a" -> "0.1a1a.AB|AB"
	testBegin("Representation");
	testComment("Check that string -> Line -> string = Identity for various positions");
	string s="0*4";
	testOutput( toString(fromString(s)), s );

	s="0*2.1a1a";
	testOutput( toString(fromString(s)), s );
	
	s="0.1a1a.AB|AB";
	testOutput( toString(fromString(s)), s );
	
	s="0*22.1a1a.ABC.DE|0*9.DE|0*2.1abABCba";
	testOutput( toString(fromString(s)), s );

	//----------------------Boundary ---------------------
	testBegin("Boundary");
	string reverseComment = "Reverse boundary ";
	s="DE";
	testComment(reverseComment + s);
	Boundary bnd = BoundaryFromString(s);
	bnd.reverse_orientation();
	testOutput( BoundaryToString(bnd), "ED" );

	s="1abABCba";
	testComment(reverseComment + s);
	bnd = BoundaryFromString(s);
	bnd.reverse_orientation();
	testOutput( BoundaryToString(bnd), "abCBAba1" );

	s="GBEa1a2";
	testComment(reverseComment + s);
	bnd = BoundaryFromString(s);
	bnd.reverse_orientation();
	testOutput( BoundaryToString(bnd), "2a1aEBG" );
}
