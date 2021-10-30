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

#ifndef LAND_H
#define LAND_H

#include "regionspr.h"

//some functions use r_str or r_vtx but don't refresh the other representation with update_representation()
//the refresh is done at optimal times in reduce() or in canonize()

//! Land is a structure of Region_spr
//! @ingroup SproutsGroup
//! @nosubgrouping
class Land : public Structure<Region_spr>  {
  public:
	Land(const Line& init) : Structure<Region_spr>(init) {};
	
	//override terminal_vertex virtual function
	Vertex terminal_vertex();
	
	//------------------------------------------------------------------
	//! @name Reduction
	//@{
	void delete_dead_vertices();     //!< delete dead vertices (degree 3)
	void delete_adjacent_vertices(); //!< delete adjacent vertices
	void delete_empty_boundaries();  //!< delete empty boundaries
	void delete_dead_regions();      //!< delete dead regions (1 life at most)
	void merge_boundaries();         //!< merge little boundaries (region equivalence theory)
	bool apply_generic_vertex();     //!< rename appropriate vertices with generic name 2
	void reduce();                   //!< reduction
	//@}
	
	//-------------------------------------------------------------------
	//! @name Canonization
	//@{
	bool split();                   //!< split the land in independant ones (if possible)
	void sort();                    //!< sort regions of the land
	void canonize();                    //!< canonization (it was called double pseudo-cano before)
	//@}
	
	//-------------------------------------------------------------------
	//! @name Distinction between vertices
	//@{
	void distinguish_vertices();	//!< recognize 1-boundary and 2-boundary vertices
	void rename_1reg_letters();	//!< rename 1-boundary vertices from letter a
	//@}
	
	//-------------------------------------------------------------
	//! @name Children
	//@{
	void prepare_children_computation();     //!< rename letters in 2-region vertices
	void compute_children();                 //!< compute land's children
	unsigned int estimate_children_number(); //!< estimate the number of children
	//@}
};

//test if two vertices lists are linked or not - used by Land::split()
bool linked(const Line&, const Line&);

#endif
