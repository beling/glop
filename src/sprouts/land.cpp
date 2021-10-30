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

#include "land.h"

//override terminal_vertex virtual function
Vertex Land::terminal_vertex() {
	return sproutsGlobal::End_Land;
}

//-------------------------------------------------------------------------------------------------
// Reduction
//-------------------------------------------------------------------------------------------------

//! Delete vertices of degree 3
void Land::delete_dead_vertices() {
//note : there is at most two dead vertices. It could be used to optimize this function more
	//-----delete generic vertex '3'-------------------
	r_vtx.remove(3);
	
	//-----delete 2-region vertices of degree 3--------
	//vector to remember the number of times a 2-region letter appears
	//size of vector is equal to the number of possible 2-region letters
	static vector<int> vertex_degree(sproutsGlobal::Last_2reg_vtx-sproutsGlobal::First_2reg_vtx+1);
	
	//initialization, convention : 0 means the letter is not used
	fill(vertex_degree.begin(), vertex_degree.end(), 0);
	
	//note : index in the vector = vertex name - sproutsGlobal::First_2reg_vtx
	//compute the degree of each vertex
	Line::iterator s;
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		if(is_2reg_vtx(*s)) {
			vertex_degree[*s-sproutsGlobal::First_2reg_vtx]+=1;
		}
	}
	
	//delete dead vertices (degree 3)
	Vertex i;
	for(i=sproutsGlobal::First_2reg_vtx; i<=sproutsGlobal::Last_2reg_vtx; i++) {
		if(vertex_degree[i-sproutsGlobal::First_2reg_vtx]==3) {
			r_vtx.remove(i);
		}
	}
}

//! Delete adjacent vertices in a boundary, e.g. 1AA. ---> 1A. or A11A. ---> 11A.
void Land::delete_adjacent_vertices() {
	//s1 and s2 are used to go through the list, s2 is always equal to s1's next element
	//s_begin is used to remember the first vertex of a Boundary
	Line::iterator s_begin=r_vtx.begin(), s1=r_vtx.begin(), s2=r_vtx.begin();
	
	//s2 initialization
	if (s2 != r_vtx.end() ) s2++;
	
	bool increment_needed=true; //indicate if we need to increment s1 and s2 at the end of the loop
	
	while(s2 != r_vtx.end()) {
		//----- detect new boundary ------------
		//update s_begin (first vertex of the boundary) when we enter a new boundary
		//note : a new boundary is "detected" when a "non-real" vertex is followed by a "real" one
		if (!is_real_vtx(*s1) && is_real_vtx(*s2)) {
			s_begin=s2;
		}
		
		//---------- test if s1 is adjacent ------
		//adjacence has meaning only for letters
		if( is_letter_vtx(*s1) ) {
			//adjacence between last and first real vertex of a boundary longer than one vertex
			if(*s2==sproutsGlobal::End_Boundary && s_begin != s1 && *s_begin == *s1) {
				s1=r_vtx.erase(s1); //erase and increment s1
				s2++;
				increment_needed = false;
			}
			
			//adjacence between two consecutive vertices
			if(*s1 == *s2) {
				// caution : we must erase s2, because s1 and s_begin can refer to the same vertex
				// and s_begin should never be erased
				s2=r_vtx.erase(s2); //erase and increment s2
				increment_needed = false;
			}
		}
		
		//---------- increment s1 and s2 if necessary -------
		if (increment_needed) {
			s1++;
			s2++;
		}
		increment_needed=true;  //initialization for next loop
	}
}

//! Delete empty boundaries
void Land::delete_empty_boundaries() {
	//s2 is always equal to the element immediately after s1
	Line::iterator s1=r_vtx.begin(), s2=r_vtx.begin();
	if (s2 != r_vtx.end()) s2++;
	
	while(s2 != r_vtx.end()) {
		if (s1==r_vtx.begin() && *s1==sproutsGlobal::End_Boundary) {
			//empty boundary in the beginning of the vertex list : .
			s1=r_vtx.erase(s1);	//erase s1 and place it on next vertex
			s2++;			//place s2 on next vertex
		} else if (*s2==sproutsGlobal::End_Boundary && (*s1==sproutsGlobal::End_Boundary || *s1==sproutsGlobal::End_Region)) {
			//empty boundary in other cases :  .. or }.
			//boundary is empty : we just delete the 'end of boundary' character
			//erase s2 and place it on next vertex (s1 stays at the same vertex)
			s2=r_vtx.erase(s2);
		} else {
			//incrementation only if we didn't delete any empty boundary
			s1++; s2++;
		}
	}
}

//! Delete dead regions (region list is supposed to be uptodate)
void Land::delete_dead_regions() {
	list<Region_spr>::iterator Ri;
	
	Ri=r_str.begin();
	while( Ri!=r_str.end()) {
		if(Ri->is_dead()) {
			//erase the dead region and place Ri on the next region
			Ri=r_str.erase(Ri);
		} else {
			//just place Ri on the next region
			Ri++;
		}
	}
}

//! Merge boundaries in regions (region equivalence theory)
void Land::merge_boundaries(){
	list<Region_spr>::iterator Ri;
	for(Ri=r_str.begin(); Ri!=r_str.end(); Ri++) {
		Ri->merge_boundaries();
	}
}

//! Generic name '2' used for 2-region vertices which occur only one time in the land
//! return true if some vertex was renamed
bool Land::apply_generic_vertex() {
	bool hasChanged=false;
	Line::iterator p, q, s;
	p=r_vtx.begin(); q=r_vtx.end();
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		//-----2-region or temporary vertices-----
		if( is_2reg_vtx(*s) || is_temp_vtx(*s) ) {
			//rename the vertex with two if it appears only once in the land
			if(count(p,q,*s)==1) {
				*s=2;
				hasChanged=true;
			}
		}
	}
	return hasChanged;
}

//! Reduction of land vertex list representation.
//! note : the order of the different treatments is very important
void Land::reduce(){
	bool hasChanged;
	change_vertex();
		delete_dead_vertices();
		delete_adjacent_vertices();
		delete_empty_boundaries();
	update_representation();
	
	change_structure();
		delete_dead_regions();
		distinguish_vertices();
		merge_boundaries();
	update_representation();
	
	hasChanged=apply_generic_vertex();    //use generic name '2'
	if(hasChanged) change_vertex();       //indicate that vertex representation has changed (if needed)
}


//-------------------------------------------------------------------------------------------------
// Canonization
//-------------------------------------------------------------------------------------------------

//! Sort land's representation
void Land::sort() {
	//sort each region
	list<Region_spr>::iterator Ri;
	for(Ri=r_str.begin(); Ri!=r_str.end(); Ri++) {
		Ri->sort();
	}
	
	//sort of regions
	//uses Representation::operator< => lineOrder()
	r_str.sort();
}

//! Canonization of the land.
//! Basically, this is two (rename+sort)
void Land::canonize() {
	change_vertex();
		rename_in_order(2);
	update_representation();
	
	change_structure();
		sort();
		
		if (exist_1bnd(r_vtx) || exist_2bnd(r_vtx)) {
			rename_1reg_letters();
		}
	update_representation();
	
	change_vertex();	//indicate that the vertex representation has changed
	rename_in_order(2);
	update_representation();
	
	change_structure();
		sort();
	update_representation();
}


//--------------------------------------------------------------------------
//split land in new independent ones
//--------------------------------------------------------------------------

//! Test if two lists of vertices have a common temp/2-region vertex.
//! If two lists are "linked", it means they are part of the same land
//! @return true if the two lists are linked
bool linked(const Line& a1, const Line& a2) {
	Line::const_iterator s1, s2;
	
	for(s1=a1.begin(); s1!=a1.end(); s1++) {
		if(is_2reg_vtx(*s1) || is_temp_vtx(*s1)) {
			for(s2=a2.begin(); s2!=a2.end(); s2++) {
				if (*s1 == *s2) {
					//we found a common temp/2-region vertex
					return true;
				}
			}
		}
	}
	
	//there is no common vertex
	return false;
}

//! Split algorithm : try to split the land in independent ones.
//! The result is stored in the Land's vertex representation
//! @return true if split was possible
bool Land::split() {
	update_representation();
	list<Line> indep_land;		//list of independent lands (split function result)
	
	list<Region_spr>::iterator Ri;
	list<Line>::iterator Lj;	//iterator on the list of independent lands
	
	Line linked_vertices;		//list of vertices linked to the current region
	list<Line> relative_indep_land;	//list of lands independent from the current region
	bool new_land;			//indicate if current region is a new independant land or not
	
	//loop on the regions
	for(Ri=r_str.begin(); Ri!=r_str.end(); Ri++) {
		//we separate all already found lands into two categories :
		quick_clear(linked_vertices);	// - linked to the current region (current region's vertex "appears")
		relative_indep_land.clear();	// - independent from the current region
		new_land=true;
		
		for(Lj=indep_land.begin(); Lj!=indep_land.end(); Lj++) {
			if (linked(*Lj, Ri->r_vtx)) {
				//add this land to the "tied list"
				quick_insert(linked_vertices, *Lj);
				new_land=false;
			} else {
				//this region doesn't interfere with land's freedom
				//so we add the land in the list of free lands
				relative_indep_land.push_back(*Lj);
			}
		}
		
		if (new_land) {
			relative_indep_land.push_back(Ri->r_vtx);	//create a new land (region independent from already existing lands)
		} else {
			quick_insert(linked_vertices, Ri->r_vtx);		//add the region to "the tied list"
		}
		quick_end(linked_vertices); //end of the insertions in linked_vertices
		
		//update the complete list of independent land
		indep_land.clear();
		indep_land.insert(indep_land.begin(), relative_indep_land.begin(), relative_indep_land.end());
		if ( !linked_vertices.empty() ) {
			indep_land.push_back(linked_vertices);
		}
	}
	
	//add 'end of land' characters
	for(Lj=indep_land.begin(); Lj!=indep_land.end(); Lj++) {
		Lj->push_back(sproutsGlobal::End_Land);
	}
	
	//result is saved in the vertices list of the land
	//this is an exceptional case where we use vertices list to store more than one land
	if(indep_land.size()>1) {
		//split occured
		quick_clear(r_vtx);
		for(Lj=indep_land.begin(); Lj!=indep_land.end(); Lj++) {
			quick_insert(r_vtx, *Lj);
		}
		quick_end(r_vtx);
		return true;
	} else {
		//split didn't occur
		return false;
	}
}


//--------------------------------------------------------------------------
//Children of the land
//--------------------------------------------------------------------------

//! Rename 1-boundary and 2-boundary letters in 2-region letters
void Land::prepare_children_computation() {
	//function useless if there's neither 1-boundary nor 2-boundary letter
	if( ! (exist_2bnd(r_vtx) || exist_1bnd(r_vtx)) ) {
		compute_children();
		return;
	}
	
	Line sav;
	
	update_representation();
	change_vertex();
	sav=r_vtx;
	
	//-----find next available 2-region Vertex
	Vertex free_vtx=sproutsGlobal::First_2reg_vtx;
	Line::iterator s, p;
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		//test 2-region Vertex and if it's greater than the current available Vertex
		if(is_2reg_vtx(*s) && *s>=free_vtx) {
			free_vtx=*s+1; //*s is used. We need a new value for current available letter
		}
	}
	
	//security on the number of used 2-region letters
	if (free_vtx>sproutsGlobal::Last_2reg_vtx) {
		Error(QString("Land::compute_children : 2-region letters capacity overflow (1)"), true);
	}
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		//if Vertex is a 1-boundary/2-boundary letter, rename it with the first available 2-region letter
		if(is_1bnd_vtx(*s) || is_2bnd_vtx(*s)) {
			p=s; p++;
			while(*p != *s) p++;
			*s=free_vtx; *p=free_vtx;
			free_vtx++; //update first unused 2-region letter
		}
	}
	
	//security on the number of used 2-region letters
	if ( (free_vtx-1) > sproutsGlobal::Last_2reg_vtx) {
		Error(QString("Land::compute_children : 2-region letters capacity overflow (2)"), true);
	}
	
	//-----Children computation
	compute_children();
	
	//-----Restore previous vertex representation of the land (without all these 2-region letters)
	change_vertex();
	r_vtx=sav;
	update_representation();
}

//! Compute the children
//! Function similar to Position::compute_children().
//specific code is clearly indicated
void Land::compute_children() {
	update_representation();
	children.clear();
	
	//specific code - compared to Position::compute_children()
	list<Region_spr>::iterator it;
	list<Region_spr>::iterator it2;
	//end of specific code
	
	set<Line>::iterator it_children;
	Line l_unused; //vertex representation of the list of unused sub-structures
	Line l_tmp;
	
	//compute children of each sub-structure
	//a child is a sub-structure child completed with the unused other substructures
	
	//loop on the substructures
	for(it=r_str.begin(); it!=r_str.end(); it++) {
		//compute substructure's children
		//specific code
		it->compute_children();
		//end of specific code
		
		//list of unused sub-structures
		quick_clear(l_unused);
		for(it2=r_str.begin(); it2!=r_str.end(); it2++) {
			if(it2!=it) {
				//this is an unused substructure -> we add it to l_unused
				quick_insert(l_unused, it2->r_vtx);
			}
		}
		quick_end(l_unused);
		
		//we add the list of unused sub-structures to each child of the used sub-structure
		//loop on the sub-structure's children
		for(it_children=it->children.begin(); it_children!=it->children.end(); it_children++) {
			quick_clear(l_tmp);					//delete temporary list
			quick_insert(l_tmp, l_unused);		//add list of unused sub-structures
			quick_insert(l_tmp, *it_children);	//add child's representation
			quick_end(l_tmp);
			l_tmp.push_back(terminal_vertex());	//add 'end of structure'
			
			children.insert(l_tmp);
		}
	}
}

//! Estimate the number of children in the land
unsigned int Land::estimate_children_number() {
	update_representation();
	
	unsigned int estimation=0;
	list<Region_spr>::iterator Ri;
	
	//sum of estimation in each "region"
	for(Ri=r_str.begin(); Ri!=r_str.end(); Ri++) {
		estimation += Ri->estimate_children_number();
	}
	
	return estimation;
}

//-------------------------------------------------------------------------
//vertices distinction
//--------------------------------------------------------------------------

//! Rename 2-region letters in 1-boundary/2-boundary ones (when needed)
void Land::distinguish_vertices() {
	list<Region_spr>::iterator Ri;
	for(Ri=r_str.begin(); Ri!=r_str.end(); Ri++) {
		Ri->distinguish_vertices();
	}
}

//! Rename 1-region vertices (ie 1-boundary and 2-boundary vertices) in their order of appearance
void Land::rename_1reg_letters() {
	list<Region_spr>::iterator Ri;
	for(Ri=r_str.begin(); Ri!=r_str.end(); Ri++) {
		Ri->rename_1reg_letters();
	}
}
