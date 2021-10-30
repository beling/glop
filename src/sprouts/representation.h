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

#ifndef REPRESENTATION_H
#define REPRESENTATION_H

#include "sproutsglobal.h"

//! @defgroup SproutsGroup Sprouts Game

//! Virtual base class representing a game object, essentially a vertex list.
//! All the other classes (Boundary, Region_spr, Land, Position) derive from Representation
//! @ingroup SproutsGroup
class Representation {
	//operator<< to print vertex list with cout
	friend ostream& operator<<(ostream&, const Representation&);
	
  public:
	Line r_vtx;           //!< vertex list representation
	set<Line> children;   //!< set of children (successors)
	
	Representation(const Line&);	//initialization with a vertex list
	Representation() {};
	virtual ~Representation() {};
	
	void reset(const Line&);
	unsigned int lives(); //!< number of lives
	
	//note : this two members have meaning only in the Structure class
	//but we define them here for simplicity of reset() and update_structure() code
	bool vertex_is_uptodate;     //indicates if the vertex list is uptodate
	bool structure_is_uptodate;  //indicates if the structure list is uptodate
	
	//purely virtual function returning the special "end of this structure" vertex
	//this function is overrided in each derived class
	virtual Vertex terminal_vertex() = 0;
	
	//comparison operator (needed by STL objects, in particular list::sort, set and map)
	bool operator<(const Representation&) const;
	bool operator==(const Representation&) const;
	
	//rename certain kind of letters in order : used in canonization
	void rename_in_order(int letterKind); //return true if a renaming occured
};


//-------------------------------------------------------------------------------
//Structure is the base class for "complex structures", ie Region_spr, Land, Position
//-------------------------------------------------------------------------------

//! Template class to represent "complex structures", ie Region_spr, Land, Position.
//! This class is a template, because we speak of "a structure of something"
//! for example : Region_spr is a structure of Boundaries
//! @ingroup SproutsGroup
template <class T>
class Structure : public Representation {
	
  public:
	list<T> r_str;				//!< structure representation (list of sub-objects)
	unsigned int topology;
	
	Structure(const Line& init) : Representation(init) {};
	Structure() {};
	
	void update_vertex();			//!< update vertex list (using the structure list)
	void update_structure();		//!< update structure list (using the vertex list)
	void update_representation();		//!< use the appropriate list to update the other
	
	//these functions must be called before making any change to a list
	//this is a secured way to remember which list is uptodate and which is not
	void change_vertex();			//!< must be called before changing the vertex list
	void change_structure();		//!< must be called before changing the structure list
};


//--------------------------------------------------------------------------
//implementation of Structure in this .h file because it's a template
//--------------------------------------------------------------------------

//! Update the list of vertex from the list of structures.
template <class T>
void Structure<T>::update_vertex() {
	typename list<T>::const_iterator Ti;
	
	quick_clear(r_vtx);
	//add the representation of the sub-structures to the list of vertex
	for(Ti=r_str.begin(); Ti != r_str.end(); Ti++) {
		quick_insert(r_vtx, Ti->r_vtx);
	}
	quick_end(r_vtx);
	
	if(topology!=sproutsGlobal::Sphere){
		r_vtx.push_back(sproutsGlobal::Topo_Region);
		r_vtx.push_back(topology);
	}
	
	//add an 'end of structure' character. For example, for region : End_Region
	r_vtx.push_back(terminal_vertex());
	
	vertex_is_uptodate=true;
}

//! Update structure (the list of sub-structures) from the list of vertices.
//! To save runtime, this function doesn't destroy old sub-structures list
//! but tries instead to update existing sub-structures, creating/deleting objects only if needed.
//! caution : be very careful to never suppose an object is a "new one"
//! (particularly with children class member for example).
template <class T>
void Structure<T>::update_structure() {
	topology=sproutsGlobal::Sphere;	//default value, will be updated later in the function if necessary
	
	//we need at least one empty T object
	Line r_empty;
	T T_empty(r_empty);
	
	// T object end vertex is T::terminal_vertex()
	Vertex end_sub_str=T_empty.terminal_vertex();
	
	bool struc_is_reg=(end_sub_str==sproutsGlobal::End_Boundary);
	
	//s_begin and s_end are the beginning and the end vertex of a T structure
	//s is the current vertex
	Line::const_iterator s_begin, s_end, s, r_vtx_end;
	
	typename list<T>::iterator Ti;	//iterator on the structure list
	
	Ti=r_str.begin();
	s_begin=r_vtx.begin();
	r_vtx_end = r_vtx.end();
	//simple loop on the vertex list searching T objects end vertex
	for(s=r_vtx.begin(); s != r_vtx_end; s++) {
		if (*s==end_sub_str) {
			//we found an end of T object
			
			//Ti must always refer to a valid structure in the structure list
			if(Ti==r_str.end()) {
				Ti=r_str.insert(r_str.end(), T_empty );
			}
			
			//place s_end after the last vertex of the T object
			s_end=s;
			s_end++;
			
			//store the found T object (vertices between s_begin and s_end) in Ti
			quick_clear(Ti->r_vtx);
			quick_insert(Ti->r_vtx, s_begin, s_end);
			quick_end(Ti->r_vtx);
			
			Ti->structure_is_uptodate=false;
			Ti->vertex_is_uptodate=true;
			
			//prepare next step
			s_begin=s_end;    //the next T object begins at s_end
			Ti++;             //place Ti on the next structure of the list
		} else
		if (struc_is_reg && *s==sproutsGlobal::Topo_Region) {
			s++;
			topology=*s;
		}
	}
	
	//destroy unused sub-structures
	if(Ti!=r_str.end()) {
		r_str.erase (Ti, r_str.end());
	}
	
	structure_is_uptodate=true;
}


//! Use the appropriate list to update the other.
//! After a call to this function, both representations are uptodate.
template <class T>
void Structure<T>::update_representation() {
	if (!vertex_is_uptodate) {
		update_vertex();
	}
	
	if(!structure_is_uptodate) {
		update_structure();
	}
}

//--------------------------------------------------------------------------
//functions that must be called to indicate a list is going to be changed
//--------------------------------------------------------------------------
//if we change a list, the other one isn't uptodate any more

//! Function that must be called before changing r_vtx (vertex representation)
template <class T>
void Structure<T>::change_vertex() {
	if (!vertex_is_uptodate) {
		Error(QString("Structure<T>::change_vertex : Attempt to change r_vtx, which is not uptodate"), true);
	}
	structure_is_uptodate=false;
}

//! Function that must be called before changing r_str (vertex representation)
template <class T>
void Structure<T>::change_structure() {
	if (!structure_is_uptodate) {
		Error(QString("Structure<T>::change_structure : Attempt to change r_str, which is not uptodate"), true);
	}
	vertex_is_uptodate=false;
}

#endif
