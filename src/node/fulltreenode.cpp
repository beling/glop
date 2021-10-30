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

#include "fulltreenode.h"

#include "../computation/database.h"
#include "../baseclass/basegame.h"

int FullTreeNode::dbPosFtiIndex = 0;

bool FullTreeNode::computeRct=true;

const int FullTreeNode::Rct_kind = 0;
const int FullTreeNode::CT_kind = 1;

//register mecanism
FullTreeNode FullTreeNode::root(0);

//game type that can be computed with this node
GameType FullTreeNode::gameType() {
	GameType t;
	t.add(GameType::ImpartialGame);
	return t;
}

void FullTreeNode::createDataStorage() {
	Rct::createDataStorage();
	//-1 means that merge is not supported
	dbPosFtiIndex = db::create(/*stringString*/ 1, QString( "Pos/FTI"), QString( "Pos/FTI"), QString("[Position+FTI]"), -1);
}

int FullTreeNode::startComputationKind() {
	if(computeRct) {
		return Rct_kind;
	} else {
		return CT_kind;
	}
}

bool FullTreeNode::computeFTI=true;

//computation is always locked
bool FullTreeNode::lockedComputation() {
	return true;
}

bool FullTreeNode::tryImmediateResult() {
	Line Pos = positionA;
	
	//check if Pos is already in the database
	if(db::find(gameString(Pos), Rct::pos_base_num)) {
		RctResult=RctfromString(db::foundString());
		
		if(computeFTI) {
			if(db::find(gameString(Pos), dbPosFtiIndex)) {
				FTIResult=FTIfromString(db::foundString());
			} else {
				cout << "error in FullTreeNode::tryImmediateResult()" << endl;
			}
		}
		
		return true;
	}
	
	//empty position : equivalent to Nim-column 0
	if (Pos.size()<=1){
		db::add( gameString(Pos), (Rct(0)).LtoString(), Rct::pos_base_num);
		RctResult=Rct(0);
		if(computeFTI) {
			db::add( gameString(Pos), FullTreeInfo().toString(), dbPosFtiIndex);
			FTIResult=FullTreeInfo();
		}
		return true;
	}
	
	return false;
}

bool FullTreeNode::computeNodeChildren(list<BaseNode> &children) {
	BaseGame g(positionA);
	g.computeChildrenSet();
	
	set<Line>::iterator Li;
	//sort and uniq on the children list
	set<Line> Pos_set;
	for(Li=g.childrenSet().begin(); Li!=g.childrenSet().end(); Li++) {
		BaseGame g(*Li);
		g.canonize();
		Pos_set.insert(g.exportToLine());
	}
	
	for(Li=Pos_set.begin(); Li!=Pos_set.end(); Li++) {
		BaseNode Ni(FullTreeNode::registeredIndex(), computation_kind);
		access(Ni)->positionA=*Li;
		children.push_back(Ni);
	}
	return false;
}

//no deduction is possible with only one child
bool FullTreeNode::resultFromOneChild(BaseNode & /*child*/) {
	return false;
}

void FullTreeNode::resultFromAllChildren(list<BaseNode> &children) {
	set<Rct> set_Rct_children;
	list<BaseNode>::iterator Pi;
	for(Pi=children.begin(); Pi!=children.end(); Pi++) {
		set_Rct_children.insert(access(*Pi)->RctResult);
	}
	
	//------check if "set_Rct_children" is already in the database---
	RctResult.deduceRctResult(set_Rct_children);
	
	//------Add to database and return the Rct----------------------
	db::add( gameString(positionA), RctResult.LtoString(), Rct::pos_base_num);
	
	//------same process for FTI-------------------------
	if(computeFTI) {
		list<FullTreeInfo> list_FTI_children;
		for(Pi=children.begin(); Pi!=children.end(); Pi++) {
			list_FTI_children.push_back(access(*Pi)->FTIResult);
		}
		
		FullTreeInfo result_FTI;
		result_FTI.FTI_computation(list_FTI_children);
		
		db::add(gameString(positionA), result_FTI.toString(), dbPosFtiIndex);
		FTIResult=result_FTI;
	}
}

//computation is always finished in one step
bool FullTreeNode::resultIsComplete() {
	return true;
}

void FullTreeNode::computeOrderParameters() {
	//nothing to do
}

Interface FullTreeNode::getParamDef() {
	Interface result;
	result.name="Full tree";
	result.addDataBaseButton(Rct::dbPosRctIndex, /*pos*/ 0, 0);
	result.linkDataBase("FT_PosRctBase");
	result.addDataBaseButton(Rct::dbRctChildrenIndex, /*pos*/ 0, 1);
	result.linkDataBase("FT_RctChildrenBase");
	result.addDataBaseButton(Rct::dbPosCTIndex, /*pos*/ 0, 2);
	result.linkDataBase("FT_CtBase");
	result.addDataBaseButton(Rct::dbCTChildrenIndex, /*pos*/ 0, 3);
	result.linkDataBase("FT_CtChildrentBase");
	result.addDataBaseButton(dbPosFtiIndex, /*pos*/ 1, 0);
	
	result.addRadioButton("Rct", true, /*pos*/ 2, 0);
	result.link(computeRct, "computeRct");
	result.addRadioButton("Ct", false, /*pos*/ 2, 1);
	
	result.addCheckButton("with Full Tree Info", false, /*pos*/ 2, 2);
	result.link(computeFTI, "withFullTreeInfo");
	
	return result;
}

//initialize the first node of a computation with the correct parameters
void FullTreeNode::initRootNode() {
	positionA=Parameter::given_position;
	
	//initialize the Rct object to Ct computation or Rct computation
	Rct::reduced = (Parameter::computation_kind==Rct_kind);
	Rct::initRctDatabases();
}

string FullTreeNode::resultString() {
	string message;
	
	if(RctResult.identifier==0){
		message += QString("Nim-heap %1").arg(RctResult.depth+RctResult.sum_with_1,0,10).toStdString();
	} else {
		if(Parameter::computation_kind==Rct_kind){
			message += string("Rct ");
		} else {
			message += string("Ct ");
		}
		
		message += QString("Level=%1").arg(RctResult.depth+RctResult.sum_with_1,0,10).toStdString();
	}
	
	return message;
}

QStringList FullTreeNode::displayStringList() {
	QStringList result;
	BaseGame g(positionA);
	result += QString::fromStdString(g.exportOptionalString());
	result +=QString::fromStdString(g.exportToString());
	return result;
}

QString FullTreeNode::displayColor() {
	return QString("lightgray");
}

//info about a position given by its string representation
QStringList FullTreeNode::infoAbout(string posString) {
	QStringList result;
	QString info;
	
	//return the full tree info of this position, if known
	if(db::find(posString, dbPosFtiIndex)) {
		FullTreeInfo FTIa=FTIfromString(db::foundString());
		if(FTIa.normal_win_loss){
			info += QString("W");
		} else {
			info += QString("L");
		}
		if(FTIa.misere_win_loss){
			info += QString("/W");
		} else {
			info += QString("/L");
		}
		
		QString normalPerfect, miserePerfect;
		if(FTIa.normal_perfect_nb_moves==global::Nim_unknown){
			normalPerfect = QString("?");
		} else {
			normalPerfect = QString("%1").arg(FTIa.normal_perfect_nb_moves, 2, 10);
		}
		if(FTIa.misere_perfect_nb_moves==global::Nim_unknown){
			miserePerfect = QString("?");
		} else {
			miserePerfect = QString("%1").arg(FTIa.misere_perfect_nb_moves, 2, 10);
		}
		
		info += QString(" perfect:%1/%2").arg(normalPerfect).arg(miserePerfect);
		info += QString(" min-max:%1/%2").arg(FTIa.min_moves,2,10).arg(FTIa.max_moves,2,10);
	}
	
	result += info;
	return result;
}

string FullTreeNode::debugStringKind() {
	string result;
	if(computation_kind == CT_kind) {
		result = string("Ct");
	} else
	if(computation_kind == Rct_kind) {
		result = string("Rct");
	}
	return result;
}
string FullTreeNode::debugStringPos() {
	return gameString(positionA);
}

string FullTreeNode::debugStringResult() {
	return RctResult.LtoString();
}
