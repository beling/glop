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

#ifndef SCORENODE_H
#define SCORENODE_H

#include "../baseclass/basenode.h"

//! Node for computing games with a concept of score
//! @ingroup NodeGroup
class ScoreNode : public BaseNode {
  public:
	NODE_REGISTER_MACRO(ScoreNode)
	
	//override the functions of BaseNode
	NODE_FUNCTION_LIST
	//virtual unsigned int traversalHeuristic(); //heuristic for the traversal of the tree
	virtual void prepareNextCycle(); 			//used to move the check database in the normal database during check cycles
	virtual string getPositionName();			//name of the start position set by the user in the interface
	virtual string extraPositionPartTitle();	//title for the column with the part of the node that is not the position
	
	//used to merge databases and files
	virtual bool treatBaseCollision(const string &pos, nimber nBase, nimber nFile, int baseIndex, bool isCheckBase); 
	virtual bool treatBaseCollision(const string &pos, string sBase, string sFile, int baseIndex, bool isCheckBase);
	
	static int dbScoreIndex, dbScoreWIndex, dbAuxLossIndex;		//datastorage indexes for this algorithm
	static const int GivenContract, FindScore;					//computation kind for this algorithm
	
	PositionString positionA;		//position part
	Score contractA;				//contract part
	win_loss winLossResult;			//result type
	bool isFirstChild;				//used to remember whether the node is the first child of its parent or not
	
	void storeWinResult();		//store in the database the fact that the current position is a Win
	bool findKnownResult(const string& pos, ScoreRange& knownResult);
	void purgeWinDatabase();	//purge the win database if the limit is reached
	void purgeLossDatabase();	//purge the loss database if the limit is reached
	void checkLoss(string compressedString, Score maxPossibleScore);
	
	//parameters for this algorithm
	static int given_contract; //contract of the starting position
	static bool computeOutcome; //!< true = compute outcome; false = compute nimber
	static bool storeLossOnly;  //!< indicate if we store only losing positions or losing/winning positions
	static bool separateWinLoss;
	static bool separateFirstLoss;
	static bool separateTmpLoss;//!< separate temporary and permanent positions
	static bool useWinPurge;	//!< activate the purge of the win database when the limit is reached
	static int winPurgeLimit;	//!< limit number of positions before a purge of the win database
	static bool useAuxPurge;	//!< activate the purge of the auxiliary loss database when the limit is reached	
	static int lossPurgeLimit;	//!< limit number of positions before a purge of the auxiliary loss database
};

#endif
