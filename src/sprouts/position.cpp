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

#include <cmath>
#include "position.h"

//needed for the extended conjecture
#include "../node/nimbernode.h"

bool Position::use_ext_conj = false;

//--------------------------------------------------------------------------
//override terminal_vertex virtual function
//--------------------------------------------------------------------------
Vertex Position::terminal_vertex() {
	return sproutsGlobal::End_Position;
}

//--------------------------------------------------------------------------
//override virtual functions inherited from BaseGame
//--------------------------------------------------------------------------

//register mecanism
Position Position::root(0);

int Position::gameCategory() {
	return GameType::ImpartialGame;
}

//indicate that graphics are not available for the game of Sprouts
bool Position::isPaintable() {
	return false;
}

void Position::resetFrom(const Line &a) {
	reset(a);
}

void Position::resetFrom(const string &a) {
	reset(fromString(a));
}

bool Position::is_terminal_position() {
	return isEmpty();
}

//indicate if any player has won
//not used for Sprouts Game
win_loss Position::Victory() {
	return global::Nim_unknown;
}

string Position::exportToString() {
	return toString(r_vtx);
}

Line Position::exportToLine() {
	return r_vtx;
}

string Position::exportOptionalString() {
	return int_to_string( lives() );
}

//title for the column displaying the optional string
string Position::optionalStringTitle() {	
	return string("Lives");
}

//! Compute the parameters used to decide the priorities between Sprouts position
void Position::computeOrderParameters(MoveOrdering& result) {
	//extended conjecture : priority to positions which are Loss when 6 zeros are deleted 
	//this parameter is 0 for prioritary positions and 1 for normal ones
	if(Position::use_ext_conj && Parameter::computation_kind==NimberNode::Nimber_kind) {
		result.withConjecture = has6zeros(r_vtx);
		if(result.withConjecture) {
			result.conjecturedPos = delete6zeros(r_vtx);
			
			//delete group of 6 zeros until there are less than 6 zeros left
			while(has6zeros(result.conjecturedPos)) {
				result.conjecturedPos = delete6zeros(result.conjecturedPos);
			}
		}
	}
	
	//minimum number of lives
	result.param.push_back(lives());
	
	//maximum number of lands
	//minus sign is used to give priority to maximal values instead of minimal
	int nbLands= - count(r_vtx.begin(), r_vtx.end(), sproutsGlobal::End_Land);
	result.param.push_back(nbLands);
	
	//maximum number of zeros (effect not really clear)
	//int nbZeros= - count(r_vtx.begin(), r_vtx.end(), (Vertex) 0);
	//result.param.push_back(nbZeros);
	
	//minimum number of lives of the smallest land (to have a convergence in RctMisere?)
	//->bad effect?
	//result.param.push_back(-smallestLandLives());
	
	//minimum number of estimated moves
	result.param.push_back(estimate_children_number());
}

int Position::lastCharacter() {
	return sproutsGlobal::End_Position;
}

//--------------------------------------------------------------------------
//canonization of the position
//--------------------------------------------------------------------------
//! Canonize the position representation
void Position::canonize() {
	list<Land>::iterator Li;
	
	//--------- step 1 : reduce lands ----------
	update_representation();
	change_structure();
	for(Li=r_str.begin(); Li!=r_str.end(); Li++) {
		Li->reduce();
	}
	
	//--------- step 2 : try to split lands into independent new ones ----------
	bool split_occured=false;
	for(Li=r_str.begin(); Li!=r_str.end(); Li++) {
		if(Li->split()) {
			split_occured = true;
		}
	}
	
	if(split_occured) {
		//important note : when split occurs, we store several lands in the initial Land object
		//we first compute position's vertex representation
		//then we compute the "real" structure representation (one Land object for each land)
		change_structure();        //indicate that the structure representation has changed
		update_representation();   //update vertex list
		change_vertex();           //pretend vertex list has changed
		update_representation();   //update structure list
	}
	
	//---------- step 3 : canonize lands ----------
	change_structure();
	Li=r_str.begin();
	while(Li!=r_str.end()) {
		//canonization
		Li->canonize();
		
		//delete the land if dead (r_vtx : "]")
		if(Li->r_vtx.size()==1){
			Li=r_str.erase(Li);
		} else {
			Li++;
		}
	}
	
	//lexicographical sort of land
	r_str.sort();
	update_representation();
}

//! Return the list of lands considered as positions
//! @return  a list of sprouts positions
list<Line> Position::sumComponents() {
	list<Line> result;	//Line=vertex representation
	Line land_tmp;
	Line::const_iterator s;
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		if (*s==sproutsGlobal::End_Position) {
			//this character is just ignored
		} else if(*s==sproutsGlobal::End_Land) {
			//add 'end of land' and 'end of position' character
			land_tmp.push_back(sproutsGlobal::End_Land);
			land_tmp.push_back(sproutsGlobal::End_Position);
			
			//add the land to the result list
			result.push_back(land_tmp);
			land_tmp.clear();
		} else {
			//add one character to the current land's vertex list
			land_tmp.push_back(*s);
		}
	}
	return result;
}

//! Children of the position
//! Result is saved in the class member "children"
//! this function is similar to Land::children
void Position::compute_children() {
	update_representation();
	children.clear();
	
	//specific code - compared to Land::compute_children()
	list<Land>::iterator it;
	list<Land>::iterator it2;
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
		it->prepare_children_computation();
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

//first draft of base game
//we simply call compute_children, but in a second step, computeChildrenSet will be deleted
void Position::computeChildrenSet() {
	compute_children();
	childrenSet() = children;
}

//! Return the type of position
//! It depends on the number of land substructures
char Position::position_type() {
	//counts the number of land
	int nb_land=count(r_vtx.begin(), r_vtx.end(), sproutsGlobal::End_Land);
	if(nb_land==0) {
		return sproutsGlobal::Empty_position;
	} else if(nb_land==1) {
		return sproutsGlobal::Single_land;
	} else {
		return sproutsGlobal::Multi_lands;
	}
}

bool Position::isSingleLand() {
	return (position_type() == sproutsGlobal::Single_land);
}

bool Position::isMultiComponents() {
	return (position_type() == sproutsGlobal::Multi_lands);
}

bool Position::isEmpty() {
	return (position_type() == sproutsGlobal::Empty_position);
}

//! Number of lives of the smallest land
unsigned int Position::smallestLandLives() {
	update_representation();
	
	unsigned int minLives=0;
	if(!r_str.empty()) minLives = r_str.begin()->lives(); //number of lives of the first land
	
	list<Land>::iterator Li;
	for(Li=r_str.begin(); Li!=r_str.end(); Li++) {
		unsigned int n = Li->lives();
		if(n<minLives) minLives = n;
	}
	
	return minLives;
}

//! Estimate the number of possible children of the position
unsigned int Position::estimate_children_number() {
	update_representation();
	
	unsigned int estimation=0;
	list<Land>::iterator Li;
	
	//sum of estimation in each land
	for(Li=r_str.begin(); Li!=r_str.end(); Li++) {
		estimation += Li->estimate_children_number();
	}
	
	return estimation;
}

//! Heuristic for the traversal of the tree (expermiental code)
// return a value representing the "difficulty" of a position
// We can try different heuristics here : 2^(nb_lives/6) ; nb_lives^2 ; nb_lives...
double Position::traversalHeuristic() {
	//use only smallest land (maybe we'll change this later)
	return pow(smallestLandLives(), 1.5);
}

//! Sprouts start position with "nb_vtx" initial spots and topology "topo"
Line Position::startPosition(int nb_vtx, int topo) {
	Line l_ini;
	int i;
	for (i=0; i<nb_vtx; i++) {
		l_ini.push_back(0);
		l_ini.push_back(sproutsGlobal::End_Boundary);
	}
	if(topo!=0){
		l_ini.push_back(sproutsGlobal::Topo_Region);
		l_ini.push_back(sproutsGlobal::Sphere+topo);
	}
	l_ini.push_back(sproutsGlobal::End_Region);
	l_ini.push_back(sproutsGlobal::End_Land);
	l_ini.push_back(sproutsGlobal::End_Position);
	return l_ini;
}

bool Position::isGivenPosition=false;
string Position::givenPosition;
int Position::startingSpots, Position::startingTopology;

//! Define the interface for parameters of Sprouts game
Interface Position::getParamDef() {
	Interface result;
	result.name="Sprouts";
	result.addRadioButton("Classic empty position", true, /*pos*/ 0, 0);
	result.addLabel("Number of spots :", /*pos*/ 0, 1);
	result.addSpinBox(2, 99, 12, /*pos*/ 0, 2);
	result.link(Position::startingSpots, "StartingSpots");
	result.addLabel("Topology :", /*pos*/ 2, 1);
	result.addSpinBox(-99, 99, 0, /*pos*/ 2, 2);
	result.link(Position::startingTopology, "Topology");
	result.addRadioButton("Given start position", false, /*pos*/ 3, 0);
	result.link(Position::isGivenPosition, "isGivenPosition");
	result.addLabel("Vertex representation :", /*pos*/ 3, 1);
	result.addLineEdit("0*3+0.A|0.A", /*pos*/ 3, 2, 1, 1);
	result.link(Position::givenPosition, "givenPosition");
	result.addCheckButton("Use extended conjecture", false, /*pos*/ 4, 0);
	result.link(Position::use_ext_conj, "useExtendedConjecture");
	#ifdef UNSUPPORTED_FEATURE
	result.addLabel("Life limit of CRT switch :", /*pos*/ 5, 0);
	result.addSpinBox(0, 50, 0, /*pos*/ 5, 1);
	result.link(Parameter::lifeLim, "RctLifeLimit");
	#endif
	return result;
}

//! Read the parameters set by the user in the interface
void Position::setParam() {
	Parameter::usePositionString = false;
	if(Position::isGivenPosition) {
		Parameter::given_position=fromString( Position::givenPosition );
	} else {
		//classic position with "0." spots
		Parameter::given_position=Position::startPosition(Position::startingSpots, Position::startingTopology);
	}
}

//! Name of the start position
string Position::getPositionName() {
	string name;	
	if(Position::isGivenPosition) {
		//free start position
		name+=Position::givenPosition;
	} else {
		name+=(QString("p=%1").arg(Position::startingSpots)).toStdString();
		//topology
		if(Position::startingTopology==1){
			name+=" Torus";
		} else
		if(Position::startingTopology==-2){
			name+=" Klein";
		} else
		if(Position::startingTopology!=0){
			name+=(QString(" / topo=%1").arg(Position::startingTopology)).toStdString();
		}
	}
	return name;
}
