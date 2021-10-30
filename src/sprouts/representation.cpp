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

#include "representation.h"

//--------------------------------------------------------------------------
//friend function to print string simply with << operator
//--------------------------------------------------------------------------
ostream& operator<<(ostream& ostr, const Representation& a) {
	//return on the stream a string corresponding to the vertex list
	return ostr << toString(a.r_vtx);
}


//--------------------------------------------------------------------------
//initializations
//--------------------------------------------------------------------------
Representation::Representation(const Line& init) {
	r_vtx=init; //init parameter is the vertex list
	vertex_is_uptodate=true;
	structure_is_uptodate=false;
}

void Representation::reset(const Line& init) {
	r_vtx=init;
	vertex_is_uptodate=true;
	structure_is_uptodate=false;
}


//--------------------------------------------------------------------------
//definitions of < and == operators to allow STL sort operations
//--------------------------------------------------------------------------

bool Representation::operator<(const Representation& a) const {
	return lineOrder(r_vtx, a.r_vtx);
}

bool Representation::operator==(const Representation& a) const {
	return r_vtx==a.r_vtx;
}

unsigned int Representation::lives() {
	unsigned int lives=0;
	unsigned int letters=0;
	
	Line::const_iterator s;
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		if(*s>=sproutsGlobal::First_vtx && *s<=sproutsGlobal::Last_vtx) {
			letters++;
		} else
		if(*s==0 || *s==1 || *s==2) {
			lives += 3 - (*s);
		}
	}
	
	//add half the number of letters to the number of lives
	lives += letters/2;
	
	return lives; 
}

//! Rename in order certain kind of letters (used in canonization functions).
//! letterKind == 0 : rename 1-boundary letters from First_1bnd_vtx.
//! letterKind == 1 : rename 2-boundary letters from First_2bnd_vtx.
//! letterKind == 2 : rename 2-region and temporary vertices from First_2reg_vtx.
void Representation::rename_in_order(int letterKind) {
	static vector<Vertex> new_names; //the vector is big so a static variable is better
	if(new_names.empty()) {
		new_names.resize(sproutsGlobal::Last_vtx+1);  //vertices new names
		fill(new_names.begin(), new_names.end(), 0);  //initialization : 0 means here "new name is not yet known"
	}
	
	list<Vertex> usedIndex; //list of index used in new_names (need to be initialized back to 0 at the end)
	Line::iterator s, s_end;
	Vertex next_letter;
	
	if(letterKind == 0) {
		next_letter=sproutsGlobal::First_1bnd_vtx;
	} else
	if(letterKind == 1) {
		next_letter=sproutsGlobal::First_2bnd_vtx;
	} else {
		next_letter=sproutsGlobal::First_2reg_vtx; //next unused name (we begin at First_2reg_vtx)
	}
	
	s_end=r_vtx.end();
	for(s=r_vtx.begin(); s!=s_end; s++) {
		//check if vertex is a 1-boundary letter in case of letterKind 0
		//   or if vertex is a 2-boundary letter in case of letterKind 1
		//   or if vertex is a temp/2-region letter in case of letterKind 2
		if( ( letterKind==0 && (is_1bnd_vtx(*s)) ) ||
		    ( letterKind==1 && (is_2bnd_vtx(*s)) ) ||
		    ( letterKind==2 && (is_2reg_vtx(*s) || is_temp_vtx(*s)) )
		  ) {
			//check if a new name was already given to this vertex
			if (new_names[*s]==0) {
				new_names[*s]=next_letter;	//give a new name (first available letter)
				usedIndex.push_back(*s);
				*s=next_letter;
				next_letter++;			//increment the first available letter
			} else {
				*s=new_names[*s];		//use the already given new name
			}
		}
	}
	
	//initialize back the vector new_names
	list<Vertex>::iterator Vi;
	for(Vi = usedIndex.begin(); Vi != usedIndex.end(); Vi++) {
		new_names[*Vi]=0;
	}
	
	//security on the number of used letters
	next_letter--; //last used letter
	if (letterKind==0 && next_letter>sproutsGlobal::Last_1bnd_vtx) {
		Error(QString("Representation::rename_in_order : 1-boundary letters capacity overflow"), true);
	}
	if (letterKind==1 && next_letter>sproutsGlobal::Last_2bnd_vtx) {
		Error(QString("Representation::rename_in_order : 2-boundary letters capacity overflow"), true);
	}
	if (letterKind==2 && next_letter>sproutsGlobal::Last_2reg_vtx) {
		Error(QString("Representation::rename_in_order : 2-region letters capacity overflow"), true);
	}
}
