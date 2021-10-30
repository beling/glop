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

#include "winlossnode.h"

#include "../computation/database.h"
#include "../baseclass/basegame.h"

//basic algorithm, that simply computes recursively a win/loss

bool WinLossNode::misereVersion = false;
bool WinLossNode::debugVictory = false;
int WinLossNode::dbWinLossIndex = 0; 
int WinLossNode::dbMisereIndex = 0;

const int WinLossNode::NormalWL = 0;
const int WinLossNode::MisereWL = 1;

//register mecanism
WinLossNode WinLossNode::root(0);

//game type that can be computed with this node
GameType WinLossNode::gameType() {
	GameType t;
	t.add(GameType::ImpartialGame);
	t.add(GameType::PartizanGameWL);
	
	//temporarily disabled because Score Games use PositionString and not Line type
	//TODO : use PositionString in WinLossNode
	//t.add(GameType::ScoreGame);
	
	return t;
}

void WinLossNode::createDataStorage() {
	dbWinLossIndex = db::create(/*string*/ 2, QString( "WinLoss"), QString( "winloss"), QString("[WinLoss:Losing_Position]"), WinLossNode::registeredIndex());
	dbMisereIndex = db::create(/*string*/ 2, QString( "WinLoss Misere"), QString( "winloss misere"), QString("[WinLoss_Misere:Losing_Position]"), WinLossNode::registeredIndex());
}

int WinLossNode::startComputationKind() {
	return MisereWL;
}

bool WinLossNode::tryImmediateResult() {
	winLossResult = global::Nim_unknown;
	
	BaseGame g(positionA);
	if(g.is_terminal_position()) {
		if(misereVersion) {
			winLossResult=global::Win;
		} else {
			winLossResult=global::Loss;
		}
	} else {
		//in the normal version only and if we are not in debugging mode
		//it is used in some games to indicate an early win/loss
		if(!misereVersion && !debugVictory) {
			winLossResult = g.Victory();
		}
	}
	
	if (winLossResult == global::Nim_unknown) {
		//look for in the database
		bool exist;
		if(misereVersion) {
			exist = db::find(gameString(positionA), dbMisereIndex, Parameter::isCheck);
		} else {
			exist = db::find(gameString(positionA), dbWinLossIndex, Parameter::isCheck);
		}
		
		if(exist) {
			//child is already known (so it's a loss)
			winLossResult = global::Loss;
		}
	}
	
	return (winLossResult != global::Nim_unknown);
}

bool WinLossNode::resultFromOneChild(BaseNode &child) {
	//-----case of a losing child -----
	if(access(child)->winLossResult==global::Loss)  {
		//One losing child means parent is a winning position
		winLossResult = global::Win;
		if(!misereVersion && debugVictory) compareToVictory(); //used to debug the function Victory()
		return true; //a local result was found, end of computation. 
	}
	
	return false;
}

void WinLossNode::resultFromAllChildren(list<BaseNode> & /*children*/) {
	//if we get there, it means all children were winning ones, so the node is a losing position
	winLossResult = global::Loss;
	
	if(misereVersion) {
		db::add(gameString(positionA), dbMisereIndex, Parameter::isCheck, /*withDefaultCheck*/ true);
	} else {
		db::add(gameString(positionA), dbWinLossIndex, Parameter::isCheck, /*withDefaultCheck*/ true);
		if(debugVictory) compareToVictory(); //used to debug the function Victory()
	}
}

//computation is always finished in one step
bool WinLossNode::resultIsComplete() {
	return true;
}

bool WinLossNode::computeNodeChildren(list<BaseNode> &children) {
	win_loss result = compute_children_Basic(children);
	if( result != global::Nim_unknown) {
		winLossResult = result;
		return true;
	}
	
	//sort children and remove duplicates
	sortChildren(children, /*unique*/ true);
	
	return false;
}

//compute the children of a given position and return Win if any of them is a Loss
win_loss WinLossNode::compute_children_Basic(list<BaseNode> &children) {
	BaseGame g(positionA);
	g.computeChildrenSet();
	
	set<Line>::iterator it;
	for(it=g.childrenSet().begin(); it!=g.childrenSet().end(); it++) {
		BaseNode child(WinLossNode::registeredIndex(), computation_kind);
		
		BaseGame g(*it);
		g.canonize();
		access(child)->positionA=g.exportToLine();
		
		child.tryImmediateResult();
		if(access(child)->winLossResult == global::Loss) return global::Win; //returns "Win" if any child is "Loss"
		if(access(child)->winLossResult != global::Win) {	//Winning children are useless
			children.push_back(child);
		}
	}
	
	return global::Nim_unknown;
}

void WinLossNode::computeOrderParameters() {
	//compute the parameters of the game
	BaseGame g(positionA);
	g.computeOrderParameters(nodeOrder);
	
	//last parameter : lexicographic order on the string representation
	nodeOrder.pos = g.exportToString();
	
	//check value : this parameter is used to guide the check computation with a previous database
	if(Parameter::isCheck) {
		Parameter::isCheck = false; //in order to use the normal database
		WinLossNode a = *this; //we need a copy to avoid any change of the current node
		if(a.tryImmediateResult()) {
			//losing child > winning child > unknown child
			nodeOrder.checkValue = global::randomCheck(a.winLossResult);
		}
		Parameter::isCheck = true;
	}
}

//if we are debugging the function Victory(), compare all the results to Victory()
void WinLossNode::compareToVictory() {
	BaseGame g(positionA);
	win_loss victoryResult = g.Victory();
	if(victoryResult != global::Nim_unknown && victoryResult != winLossResult) {
		//there is an error in Victory, compute it again in debug mode
		Parameter::debugGames = true;
		BaseGame g2(positionA);
		cout << "WinLossResult="   << global::toString(winLossResult) << 
		        " Victory()="      << global::toString(victoryResult) << " " <<
				g2.exportToString() << endl;
		g2.Victory();
		cout << endl;
		Parameter::debugGames = false;
	}
}

Interface WinLossNode::getParamDef() {
	Interface result;
	result.name="WinLoss";
	result.addDataBaseButton(dbWinLossIndex, /*pos*/ 0, 0);
	result.linkDataBase("WinLossBase");
	result.addDataBaseButton(dbWinLossIndex + 1, /*pos*/ 0, 1);
	result.linkDataBase("WinLossCheck");
	result.addDataBaseButton(dbMisereIndex, /*pos*/ 1, 0);
	result.linkDataBase("WinLossMisereBase");
	result.addDataBaseButton(dbMisereIndex + 1, /*pos*/ 1, 1);
	result.linkDataBase("WinLossMisereCheck");
	
	result.addCheckButton("Activate check", false, /*pos*/ 0, 2);
	result.link(Parameter::isCheck, "isCheck");
	result.addLabel("Random check cycles :", /*pos*/ 1, 2);	
	result.addSpinBox(0, 99, 0, /*pos*/ 1, 3);
	result.link(Parameter::RandomCheckCycles, "RandomCheckCycles");
	
	result.addLabel("Game version :", /*pos*/ 2, 0);
	result.addRadioButton("normal", true, /*pos*/ 2, 1);
	result.addRadioButton("misere", false, /*pos*/ 2, 2);
	result.link(WinLossNode::misereVersion, "isMisere");
	
	#ifdef UNSUPPORTED_FEATURE
	result.addCheckButton("Debug Victory", false, /*pos*/ 2, 3);
	result.link(WinLossNode::debugVictory, "debugVictory");
	#endif
	
	return result;
}

//initialize the first node of a computation with the correct parameters
void WinLossNode::initRootNode() {
	positionA=Parameter::given_position;
}

string WinLossNode::resultString() {
	string result;
	if(misereVersion) {
		result += "misere=";
	} else {
		result += "outcome=";
	}
	result += global::toString(winLossResult);
	return result;
}

QStringList WinLossNode::displayStringList() {
	QStringList result;
	BaseGame g(positionA);
	result += QString::fromStdString(g.exportOptionalString());
	result +=QString::fromStdString(g.exportToString());
	return result;
}

QString WinLossNode::displayColor() {
	return QString("transparent");
}

QStringList WinLossNode::infoAbout(string posString) {
	QStringList info;
	WinLossNode node_tmp;
	BaseGame h(posString);
	node_tmp.positionA = h.exportToLine();
	
	if(node_tmp.tryImmediateResult()) {
		info += QString(global::toString(node_tmp.winLossResult).c_str());	//displayed text
		if(node_tmp.winLossResult==global::Loss) info += QString("red");	//displayed color
	}
	
	return info;
}

string WinLossNode::debugStringKind() {
	string result = string("WL");
	return result;
}

string WinLossNode::debugStringPos() {
	return gameString(positionA);
}

string WinLossNode::debugStringResult() {
	if(winLossResult == global::Loss) {
		return string("L");
	} else {
		return string("W");
	}
}

//used to move the check database in the normal database during check cycles
void WinLossNode::prepareNextCycle() {
	if(misereVersion) {
		cout << db::storeVect[dbMisereIndex].size() << " -> " << db::storeVect[dbMisereIndex+1].size() << endl;
		db::move(dbMisereIndex+1, dbMisereIndex);	
	} else {
		cout << db::storeVect[dbWinLossIndex].size() << " -> " << db::storeVect[dbWinLossIndex+1].size() << endl;
		db::move(dbWinLossIndex+1, dbWinLossIndex);
	}
}
