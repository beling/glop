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

#include "boundary.h"

//Override of terminal_vertex virtual function
Vertex Boundary::terminal_vertex() {
	return sproutsGlobal::End_Boundary;
}

//! A boundary has a different representation for each possible circular rotations.
//! We find the best boundary representation for lexicographical order by successive rotations.
//! The boundary keeps the same orientation (it isn't reversed).
//! e.g. a2a1A. ---> 1Aa2a.
//! @return r_vtx is updated with the best representation
void Boundary::sort() {
	int length=r_vtx.size();	//boundary length
	if (length<3) return;		//sort is useless when boundary is shorter than 3 characters (at least "xy." is needed)
	
	r_vtx.pop_back();	//delete 'end of boundary' character (rotation is easier)
	Line rotations=r_vtx;	//temporary object containing the boundary's rotations
	
	//iterators used by rotate function
	Line::iterator p, q, r;
	p=rotations.begin();
	r=rotations.begin(); r++; //rotate character by character on the left
	q=rotations.end();
	
	//look for the best rotation (r_vtx.size()-1 possible rotations, the last one is unnecessary)
	//r_vtx is always equal to the best rotation found
	int i;
	for(i=1;i<length-1;i++) { 
		rotate(p,r,q);		//rotate one character
		if (lineOrder(rotations,r_vtx)) r_vtx=rotations; //the new rotation is better
	}
	
	r_vtx.push_back(sproutsGlobal::End_Boundary); //add back the 'end of boundary' character
}

//! Reverse the boundary's orientation
//! e.g. 1Aa2a. ---> a2aA1.
//! @return r_vtx is reversed
void Boundary::reverse_orientation() {
	r_vtx.pop_back();		//delete 'end of boundary' character
	r_vtx.reverse();		//reverse orientation with STL reverse algorithm
	r_vtx.push_back(sproutsGlobal::End_Boundary);	//add back 'end of boundary' character
}

//! Find the best representation of the boundary, and take into account the two possible orientations.
//! This algorithm is used only for non orientable surfaces.
//! e.g. a2a1A. ---> 1a2aA.
//! @return r_vtx is updated with the smallest couple rotation/orientation
void Boundary::sort_unoriented(){
	sort();
	Line sav=r_vtx;
	reverse_orientation();
	sort();
	if (lineOrder(sav,r_vtx)) r_vtx=sav; //keep the best orientation
}

//! Indicate if the boundary is a single unused spot
//! @return true if the boundary is "0."
bool Boundary::is_single_spot() const {
	if (r_vtx.size()==2) {
		if (r_vtx.front()==0) return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
// Boundary's children computation
//-------------------------------------------------------------------------------------------------

//! 1.Boundary's children when linked to itself.
//! This function implements rules of linking (valid moves or not).
//! A child is a pair of boundaries : one in the inner region and one in the outer region.
//! If boundary = "a1 ... ar .", it returns the couples : ( aj ... ar a1 ... ai # . ; ai ... aj # . ).
//! Important note : this function is asymmetric, it can return the couple (B1;B2) and not the couple (B2;B1)
//! @return the set of children is stored in children_couple
void Boundary::compute_1bnd_children() {
  //note : boundary should not be empty (because of canonization)
  //we treat differently the single vertex boundaries (0. 1. or  2.) and the other ones
	children_couple.clear();	//children set
	
	if (r_vtx.size()<2) {
		Error(QString("Boundary::compute_1bnd_children : Boundary is empty"), false);
		return;
	}
	
	Line bnd;		//temporary boundary object for successive rotations
	couple couple_bnd;	//pair of boundaries : inner+outer
	
	//----------Boundary is a single vertex----------------------------------------------------
	if (r_vtx.size()==2) {
		//only possible move : link the single vertex to itself
		//which is a valid move only if vertex degree is 0 or 1
		Vertex v=r_vtx.front();
		if (v==0 || v==1) {
		//v==0 => couple = ( -#. ; -#. )
		//v==1 => couple = ( #. ; #. )
			//boundary in the outer region
			if(v==0) {bnd.push_back(sproutsGlobal::Child_vtx_1);}		//vertex "0" is renamed "Child_vtx_1"
			if(v==1) {bnd.push_back(3);}					//vertex "1" is renamed "3"
			bnd.push_back(sproutsGlobal::Child_vtx_3);			//creation of a new vertex
			bnd.push_back(sproutsGlobal::End_Boundary);			//'end of boundary'
			couple_bnd.first=bnd;					//store result
			//boundary in the inner region : the same
			couple_bnd.second=bnd;					//store result
			
			children_couple.insert(couple_bnd);
		}
		return;
	}
	
	//----------Boundary has more than one vertex----------------------------------------------
	r_vtx.pop_back();	//delete 'end of boundary' character (we'll push it back later)
	
	Line::iterator s1, s2;	//vertex iterator
	Vertex v1, v2;	//vertex value
	
	//double loop that try to link all possible couple of vertices in the boundary
	for(s1=r_vtx.begin(); s1!=r_vtx.end(); s1++) {
	for(s2=s1; s2!=r_vtx.end(); s2++) {
		//test if move is possible
		//forbidden links : a vertex "2" to itself or a letter to the same letter
		if( !(s1==s2 && *s1==2) && !(*s1==*s2 && *s1>2) ) {
			v1=*s1; v2=*s2;	//save vertices name
			
			//-----give a real name to generic vertices------------------------------
			//we need to give generic vertices a real name when they are used in a link
			if(s1!=s2) {
				//link two different vertices
				if(v1==1) {*s1=sproutsGlobal::Child_vtx_1;}	//a vertex of degree 1 becomes a letter
				if(v2==1) {*s2=sproutsGlobal::Child_vtx_2;}	//we need different letters for s1 and s2
				//vertices of degree 2 -> degree 3 (they'll be erased on canonization step)
				if(v1==2) {*s1=3;}
				if(v2==2) {*s2=3;}
			} else {
				//link a vertex to itself (possible only if its degree is 1)
				//vertex of degree 1 -> degree 3
				if(v1==1) {*s1=3;}
			}
			
			//-----boundary's child in the outer region------------------------------
			//aj ... ar a1 ... ai # .
			quick_clear(couple_bnd.first);
			quick_insert(couple_bnd.first, s2, r_vtx.end());
			quick_insert(couple_bnd.first, r_vtx.begin(), s1);	//caution : s1 is not inserted yet
			quick_end(couple_bnd.first);
			couple_bnd.first.push_back(*s1);
			couple_bnd.first.push_back(sproutsGlobal::Child_vtx_3);		//add a new vertex (with an available name)
			couple_bnd.first.push_back(sproutsGlobal::End_Boundary);		//end of boundary
			
			//-----boundary's child in the inner region------------------------------
			//ai ... aj # .
			quick_clear(couple_bnd.second);
			quick_insert(couple_bnd.second, s1, s2);		//caution : s2 is not inserted yet
			quick_end(couple_bnd.second);
			couple_bnd.second.push_back(*s2);			//add s2 in all cases, we don't care that s1!=s2 or s1==s2
			couple_bnd.second.push_back(sproutsGlobal::Child_vtx_3);	//add a new vertex
			couple_bnd.second.push_back(sproutsGlobal::End_Boundary);
			
			//-----add the pair (outer-inner) to the list of children----------------
			children_couple.insert(couple_bnd);
			
			*s1=v1; *s2=v2;	//give back to vertices their correct names
		}
	}}
	
	r_vtx.push_back(sproutsGlobal::End_Boundary);	//put back the 'end of boundary' character
}

//! 2.Boundary's children when linked to another boundary.
//! There's no linking rule in this function (rules are enforced in Region_spr::compute_2bnd_children).
//! Algorithm consists mainly in rotation (complete theory on separate papers)
//! @return the set of children is stored in children (inherited from Representation)
void Boundary::compute_2bnd_children() {
  //note 1: boundary should not be empty (canonization should have been used)
  //note 2: children are not boundaries but boundary parts, ready to be linked with other boundary parts
  //        this is why children don't have any 'end of boundary' character
	children.clear();
	
	if (r_vtx.size()<=1) {
		Error(QString("Boundary::compute_2bnd_children : Boundary is empty"), false);
		return;
	}
	
	//----------compute children-----------------------------------------------------
	Line bnd=r_vtx;		//temporary object for successive rotations
	bnd.pop_back();		//delete 'end of boundary' character
	Line::iterator p, q, r;
	Vertex v;		//first character of a particular rotation
	int i;
	int length=bnd.size();	//there are "length" different children
	
	for(i=0; i<length; i++) {
		v=bnd.front();
		//in case of a generic vertex, we need to rename it
		switch(v) {
		case 0:
			//0->1
			bnd.pop_front();
			bnd.push_front(1);
			children.insert(bnd);
			bnd.pop_front();
			bnd.push_front(0);
			break;
		case 1:
			//1->Child_vtx_1(reserved name)
			//Child_vtx_1 is renamed in Child_vtx_2 in Region_spr::compute_2bnd_children when necessary
			//e.g. 1A2B -> -A2B-
			bnd.pop_front();
			bnd.push_front(sproutsGlobal::Child_vtx_1);
			bnd.push_back(sproutsGlobal::Child_vtx_1);
			children.insert(bnd);
			bnd.pop_front();
			bnd.pop_back();
			bnd.push_front(1);	//add back the deleted 1
			break;
		case 2:
			//2->3
			//e.g. 2CBA -> 3CBA
			//---Warning : bug fixed with r1567 (can occur only with positions coming from surfaces)
			//---we push a "3" because of the problem of a Vertex of degree 4 in Region_spr::compute_2bnd_children()
			bnd.pop_front();
			bnd.push_front(3);
			children.insert(bnd);
			bnd.pop_front();
			bnd.push_front(2);
			break;
		default:
			//in case of a "normal" vertex we add the vertex at the end of boundary
			//it will disappear with its other occurrences at the next canonization
			//e.g. A2BC -> A2BCA
			bnd.push_back(v);
			children.insert(bnd);	//add the obtained boundary to the children list
			bnd.pop_back();			//delete from the boundary the added vertex
		}
		
		//rotation of bnd object
		p=bnd.begin();
		r=bnd.begin(); r++; //rotation of one vertex only to the left
		q=bnd.end();
		rotate(p,r,q);
	}
}


//-------------------------------------------------------------------------
// vertices distinction :
// these functions renames appropriate 2-region vertices in 1-boundary
// (appears two times in the same boundary)
//--------------------------------------------------------------------------

//! Rename 2-region and temporary vertices in 1-boundary ones (when needed)
//! note : when this function is called, there is no 1-boundary vertex already used
void Boundary::distinguish_vertices() {
	Line::iterator s, p, q;
	Vertex next_letter=sproutsGlobal::First_1bnd_vtx;	//first unused 1-boundary Vertex
	p=r_vtx.begin();
	q=r_vtx.end();
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		//test if Vertex is a temp/2-region one + appears two times in the boundary (->use 1-boundary vertex in that case)
		if(is_2reg_vtx(*s) || is_temp_vtx(*s)) {
			if(count(p,q,*s)==2) {
				//WARNING ! for an unknown reason we need to precise explicitly Vertex type in the replace function
				replace(p, q, (Vertex) *s, (Vertex) next_letter); //rename
				next_letter++; //update first unused 1-boundary Vertex
			}
		}
	}
	
	//security on the number of used letters
	next_letter--; //last used 1-boundary letter
	if (next_letter>sproutsGlobal::Last_1bnd_vtx)
		Error(QString("Boundary::distinguish_vertices : 1-boundary letters capacity overflow"), true);
}
