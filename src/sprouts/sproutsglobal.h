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

#ifndef SPROUTSGLOBAL_H
#define SPROUTSGLOBAL_H

#include "../global.h"
#include "../gameglobal.h"

//! Useful function to insert a line at the end of another line
void addLine(Line& lineA, const Line& lineB);

//functions to update quickly a Line (faster than clear-insert)
void quick_clear(Line& lineA); //must be called before beginning an update of lineA
void quick_insert(Line& lineA, const Line& lineB); //to insert a line in lineA
void quick_insert(Line& lineA, Line::const_iterator l_begin, Line::const_iterator l_end); //to insert a range
void quick_end(Line& lineA);			//to end the update process (remove useless objects)


//! Conversions between string and Line types
string toString(const Line& r_vtx);
Line fromString(const string& stringA);
Line oldFromString(const string& stringA);

//! Short notation of positions with a lot of zeros
string zero_notation(const string& stringA);
string expand_zero(const string& stringA);

string old_zero_notation(const string& stringA);
string old_expand_zero(const string& stringA);

//! Powers of 2 (with overflow handling)
unsigned long powers_of_two(int n);

//! Powers of a number b (without overflow handling)
unsigned long powers_estimate(int b, int n);

bool has6zeros(const Line&);
Line delete6zeros(const Line&);

//! Order on the Lines, used for canonization
bool lineOrder(const Line& La, const Line& Lb);

//! Constant values for particular Vertices
//! @ingroup SproutsGroup
class sproutsGlobal {
  public :
	//special vertices values (mostly used through the above functions "handle vertices")
	static const Vertex First_vtx, First_1bnd_vtx, Last_1bnd_vtx, First_2bnd_vtx, Last_2bnd_vtx, First_2reg_vtx, Last_2reg_vtx;
	static const Vertex Child_vtx_1, Child_vtx_2, Child_vtx_3, Last_vtx;
	static const Vertex End_Boundary, Topo_Region, End_Region, End_Land, End_Position;
	static const Vertex Sphere, First_Torus, First_Crosscap;
	
	//position kind
	static const char Empty_position, Single_land, Multi_lands;
	
	//static variable used by the quick-update process
	static Line::iterator line_virtual_end;
};

//! Useful functions to handle the different kind of vertices
bool is_1bnd_vtx(const Vertex&);
bool is_2bnd_vtx(const Vertex&);
bool is_2reg_vtx(const Vertex&);
bool is_temp_vtx(const Vertex&);
bool is_letter_vtx(const Vertex&);
bool is_real_vtx(const Vertex&);
bool exist_2bnd(const Line&);
bool exist_1bnd(const Line&);

#endif
