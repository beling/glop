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

#ifndef FULLTREEINFO_H
#define FULLTREEINFO_H

#include "../global.h"

//! Informations that are computed with a full tree exploration
//! @ingroup NodeGroup
class FullTreeInfo {
  public :
	FullTreeInfo();
	
	//Normal Win/Loss : Loss==0, Win==1
	bool normal_win_loss;
	//Misere Win/Loss : Loss==0, Win==1
	bool misere_win_loss;
	
	//nimbers
	nimber normal_nimber;
	nimber misere_nimber;
	
	//maximal number of moves of the game
	unsigned char tree_depth;
	
	//number of moves if the winner plays perfectly
	//global::Nim_unknown if several numbers are possible
	unsigned char normal_perfect_nb_moves;
	unsigned char misere_perfect_nb_moves;
	
	//minimal and maximal number of moves that the first player can force
	unsigned char min_moves;
	unsigned char max_moves;
	
	//number of moves that the first player can force
	set<unsigned char> first_player_forced_nb_moves;
	//number of moves that the second player can force
	set<unsigned char> second_player_forced_nb_moves;
	
	void FTI_computation(const list<FullTreeInfo> &list_FTI_children);
	string toString() const;
};

FullTreeInfo FTIfromString(const string& stringA);

#endif
