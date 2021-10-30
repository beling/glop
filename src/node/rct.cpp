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

//Constant values and functions used in the game class

#include "rct.h"
#include "../computation/database.h"
#include "../baseclass/basegame.h"
#include "../stringconverter.h"

//-------------------------------------------------------------------------------------------------
// Constructors / operators
//-------------------------------------------------------------------------------------------------

int Rct::dbPosRctIndex = -1, Rct::dbRctChildrenIndex = -1, Rct::dbPosCTIndex = -1, Rct::dbCTChildrenIndex = -1;

const int Rct::NimColumnLimit=200;

//this function can be called by fulltreenode::createDataStorage or rctmiserenode::createDataStorage
//but we don't know in which order it will be called, so we need to check if the base has already been created or not
//Note : the index of the node corresponding to the databases -1 means that merge is not supported 
void Rct::createDataStorage() {
	if(dbPosRctIndex == -1) {
		//the datastorage has not been created yet
		dbPosRctIndex = db::create(/*stringString*/ 1, QString( "Pos/Rct"), QString( "Position/Rct"), QString("[Position+Rct]"), -1);
		dbRctChildrenIndex = db::create(/*stringString*/ 1, QString( "Rct/Children"), QString( "Rct/children"), QString("[Rct+Rcts_children]"), -1);
		dbPosCTIndex = db::create(/*stringString*/ 1, QString( "Pos/CT"), QString( "Position/CT"), QString("[Position+CT]"), -1);
		dbCTChildrenIndex = db::create(/*stringString*/ 1, QString( "CT/Children"), QString( "CT/children"), QString("[CT+CTs_children]"), -1);
	}
}

//Rct corresponding to a Nim-column
Rct::Rct(nimber column){
	//sum_with_1 : odd nimber = 1 + even nimber
	sum_with_1=column%2;
	//depth : always even, because real_depth = depth + sum_with_1
	depth=column-sum_with_1;
	//a Nim-column has identifier "0" in its level
	identifier=0;
	//misere win/loss, and min or max number of moves
	if(column == 1){
		misere_win_loss=0;
	} else {
		misere_win_loss=1;
	}
}

//initialize the Rct from another one
void Rct::initFromRct(const Rct &r) {
	depth = r.depth;
	identifier = r.identifier;
	sum_with_1 = r.sum_with_1;
	misere_win_loss = r.misere_win_loss;
}

//equal operator
bool Rct::operator==(const Rct& a) const{
	if (depth != a.depth)			return false;
	if (sum_with_1 != a.sum_with_1)		return false;
	if (identifier != a.identifier)		return false;
	return true;
}

//compare operator
bool Rct::operator<(const Rct& a) const{
	if (depth != a.depth) return (depth<a.depth);
	if (sum_with_1 != a.sum_with_1) return (sum_with_1<a.sum_with_1);
	return (identifier<a.identifier);
}

//--------------------------------------------------------------------------
// Initializations
//--------------------------------------------------------------------------

int Rct::pos_base_num;
int Rct::children_base_num;
bool Rct::reduced; //indicate if we are computing CT or Rct
vector<unsigned long> Rct::last_identifier;		//keep the memory of the last identifier given to a new Rct, for each depth
vector<bool> Rct::nimColumnStored;					//indicate if the nimColumn Rct(i) is stored in the Rct/children database or not

//several initializations concerning the databases
void Rct::initRctDatabases() {
	//pointers to the right databases
	if(reduced){
		pos_base_num = dbPosRctIndex;
		children_base_num = dbRctChildrenIndex;
	} else {
		pos_base_num = dbPosCTIndex;
		children_base_num = dbCTChildrenIndex;
	}
	
	//vector to keep the memory of the last identifier given to a new Rct, for each depth
	last_identifier.assign(NimColumnLimit+1,0);
	nimColumnStored.assign(NimColumnLimit+1,false);
	
	//the nim-column 0 always appear in a Ct or Rct computation
	//we need to add manually for Ct computation (TODO : why?)
	storeNimColumn(0);
	
	//----------------------------------------
	//Initialize the vector with the identifiers (in the case of databases added from a file)
	//warning : this code accesses directly to the database, until we find a better way
	map<string, string> &refToChildren = db::storeVect[children_base_num].mapStringString;
	
	//initialize the vector according to the database already loaded
	map<string, string>::const_iterator Rct_it;
	for(Rct_it=refToChildren.begin();Rct_it!=refToChildren.end();Rct_it++){
		Rct Rct_tmp=RctfromString(Rct_it->first);
		if(Rct_tmp.identifier>last_identifier[Rct_tmp.depth]){
			last_identifier[Rct_tmp.depth]=Rct_tmp.identifier;
		}
	}
	//---------------------------------------------
}

//add the nimColumn of size n to the Rct/children database
//we add the nimColumn only if it is not already added
void Rct::storeNimColumn(int n) {
	if(nimColumnStored[n]) return; //the nimColumn is already stored
	
	//add (Nim-column N -> children 0;1;...;N-1) to the database
	set<Rct> da_children;
	for(int i=0;i<n;i++){
		da_children.insert(Rct(i));
	}
	db::add((Rct(n)).LtoString(), setRcttoString(da_children), children_base_num);
	
	nimColumnStored[n] = true;
}

//-------------------------------------------------------------------------
// Computation of a (R)CT from the set of its children
//-------------------------------------------------------------------------

//find which Rct corresponds to this set of children
void Rct::deduceRctResult(const set<Rct> &set_Rct_children) {
	bool result_found = db::findInverse(setRcttoString(set_Rct_children), children_base_num);
	if(result_found) {
		initFromRct( RctfromString(db::foundInverse()) );
		return;
	}
	
	//-----Reduction-----------------------------------------------------------------
	//Find if Pos is "Win" or "Loss" (Loss=0 / Win=1)
	findWinLoss(set_Rct_children);
	
	if(reduced){
		if(!result_found) result_found=reduce_Rct_1(set_Rct_children);
		if(!result_found) result_found=reduce_Rct_2(set_Rct_children);
		if(!result_found) result_found=reduce_Rct_3(set_Rct_children);
		if(!result_found) result_found=reduce_Rct_4(set_Rct_children);
	}
	
	if(result_found) {
		if(identifier==0) {
			//this is a nim column, we add it to the Rct/children database
			if(sum_with_1) {
				storeNimColumn(depth+1);
			} else {
				storeNimColumn(depth);
			}
		}
	}
	
	//-----New Canonical (Reduced) Tree----------------------------------------------
	if(!result_found) createNewRct(set_Rct_children);
}

//compute the misere Win/Loss of a Rct from its children
void Rct::findWinLoss(const set<Rct> &set_Rct_children) {
	misere_win_loss=0;
	set<Rct>::iterator Rct_i;
	for(Rct_i=set_Rct_children.begin();Rct_i!=set_Rct_children.end();Rct_i++){
		if( Rct_i->misere_win_loss == 0 ){	//if at least a child is "Loss"...
			misere_win_loss=1;		//...Pos is "Win".
			return;
		}
	}
}

//Reduction 1 : each child is a Nim_column, and at least one is 0 or 1
//return true if a reduction was possible and modify the Rct consequently
bool Rct::reduce_Rct_1(const set<Rct> &set_Rct_children) {
	bool a_child_is_0_or_1=false;
	
	set<Rct>::iterator Rct_i;
	for(Rct_i=set_Rct_children.begin() ; Rct_i!=set_Rct_children.end(); Rct_i++){
		if(Rct_i->identifier == 0){
			if(Rct_i->depth == 0){
				a_child_is_0_or_1=true;
			}
		} else {
			//one child is not a nim-column, the reduction is not possible
			return false;
		}
	}
	
	if(a_child_is_0_or_1 ){
		int mex=0;
		Rct_i=set_Rct_children.begin();
		while (Rct_i != set_Rct_children.end()){
			if(*Rct_i == Rct(mex)){
				mex++;
				Rct_i++;
			} else{
				Rct_i = set_Rct_children.end();
			}
		}
		if(mex > NimColumnLimit){
			Error(QString("Too big Nim_column (>%1) in Rct::reduce_Rct_1").arg(NimColumnLimit), true);
		} else{
			//initialize the current Rct to Rct(mex)
			initFromRct(Rct(mex) );
			return true;
		}
	}
	
	return false;
}

//Reduction 2 : check if "empty set" is a reducer (possible only if Pos is "Win")
//return true if a reduction was possible and store the result in result_Rct
bool Rct::reduce_Rct_2(const set<Rct> &set_Rct_children) {
	//reduction is possible only if Pos is "Win"
	if(misere_win_loss!=global::Win) return false;
	
	set<Rct>::iterator Rct_i=set_Rct_children.begin();
	while(Rct_i!=set_Rct_children.end()){
		//check if "Nim-column 0" is a child of *Rct_i
		//it is the case iff the first character of the string of the children is 0
		if(db::find(Rct_i->LtoString(), children_base_num)) {
			if( (db::foundString())[0] != '0' ){
				//empty set is not a reducer
				return false;
			}
		} else {
			cout << "Error in Rct::reduce_Rct_2" << endl;
		}
		Rct_i++;
	}
	
	//the reduction is possible, initialize the current Rct to Rct(0)
	initFromRct( Rct(0) );
	return true;
}

//Reduction 3 : check if any set {child_1 ; ... ; child_k} is a reducer
//(with 1 <= k < nb_children)
bool Rct::reduce_Rct_3(const set<Rct> &set_Rct_children) {
	map<string, string>::const_iterator Rct_it;
	set<Rct> reducer;
	set<Rct>::iterator Rct_i=set_Rct_children.begin();
	set<Rct>::iterator Rct_j, Rct_k; Rct_j=Rct_i;
	if(Rct_i != set_Rct_children.end()){
		Rct_j++;
		reducer.insert(*Rct_i);
	}
	string reducer_str=setRcttoString(reducer);
	//at each loop, we test the reducer {child_1 ; ... ; *Rct_i}
	while(Rct_j != set_Rct_children.end()){
		//a reducer is always like : {child_1 ; ... ; child_i}
		//with real_depth(child_1), ... , real_depth(child_i)   <= value
		//and  real_depth(child_i+1), ... , real_depth(child_k) >= value+2
		if( (Rct_i->depth+Rct_i->sum_with_1+1) < (Rct_j->depth+Rct_j->sum_with_1) ){
			//check if the reducer is already in the database
			//it usually is, because it has already been (recursively) computed,
			//but it is possible that it isn't, when the reducer itself is reducible
			//for example : when we try to reduce {1;2+}, we try the reducer {1}
			//but {1} isn't in the database because it is reducible in 0
			if(db::findInverse(reducer_str, children_base_num)){
				bool good_reducer=true;
				Rct foundRct = RctfromString(db::foundInverse());
				for(Rct_k=Rct_j;Rct_k!=set_Rct_children.end() && good_reducer;Rct_k++){
					if(!db::find(Rct_k->LtoString(), children_base_num)) {
						cout << "Error in Rct::reduce_Rct_3" << endl;
					}
					set<Rct> setRcttmp=setRctfromString(db::foundString());
					if(setRcttmp.find(foundRct)==setRcttmp.end() ){
						good_reducer=false;
					}
				}
				if(good_reducer){
					initFromRct( foundRct );
					return true;
				}
			}
		}
		Rct_i++; Rct_j++;
		reducer.insert(*Rct_i);
		reducer_str=setRcttoString(reducer);
	}
	
	return false;
}

//Reduction 4 : try to factorize with Nim-column "1" (ie to write Pos=Pos'+1)
bool Rct::reduce_Rct_4(const set<Rct> &set_Rct_children) {
	//simple cases where factorization is impossible
	if(set_Rct_children.size()<2) return false;
	
	set<Rct>::reverse_iterator Rct_z=set_Rct_children.rbegin();
	set<Rct>::reverse_iterator Rct_y=set_Rct_children.rbegin();
	Rct_y++;
	if( (Rct_z->sum_with_1==1) || (Rct_y->depth==Rct_z->depth) ) return false;
	
	//test if Pos = *Rct_z + 1 (sum of games) by computing the children of (*Rct_z + 1)
	set<Rct> set_Rct_candidate;
	//insert child 1 : *Rct_z
	set_Rct_candidate.insert(*Rct_z);
	if(!db::find(Rct_z->LtoString(), children_base_num)) {
		cout << "Error in Rct::reduce_Rct_4" << endl;
	}
	set<Rct> set_Rct_tmp=setRctfromString(db::foundString());
	set<Rct>::iterator Rct_j;
	//insert other children : son(*Rct_z) + 1
	for(Rct_j=set_Rct_tmp.begin();Rct_j!=set_Rct_tmp.end();Rct_j++){
		Rct grandson=*Rct_j;
		grandson.sum_with_1=1-grandson.sum_with_1;
		set_Rct_candidate.insert(grandson);
	}
	
	//case : Pos = *Rct_z + 1
	if(set_Rct_children==set_Rct_candidate){
		bool misere_wl_save = misere_win_loss;
		initFromRct( *Rct_z );
		sum_with_1=1;
		misere_win_loss=misere_wl_save;
		db::add(LtoString(), setRcttoString(set_Rct_children), Rct::children_base_num);
		return true;
	}
	
	return false;
}

//it's the first time we meet this Rct, so we give him a name
void Rct::createNewRct(const set<Rct> &set_Rct_children) {
	//Find the depth
	findDepth(set_Rct_children);
	
	//sum with 1 : always 0 if not reduced
	//and it's also 0 if reduced, else reduce_Rct_4() would have returned true
	sum_with_1=0;
	
	//Find the identifier (number of the Rct in its level)
	last_identifier[depth]++;
	identifier=last_identifier[depth];
	
	if(identifier>100000000){
		Error(QString("Rct.identifier overflow (>100 millions) in Rct::createNewRct"), true);
	}
	
	//Add to children database
	db::add(LtoString(), setRcttoString(set_Rct_children), children_base_num);
}

//Find the depth : max{children depths}+1
void Rct::findDepth(const set<Rct> &set_Rct_children) {
	set<Rct>::iterator Rct_i;
	depth=0;
	for(Rct_i=set_Rct_children.begin();Rct_i!=set_Rct_children.end();Rct_i++){
		if((Rct_i->depth+Rct_i->sum_with_1) >= depth){
			depth=1+Rct_i->depth+Rct_i->sum_with_1;
		}
	}
	if(depth>NimColumnLimit) {
		Error(QString("Rct depht overflow (>%1) in Rct::findDepth").arg(NimColumnLimit), true);
	}
}

//-------------------------------------------------------------------------
// Simplification of a multiset<Rct> for the misere complex algorothm
//-------------------------------------------------------------------------

//simplify a multiset<Rct> : removes the Nim-columns-1 of each Rct that have such a column,
//and returns 1 if there is an odd number of such removals, 0 if the number is even
//also clears the remaining Nim-columns-0.
bool simplify(multiset<Rct>& the_multiset){
	if(the_multiset.empty()) return 0;
	
	multiset<Rct> new_multi;
	Rct new_Rct;
	multiset<Rct>::iterator Rct_i;
	bool result=0;
	//remove the Nim-columns-1
	for(Rct_i=the_multiset.begin();Rct_i!=the_multiset.end();Rct_i++){
		new_Rct=*Rct_i;
		if(new_Rct.sum_with_1){
			new_Rct.sum_with_1=0;
			//determine if new_Rct is Win or Loss in misere, with the database
			if(new_Rct.identifier==0){
				new_Rct.misere_win_loss=1;
			} else {
				//TODO : test un peu casse-gueule, a ameliorer
				if(!db::find(new_Rct.LtoString(), Rct::dbRctChildrenIndex)){
					new_Rct.misere_win_loss=!(new_Rct.misere_win_loss);
				}
			}
			result=(!result);
		}
		new_multi.insert(new_Rct);
	}
	the_multiset=new_multi;
	//remove the Nim-columns-0
	bool end_of_removal=false;
	while(!end_of_removal){
		if(the_multiset.empty()){
			end_of_removal=true;
		} else {
			if(*(the_multiset.begin()) == Rct(0)){
				the_multiset.erase(the_multiset.begin());
			} else {
				end_of_removal=true;
			}
		}
	}
	//return the parity of Nim-columns-1 removed
	return result;
}

//--------------------------------------------------------------------------
// String conversion functions
//--------------------------------------------------------------------------

//StringConverter for Rct objects
//WARNING + TODO : there should be a simple way to distinguish between nim-colum or general Rct in the string
//the current method based only on the number of token is very risky
StringConverter& operator<<(StringConverter& conv, const Rct& x) {
	if(x.identifier==0){
		//the Rct is a Nim-column
		conv << (int) (x.depth+x.sum_with_1);
	} else {
		conv << (int) x.depth << "-" << x.identifier;
		
		//the following informations are useful only for reduced trees
		if(Rct::reduced) {
			if(x.sum_with_1) conv << "+1";
			conv << "-" << CvCd("BWL") << x.misere_win_loss;
		}
	}
	return conv;
}

StringConverter& operator>>(StringConverter& conv, Rct& x) {
	string a = conv.getFirstToken();
	x = RctfromString(a);
	return conv;
}

//Rct ---> string
string Rct::LtoString() const{
	StringConverter conv;
	conv << *this;
	return conv.getString();
}

//string ---> Rct
//WARNING : this function is valid only if the stringA contains only one Rct.
Rct RctfromString(const string& stringA){
	StringConverter conv(stringA);
	conv >>  CvCd("UT"); //use '-' as a separator between elements;
	
	if(conv.tokenNumber() == 1) { //Nim-column
		unsigned char depth;
		conv >> depth;
		return Rct(depth);
	}
	
	Rct result;
	result.depth=0;
	result.identifier=0;
	result.sum_with_1=0;
	result.misere_win_loss=global::Loss;
	
	conv >> result.depth;
	if((int) conv.testFirstToken().find("+") == -1) {
		conv >> result.identifier;
	} else {
		conv >> CvCd("E+") >> result.identifier >> CvCd("E-");
		conv.getFirstToken(); //erase "1-"
		result.sum_with_1 = 1;
	}
	if(Rct::reduced) {
		conv >> CvCd("BWL") >> result.misere_win_loss;
	}
	return result;
}

//set<Rct> ---> string
string setRcttoString(const set<Rct>& the_set){
	StringConverter conv;
	conv << CvCd("LF") /*no space after the last element*/ << the_set; 
	return conv.getString();
}

//string ---> set<Rct>
set<Rct> setRctfromString(const string& stringA){
	StringConverter conv(stringA);
	set<Rct> result;
	conv >> CvCd("E%") /*to avoid cutting at '-' */ >> result;
	return result;
}
