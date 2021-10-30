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

#include "rctmiserenode.h"
#include "fulltreenode.h"

#include "../computation/database.h"
#include "../baseclass/basegame.h"

//--------------------------------------------------PosRct-----------------------------------
//PosRct ---> string
string PosRctToString(const PosRct& PosRctA) {
	StringConverter conv;
	conv << gameString(PosRctA.pos); // position
	conv << CvCd("B10") << string(" ") << PosRctA.nimCol << string(" "); //Nim-column 0/1
	conv << CvCd("LF") /*no space after last element*/ << PosRctA.RctSet; //multiset of Rct
	return conv.getString();
}

//string ---> PosRct
PosRct PosRctFromString(const string& stringA) {
	PosRct result;
	StringConverter conv(stringA);
	conv >> CvCd("E "); //separator between elements is the space character
	string a = conv.getFirstToken();
	result.pos = BaseGame(a).exportToLine(); 
	
	conv >> CvCd("B10") >> result.nimCol;
	conv >> CvCd("E%") /*to avoid cutting at '-' */ >> result.RctSet;
	return result;
}

//PosRct, RctPart only ---> string (used only in display)
string RctPartToString(const PosRct& PosRctA) {
	StringConverter conv;
	conv << CvCd("B10") << string(" ") << PosRctA.nimCol << string(" "); //Nim-column 0/1
	conv << CvCd("LF") /*no space after last element*/ << PosRctA.RctSet; //multiset of Rct
	return conv.getString();
}

//--------------------------------------------------RctMisereNode-----------------------------------
int RctMisereNode::dbRctMisereIndex = 0;

const int RctMisereNode::Misere_complex=0;
const int RctMisereNode::RctInMisere=1;
const int RctMisereNode::GrundyValue=2;

bool RctMisereNode::computeOutcome = true;
nimber RctMisereNode::currentGrundyValue=0; //used only when computing the grundy-value

//register mecanism
RctMisereNode RctMisereNode::root(0);

//game type that can be computed with this node
GameType RctMisereNode::gameType() {
	GameType t;
	t.add(GameType::ImpartialGame);
	return t;
}

void RctMisereNode::createDataStorage() {
	Rct::createDataStorage();
	dbRctMisereIndex = db::create(/*string*/ 2, QString( "Rct Misere"), QString( "rct misere"), QString("[Misere_Complex:Losing_Position+Rcts]"), RctMisereNode::registeredIndex());
}

int RctMisereNode::startComputationKind() {
	if(computeOutcome) {
		return Misere_complex; //the user has chosen to compute the outcome
	} else {
		return GrundyValue;  //the user has chosen to compute the grundy value
	}
}

void RctMisereNode::computeOrderParameters() {
	PosRct Rcta = posRctA;
	
	//compute the parameters of the game
	//the first parameter should be something indicating the complexity of the position
	BaseGame g(Rcta.pos);
	g.computeOrderParameters(nodeOrder);
	
	//chnage the first parameter so that it becomes a "pseudo lives" value, ie :
	//number of lives of the position part
	//+ 0/1 according to the 0/1 Nim-column part
	//+ the sum of all the Rct's depths
	if(!nodeOrder.param.empty()) {
		int nb_livesA=nodeOrder.param.front();
		multiset<Rct>::iterator Rct_i;
		int Rct_depths_A=0;
		for(Rct_i=Rcta.RctSet.begin();Rct_i!=Rcta.RctSet.end();Rct_i++){
			Rct_depths_A += Rct_i->depth;
		}
		
		*(nodeOrder.param.begin()) = nb_livesA+Rcta.nimCol+Rct_depths_A;
	}
	
	//last parameter : lexicographic order
	nodeOrder.pos = PosRctToString(Rcta);
	
	//check value : this parameter is used to guide the check computation with a previous database
	if(Parameter::isCheck) {
		Parameter::isCheck = false; //in order to use the normal database
		RctMisereNode a = *this;
		if(a.tryImmediateResult()) {
			//losing child > winning child > unknown child
			nodeOrder.checkValue = global::randomCheck(a.winLossResult);
		}
		Parameter::isCheck = true;
	}
}

bool RctMisereNode::tryImmediateResult() {
	apply_known_Rct();
	winLossResult=global::Nim_unknown;
	
	BaseGame g(posRctA.pos);
	if(g.is_terminal_position() && posRctA.RctSet.empty()) {
		if(posRctA.nimCol == 0) {
			winLossResult=global::Win;
		} else {
			winLossResult=global::Loss;
		}
	}
	
	if (winLossResult == global::Nim_unknown) {
		//look for in the database
		bool found;
		found = db::find(PosRctToString(posRctA), dbRctMisereIndex, Parameter::isCheck);
		
		if(found) {
			//child is already known (so it's a loss)
			winLossResult = global::Loss;
		}
	}
	
	return (winLossResult != global::Nim_unknown);
}

//Simplify the given position with known Rct
void RctMisereNode::apply_known_Rct() {
	//nothing to do if position part is empty
	BaseGame g(posRctA.pos);
	if(g.is_terminal_position()) return;
	int lastCharacter = g.lastCharacter();
	
	//list of independent components
	list<Line> components=g.sumComponents();
	
	// ------------- look for known components in the database ----------------
	Line unknown_components;  //representation with only the unknown components
	
	list<Line>::iterator Li;
	
	//look for in the database the Rct of each component
	for(Li=components.begin(); Li!=components.end(); Li++) {
		if( db::find(gameString(*Li) , Rct::dbPosRctIndex) ) {
			//known component
			posRctA.RctSet.insert( RctfromString(db::foundString()) );
		} else {
			//unknown component
			unknown_components.insert(unknown_components.end(), Li->begin(), Li->end());
			if(lastCharacter!=0) {
				unknown_components.pop_back(); //delete position's last character
			}
		}
	}
	
	if(lastCharacter!=0) {
		unknown_components.push_back(lastCharacter);
	}
	posRctA.pos=unknown_components;
	
	//simplify the multiset
	if(simplify(posRctA.RctSet)){
		posRctA.nimCol = (!posRctA.nimCol);
	}
}

bool RctMisereNode::computeNodeChildren(list<BaseNode> &children) {
	//if there is more than one component, create the list of them and count the number of lives
	BaseGame g(posRctA.pos);
	if( g.isMultiComponents() ) {
		list<Line> components=g.sumComponents();
		list<Line>::iterator Li;
		for(Li = components.begin(); Li != components.end(); Li++) {
			//get the "number of lives" of the game : TODO : the "number of lives" is a concept specific to sprouts
			//this is the fist sort parameter
			MoveOrdering paramTmp;
			BaseGame(*Li).computeOrderParameters(paramTmp);
			int nbLives = 0;
			if(!paramTmp.param.empty()) {
				nbLives=paramTmp.param.front();
			}
			
			//criteria to switch to a rct computation : TODO : we should use a 0/1 criteria
			if(nbLives < Parameter::lifeLim) {
				//we found a component which should be changed in Rct
				//it becomes the single child of the current position
				BaseNode Ni(FullTreeNode::registeredIndex(), FullTreeNode::Rct_kind);
				FullTreeNode::access(Ni)->positionA = *Li;
				children.push_back(Ni);
				
				//the computation_kind of the current node is changed to RctInMisere
				//it will go back to Misere_complex after the rct has been computed
				computation_kind=RctInMisere;
				
				return false;
			}
		}
	}
	
	return computeMisereChildren(children);
}

//children computation in the misere complex case
bool RctMisereNode::computeMisereChildren(list<BaseNode> &children) {
	//--------------- children of the position part -----------------------------
	if(posRctA.pos.size()>1){
	BaseGame g(posRctA.pos);
	g.computeChildrenSet();
	
	set<Line>::iterator it;
	for(it=g.childrenSet().begin(); it!=g.childrenSet().end(); it++) {
		PosRct posRcti;
		BaseGame g(*it);
		g.canonize();
		posRcti.pos=g.exportToLine();
		posRcti.nimCol = posRctA.nimCol;
		posRcti.RctSet = posRctA.RctSet;
		
		if(addNewChild(children, posRcti)) {
			return true;	//we found an immediate result
		}
	}}
	
	//--------------- children of the nim column part -----------------------------
	if(posRctA.nimCol == 1) {
		PosRct posRcti;
		posRcti.pos = posRctA.pos;
		posRcti.nimCol = 0;
		posRcti.RctSet = posRctA.RctSet;
		
		if(addNewChild(children, posRcti)) {
			return true;	//we found an immediate result
		}
	}
	
	//--------------- children of the Rct part -----------------------------
	multiset<Rct>::iterator Rct_i=posRctA.RctSet.begin();
	multiset<Rct>::iterator Rct_j;
	set<Rct> setRcttmp;
	set<Rct>::iterator Rct_k;
	Rct Rct_sav;
	//we need to compute each way to "move" in *Rct_i,
	//where Rct_i have to point to each different Rct of the multiset
	//(it's useless to compute the children of 2 identical Rct of the multiset)
	while(Rct_i!=posRctA.RctSet.end()) {
		//remove one value of *Rct_i of the set...
		multiset<Rct> multi=posRctA.RctSet;
		Rct_j=multi.begin();
		while(!(*Rct_j == *Rct_i)){
			Rct_j++;
		}
		multi.erase(Rct_j);
		
		if(db::find(Rct_i->LtoString() , Rct::dbRctChildrenIndex)) {
			setRcttmp=setRctfromString(db::foundString());
		} else {
			cout << "error in RctMisereNode::computeMisereChildren" << endl;
		}
		
		//...then replace this value by a child of *Rct_i
		//(Rct_k points to each possible child of *Rct_i)
		for(Rct_k=setRcttmp.begin();Rct_k!=setRcttmp.end();Rct_k++){
			PosRct posRcti;
			posRcti.pos = posRctA.pos;
			posRcti.nimCol = posRctA.nimCol;
			posRcti.RctSet = multi;
			posRcti.RctSet.insert(*Rct_k);
			//simplification of Nim-columns 0 and 1
			if(simplify(posRcti.RctSet)){
				posRcti.nimCol = (!posRcti.nimCol);
			}
			
			if(addNewChild(children, posRcti)) {
				return true;	//we found an immediate result
			}
		}
		
		//find the next Rct different of *Rct_i in the original multiset
		Rct_sav=*Rct_i;
		bool found=false;
		while( Rct_i!=posRctA.RctSet.end() && (!found) ){
			if(*Rct_i==Rct_sav){
				Rct_i++;
			} else {
				found=true;
			}
		}
	}
	
	//sort children and remove duplicates
	sortChildren(children, /*unique*/ true);
	
	return false;
}

//add PosRcti to the list of children
//return true if we have deduced an immediate result
bool RctMisereNode::addNewChild(list<BaseNode> &children, const PosRct& posRcti) {
	BaseNode Ni(RctMisereNode::registeredIndex(), Misere_complex);
	access(Ni)->posRctA=posRcti;
	
	if(Ni.tryImmediateResult()) {
		if(access(Ni)->winLossResult==global::Loss) {
			winLossResult = global::Win; //the result is "Win" if the child is "Loss"
			return true;
		}
	}
	
	if(access(Ni)->winLossResult!=global::Win) {	//Winning children are useless
		children.push_back(Ni);
	}
	
	return false;
}

bool RctMisereNode::resultFromOneChild(BaseNode &child) {
	//case of RctInMisere : the computation of the Rct is finished, there is nothing more to do
	if(computation_kind==RctInMisere) {
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

void RctMisereNode::resultFromAllChildren(list<BaseNode> & /*children*/) {
	//case of RctInMisere : normally, we shouldn't come here, because computation ends with local result
	//this is only a security
	if(computation_kind==RctInMisere) {
		//there is nothing to do
		return;
	}
	
	//if we get there, it means all children were winning ones, so the node is a losing position
	winLossResult = global::Loss;
	db::add(PosRctToString(posRctA), dbRctMisereIndex, Parameter::isCheck, /*withDefaultCheck*/ true);
}

bool RctMisereNode::resultIsComplete() {
	if(computation_kind == RctInMisere) {
		//for the next step, go back to the misereComplex algorithm
		computation_kind=Misere_complex;
		return false;
	} else
	if(computation_kind == GrundyValue) {
		//in case of a Grundy value computation, the computation ends when we have found a loss
		if(winLossResult == global::Loss) {
			return true;
		} else {
			//computation is not finished, we need to increment the nim-column part before trying a new step
			posRctA.RctSet.clear();
			posRctA.nimCol = 0;
			currentGrundyValue++;
			
			//Nimber-column bigger than NimColumnLimit are not possible in the Rct class
			if(currentGrundyValue>Rct::NimColumnLimit){
				Error(QString("Too big misere-grundy value in RctMisereNode::resultIsComplete"), true);
			}
			//cout << posRctA.RctSet.size() << " " << currentGrundyValue;
			
			//WARNING : we suppose that the computation of the Grundy-values is only used on the root node
			//we need to initialize back the position part, because in some cases it has been modified with known Rct
			posRctA.pos = Parameter::given_position;
			posRctA.RctSet.insert(Rct(currentGrundyValue)); // add the nim-column of value currentGrundyValue
			return false;
		}
	} else {
		return true;
	}
}

Interface RctMisereNode::getParamDef() {
	Interface result;
	result.name="Rct Misere";
	result.addDataBaseButton(dbRctMisereIndex, /*pos*/ 0, 0);
	result.linkDataBase("RctMisereBase");
	result.addDataBaseButton(dbRctMisereIndex + 1, /*pos*/ 0, 1);
	result.linkDataBase("RctMisereCheck");
	
	result.addDataBaseButton(Rct::dbPosRctIndex, /*pos*/ 1, 0);
	result.linkDataBase("Rct_PosRctBase");
	result.addDataBaseButton(Rct::dbRctChildrenIndex, /*pos*/ 1, 1);
	result.linkDataBase("Rct_RctChildrenBase");
	
	result.addCheckButton("Activate check", false, /*pos*/ 0, 2);
	result.link(Parameter::isCheck, "isCheck");
	result.addLabel("Random check cycles :", /*pos*/ 1, 2);	
	result.addSpinBox(0, 99, 0, /*pos*/ 1, 3);
	result.link(Parameter::RandomCheckCycles, "RandomCheckCycles");
	
	result.addLabel("Compute :", /*pos*/ 2, 1);
	result.addRadioButton("outcome", true, /*pos*/ 2, 2);
	result.link(RctMisereNode::computeOutcome, "computeOutcome");
	result.addRadioButton("grundy-value", false, /*pos*/ 2, 3);
	
	return result;
}

//initialize the first node of a computation with the correct parameters
void RctMisereNode::initRootNode() {
	PosRct cleanPosRct; //to be sure that PosRctA is a new object
	posRctA = cleanPosRct;
	posRctA.pos = Parameter::given_position;
	
	//initialize the Rct objects
	Rct::reduced=true;	//we use Rct and not CT
	Rct::initRctDatabases();
	
	//initialize the grundy value
	currentGrundyValue = 0;
}

string RctMisereNode::resultString() {
	string result;
	if(!computeOutcome) {
		result = QString("mis. Grundy=%1").arg(currentGrundyValue).toStdString();
	} else {
		result += "misere=";
		result += global::toString(winLossResult);
	}
	return result;
}

QStringList RctMisereNode::displayStringList() {
	QStringList result;
	BaseGame g(posRctA.pos);
	result += QString::fromStdString( g.exportOptionalString() );
	result += QString::fromStdString(g.exportToString() );
	result +=QString::fromStdString( RctPartToString(posRctA) );
	return result;
}

//title for the column with the part of the node that is not the position
string RctMisereNode::extraPositionPartTitle() {
	return string("Rct part");
}

QString RctMisereNode::displayColor() {
	if(computation_kind == GrundyValue){
		return QString("yellow");
	} else {
		return QString("transparent");
	}
}

//info about a position given by its string
QStringList RctMisereNode::infoAbout(string posString) {
	QStringList info;
	RctMisereNode node_tmp;
	PosRct cleanPosRct;
	node_tmp.posRctA = cleanPosRct;
	BaseGame h(posString);
	node_tmp.posRctA.pos = h.exportToLine();
	
	if(node_tmp.tryImmediateResult()) {
		info += QString(global::toString(node_tmp.winLossResult).c_str());	//displayed text
		if(node_tmp.winLossResult==global::Loss) info += QString("red");	//displayed color
	}
	
	return info;
}

string RctMisereNode::debugStringKind() {
	string result;
	if(computation_kind == Misere_complex) {
		result = string("MCx");
	} else
	if(computation_kind == RctInMisere) {
		result = string("RctMis");
	} else
	if(computation_kind == GrundyValue) {
		result = string("GrdVal");
	}
	return result;
}

string RctMisereNode::debugStringPos() {
	return PosRctToString(posRctA);
}

string RctMisereNode::debugStringResult() {
	if(winLossResult == global::Loss) {
		return string("L");
	} else {
		return string("G");
	}
}

//used to move the check database in the normal database during check cycles
void RctMisereNode::prepareNextCycle() {
	cout << db::storeVect[dbRctMisereIndex].size() << " -> " << db::storeVect[dbRctMisereIndex+1].size() << endl;
	db::move(dbRctMisereIndex+1, dbRctMisereIndex);
}
