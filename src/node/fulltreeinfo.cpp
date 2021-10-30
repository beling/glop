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

#include "fulltreeinfo.h"
#include "../computation/database.h"
#include "../stringconverter.h"

//constructor : corresponding to the terminal position
FullTreeInfo::FullTreeInfo(){
	normal_win_loss=global::Loss;
	misere_win_loss=global::Win;
	normal_nimber=0;
	misere_nimber=1;
	tree_depth=0;
	normal_perfect_nb_moves=0;
	misere_perfect_nb_moves=0;
	min_moves=0;
	max_moves=0;
	first_player_forced_nb_moves.clear();
	first_player_forced_nb_moves.insert(0);
	second_player_forced_nb_moves.clear();
	second_player_forced_nb_moves.insert(0);
}

//compute information from the children's informations
void FullTreeInfo::FTI_computation(const list<FullTreeInfo> &list_FTI_children) {
	normal_win_loss=global::Loss;
	misere_win_loss=global::Loss;
	set<nimber> normal_nimber_set;
	set<nimber> misere_nimber_set;
	tree_depth=0;
	set<unsigned char> normal_perfect_set;
	set<unsigned char> misere_perfect_set;
	normal_perfect_nb_moves=global::Nim_unknown;
	misere_perfect_nb_moves=global::Nim_unknown;
	min_moves=255;	//value bigger than the potential result
	max_moves=0;
	first_player_forced_nb_moves.clear();
	second_player_forced_nb_moves.clear();
	set<unsigned char>::iterator Ci;
	
	//case of the terminal position
	if(list_FTI_children.empty()){
		misere_win_loss=global::Win;
		min_moves=0;
		normal_perfect_nb_moves=0;
		misere_perfect_nb_moves=0;
		first_player_forced_nb_moves.insert(0);
		second_player_forced_nb_moves.insert(0);
	}
	
	list<FullTreeInfo>::const_iterator FTI_i;
	//first loop on children :
	//computation of normal_win_loss, misere_win_loss, tree_depth, min_moves, max_moves
	//pre-computation of normal_nimber, misere_nimber
	for(FTI_i=list_FTI_children.begin();FTI_i!=list_FTI_children.end();FTI_i++){
		if(FTI_i->normal_win_loss==global::Loss){
			normal_win_loss=global::Win;
		}
		if(FTI_i->misere_win_loss==global::Loss){
			misere_win_loss=global::Win;
		}
		normal_nimber_set.insert(FTI_i->normal_nimber);
		misere_nimber_set.insert(FTI_i->misere_nimber);
		if(FTI_i->tree_depth>=tree_depth){
			tree_depth=FTI_i->tree_depth+1;
		}
		if( (FTI_i->min_moves+1) > max_moves){
			max_moves=FTI_i->min_moves+1;
		}
		if( (FTI_i->max_moves+1) < min_moves){
			min_moves=FTI_i->max_moves+1;
		}
		//computation of the number of moves that the first player can force
		for(Ci=FTI_i->second_player_forced_nb_moves.begin();Ci!=FTI_i->second_player_forced_nb_moves.end();Ci++){
			first_player_forced_nb_moves.insert(*Ci+1);
		}
		//computation of the number of moves that the second player can force
		if(FTI_i==list_FTI_children.begin()){
			for(Ci=FTI_i->first_player_forced_nb_moves.begin();Ci!=FTI_i->first_player_forced_nb_moves.end();Ci++){
				second_player_forced_nb_moves.insert(*Ci+1);
			}
		} else{
			for(Ci=second_player_forced_nb_moves.begin();Ci!=second_player_forced_nb_moves.end();){
				if(FTI_i->first_player_forced_nb_moves.find(*Ci-1)==FTI_i->first_player_forced_nb_moves.end()){
					second_player_forced_nb_moves.erase(Ci++);	//ugly
				} else {
					Ci++;
				}
			}
		}
	}
	
	//computation of normal_nimber and misere_nimber
	normal_nimber=0;
	misere_nimber=0;
	set<nimber>::iterator Ni=normal_nimber_set.begin();
	while (Ni != normal_nimber_set.end()){
		if(*Ni == normal_nimber){
			normal_nimber++;
			Ni++;
		} else{
			Ni=normal_nimber_set.end();
		}
	}
	Ni=misere_nimber_set.begin();
	if(Ni==misere_nimber_set.end()) misere_nimber=1;	//misere_nimber==1 for the empty position
	while (Ni != misere_nimber_set.end()){
		if(*Ni == misere_nimber){
			misere_nimber++;
			Ni++;
		} else{
			Ni=misere_nimber_set.end();
		}
	}
	
	//second loop on children :
	//pre-computation of normal_perfect_nb_moves, misere_perfect_nb_moves
	for(FTI_i=list_FTI_children.begin();FTI_i!=list_FTI_children.end();FTI_i++){
		if(normal_win_loss==global::Loss || FTI_i->normal_win_loss==global::Loss){
			normal_perfect_set.insert(FTI_i->normal_perfect_nb_moves);
		}
		if(misere_win_loss==global::Loss || FTI_i->misere_win_loss==global::Loss){
			misere_perfect_set.insert(FTI_i->misere_perfect_nb_moves);
		}
	}
	
	//computation of normal_perfect_nb_moves, misere_perfect_nb_moves
	if(normal_perfect_set.size()==1 && *(normal_perfect_set.begin())!=global::Nim_unknown){
		normal_perfect_nb_moves=*(normal_perfect_set.begin())+1;
	}
	if(misere_perfect_set.size()==1 && *(misere_perfect_set.begin())!=global::Nim_unknown){
		misere_perfect_nb_moves=*(misere_perfect_set.begin())+1;
	}
}

string FullTreeInfo::toString() const{
	StringConverter conv;
	conv << CvCd("UT"); //use '-' as a separator between elements;
	conv << normal_nimber << misere_nimber << tree_depth;
	if(normal_perfect_nb_moves==global::Nim_unknown){
		conv << "?";
	} else {
		conv << normal_perfect_nb_moves;
	}
	if(misere_perfect_nb_moves==global::Nim_unknown){
		conv << "?";
	} else {
		conv << misere_perfect_nb_moves;
	}
	conv << min_moves << max_moves;
	conv << CvCd("S.") << first_player_forced_nb_moves;
	conv << CvCd("UF"); //don't use '-' after the last element;
	conv << second_player_forced_nb_moves;
	return conv.getString();
}

//string ---> FullTreeInfo
FullTreeInfo FTIfromString(const string& stringA){
	FullTreeInfo result;
	result.first_player_forced_nb_moves.clear();
	result.second_player_forced_nb_moves.clear();
	StringConverter conv(stringA);
	conv >> CvCd("UT"); //use '-' as a separator between elements;
	conv >> result.normal_nimber >> result.misere_nimber >> result.tree_depth; 
	if(conv.testFirstToken() == string("?")){
		result.normal_perfect_nb_moves=global::Nim_unknown;
		conv.getFirstToken(); //delete the "?"
	} else {
		conv >> result.normal_perfect_nb_moves;
	}
	if(conv.testFirstToken() == string("?")){
		result.misere_perfect_nb_moves=global::Nim_unknown;
		conv.getFirstToken(); //delete the "?"
	} else {
		conv >> result.misere_perfect_nb_moves;
	}
	conv >> result.min_moves >> result.max_moves;
	conv >> CvCd("S.") >> result.first_player_forced_nb_moves;
	conv >> result.second_player_forced_nb_moves;
	
	//init the values of normal_win_loss and misere_win_loss
	if(result.normal_nimber==0){
		result.normal_win_loss=global::Loss;
	} else {
		result.normal_win_loss=global::Win;
	}
	if(result.misere_nimber==0){
		result.misere_win_loss=global::Loss;
	} else {
		result.misere_win_loss=global::Win;
	}
	
	return result;
}
