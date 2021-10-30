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

#ifndef RCT_H
#define RCT_H

#include "../global.h"
#include "../stringconverter.h"

//! Canonical Trees and Canonical Reduced Trees
//! @ingroup NodeGroup
class Rct {
  public :
	Rct() {};
	Rct(nimber);	//Rct corresponding to a Nim-column
	void initFromRct(const Rct &r);		//initialize the Rct from another one
	
	//depth of the game tree
	unsigned char depth;
	//identifier of the tree amongst the trees that have the same depth
	unsigned long identifier;
	//1 if the Rct is the sum of the tree of identifier and Nim-column "1", else 0
	//the real depth of the Rct is depth+sum_with_1
	//Note : always 0 for a CT, except for odd Nim-columns
	bool sum_with_1;
	
	//Misere Win/Loss : Loss==0, Win==1 (only for reduced)
	bool misere_win_loss;
	
	//-------------functions--------------------
	void findWinLoss(const set<Rct> &set_Rct_children);
	void findDepth(const set<Rct> &set_Rct_children);
	
	string LtoString() const;
	
	bool operator==(const Rct& a) const;
	bool operator<(const Rct& a) const;
	
	//-----------------------------------------------------------------------
	//! @name Rct
	//@{
	static bool reduced; //indicate if we are computing CT or Rct
	static int pos_base_num, children_base_num;
	static vector<unsigned long> last_identifier;	//remember the last identifier given to a new Rct, for each depth
	static vector<bool> nimColumnStored;			//indicate if the nimColumn Rct(i) is stored in the Rct/children database or not
	
	static void initRctDatabases();		//initialization of Rct database
	static void storeNimColumn(int n);	//add the nimColumn of size n to the Rct/children database
	
	void deduceRctResult(const set<Rct> &set_Rct_children);
	bool reduce_Rct_1(const set<Rct> &set_Rct_children);
	bool reduce_Rct_2(const set<Rct> &set_Rct_children);
	bool reduce_Rct_3(const set<Rct> &set_Rct_children);
	bool reduce_Rct_4(const set<Rct> &set_Rct_children);
	void createNewRct(const set<Rct> &set_Rct_children);
	//@}
	
	//datastorage index for rct/ct
	static void createDataStorage();
	static int dbPosRctIndex, dbRctChildrenIndex, dbPosCTIndex, dbCTChildrenIndex;
	static const int NimColumnLimit;
};

StringConverter& operator<<(StringConverter& conv, const Rct& x);
StringConverter& operator>>(StringConverter& conv, Rct& x);

Rct RctfromString(const string& stringA);
set<Rct> setRctfromString(const string& stringA);
string setRcttoString(const set<Rct>& the_set);

bool simplify(multiset<Rct>& the_multiset);



#endif
