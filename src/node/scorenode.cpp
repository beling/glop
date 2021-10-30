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

#include "scorenode.h"

#include "../computation/database.h"
#include "../baseclass/basegame.h"
#include "../stringconverter.h"

//-----------------------Score Node ----------------------
int ScoreNode::given_contract=0;
bool ScoreNode::computeOutcome = true;
bool ScoreNode::storeLossOnly = true;
bool ScoreNode::separateWinLoss=false;
bool ScoreNode::separateFirstLoss=false;
bool ScoreNode::useWinPurge = true;
int ScoreNode::winPurgeLimit = 0;
bool ScoreNode::useAuxPurge = false;
bool ScoreNode::separateTmpLoss = false;
int ScoreNode::lossPurgeLimit = 0;
int ScoreNode::dbScoreIndex = 0;
int ScoreNode::dbScoreWIndex = 0;
int ScoreNode::dbAuxLossIndex = 0;

const int ScoreNode::GivenContract = 0;
const int ScoreNode::FindScore = 1;

//register mecanism
ScoreNode ScoreNode::root(0);

//game type that can be computed with this node
GameType ScoreNode::gameType() {
	GameType t;
	t.add(GameType::ScoreGame);
	return t;
}

void ScoreNode::createDataStorage() {
	dbScoreIndex     = db::create(/*string-nimber*/ 0, QString("Score"), QString("score"), QString("[Positions+Score]"), ScoreNode::registeredIndex());
	dbAuxLossIndex = db::create(/*string-nimber*/ 0, QString("Score Aux."), QString("scoreAux"), QString("[Positions+Score]"), ScoreNode::registeredIndex());
	dbScoreWIndex    = db::create(/*string-string*/ 1, QString("ScoreW"), QString("scorew"), QString("[Positions+ScoreW]"), ScoreNode::registeredIndex());
}

int ScoreNode::startComputationKind() {
	if(computeOutcome) {
		return GivenContract; //the user has chosen to compute the outcome
	} else {
		return FindScore;  //the user has chosen to compute the nimber
	}
}

//the result is immediate in one the following cases : contract < 0 or contract > availableScore 
bool ScoreNode::tryImmediateResult() {
	//cout << "TryImmediateResult : " << positionA << " " << (int) contractA << endl;
	winLossResult = global::Nim_unknown;
	
	if(contractA <= 0) {
		winLossResult = global::Win;
		return true;
	}
	
	BaseGame g(positionA);
	ScoreRange availableScore = g.availableScore(/*not only max score*/ false);
	if(contractA > availableScore.maxPossibleScore) {
		winLossResult = global::Loss;
		return true;
	} else
	if(contractA <= availableScore.minPossibleScore) {
		winLossResult = global::Win;
		return true;
	}
	
	//look for in the database of already known results
	string compressedString = g.compress(positionA);
	ScoreRange range;
	if(findKnownResult(compressedString, range) ) {
		if(contractA<=range.minPossibleScore) {
			winLossResult = global::Win;	//we know from the database that this contract is possible
			return true;
		}
		
		if(contractA>range.maxPossibleScore) {
			winLossResult = global::Loss;	//we know from the database that this contract is not possible
			return true;
		}
	}
	
	return false;
}

bool ScoreNode::findKnownResult(const string& pos, ScoreRange& knownResult) {
	bool foundW = false;
	bool foundL = false;
	if(!storeLossOnly) {
		foundW = db::find(pos, dbScoreWIndex, Parameter::isCheck);
		if(foundW) knownResult.fromString(db::foundString());		//initialize the range with already known results
	}
	
	//add also the knowledge of loss databases if loss are stored
	if(storeLossOnly || separateWinLoss) {
		foundL = db::find(pos, dbScoreIndex, Parameter::isCheck);
		if(!foundL && (separateFirstLoss || separateTmpLoss) ) {
			//look instead in the auxiliary Loss database
			foundL = db::find(pos, dbAuxLossIndex, Parameter::isCheck);
		}
		
		//decrease the maximum possible score with the found result
		if(foundL) knownResult.decreaseMaxTo(db::foundNimber());
	}
	
	return (foundW || foundL);
}

bool ScoreNode::resultFromOneChild(BaseNode &child) {
	//-----case of a losing child -----
	if(access(child)->winLossResult==global::Loss)  {
		//One losing child means parent is a winning position
		winLossResult = global::Win;
		if(!storeLossOnly) storeWinResult();
		return true; //a local result was found, end of computation for the current node 
	}
	
	return false;
}

//store in the database the fact that the current position is a Win
void ScoreNode::storeWinResult() {
	//look for in the database the existing results
	ScoreRange range;
	string compressedString = gameCompress(positionA);
	bool found = db::find(compressedString, dbScoreWIndex, Parameter::isCheck);
	if(found) {
		//initialize the range with already known results
		range.fromString(db::foundString());
	} else {
		//initialize the maximum possible score with the available score
		BaseGame g(positionA);
		ScoreRange availableScore = g.availableScore(/*max score only*/ true);
		range.decreaseMaxTo(availableScore.maxPossibleScore);
	}
	
	//update the range with the new result : the minimum possible score is at least the contract
	range.increaseMinTo(contractA);
	
	//update the database
	purgeWinDatabase();
	db::add(compressedString, range.toString(), dbScoreWIndex, Parameter::isCheck);
}

void ScoreNode::resultFromAllChildren(list<BaseNode> & /*children*/) {
	//if we get there, it means all children were winning ones, so the node is a losing position
	winLossResult = global::Loss;
	
	//look for in the database the existing results
	ScoreRange range;
	string compressedString = gameCompress(positionA);
	findKnownResult(compressedString, range);
	
	//update the range with the new result : the maximum possible score is strictly less than the contract
	range.decreaseMaxTo(contractA-1);
	
	//update the database
	if(storeLossOnly || separateWinLoss) {
		if(separateFirstLoss && isFirstChild)  {
			db::add(compressedString, range.maxPossibleScore, dbAuxLossIndex, Parameter::isCheck);
		} else
		if(separateTmpLoss) {
			//we store some positions in the main loss score base
			//and the other ones in the auxiliary base that is purged regularly
			//the criteria is defined by the game
			BaseGame g(positionA);
			if(g.isPermanentPosition()) {
				if(Parameter::isCheck) checkLoss(compressedString, range.maxPossibleScore);
				db::add(compressedString, range.maxPossibleScore, dbScoreIndex, Parameter::isCheck);
			} else {
				purgeLossDatabase();
				db::add(compressedString, range.maxPossibleScore, dbAuxLossIndex, Parameter::isCheck);
			}
		} else {
			if(Parameter::isCheck) checkLoss(compressedString, range.maxPossibleScore);
			db::add(compressedString, range.maxPossibleScore, dbScoreIndex, Parameter::isCheck);
		}
	} else {
		purgeWinDatabase();
		db::add(compressedString, range.toString(), dbScoreWIndex, Parameter::isCheck);
	}
}

//purge the win database if the limit is reached
void ScoreNode::purgeWinDatabase() {
	//TODO : there should be db::size(numIndex) and db::clear(numIndex) functions instead of this direct access
	if(useWinPurge) {
		int a = 0;
		if(Parameter::isCheck) a = 1;
		if(db::storeVect[dbScoreWIndex+a].size() >= winPurgeLimit*1000) db::storeVect[dbScoreWIndex+a].clear();
	}
}

//purge the loss database if the limit is reached
void ScoreNode::purgeLossDatabase() {
	if(useAuxPurge) {
		int a = 0;
		if(Parameter::isCheck) a = 1;
		if(db::storeVect[dbAuxLossIndex+a].size() >= lossPurgeLimit*1000) {
			db::storeVect[dbAuxLossIndex+a].clear();
		}
	}
}

//check a loss condition on a position stored in the main Score base
void ScoreNode::checkLoss(string compressedString, Score maxPossibleScore) {
	bool exist = db::find(compressedString, dbScoreIndex, /*isCheck*/ false);	
	if(exist) {
		//if the condition on max Score is more powerful than in the previous computation, then it is a new result
		Score previousMax = db::foundNimber();
		if( maxPossibleScore < previousMax ) {
			db::storeVect[dbScoreIndex+1].status = 1;
			cout << "New score condition: " << compressedString << " " << (int) maxPossibleScore << "<" << (int) previousMax << endl;
		}
	} else {
		//it is a new result, change the status of the check database
		db::storeVect[dbScoreIndex+1].status = 1;
		cout << "New position: " << compressedString << " " << (int) maxPossibleScore << endl;
	}
}

//Merge two loss results. This function is called when a position from a file collide with an existing position in the database
bool ScoreNode::treatBaseCollision(const string & pos, nimber nBase, nimber nFile, int baseIndex, bool isCheckBase) {
	//we keep in the base only the lowest loss value ( = strongest condition)
	if(nBase<=nFile) {
		return true; //nothing to do, the base already contains the lowest value
	} else {
		db::add(pos, nFile, baseIndex, isCheckBase);
		return true;
	}
}

//Merge two win results. This function is called when a position from a file collide with an existing position in the database
bool ScoreNode::treatBaseCollision(const string & pos, string nBase, string nFile, int baseIndex, bool isCheckBase) {
	ScoreRange range;
	range.fromString(nBase);	//initialize the range with the value in database
	range.mergeString(nFile);	//add the information of the range from the file
	
	//case of inconsistency
	if(range.minPossibleScore > range.maxPossibleScore) return false;
	
	string newRange = range.toString();
	if(newRange != nBase) {
		db::add(pos, newRange, baseIndex, isCheckBase);
	}
	return true;
}

bool ScoreNode::resultIsComplete() {
	if(computation_kind == FindScore) {
		//the computation ends when we have found a loss
		if(winLossResult == global::Loss) {
			return true;
		} else {
			//computation is not finished, we need to increment the contract
			contractA++;
			if(contractA>ScoreRange::Score_max){
				Error(QString("Too big contract in ScoreNode::resultIsComplete"), true);
			}
			return false;
		}
	}
	
	return true;
}

//compute the list of children
//return true if the result of the position has been deduced immediatly from one child
bool ScoreNode::computeNodeChildren(list<BaseNode> &children) {
	//------- move in the position part -------------
	BaseGame g(positionA);
	ScoreRange availableScore = g.availableScore(/*max score only*/ true);
	g.computeOptionsSet();
	
	set<PositionString>::iterator it;
	for(it=g.optionsSet().begin(); it!=g.optionsSet().end(); it++) {
		//read the score variation part of the string ('K' is the separator)
		int scoreVariation=0;
		string copy = *it;
		if(!copy.empty()) {
			if(copy[0]=='J') {
				copy.erase(0, 1);
				StringConverter conv(copy);
				conv << CvCd("UT") << CvCd("EK"); //UT : activate separator, EK = use K as separator
				conv >> scoreVariation;
			}
		}
		
		BaseNode child(ScoreNode::registeredIndex(), GivenContract);
		
		BaseGame g(*it);
		g.canonize();
		string canonizedString = g.exportToString();
		if(!canonizedString.empty()) {
			if(canonizedString[0]=='J') {
				//cout << *it << " " << canonizedString << " ";
				canonizedString.erase(0, 1);
				StringConverter conv(canonizedString);
				conv << CvCd("UT") << CvCd("EK"); //UT : activate separator, EK = use K as separator
				int scoreVariation2=0;
				conv >> scoreVariation2;
				scoreVariation += scoreVariation2;  //add this new score variation to the previous one
				
				//erase the useless part of the string (all characters up to the letter K included)
				int p=0;
				while(canonizedString[p]!='K' && p<(int)canonizedString.size()-1) p++;
				canonizedString.erase(0, p+1); //erase p+1 characters
				//cout << canonizedString << endl;
			}
		}
		
		access(child)->positionA = canonizedString;
		if(availableScore.maxPossibleScore - contractA + 1 > scoreVariation) {
			access(child)->contractA = availableScore.maxPossibleScore - contractA + 1 - scoreVariation;
		} else {
			//since contractA is an unsigned char, we cannot accept negative numbers
			access(child)->contractA = 0;
		}
		access(child)->isFirstChild = false;
		
		child.tryImmediateResult();
		if(access(child)->winLossResult == global::Loss) {
			//the node result is "Win" if any child is "Loss"
			winLossResult = global::Win;
			if(!storeLossOnly) storeWinResult();
			return true; //the result have been deduced immediately from one child
		}
		
		if(access(child)->winLossResult != global::Win) {	//Winning children are useless
			children.push_back(child);
		}
	}
	
	//----------------sort children and remove duplicates----------------------
	if(separateFirstLoss && Parameter::isCheck) {
		//TODO : this algorithm is not correct, because the first child should stay first except if he is known to be not losing
		//special case of a check computation with storage of only the losing children
		//we need a first sort to identify which child is "the first one" for the normal algorithm
		Parameter::isCheck = false;
		MoveOrdering::ignoreCheckValue = true;
		sortChildren(children, /*unique*/ true);
		if(!children.empty()) {
			list<BaseNode>::iterator Bi = children.begin();
			access(*Bi)->isFirstChild = true;
		}
		
		//we need a second sort to take into account the known results of the normal database
		Parameter::isCheck = true;
		MoveOrdering::ignoreCheckValue = false;
		sortChildren(children, /*unique*/ true);
	} else {
		sortChildren(children, /*unique*/ true);
		if(!children.empty()) {
			list<BaseNode>::iterator Bi = children.begin();
			access(*Bi)->isFirstChild = true;
		}
	}
	
	return false;
}

//note : this is exactly the same function as WinLossNode::computeOrderParameters()
void ScoreNode::computeOrderParameters() {
	//compute the parameters of the game
	BaseGame g(positionA);
	g.computeOrderParameters(nodeOrder);
	nodeOrder.param.push_front(-contractA); //priority to big contracts
	
	//last parameter : lexicographic order on the string representation
	nodeOrder.pos = positionA;
	
	//check value : this parameter is used to guide the check computation with a previous database
	if(Parameter::isCheck) {
		Parameter::isCheck = false; //in order to use the normal database
		ScoreNode a = *this; //we need a copy to avoid any change of the current node
		if(a.tryImmediateResult()) {
			//losing child > winning child > unknown child
			nodeOrder.checkValue = global::randomCheck(a.winLossResult);
		}
		Parameter::isCheck = true;
	}
}

Interface ScoreNode::getParamDef() {
	Interface result;
	result.name="Score";
	result.addDataBaseButton(dbScoreIndex, /*pos*/ 0, 0);
	result.linkDataBase("ScoreBase");
	result.addDataBaseButton(dbScoreIndex + 1, /*pos*/ 0, 1);
	result.linkDataBase("ScoreCheck");

	result.addDataBaseButton(dbScoreWIndex, /*pos*/ 1, 0);
	result.linkDataBase("ScoreWBase");
	result.addDataBaseButton(dbScoreWIndex + 1, /*pos*/ 1, 1);
	result.linkDataBase("ScoreWCheck");

	#ifdef UNSUPPORTED_FEATURE
	result.addDataBaseButton(dbAuxLossIndex, /*pos*/ 2, 0);
	result.linkDataBase("ScoreAuxBase");
	result.addDataBaseButton(dbAuxLossIndex + 1, /*pos*/ 2, 1);
	result.linkDataBase("ScoreAuxCheck");
	#endif
	
	result.addLabel("Start Contract part :", /*pos*/ 3, 0);
	result.addSpinBox(0, 99, 4, /*pos*/ 3, 1);
	result.link(given_contract, "startContractPart");
	
	result.addCheckButton("Store Loss only", false, /*pos*/ 0, 2);
	result.link(storeLossOnly, "lossOnly");

	result.addCheckButton("Sep. W/L", true, /*pos*/ 0, 3);
	result.setToolTip("Separate win and loss positions.");
	result.link(separateWinLoss, "sepWL");

	#ifdef UNSUPPORTED_FEATURE
	result.addCheckButton("Sep. tmp L", false, /*pos*/ 0, 4);
	result.setToolTip("Separate permanent and temporary loss positions.\nThe condition for permanent positions is defined by the game.");
	result.link(separateTmpLoss, "firstLoss");
	
	result.addCheckButton("Sep. first L", false, /*pos*/ 0, 5);
	result.setToolTip("Separate loss positions that are the choice by default for the order on the positions.");
	result.link(separateFirstLoss, "firstLoss");
	#endif
	
	result.addCheckButton("Purge W (kpos):", true, /*pos*/ 1, 2);
	result.link(useWinPurge, "usewinpurge");
	result.addSpinBox(0, 9999, 300, /*pos*/ 1, 3);
	result.link(winPurgeLimit, "winpurge");
	
	#ifdef UNSUPPORTED_FEATURE
	result.addCheckButton("Purge Aux (kpos):", true, /*pos*/ 1, 4);
	result.link(useAuxPurge, "useauxpurge");	
	result.addSpinBox(0, 9999, 300, /*pos*/ 1, 5);	//no purge by default
	result.link(lossPurgeLimit, "losspurge");
	#endif
	
	result.addCheckButton("Activate check", false, /*pos*/ 2, 2);
	result.link(Parameter::isCheck, "isCheck");
	result.addLabel("Random cycles:", /*pos*/ 2, 3);
	result.addSpinBox(0, 99, 0, /*pos*/ 2, 4);
	result.link(Parameter::RandomCheckCycles, "RandomCheckCycles");
	
	result.addLabel("Compute :", /*pos*/ 3, 2);
	result.addRadioButton("outcome", true, /*pos*/ 3, 3);
	result.link(ScoreNode::computeOutcome, "computeOutcome");
	result.addRadioButton("score", false, /*pos*/ 3, 4);
	
	return result;
}

//initialize the first node of a computation with the correct parameters
void ScoreNode::initRootNode() {
	positionA=Parameter::given_positionString;
	if(ScoreNode::computeOutcome) {
		contractA=given_contract;
	} else {
		//in case of a score computation, ignore the start contract part
		contractA=0;
	}
	isFirstChild = false; //used to always store the result of the root node 
}

//name of the start position set by the user in the interface
string ScoreNode::getPositionName() {
	string result = BaseGame(Parameter::gameChoice).getPositionName();			//position name of the game
	
	//note : in case of a score computation, ignore the start contract part
	if(ScoreNode::computeOutcome) {
		result += (QString(" / contract=%1").arg(given_contract)).toStdString();	//add the contract part
	}
	
	return result;
}

string ScoreNode::resultString() {
	string result;
	if(computeOutcome) {
		result += "outcome=";
		result += global::toString(winLossResult);
	} else {
		result += "score=";
		result += QString("%1").arg(contractA - 1, 2, 10).toStdString();
	}
	return result;
}

//display the contract followed by the position
QStringList ScoreNode::displayStringList() {
	QStringList result;
	BaseGame g(positionA);
	result += QString::fromStdString(g.exportOptionalString());
	result += QString::fromStdString(g.exportToString() );
	result += QString( " %1" ).arg((int) contractA, 2, 10 );
	return result;
}

//title for the column with the part of the node that is not the position
string ScoreNode::extraPositionPartTitle() {
	return string("Contract");
}

QString ScoreNode::displayColor() {
	return QString("transparent");
}

//info about a position given by its string
//the first string is the displayed info, the second (optional) one is the color
QStringList ScoreNode::infoAbout(string posString) {
	QStringList info;
	string compressedString = gameCompress(posString);
	ScoreRange range;
	
	//add the information from the main score (loss) database
	bool foundA = db::find(compressedString, dbScoreIndex, Parameter::isCheck);
	if(foundA) range.decreaseMaxTo(db::foundNimber());
	
	//add the information from the auxiliary score (loss) database
	bool foundB = db::find(compressedString, dbAuxLossIndex, Parameter::isCheck);
	if(foundB) range.decreaseMaxTo(db::foundNimber());
	
	//add the information from the score (win) database
	bool foundC = db::find(compressedString, dbScoreWIndex, Parameter::isCheck);
	if(foundC) range.mergeString(db::foundString());
	
	//return the information
	if(foundA || foundB || foundC) {
		QString displayedString = QString(range.toString().c_str());
		
		/*
		This code doesn't work because the contract is not the same for all positions !
		bool winLossKnown = false;
		if(given_contract > range.maxPossibleScore) {
			winLossKnown = true;
			displayedString += QString(" Loss");
			//winLossResult = global::Loss;
		}
		if(given_contract <= range.minPossibleScore) {
			winLossKnown = true;
			displayedString += QString(" Win");
		}
		*/
		
		info += displayedString;
		//if(winLossKnown) info += QString("red");		//displayed color
	
	}
	return info;
}

string ScoreNode::debugStringKind() {
	string result;
	if(computation_kind == GivenContract) {
		result = string("GvC");
	} else
	if(computation_kind == FindScore) {
		result = string("FnS");
	}
	return result;
}

string ScoreNode::debugStringPos() {
	//string result= gameString(positionA) + string(" ");
	string result= positionA + string(" ");
	result += QString( "%1 " ).arg((int) contractA, 2, 10 ).toStdString();
	return result;
}

string ScoreNode::debugStringResult() {
	if(winLossResult == global::Loss) {
		return string("L");
	} else {
		return string("G");
	}
}

//used to move the check database in the normal database during check cycles
void ScoreNode::prepareNextCycle() {
	if(storeLossOnly || separateWinLoss) {
		cout << db::storeVect[dbScoreIndex].size() << " -> " << db::storeVect[dbScoreIndex+1].size() << endl;
	} else {
		cout << db::storeVect[dbScoreWIndex].size() << " -> " << db::storeVect[dbScoreWIndex+1].size() << endl;
	}
	db::move(dbScoreIndex+1, dbScoreIndex);
	db::move(dbAuxLossIndex+1, dbAuxLossIndex);
	db::move(dbScoreWIndex+1, dbScoreWIndex);
}
