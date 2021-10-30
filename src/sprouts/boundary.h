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

#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "representation.h"

typedef pair<Line,Line> couple;

//! a Boundary has just a representation (it has no structure)
//! @ingroup SproutsGroup
//! @nosubgrouping
class Boundary : public Representation  {
  public:
	Boundary(const Line& init)  : Representation(init) {};
	
	//override terminal_vertex virtual function
	Vertex terminal_vertex();
	
	//----------------------------------------------------------
	//! @name Reduction/canonization
	//@{
	void sort();			//!< sort algorithm (by successive rotations)
	void reverse_orientation();	//!< inversion of boundary's orientation
	void sort_unoriented();		//!< sort algorithm for non orientable surfaces
	bool is_single_spot() const;	//!< return true if the boundary is "0."
	void distinguish_vertices();	//!< rename (if possible) vertices in 1-boundary ones
	//@}
	
	//----------------------------------------------------------
	//! @name Children
	//@{
	void compute_1bnd_children();	//!< children when the boundary is linked to itself
	void compute_2bnd_children();	//!< children bribe when the boundary is linked to another one
	
	set<couple> children_couple;		//!< set to save compute_1bnd_children() results and avoid useless recomputing
	//@}
};

#endif
