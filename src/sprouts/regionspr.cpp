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

#include "regionspr.h"

//override terminal_vertex virtual function
Vertex Region_spr::terminal_vertex() {
	return sproutsGlobal::End_Region;
}

//! Indicate whether the region is orientable or not.
//! @return true if the region is orientable
bool Region_spr::orientable() {
	if(topology<=sproutsGlobal::First_Crosscap) {
		return false;
	} else {
		return true;
	}
}

//! Indicate the genus of the region.
//! @return Sphere : 0
int Region_spr::genusValue() {
	if(orientable()){
		//Torus with n handles : n
		return topology-sproutsGlobal::First_Torus+1;
	} else {
		//Connected sum of n cross-caps : n
		return sproutsGlobal::First_Crosscap-topology+1;
	}
}

//-------------------------------------------------------------------------------------------------
// Reduction/canonization functions
//-------------------------------------------------------------------------------------------------

//! Sort each boundary, and then the list of boundaries
void Region_spr::simple_sort() {
	//sort each boundary
	list<Boundary>::iterator Bi;
	for(Bi=r_str.begin(); Bi!=r_str.end(); Bi++) {
		Bi->sort();
	}
	
	//uses Representation::operator< => lineOrder()
	r_str.sort();	//sort the list of boundaries
}

//! Sort region (with possible orientation inversion)
void Region_spr::sort() {
	update_representation();
	list<Boundary>::iterator Bi;
	
	//-----case of a non orientable surface------------
	if( !orientable() ){
		change_structure();
		for(Bi=r_str.begin(); Bi!=r_str.end(); Bi++) {
			Bi->sort_unoriented();
		}
		//uses Representation::operator< => lineOrder()
		r_str.sort();		//sort boundaries
		update_representation();
	} else {
	//-----case of an orientable surface---------------
		//----------normal orientation---------------------
		change_structure();
			simple_sort();	//sort boundaries
		update_representation();
		Line r_sav=r_vtx;	//save the result
		
		//----------inverse orientation--------------------
		//reverse each boundary
		change_structure();
			for(Bi=r_str.begin(); Bi!=r_str.end(); Bi++) {
				Bi->reverse_orientation();
			}
			simple_sort();	//sort boundaries
		update_representation();
		//----------compare results------------------------
		//keep the best orientation
		if (lineOrder(r_sav,r_vtx)) {
			change_vertex();
			r_vtx=r_sav;
		}
	}
}

//! @return true when the region is dead
//(no move possible : because one single vertex or less in the region)
bool Region_spr::is_dead() const {
	Line::const_iterator s;
	int lives=0;
	Vertex s_save=0;	//to avoid to keep alive A.A.} (happens only with surfaces)
	
	//this loop stops as soon as the region is proved alive
	for(s=r_vtx.begin(); s != r_vtx.end() && lives<2; s++) {
		if(is_letter_vtx(*s) && (*s != s_save)) {
			//normal letter : one life
			lives += 1;
			s_save = *s;
		} else
		if(*s<=2) {
			//generic vertex : (3-*s) lives
			lives += (3-*s);
		}
	}
	
	//returns dead/alive state (true if less than 2 lives)
	return (lives<2);
}

//! Simplifications if the region has at most 3 lives.
//! Removes all the End_Boundary characters (only the last one remains).
//! Also cleans information about surfaces, and changes 1-boundary and 2-boundary vertices into '2'
void Region_spr::merge_boundaries() {
	Line::iterator s;
	int half_lives=0;		//half of number of lives
	
	//this loop stops as soon as the region is found with more than 3 lives
	for(s=r_vtx.begin(); s != r_vtx.end() && half_lives<=6; s++) {
		//after Land::distinguish_vertices(), these 2 vertices types occur 2 times in each region
		//so half_lives will necessary be even at the end of the loop
		if(is_1bnd_vtx(*s) || is_2bnd_vtx(*s)) {
			half_lives += 1;
		} else if(is_2reg_vtx(*s) || is_temp_vtx(*s)) {
			half_lives += 2;
		} else if(*s<=2) {
			//generic vertex : (3-*s) half-lives
			half_lives += (3-*s)*2;
		}
	}
	
	//there's something to do only if there's 2 or 3 lives
	if(half_lives==4 || half_lives==6){
		change_vertex();
		
		Line result;
		int half_2=0;		//half of number of '2'
		
		for(s=r_vtx.begin(); s != r_vtx.end(); s++) {
			if(is_1bnd_vtx(*s) || is_2bnd_vtx(*s)) {
				half_2 += 1;
			} else if(*s==2){
				half_2 += 2;
			} else if(is_real_vtx(*s)) {	//*s = 2-region Vertex (eventually tmp) or '0' or '1'
				result.push_back(*s);
			}
		}
		
		int i;
		for(i=1;i<=half_2/2;i++){
			result.push_back(2);
		}
		result.push_back(sproutsGlobal::End_Boundary);
		result.push_back(sproutsGlobal::End_Region);
		r_vtx=result;
	}
}


//-------------------------------------------------------------------------------------------------
//Children computation
//-------------------------------------------------------------------------------------------------

//! Compute all the children of the region.
//! There are two different kinds of children :
//! those obtained by linking a boundary to itself
//! and those obtained by linking two different boundaries
void Region_spr::compute_children() {
	update_representation();
	children.clear();
	compute_1bnd_children();		//children obtained by linking a boundary to itself
	compute_2bnd_children();	//children obtained by linking two different boundaries
}

//return the position of the last zero boundary minus one
//for example 0.0.0.0.0.1a1a.} returns the position of the 4th 0.
//it is used when computing boundary children to avoid redondant computations
list<Boundary>::iterator Region_spr::second_to_last_zero() {
	list<Boundary>::iterator Bi=r_str.begin();
	while(Bi!=r_str.end() && Bi->is_single_spot()) Bi++;
	if(Bi!=r_str.begin()) Bi--;
	if(Bi!=r_str.begin()) Bi--;
	return Bi;
}

//return the position of the last zero boundary
//for example 0.0.0.0.0.1a1a.} returns the position of the 5th 0.
list<Boundary>::iterator Region_spr::last_zero() {
	list<Boundary>::iterator Bi=r_str.begin();
	while(Bi!=r_str.end() && Bi->is_single_spot()) Bi++;
	if(Bi!=r_str.begin()) Bi--;
	return Bi;
}

//useful function to add topology info to a list
//used in the following Region_spr::compute_1bnd_children() function
//line is modified according to orientable and genus parameters
void add_topo(Line& line, bool isOrientable, int genus) {
	//indicate topology only if it isn't a sphere (sphere by default)
	if(genus>0) {
		line.push_back(sproutsGlobal::Topo_Region);
		if(isOrientable) {
			line.push_back(sproutsGlobal::First_Torus-1+genus);
		} else {
			line.push_back(sproutsGlobal::First_Crosscap+1-genus);
		}
	}
	
	line.push_back(sproutsGlobal::End_Region);	//add 'end of region' character in any case
}

//useful function to reverse orientation of a list of boundaries
Line reverseOrientation(const Line& line) {
	if( line.empty() ) return line;
	Line result = line;
	result.pop_back();
	result.reverse();
	result.push_back(sproutsGlobal::End_Boundary);
	return result;
}

//useful function to merge two boundaries in case of a II.B.2 move
Line mergeIIB2(const Line& lineA, const Line& lineB) {
	Line result;
	addLine(result, lineB);		//copy "ai ... aj # ."
	result.pop_back();		//remove End_of_boundary
	result.pop_back();		//remove Child_vtx_3
	result.reverse();		//now we have "aj ... ai"
	result.push_back(sproutsGlobal::Child_vtx_3);	//now we have "aj ... ai #"
	addLine(result, lineA);		//now we have "aj ... ai # aj ... ar a1 ... ai # ."
	return result;
}

//! Children of a region when a boundary is linked to itself
void Region_spr::compute_1bnd_children() {
	list<Boundary>::iterator Bi, Bk;
	set<couple>::const_iterator Ci, Rj;	//for children and partition of Bi
	
	int genus = genusValue();
	int i,j;
	unsigned long k,q;
	
	list<Boundary> unused_bnd;	//list of "unused" boundaries
	Line child;			//one child of the region
	Line L_tmp;
	
	//loop on the boundary list
	for(Bi=last_zero(); Bi!=r_str.end(); Bi++) {
		Bi->compute_1bnd_children();	//children of Bi (a child is a pair of boundaries)
		
		//----- a) create list of unused boundaries -------------------
		unused_bnd.clear();
		for(Bk=r_str.begin(); Bk!=r_str.end(); Bk++) {
			if(Bk!=Bi) {
				//this is an unused boundary -> we add it to unused_bnd
				unused_bnd.push_back(*Bk);
			}
		}
		
		//----- b) create set of all possible partitions in two blocks-
		partition(unused_bnd);
		
		//----- c) create children of the region-----------------------
		for(Ci=Bi->children_couple.begin(); Ci!=Bi->children_couple.end(); Ci++) {
		
		//---Type II.A (a)(b)
		for(Rj=possible_partition.begin(); Rj!=possible_partition.end(); Rj++) {
		for(i=0;i<=genus;i++){
		if (orientable() || (i!=0 && i!=genus)) {
			quick_clear(child);
			
			quick_insert(child, Ci->first);
			quick_insert(child, Rj->first);
			quick_end(child);	//add_topo is not ready for the quick insertion, so we must stop here
			add_topo(child, orientable(), i);	//infos on topology
			
			addLine(child, Ci->second);
			addLine(child, Rj->second);
			add_topo(child, orientable(), genus-i);	//infos on topology
			
			//add the child to the region's list of children
			children.insert(child);
		}}}
		
		//---Type II.A (c)
		if (!orientable()) {
		for(Rj=possible_partition.begin(); Rj!=possible_partition.end(); Rj++) {
		for(i=0;i<(genus+1)/2;i++){
		//Dtmp : for the boundaries that will go in the orientable surface
		Region_spr Dtmp(Rj->first);
		//delete the "0" (whose orientation isn't important)
		//it uses the fact that the partitions always begin with the 0.0.0...
		int nb_zeros=0;
		bool finished=false;
		while(!Dtmp.r_vtx.empty() && !finished){
			if(Dtmp.r_vtx.front()==0){
				nb_zeros++;
				Dtmp.r_vtx.pop_front();		//delete "0"
				Dtmp.r_vtx.pop_front();		//delete "."
			} else {
				finished=true;
			}
		}
		//size of the remaining boundaries
		Dtmp.r_vtx.push_back(sproutsGlobal::End_Region);
		Dtmp.update_representation();
		int n=Dtmp.r_str.size();
		q=powers_of_two(n); //with overflow handling
		//there is q possible choices for the orientation of the boundaries of Dtmp
		for(k=0;k<q;k++) {
		//eps : because Boundary::compute_1bnd_children() is asymmetric
		//so Ci->first and Ci->second must be used in the 2 ways
		for(int eps=0;eps<=1;eps++){
			child.clear();
			
			//orientable surface boundary
			if(eps==0){
				addLine(child, Ci->first);
			} else{
				addLine(child, Ci->second);
			}
			
			//add previously deleted zeros
			for(j=0; j<nb_zeros; j++){
				child.push_back(0);
				child.push_back(sproutsGlobal::End_Boundary);
			}
			
			//copy the partition first block, choosing orientation
			if (n!=0) {
			for(j=0, Bk=Dtmp.r_str.begin(); j<n; j++, Bk++) {
				//we decide here whether j-th boundary has normal or reverse orientation
				if( ((k) & (powers_of_two(j))) == powers_of_two(j)) {
					addLine(child, Bk->r_vtx);
				} else {
					addLine(child, reverseOrientation(Bk->r_vtx));
				}
			}}
			
			add_topo(child, true, i);		//infos on topology (T^i)
			
			//non-orientable surface boundary
			if(eps==1){
				addLine(child, Ci->first);
			} else{
				addLine(child, Ci->second);
			}
			
			addLine(child, Rj->second);		//copy the partition second block
			add_topo(child, false, genus-2*i);	//infos on topology (P^(n-2i))
			
			children.insert(child);			//add the child to the region's list of children
		}}
		}}}
		
		//---Type II.B.1 (c) or II.B.2 (b)
		if( !orientable() ){
		//reverse the "outer boundary"
		L_tmp=reverseOrientation(Ci->second);
		
		for(Rj=possible_partition.begin(); Rj!=possible_partition.end(); Rj++) {
			child.clear();
			
			//Boundaries in which we played
			if(genus%2==0) {
				addLine(child, Ci->first);		//copy the "inner boundary"
				addLine(child, L_tmp);			//copy the "outer boundary" (reversed)
			} else {
				addLine(child, mergeIIB2(Ci->first, Ci->second) );
			}
			
			//other boundaries
			addLine(child, Rj->first);		//copy the partition first block
			addLine(child, reverseOrientation(Rj->second) );	//copy the partition second block with orientation inverted
			
			if(genus%2==0) {
				add_topo(child, true, (genus-2)/2);	//infos on topology
			} else {
				add_topo(child, true, (genus-1)/2);	//infos on topology
			}
			
			children.insert(child);
		}}
		
		//---Type II.B.1 (a)(b)
		if( (orientable() && genus>=1) || (!orientable() && genus>=3) ){
			child.clear();
			
			addLine(child, Ci->first);			//copy the "inner boundary"
			addLine(child, Ci->second);			//copy the "outer boundary"
			for(Bk=unused_bnd.begin();Bk!=unused_bnd.end();Bk++){		//other boundaries
				addLine(child, Bk->r_vtx);
			}
			//infos on topology
			if(orientable()) {
				add_topo(child, orientable(), genus-1);
			} else {
				add_topo(child, orientable(), genus-2);
			}
			children.insert(child);
		}
		
		//---Type II.B.2 (a)
		if( !orientable() && genus>=2 ){
			child.clear();
			
			//Boundary in which we played
			addLine(child, mergeIIB2(Ci->first, Ci->second) );
			
			//other boundaries
			for(Bk=unused_bnd.begin();Bk!=unused_bnd.end();Bk++){
				addLine(child, Bk->r_vtx);
			}
			
			add_topo(child, false, genus-1);	//infos on topology
			children.insert(child);
		}
		
		}
	}
}

//! Children of a region when we link two different boundaries
void Region_spr::compute_2bnd_children() {
  //note: children are added to children
	list<Boundary>::iterator Bi, Bj, Bk;	//boundaries list iterators
	set<Line>::iterator Ci, Cj;		//children set iterators
	
	Line unused_bnd;	//vertex list of unused boundaries
	Line child;		//region's child
	
	//for each boundary, compute children (boundary bribes)
	for(Bi=second_to_last_zero(); Bi!=r_str.end(); Bi++) {
		Bi->compute_2bnd_children();
	}
	
	//double loop on boundaries
	for(Bi=second_to_last_zero(); Bi!=r_str.end(); Bi++) {
	for(Bj=Bi, Bj++  ; Bj!=r_str.end(); Bj++) {
		//--- a) create unused_bnd (vertex list of unused boundaries)
		unused_bnd.clear();
		for(Bk=r_str.begin(); Bk!=r_str.end(); Bk++) {
			if(Bk!=Bi && Bk!=Bj) {
				addLine(unused_bnd, Bk->r_vtx);
			}
		}
		
		//--- b) double loop on Bi and Bj children
		for(Ci=Bi->children.begin(); Ci!=Bi->children.end(); Ci++) {
		for(Cj=Bj->children.begin(); Cj!=Bj->children.end(); Cj++) {
			//-----rename Child_vtx_1 in Child_vtx_2 if necessary--
			// necessary only if both Ci and Cj use the letter Child_vtx_1
			// this letter can appear only at the front of Ci and Cj boundary bribes
			// (case 1 in Boundary::compute_2bnd_children)
			Line L_tmp=*Cj;
			if(!(Cj->empty()) && !(Ci->empty())) {
				if(Cj->front()==sproutsGlobal::Child_vtx_1 && Ci->front()==sproutsGlobal::Child_vtx_1) {
					//WARNING ! it is more reasonable to precise explicitly Vertex type in the replace function as it is necessary in Boundary::distinguish_vertices()
					replace(L_tmp.begin(), L_tmp.end(), (Vertex) sproutsGlobal::Child_vtx_1, (Vertex) sproutsGlobal::Child_vtx_2);
				}
			}
			
			//-----union of two boundary's bribes------------------
			//lim=0 : nothing to do because the child has a vertex of degree 4
			//lim=1 : normal case (occurs both on orientable and non orientable surfaces)
			//lim=2 : optional case, only when the region isn't orientable
			int lim = 1;
			if(! orientable()) lim = 2;
			
			//-----bad child if there's a vertex of degree 4---------
			//(can occur only with positions coming from surfaces)
			//ex : 0.ab.ab.}] and b is linked to itself
			if(!(Cj->empty()) && !(Ci->empty())) {
			//Warning ! Do not forget the "empty" test or you'll regret it... (random evaluation of the following test)
			//Warning 2 ! the following test is responsible of the bug fixed with r1567
				if( is_2reg_vtx(Ci->front()) && is_2reg_vtx(Cj->front()) && (Ci->front()==Cj->front()) ) {
					lim=0;
				}
			}
			
			for(int i=1;i<=lim;i++){
				//-----reverse L_tmp orientation-------------------------
				if(i==2) {	//only for non orientable surfaces
					L_tmp.reverse();
				}
				
				//-----merge old boundaries------------------------------
				child.clear();
				addLine(child, *Ci);				//add first bribe (Ci)
				child.push_back(sproutsGlobal::Child_vtx_3);	//add a new vertex
				addLine(child, L_tmp);				//add second bribe (Cj)
				child.push_back(sproutsGlobal::Child_vtx_3);	//add same new vertex
				child.push_back(sproutsGlobal::End_Boundary);	//add 'end of boundary'
				
				//-----add unused boundaries-----------------------------
				addLine(child, unused_bnd);
				
				//-----topology------------------------------------------
				add_topo(child, orientable(), genusValue() );
				
				//-----add the regions' child to the storage-------------
				children.insert(child);
			}
		}}
	}}
}

//! Create the set of all possible partitions in two blocks.
//! If there are n boundaries, then there are 2^n partitions :
//! - + B1...Bk ; B1 + B2...Bk ; ... ; B2...Bk + B1 ; B1...Bk + -
void Region_spr::partition(const list<Boundary>& l_bnd) {
  //note 1 : result is stored in class member : possible_partition
  //note 2 : we use two different algorithms to treat more efficiently 0. boundaries
	
	possible_partition.clear();
	
	Line r1;	//one possible partition's first block
	Line r2;	//one possible partition's second block
	
	//convention (as in math) : if boundaries list is empty, the only possible partition is (empty, empty)
	if (l_bnd.empty()) {
		possible_partition.insert(couple(r1,r2));
		return;
	}
	
	list<Boundary> l_normal_bnd;   //list of normal boundaries (all but 0.)
	
	set<couple> zero_partition;    //all possible two-blocks partitions of 0. boundaries
	set<couple> normal_partition;  //all possible two-blocks partitions of normal boundaries
	
	unsigned long n, i, j, q;
	list<Boundary>::const_iterator Bj;
	
	//-----step 1 : count and remove (0.) boundaries-------------------------------
	n=0;
	for(Bj=l_bnd.begin(); Bj!=l_bnd.end(); Bj++) {
		if(Bj->is_single_spot()) {     //test if boundary == 0.
			n++;
		} else {
			l_normal_bnd.push_back(*Bj);
		}
	}
	
	//-----step 2 : partitions of 0. boundaries----------------------------
	if(n!=0) { //there is n+1 possible partitions
		for(i=0; i<n; i++) {
			r2.push_back(0);
			r2.push_back(sproutsGlobal::End_Boundary);
		}
		zero_partition.insert( couple(r1,r2) );
		for(i=0; i<n; i++) {
			r1.push_back(0);
			r1.push_back(sproutsGlobal::End_Boundary);
			r2.pop_back();
			r2.pop_back();
			//add this partition to the list of 0. boundaries partitions
			zero_partition.insert( couple(r1,r2) );
		}
	}
	
	//-----step 3 : partitions of normal boundaries------------------------
	n=l_normal_bnd.size();
	
	if (n!=0) {
		q=powers_of_two(n); //with overflow handling
		
		//there is q possible partitions
		for(i=0;i<q;i++) {
			//bits of i "represents" the partition : 0 means first block and 1 means second one
			r1.clear();
			r2.clear();
			
			for(j=0, Bj=l_normal_bnd.begin(); j<n; j++, Bj++) {
				//we decide here whether j-th boundary is in the first or the second block
				//caution, you need to think 1 or 2 seconds to understand the following test
				if( ((i) & (powers_of_two(j))) == powers_of_two(j)) {
					addLine(r1, Bj->r_vtx);
				} else {
					addLine(r2, Bj->r_vtx);
				}
			}
			
			//add partition to the list of normal boundaries partition
			normal_partition.insert( couple(r1,r2) );
		}
	}
	
	//-----step 4 : complete set of partitions-----------------------------
	
	//if there is no "0. boundaries" partition
	if(zero_partition.empty()) {
		possible_partition=normal_partition;
		return;
	}
	
	//if there is no normal boundaries partition
	if(normal_partition.empty()) {
		possible_partition=zero_partition;
		return;
	}
	
	//both normal and zero boundaries exist
	set<couple>::iterator Ci;  //iterator on the set of partitions of zero boundaries
	set<couple>::iterator Cj;  //iterator on the set of partitions of normal boundaries
	
	//we have one complete partition for each couple of partitions of the zero and normal boundaries
	for(Ci=zero_partition.begin(); Ci!=zero_partition.end(); Ci++) {
	for(Cj=normal_partition.begin(); Cj!=normal_partition.end(); Cj++) {
		//r1 = (Ci->first) U (Cj->first)
		quick_clear(r1);
		quick_insert(r1, Ci->first);
		quick_insert(r1, Cj->first);
		quick_end(r1);
		
		//r2 = (Ci->second) U (Cj->second)
		quick_clear(r2);
		quick_insert(r2, Ci->second);
		quick_insert(r2, Cj->second);
		quick_end(r2);
		
		possible_partition.insert( couple(r1,r2) );
	}}
}


//-------------------------------------------------------------------------------------------------
// Children number estimation
//-------------------------------------------------------------------------------------------------

//! Estimates the number of children (by estimating the number of total possible moves)
unsigned int Region_spr::estimate_children_number() {
	update_representation();
	
	unsigned int estimation=0;		//estimation value
	
	list<Boundary>::const_iterator Bi, Bj;
	unsigned int vtx_number;		//number of vertex of a boundary
	unsigned int pw2,pw3;
	int coef;
	
	//topology
	int genus = genusValue();
	
	//----------- number of moves linking two different boundaries : type I ---------
	//for all couple (Bi, Bj) of boundaries,
	//there is (number of vertex of Bi)*(number of vertex of Bj) possible moves
	for(Bi=r_str.begin(); Bi!=r_str.end(); Bi++) {
	for(Bj=Bi, Bj++;      Bj!=r_str.end(); Bj++) {
		coef=1;
		if(!orientable()) {
			coef=2;
		}
		estimation += (Bi->r_vtx.size()-1)*(Bj->r_vtx.size()-1)*coef;
	}}
	
	//---------- number of moves linking a boundary to itself : type II -------------
	if (!r_str.empty()) {
		//note : we don't need to handle overflow when computing powers of two or three
		pw2=powers_estimate(2, r_str.size()-1);
		pw3=powers_estimate(3, r_str.size()-1);
		if(orientable()) {
			coef=pw2*(genus+1);
			if(genus>=1) coef++;
		} else {
			coef=(pw3*(genus+1)+pw2);
			if(genus>=2) coef+=(pw2*(genus-1)+1);
			if(genus>=3) coef++;
		}
		for(Bi=r_str.begin(); Bi!=r_str.end(); Bi++) {
			vtx_number = Bi->r_vtx.size()-1;
			estimation += vtx_number*vtx_number*coef/2;
		}
	}
	
	return estimation;
}

//-------------------------------------------------------------------------
//Vertices distinction
//--------------------------------------------------------------------------

//! Rename 2-region letters in 1-boundary ones or 2-boundary (when needed)
//! note : when this function is called, there is no 1-boundary/2-boundary letters already used
void Region_spr::distinguish_vertices() {
	update_representation();
	change_structure();
	
	//Rename 2-region and temporary vertices in 1-boundary ones (when needed)
	list<Boundary>::iterator Ci;
	for(Ci=r_str.begin(); Ci!=r_str.end(); Ci++) {
		Ci->distinguish_vertices();
	}
	
	update_representation();
	change_vertex();
	
	//Rename 2-region and temporary vertices in 2-boundary ones (when needed)
	Line::iterator s, p, q;
	Vertex next_letter=sproutsGlobal::First_2bnd_vtx;	//first unused 2-boundary Vertex
	p=r_vtx.begin();
	q=r_vtx.end();
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		//test if Vertex is a temp/2-region one + appears two times in the region (->use 2-boundary Vertex in that case)
		if(is_2reg_vtx(*s) || is_temp_vtx(*s)) {
			if(count(p,q,*s)==2) {
				//WARNING ! for an unknown reason we need to precise explicitly Vertex type in the replace function
				replace(p, q, (Vertex) *s, (Vertex) next_letter); //rename
				next_letter++; //update first unused 2-boundary Vertex
			}
		}
	}
	
  //security on the number of used letters
	next_letter--; //last used 2-boundary letter
	if (next_letter>sproutsGlobal::Last_2bnd_vtx) {
		Error(QString("Region_spr::distinguish_vertices : 2-boundary letters capacity overflow"), true);
	}
}

//! Rename 1-region vertices (ie 1-boundary and 2-boundary vertices) in their order of appearance
void Region_spr::rename_1reg_letters() {
	//rename 2-boundary vertices in order
	if (exist_2bnd(r_vtx)) {
		rename_in_order(1);
	}
	
	//rename 1-boundary vertices in order
	if (exist_1bnd(r_vtx)) {
		update_representation();
		change_structure();
		
		list<Boundary>::iterator Ci;
		for(Ci=r_str.begin(); Ci!=r_str.end(); Ci++) {
			if (exist_1bnd(Ci->r_vtx)) {
				Ci->rename_in_order(0);
			}
		}
		
		update_representation();
	}
}
