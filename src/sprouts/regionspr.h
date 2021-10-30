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

#ifndef REGIONSPR_H
#define REGIONSPR_H

#include "boundary.h"

//! Region_spr is a Structure of Boundary.
//! @ingroup SproutsGroup
//! @nosubgrouping
class Region_spr : public Structure<Boundary>  {
  public:
	Region_spr(const Line& init) : Structure<Boundary>(init) {};
	
	//override terminal_vertex virtual function
	Vertex terminal_vertex();
	
	bool orientable(); //indicate whether the region is orientable or not
	int genusValue();
	
	//-------------------------------------------------------------------
	//! @name Reduction/canonization
	//@{
	void simple_sort();		//!< sort boundaries only
	void sort();			//!< sort boundaries (with possible inversion of orientation)
	bool is_dead() const;		//!< return true when region is dead
	void merge_boundaries();	//!< merge little boundaries (theory of region equivalence)
	//@}
	
	//----------------------------------------------------------------------------
	//! @name Children
	//@{
	void compute_children();			//!< complete list of children
	void compute_1bnd_children();		//!< children when a boundary is linked to itself
	void compute_2bnd_children();		//!< children when two different boundaries are linked
	list<Boundary>::iterator last_zero();
	list<Boundary>::iterator second_to_last_zero();
	
	set<couple> possible_partition;			//!< object to save boundaries repartition
	void partition(const list<Boundary>&);		//!< compute all ways to cut a list of boundaries in two parts

	unsigned int estimate_children_number();	//!< estimate the number of children
	//@}
	
	//---------------------------------------------------------------
	//! @name vertices distinction
	//@{
	void distinguish_vertices();
	void rename_1reg_letters();
	//@}
};

#endif
