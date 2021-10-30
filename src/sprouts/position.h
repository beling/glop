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

#ifndef POSITION_H
#define POSITION_H

#include "land.h"
#include "../baseclass/basegame.h"

//! Position is a Structure of Land.
//! @ingroup SproutsGroup
class Position : public Structure<Land>, public BaseGame {
  public:
	Position(const Line& init) : Structure<Land>(init) {};
	Position() {};
	
	GAME_REGISTER_MACRO(Position)
	
	//inherited from BaseGame
	virtual void resetFrom(const Line &a);  //!< reset the object
	virtual void resetFrom(const string &a);  //!< reset the object
	virtual bool is_terminal_position();
	virtual win_loss Victory();
	virtual string exportToString();
	virtual Line exportToLine();
	virtual string exportOptionalString();
	virtual string optionalStringTitle();		//!< title for the column displaying the optional string
	
	virtual int gameCategory();
	virtual bool isPaintable();		//!< (optional) indicate if graphics are available
	virtual void computeOrderParameters(MoveOrdering& result);
	virtual int lastCharacter();
	//override terminal_vertex virtual function
	Vertex terminal_vertex();
	
	virtual void canonize();                      //!< canonization
	virtual list<Line> sumComponents();        //!< list of lands (given by their Line representation)
	void compute_children();                  //!< children of the position
	char position_type();                     //!< type of the position (multiple, single, or no land)
	virtual bool isMultiComponents();
	bool isSingleLand();
	bool isEmpty();
	unsigned int estimate_children_number();  //!< estimates the number of children
	unsigned int smallestLandLives();	//!< number of lives of the smallest land
	virtual double traversalHeuristic();		//!< Heuristic for the traversal of the tree
	
	static Line startPosition(int, int);	//sprouts start position
	
	//parameter specific to sprouts
	static bool use_ext_conj;          //use extended conjecture to guide computation
	
	//first draft of basegame
	virtual void computeChildrenSet();
	
	//parameter interface
	static bool isGivenPosition;
	static string givenPosition;
	static int startingSpots, startingTopology;
	virtual Interface getParamDef();
	virtual void setParam();
	virtual string getPositionName();
};

#endif
