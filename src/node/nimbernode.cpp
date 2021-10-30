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
#include "nimbernode.h"

#include "../computation/database.h"
#include "../baseclass/basegame.h"

int NimberNode::given_nimber = 0;
bool NimberNode::computeOutcome = true;
int NimberNode::dbNimberIndex = 0;

const int NimberNode::Nimber_kind = 0;
const int NimberNode::MultiComponents = 1;
const int NimberNode::Nimber_012 = 2; 

const nimber NimberNode::Nim_max=250; //maximal value of a valid nimber

//register mecanism
NimberNode NimberNode::root(0);

//game type that can be computed with this node
GameType NimberNode::gameType() {
	GameType t;
	t.add(GameType::ImpartialGame);
	return t;
}

void NimberNode::createDataStorage() {
	dbNimberIndex = db::create(/*stringnimber*/ 0, QString( "Nimber"), QString( "nimber"), QString("[Positions+Nimber]"), NimberNode::registeredIndex());
}

int NimberNode::startComputationKind() {
	if(computeOutcome) {
		return Nimber_kind; //the user has chosen to compute the outcome
	} else {
		return Nimber_012;  //the user has chosen to compute the nimber
	}
}

//returns the nimber resulting of a list of nimber, using "exclusive or"
//a complete game nimber is the xor of the nimber of its independent subgames
nimber NimberNode::nimber_xor(list<nimber>& l_nimber) {
	nimber result=0;	//identity element for "exclusive or"
	list<nimber>::const_iterator Ni;
	
	for(Ni=l_nimber.begin(); Ni!=l_nimber.end(); Ni++) {
		result ^= *Ni;	//exclusive or (c++ operator)
		
		if(result>Nim_max){
			Error(QString("Too big nimber in NimberNode::nimber_xor"), true);
		}
	}
	
	return result;
}

bool NimberNode::tryImmediateResult() {
	apply_known_nimbers();	//replace already known components by their nimber
	
	winLossResult = global::Nim_unknown;
	
	BaseGame g(positionA);
	if(g.is_terminal_position()) {
		if(nimberA==0){
			winLossResult=global::Loss;
		}
		else{	//case nimber>=1
			winLossResult=global::Win;
		}
	}
	
	//for Games where the result is known before the end of the game
	win_loss r = g.Victory();
	if(r != global::Nim_unknown) {
		if(r == global::Loss) {
			if(nimberA==0){
				winLossResult=global::Loss;
				
				//add this result to the database
				//(TODO : maybe useless ? )
				db::add(positionA, nimberA, dbNimberIndex, Parameter::isCheck, /*withDefaultCheck*/ true);			
			}
			else{	//case nimber>=1
				winLossResult=global::Win;
			}
		} else
		if(r == global::Win && nimberA==0) {
			winLossResult=global::Win;
		}
	}
	
	return (winLossResult != global::Nim_unknown);
}

//Simplify the position with known nimbers
//TODO : use PositionString directly in sumComponents
void NimberNode::apply_known_nimbers() {
	//nothing to do if position part is empty
	BaseGame g(positionA);
	if(g.is_terminal_position()) return;
	int lastCharacter = g.lastCharacter();
	
	//list of components (given by their vertex representation)
	list<Line> components=g.sumComponents();
	
	// ------------- look for known components in the database ----------------
	list<nimber> known_nimbers; //list of nimbers of the known components
	Line unknown_components;  //vertex representation with only the unknown components
	
	bool found;
	list<Line>::iterator Li;
	
	//for each component, look for its nimber in the database
	for(Li=components.begin(); Li!=components.end(); Li++) {
		found = db::find(gameString(*Li), dbNimberIndex, Parameter::isCheck);
		if(found) {
			//known component
			known_nimbers.push_back(db::foundNimber());
		} else {
			//unknown component
			unknown_components.insert(unknown_components.end(), Li->begin(), Li->end());
			
			if(lastCharacter!=0) {
				unknown_components.pop_back(); //delete position's end character (End_Position)
			}
		}
	}
	
	if(lastCharacter!=0) {
		unknown_components.push_back(lastCharacter);
	}
	
	//convert into PositionString
	BaseGame g2(unknown_components);
	positionA=g2.exportToString();
	
	if(!known_nimbers.empty()) {
		known_nimbers.push_back(nimberA);	//add nimber part to the list of nimbers
		nimberA=nimber_xor(known_nimbers);	//compute resulting nimber with xor
	}
}

bool NimberNode::resultFromOneChild(BaseNode &child) {
	if(computation_kind == MultiComponents) {
		//we need to compute the nimber of only one component
		return true;
	}
	
	//-----case of a losing child -----
	if(access(child)->winLossResult==global::Loss)  {
		//One losing child means parent is a winning position
		winLossResult = global::Win;
		return true; //a local result was found, end of computation. 
	}
	
	return false;
}

void NimberNode::resultFromAllChildren(list<BaseNode> & /*children*/) {
	//if we get there, it means all children were winning ones, so the node is a losing position
	//the position part's nimber is then equal to the nimber part - we add this result in database
	winLossResult = global::Loss;
	
	db::add(positionA, nimberA, dbNimberIndex, Parameter::isCheck, /*withDefaultCheck*/ true);
}

bool NimberNode::resultIsComplete() {
	if(computation_kind == MultiComponents) {
		//for the next step, use a normal algorithm
		computation_kind=Nimber_kind;
		return false;
	} else
	if(computation_kind == Nimber_012) {
		//in case of a Nimber_012 computation, the computation ends when we have found a loss
		//or when the position part is empty (case of an already known position)
		BaseGame g(positionA);
		if(winLossResult == global::Loss || g.is_terminal_position()) {
			return true;
		} else {
			//computation is not finished, we need to increment nimber part before trying a new step
			nimberA++;
			if(nimberA>Nim_max){
				Error(QString("Too big nimber in Node::resultIsComplete"), true);
			}
			return false;
		}
	} else {
		return true;
	}
}

bool NimberNode::computeNodeChildren(list<BaseNode> &children) {
	BaseGame g(positionA);
	if( g.isMultiComponents() ) {
		//change the kind of computation
		computation_kind=MultiComponents;
		
		//the children of the position are the independent components
		list<Line> components=g.sumComponents();
		list<Line>::iterator Li;
		for(Li = components.begin(); Li != components.end(); Li++) {
			BaseNode child(NimberNode::registeredIndex(), Nimber_012);
			//convert into PositionString
			BaseGame g2(*Li);
			access(child)->positionA = g2.exportToString();
			access(child)->nimberA = 0; //the first nimber to be tried will be 0
			children.push_back(child);
		}
		
		//sort children WITHOUT removing duplicates
		sortChildren(children, /*unique*/ false);
		
		return false;  //no immediate result was found
	}
	
	win_loss result = compute_children_Nimber(children);
	if( result != global::Nim_unknown) {
		winLossResult = result;
		return true;
	}
	
	//sort children and remove duplicates
	sortChildren(children, /*unique*/ true);
	
	return false;
}


//Compute the children of the position and return Win if any of them is a Loss
win_loss NimberNode::compute_children_Nimber(list<BaseNode> &children) {
	//------- move in the nimber part -------------
	int i;
	for(i=0; i<nimberA; i++) {
		BaseNode child(NimberNode::registeredIndex(), Nimber_kind);
		access(child)->positionA = positionA;	//position part is unchanged
		access(child)->nimberA = i;
		children.push_back(child);
	}
	
	//------- move in the position part -------------
	//TODO : passage à computeOptionsSet
	BaseGame g(positionA);
	g.computeChildrenSet();

	//test
	//BaseGame g2(positionA);
	//g2.computeOptionsSet();
	
	//cout << g.childrenSet().size() << " " << g2.optionsSet().size() << endl;
	
	set<Line>::iterator it;
	set<PositionString>::iterator it2;
	for(it=g.childrenSet().begin()/*, it2=g2.optionsSet().begin()*/; it!=g.childrenSet().end()/* && it2!=g2.optionsSet().end()*/; it++/*, it2++*/) {
		BaseNode child(NimberNode::registeredIndex(), Nimber_kind);
		
		BaseGame h(*it);
		//cout << h.exportToString() << " " << *it2 << endl;
		
		h.canonize();
		access(child)->positionA = h.exportToString();
		access(child)->nimberA = nimberA;
		
		child.tryImmediateResult();
		if(access(child)->winLossResult == global::Loss) return global::Win; //returns "Win" if any child is "Loss"
		if(access(child)->winLossResult != global::Win) {	//Winning children are useless
			children.push_back(child);
		}
	}
	
	return global::Nim_unknown;
}

void NimberNode::computeOrderParameters() {
	//compute the parameters of the game
	//the first parameter should be something indicating the complexity of the position
	BaseGame g(positionA);
	g.computeOrderParameters(nodeOrder);
	
	//change the first parameter so that it becomes a "pseudo lives" value
	if(!nodeOrder.param.empty()) {
		//number of lives + coefficient * nimber
		*(nodeOrder.param.begin()) = nodeOrder.param.front() + 4 * nimberA;
	}
	
	//add the result of the conjecture in the front of the list
	//no conjecture = 1, loss conjecture= 0 (compute first), win conjecture = 2 (compute last)
	int priority = 1;
	if(nodeOrder.withConjecture) {
		NimberNode n;
		//convert into PositionString
		//TODO : conjecturedPos should be itself a PositionString
		BaseGame g2(nodeOrder.conjecturedPos);
		n.positionA = g2.exportToString();
		n.nimberA = nimberA;	//use the same nimber part as the current node
		if(n.tryImmediateResult()) {
			if(n.winLossResult==global::Loss) {
				priority = 0;
			} else
			if(n.winLossResult==global::Win) {
				priority = 2;
			}
		}
	}
	nodeOrder.param.push_front(priority);
	
	//last parameter : lexicographic order on the string representation
	//WARNING : string doesn't have the same order as Line representation (used before)
	//TODO : there should be the nimber in the string representation
	nodeOrder.pos = g.exportToString();
	
	//check value : this parameter is used to guide the check computation with a previous database
	//note about the check of a sum of components :
	//   if the posiion is the component of a sum, we need to find if it s nimber is known
	//   the nimber is known if and only if tryImmediateResult returns something different from "unknown" (i.e. win or loss)
	//   for this reason, it is important to distinguish "win" and "unknown" in the randomCheck function
	if(Parameter::isCheck) {
		//search in the normal database if we already know something about the position
		Parameter::isCheck = false;				//in order to use the normal database
		NimberNode a = *this;					//we need a copy to avoid any change of the current node
		a.winLossResult = global::Nim_unknown;
		a.tryImmediateResult();
		Parameter::isCheck = true;
		
		// loss > win > unknown with a random order in each of these three categories (if random check is used)
		nodeOrder.checkValue = global::randomCheck(a.winLossResult);
	}
}

Interface NimberNode::getParamDef() {
	Interface result;
	result.name="Nimber";
	result.addDataBaseButton(dbNimberIndex, /*pos*/ 0, 0);
	result.linkDataBase("NimberBase");
	result.addDataBaseButton(dbNimberIndex + 1, /*pos*/ 0, 1);
	result.linkDataBase("NimberCheck");
	
	result.addLabel("Start Nimber part :", /*pos*/ 1, 0);
	result.addSpinBox(0, 99, 0, /*pos*/ 1, 1);
	result.link(given_nimber, "startNimberPart");

	result.addCheckButton("Activate check", false, /*pos*/ 0, 2);
	result.link(Parameter::isCheck, "isCheck");
	result.addLabel("Random check cycles :", /*pos*/ 1, 2);	
	result.addSpinBox(0, 99, 0, /*pos*/ 1, 3);
	result.link(Parameter::RandomCheckCycles, "RandomCheckCycles");
	
	result.addLabel("Compute :", /*pos*/ 2, 0);
	result.addRadioButton("outcome", true, /*pos*/ 2, 1);
	result.link(NimberNode::computeOutcome, "computeOutcome");
	result.addRadioButton("nimber", false, /*pos*/ 2, 2);
	
	return result;
}

//initialize the first node of a computation with the correct parameters
void NimberNode::initRootNode() {
	BaseGame g(Parameter::given_position);
	positionA=g.exportToString();
	if(NimberNode::computeOutcome) {
		nimberA= (nimber) given_nimber;
	} else {
		//if the computation of the nimber is asked, ignore the start nimber part
		nimberA = 0;
	}
}

//name of the start position set by the user in the interface
string NimberNode::getPositionName() {
	string result = BaseGame(Parameter::gameChoice).getPositionName();					//position name of the game
	//we ignore the start nimber part in case of a nimber computation
	if(NimberNode::computeOutcome && given_nimber != 0) {
		result += (QString(" / nim-part=%1").arg(given_nimber)).toStdString();	//add the nimber part
	}
	return result;
}

string NimberNode::resultString() {
	string result;
	if(computeOutcome) {
		result += "outcome=";
		result += global::toString(winLossResult);
	} else {
		result = QString("nimber=%1").arg((int) nimberA).toStdString();
	}
	return result;
}

QStringList NimberNode::displayStringList() {
	QStringList result;
	
	//position + nimber
	int nimVal = (int) nimberA;
	BaseGame g(positionA);
	result += QString::fromStdString(g.exportOptionalString());
	result += QString::fromStdString(positionA);
	result += QString( " %1" ).arg(nimVal, 2, 10 );
	return result;
}

//title for the column with the part of the node that is not the position
string NimberNode::extraPositionPartTitle() {
	return string("Nimber");
}

QString NimberNode::displayColor() {
	if(computation_kind == Nimber_012){
		return QString("yellow");
	} else {
		return QString("transparent");
	}
}

//info about a position given by its string
//the first string is the displayed info, the second (optional) one is the color
QStringList NimberNode::infoAbout(string posString) {
	QStringList info;
	NimberNode node_tmp;
	node_tmp.positionA = posString;
	node_tmp.nimberA = 0;
	node_tmp.apply_known_nimbers();
	
	BaseGame g(node_tmp.positionA);
	if(g.is_terminal_position()){	//every land is known, empty position : !
		info += QString("%1").arg(node_tmp.nimberA,2,10);	//displayed text
		if(node_tmp.nimberA==0) info += QString("red");		//displayed color
	}
	
	return info;
}

string NimberNode::debugStringKind() {
	string result;
	if(computation_kind == Nimber_kind) {
		result = string("Def");
	} else
	if(computation_kind == Nimber_012) {
		result = string("Nim");
	} else
	if(computation_kind == MultiComponents) {
		result = string("Cpn");
	}
	return result;
}

string NimberNode::debugStringPos() {
	string result= positionA + string(" ");
	result += QString( "%1 " ).arg((int) nimberA, 2, 10 ).toStdString();
	return result;
}

string NimberNode::debugStringResult() {
	if(winLossResult == global::Loss) {
		return string("L");
	} else {
		return string("G");
	}
}

//! Heuristic for the traversal of the tree (experimental code)
// return a value reprensenting the "difficulty" of the node
// (currently) 2^(nimber)*difficulty(position)
unsigned int NimberNode::traversalHeuristic() {
	//heuristic of the position
	BaseGame g(positionA);
	double heuristic_pos=g.traversalHeuristic();
	
	//correction taking into account the nimber value
	double a = nimberA;
	double nim_factor = pow(2, a);
	
	double heuristic = nim_factor * heuristic_pos;
	
	unsigned int total = floor(heuristic);
	if(total==0) total = 1;
	return total;
}

//used to move the check database in the normal database during check cycles
void NimberNode::prepareNextCycle() {
	cout << db::storeVect[dbNimberIndex].size() << " -> " << db::storeVect[dbNimberIndex+1].size() << endl;
	db::move(dbNimberIndex+1, dbNimberIndex);
}
