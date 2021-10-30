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

#ifndef DOTSANDBOXES_H
#define DOTSANDBOXES_H

#include "splitboard.h"

//! Game of Dots and Boxes
//! @ingroup BoardGroup
class DotsAndBoxes : public SplitBoard {
  public:
	DotsAndBoxes();
	DotsAndBoxes(const Line& board_ini);
	DotsAndBoxes(const string& board_ini);
	
	GAME_REGISTER_MACRO(DotsAndBoxes)
	
	//---------------inherited from BaseGame----------------------
	virtual int gameCategory();
	virtual void computeOptionsSet();
	virtual win_loss Victory(); //indicate which player has won
	virtual void computeOrderParameters(MoveOrdering& result);
	
	virtual bool is_terminal_position(); //indicate whether it is a terminal position or not
	
	virtual ScoreRange availableScore(bool maxScoreOnly); //indicate the number of remaining boxes for this position
	virtual string compress(const string &input);	//(optional) compress the given string
	virtual string uncompress(const string &input);	//(optional) uncompress the given string
	void addUncompressTo(string &output, unsigned char compressedChar);
	virtual bool isPermanentPosition();				//(optional) indicate if the position should be stored permanently or not
	string exportOptionalString();					//(optional) string displayed in the Lives column
	virtual string optionalStringTitle();		//!< title for the column displaying the optional string
	
	//parameter interface
	virtual Interface getParamDef();
	virtual string getPositionName();
	virtual void setParam();
	
	//---------------inherited from Board----------------------
	virtual string gameSpecificString();	//used to add data specific to Dots to the string representation  
	
	//---------------inherited from SplitBoard----------------------
	virtual void gameSpecificCanonization();
	
	//--------functions specific to this game
	static int dotsRows;
	static int dotsColumns;
	static string dotsGivenPosition;
	static bool isAmerican, isSwedish, isIcelandic;
	static bool useUltraWeak;
	static bool useChainOpening;
	static int storeLowCondition;
	static int storeUpCondition;
	
	void initDotsBoard();
	int countEdges();
	void addBoardToOptionsSet();
	int estimateChildrenNumber();
	
	//---------------- White black token theorem ------------
	bool whiteBlackTokenTheorem();
	bool chainOpeningEquivalences(int i0=-1, int j0=-1);
	int ultraWeakTheorem();
	int colorCapturableComponent();
	int colorCapturableComponent_currentBoard();
	bool colorCapturableCoin(int i, int j);
	void attributeColoredComponent();
	void attributeColoredComponent_currentBoard();
	bool searchCoinPair();
	bool searchCoinPair_currentBoard();
	bool isCoinPair(int i, int j);
	bool searchColoredLinked(int i, int j, bool linkedCoinWithColor, int length, int forbiddenDirection=-1);
	bool searchDoublePair();
	bool searchDoublePair_currentBoard();
	bool isDoublePair(int i, int j);
	string coinPairString();
	string linkedCoinPairString();
	string fourCoinsString();
	
	//---------------------- Canonisation ----------------
	int isLinearChain();
	int coinDegree(int i, int j, bool countArrows);
	bool isCaterpillar();
	string canonizeCaterpillar();
	bool canonizeRightAngles();
	bool canonizeRightAngle_oneBlock(int i, int j);
	int distanceToCenter(int i, int j);
	void deleteSmallBoards(int doubleArrows_number, int singleArrowPair_number, int singleDoublePair_number, int doubleArrowPair_number);
	
	int scoreVariation;
	//--------------------display ------------------
	virtual string patternForCell(int i, int j);		//return the name of the pattern for a given cell
};

#endif
