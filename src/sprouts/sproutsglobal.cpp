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

//Constant values and functions used in the whole project

#include "sproutsglobal.h"
#include <QString>

//special characters used in string
#define Char_End_Boundary '.'
#define Char_Topo_Region  '@'
#define Char_End_Region   '|'
#define Char_End_Land     '+'
#define Char_Empty_Position '!'

#define old_End_Boundary '.'
#define old_Topo_Region  '{'
#define old_End_Region   '}'
#define old_End_Land     ']'
#define old_End_Position '!'

//----------Special vertices values----------------------------------------------------------------

//0, 1, 2 and 3 are reserved as generic vertices

//"normal" vertex range from First_vtx to Last_vtx :
//1-boundary, 2-boundary, 2-region vertices, and the 3 special vertices for children computation
const Vertex sproutsGlobal::First_vtx=4;
const Vertex sproutsGlobal::Last_vtx=306;

//1-boundary, 2-boundary and 2-region vertices (2-boundary vertices are useful only for surfaces)
//max : 100 of each type, it can be easily increased
const Vertex sproutsGlobal::First_1bnd_vtx=4;
const Vertex sproutsGlobal::Last_1bnd_vtx=103;
const Vertex sproutsGlobal::First_2bnd_vtx=104;
const Vertex sproutsGlobal::Last_2bnd_vtx=203;
const Vertex sproutsGlobal::First_2reg_vtx=204;
const Vertex sproutsGlobal::Last_2reg_vtx=303;

//Child_vtx_1, Child_vtx_2, Child_vtx_3 are reserved letters used to name a new vertex
//they are used in Boundary::compute_1bnd_children() and Boundary::compute_2bnd_children()
const Vertex sproutsGlobal::Child_vtx_1=304;
const Vertex sproutsGlobal::Child_vtx_2=305;
const Vertex sproutsGlobal::Child_vtx_3=306;

//Special vertices indicating the end of a structure in a vertices list representation
const Vertex sproutsGlobal::End_Boundary=307;
const Vertex sproutsGlobal::Topo_Region=308;	//useful only with surfaces
const Vertex sproutsGlobal::End_Region=309;
const Vertex sproutsGlobal::End_Land=310;
const Vertex sproutsGlobal::End_Position=311;

//Vertices for describing topology of surfaces
//Warning ! They mustn't have the same numbers as the End_Structure characters
//because of Structure<T>::update_structure()
const Vertex sproutsGlobal::Sphere=10000;
//Torii : T1=10001, T2=10002 ...
const Vertex sproutsGlobal::First_Torus=10001;
//Crosscaps : P1=9999, P2=9998 ...
const Vertex sproutsGlobal::First_Crosscap=9999;

//----------constant values to indicate the kind of position---------------------------------------
//used as return values of Position::position_type()

//empty position
const char sproutsGlobal::Empty_position=1;

//position is constituted of a single land
const char sproutsGlobal::Single_land=2;

//position is constituted of two or more lands
const char sproutsGlobal::Multi_lands=3;


//---------------useful function to avoid heavy syntax of insert--------------------------------
//add the list of vertex lineB to the list of vertex lineA
void addLine(Line& lineA, const Line& lineB) {
	lineA.insert(lineA.end(), lineB.begin(), lineB.end());
}

//the following functions can be used to update a list quickly by reusing already allocated memory
//must be called before beginning an update of lineA
//initialize the virtual end
void quick_clear(Line& lineA) {
	sproutsGlobal::line_virtual_end = lineA.begin(); //the virtual end is at the beginning of the list
}

//to insert a line LineB at the virtual end of lineA
void quick_insert(Line& lineA, const Line& lineB) {
	Line::const_iterator Bi;
	Line::iterator lA_end = lineA.end();
	Line::const_iterator lB_end = lineB.end();
	for(Bi=lineB.begin(); Bi!=lB_end && sproutsGlobal::line_virtual_end!= lA_end; Bi++) {
		*sproutsGlobal::line_virtual_end = *Bi; //update the element pointed by the virtual end with Bi
		sproutsGlobal::line_virtual_end++;      //increment the virtual end
	}
	
	//if the copy process is not finished, insert [Bi...lineB.end()[ with the usual insert function
	if(Bi!=lB_end) {
		lineA.insert(lA_end, Bi, lB_end);
		sproutsGlobal::line_virtual_end = lineA.end(); //update the virtual end (warning lineA.end() has changed)
	}
}

//similar function to insert a range [l_begin, l_end [
void quick_insert(Line& lineA, Line::const_iterator l_begin, Line::const_iterator l_end) {
	Line::const_iterator Bi;
	Line::iterator lA_end = lineA.end();
	for(Bi=l_begin; Bi!=l_end && sproutsGlobal::line_virtual_end!= lA_end; Bi++) {
		*sproutsGlobal::line_virtual_end = *Bi; //update the element pointed by the virtual end with Bi
		sproutsGlobal::line_virtual_end++;      //increment the virtual end
	}
	
	//if the copy process is not finished, insert [Bi...lineB.end()[ with the usual insert function
	if(Bi!=l_end) {
		lineA.insert(lA_end, Bi, l_end);
		sproutsGlobal::line_virtual_end = lineA.end(); //update the virtual end (warning lineA.end() has changed)
	}
}

//to end the update process
//remove the unused objects after the virtual end
void quick_end(Line& lineA) {
	lineA.erase (sproutsGlobal::line_virtual_end, lineA.end());
}

Line::iterator sproutsGlobal::line_virtual_end;


//-------------------------------------------------------------------------------------------------
// Line <---> string conversion
//-------------------------------------------------------------------------------------------------

//! Convert a list of vertices in a string
string toString(const Line& r_vtx) {
	string result;
	Line::const_iterator s=r_vtx.begin();
	bool topo_info=false;
	bool topo_info_2=false;
	
	//case of the empty position : represented by a void Region
	if(*s==sproutsGlobal::End_Position){
		result += Char_Empty_Position;
		return result;
	}
	
	//determine if we can use compact notation (a-l, z-o, A-Y)
	//or if we must use extended one (m(15), n(24) or Z(53))
	bool compact_1bnd=true;
	bool compact_2bnd=true;
	bool compact_2reg=true;
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		if(*s>=sproutsGlobal::First_1bnd_vtx+12 && *s<=sproutsGlobal::Last_1bnd_vtx) compact_1bnd=false;
		if(*s>=sproutsGlobal::First_2bnd_vtx+12 && *s<=sproutsGlobal::Last_2bnd_vtx) compact_2bnd=false;
		if(*s>=sproutsGlobal::First_2reg_vtx+25 && *s<=sproutsGlobal::Last_2reg_vtx) compact_2reg=false;
	}
	
	for(s=r_vtx.begin(); s!=r_vtx.end(); s++) {
		if(topo_info){
			//what follows is only for one character after Topo_Region
			unsigned int genus;
			if(*s<=sproutsGlobal::First_Crosscap) {	//non-orientable surface
				result += '-';
				genus = sproutsGlobal::First_Crosscap-*s+1;
			} else {				//orientable surface
				genus = *s-sproutsGlobal::First_Torus+1;
			}
			result += int_to_string(genus);
			topo_info=false;
			topo_info_2=true;
		} else {
		
		switch(*s) {
			//---only for tests
			case sproutsGlobal::Child_vtx_1 : result += '-'; break;
			case sproutsGlobal::Child_vtx_2 : result += '='; break;
			case sproutsGlobal::Child_vtx_3 : result += '#'; break;
			//---
			case sproutsGlobal::Topo_Region  : result.resize(result.length()-1); result += Char_Topo_Region; topo_info=true; break;
			case sproutsGlobal::End_Boundary : result += Char_End_Boundary; break;
			case sproutsGlobal::End_Region   : if(!topo_info_2) result.resize(result.length()-1); else topo_info_2=false;
							   result += Char_End_Region;   break;
			case sproutsGlobal::End_Land     : result.resize(result.length()-1); result += Char_End_Land;     break;
			case sproutsGlobal::End_Position : result.resize(result.length()-1); break;
			case 0 : result += '0'; break;
			case 1 : result += '1'; break;
			case 2 : result += '2'; break;
			case 3 : result += '3'; break;
			default:
			if (is_1bnd_vtx(*s)) {
				if (compact_1bnd) {
					//First_1bnd_vtx->a, First_1bnd_vtx+1->b, etc
					result += (unsigned char) (*s-sproutsGlobal::First_1bnd_vtx+'a');
				} else {
					result += 'm';
					result += '(';
					result += int_to_string(*s-sproutsGlobal::First_1bnd_vtx+1);
					result += ')';
				}
			} else if (is_2bnd_vtx(*s)) {
				if (compact_2bnd) {
					//First_2bnd_vtx->z, First_2bnd_vtx+1->y, etc
					result += (unsigned char) (sproutsGlobal::First_2bnd_vtx-*s+'z');
				} else {
					result += 'n';
					result += '(';
					result += int_to_string(*s-sproutsGlobal::First_2bnd_vtx+1);
					result += ')';
				}
			} else if (is_2reg_vtx(*s)) {
				if (compact_2reg) {
					//First_2reg_vtx->A, First_2reg_vtx+1->B, etc
					result += (unsigned char) (*s-sproutsGlobal::First_2reg_vtx+'A');
				} else {
					result += 'Z';
					result += '(';
					result += int_to_string(*s-sproutsGlobal::First_2reg_vtx+1);
					result += ')';
				}
			}
		}
		
		}
	}
	
	return zero_notation(result);
}

//! Convert a string to a list of vertex
Line fromString(const string& stringA) {
	//-----------
	//if we are converting old files to new files, use the old fromString function (rev 1453)
	if(Parameter::isConvertingFiles) return oldFromString(stringA);
	//------------
	
	Line r_vtx;		//returned vertices list
	
	//case of the empty position
	if(stringA.size()==1) {
		if(stringA[0]==Char_Empty_Position) {
			r_vtx.push_back(sproutsGlobal::End_Position);
			return r_vtx;
		}
	}
	
	string st=expand_zero(stringA);
	
	unsigned char c;
	
	Vertex genus=0;
	bool topo_info=false;
	bool orientable=true;
	
	bool extended_1bnd=false;
	bool extended_2bnd=false;
	bool extended_2reg=false;
	Vertex extended_vtx=0;
	
	int i;
	int n=st.length();
	
	for(i=0;i<n;i++) {
		c=st[i];
		if(topo_info){
			if(c=='-') {
				orientable=false;
			} else
			if(c>='0' && c<='9') {
				genus *= 10;
				genus += (c-'0');
			} else {
				if(orientable){
					r_vtx.push_back(sproutsGlobal::First_Torus-1+genus);
				} else {
					r_vtx.push_back(sproutsGlobal::First_Crosscap+1-genus);
				}
				topo_info=false; orientable=true; genus=0;
				r_vtx.push_back(sproutsGlobal::End_Region);
				if(c==Char_End_Land) r_vtx.push_back(sproutsGlobal::End_Land);
			}
		} else if (extended_1bnd || extended_2bnd || extended_2reg){
			switch(c) {
				case '(' : break;
				case ')' :	if(extended_1bnd){
							r_vtx.push_back(extended_vtx+sproutsGlobal::First_1bnd_vtx-1);
							extended_1bnd=false;
						} else if (extended_2bnd) {
							r_vtx.push_back(extended_vtx+sproutsGlobal::First_2bnd_vtx-1);
							extended_2bnd=false;
						} else {
							r_vtx.push_back(extended_vtx+sproutsGlobal::First_2reg_vtx-1);
							extended_2reg=false;
						}
						extended_vtx=0; break;
				default :
				extended_vtx *= 10;
				extended_vtx += (c-'0');
			}
		} else {
			switch(c) {
				case Char_Topo_Region  :  r_vtx.push_back(sproutsGlobal::End_Boundary);
							  r_vtx.push_back(sproutsGlobal::Topo_Region); topo_info=true; break;
				case Char_End_Boundary :  r_vtx.push_back(sproutsGlobal::End_Boundary); break;
				case Char_End_Region   :  r_vtx.push_back(sproutsGlobal::End_Boundary);
							  r_vtx.push_back(sproutsGlobal::End_Region);   break;
				case Char_End_Land     :  r_vtx.push_back(sproutsGlobal::End_Boundary);
							  r_vtx.push_back(sproutsGlobal::End_Region);
							  r_vtx.push_back(sproutsGlobal::End_Land);     break;
				case '0' : r_vtx.push_back(0); break;
				case '1' : r_vtx.push_back(1); break;
				case '2' : r_vtx.push_back(2); break;
				case '3' : r_vtx.push_back(3); break;
				default:
				if(c>='a' && c <='l') {
					//a->First_1bnd_vtx, b->First_1bnd_vtx+1, etc
					r_vtx.push_back(c-'a'+sproutsGlobal::First_1bnd_vtx);
				} else if(c>='o' && c <='z') {
					//z->First_2bnd_vtx, y->First_2bnd_vtx+1, etc
					r_vtx.push_back('z'-c+sproutsGlobal::First_2bnd_vtx);
				} else if(c>='A' && c <='Y') {
					//A->First_2reg_vtx, B->First_2reg_vtx+1, etc
					r_vtx.push_back(c-'A'+sproutsGlobal::First_2reg_vtx);
				} else if(c=='m') {
					//extended 1bnd vertices : m(1), m(2)... (m(1) corresponds to a,...)
					extended_1bnd=true;
				} else if(c=='n') {
					//extended 2bnd vertices : n(1), n(2)... (n(1) corresponds to z,...)
					extended_2bnd=true;
				} else if(c=='Z') {
					//extended 2reg vertices : Z(1), Z(2)... (Z(1) corresponds to A,...)
					extended_2reg=true;
				}
			}
		}
	}
	
	if(topo_info){
		 if(orientable){
			r_vtx.push_back(sproutsGlobal::First_Torus-1+genus);
		} else {
			r_vtx.push_back(sproutsGlobal::First_Crosscap+1-genus);
		}
	} else {
		r_vtx.push_back(sproutsGlobal::End_Boundary);
	} 
	r_vtx.push_back(sproutsGlobal::End_Region);
	r_vtx.push_back(sproutsGlobal::End_Land);
	r_vtx.push_back(sproutsGlobal::End_Position);
	return r_vtx;
}


//-------------------------------------------------------------------------------------------------
// Short zero notation : conversions between 0.0.0. and 0*3.
//-------------------------------------------------------------------------------------------------

//return a string with n zeros : "0.0.(...).0"
string list_of_zero( int n) {
	string result;
	
	if(n==0) return result;
	
	result += '0';
	
	for(int i=1; i<n; i++) {
		result += Char_End_Boundary;
		result += '0';
	}
	
	return result;
}


//short notation of positions with a lot of zeros
//it aims at visual comfort in the interface
string zero_notation(const string& stringA) {
	string result;
	unsigned char c;
	
	//"pattern" indicates where we are in patterns like "0.0.0."
	//0 means we are not in the pattern
	//1 means we just found a first "0"
	//2 means we just found a "." after the first "0"
	//3 means we just found a new "0", at least the second
	//4 means we just found a "." after a "0" (at least the second "0")
	int pattern=0;
	
	int n=0;	//n is the number of "0" appearing in the pattern

	int i;
	int lg=stringA.length();
	for(i=0;i<lg;i++) {
		c=stringA[i];
		if(pattern==0 && c=='0') {
			pattern=1;
			n=1;
			result += c;
		} else 
		if(pattern==0 && c!='0') {
			result += c;
		} else 
		if(pattern==1 && c==Char_End_Boundary){
			pattern=2;
		} else
		if(pattern==1 && c!=Char_End_Boundary){
			pattern=0;
			n=0;
			result += c;
		} else
		if(pattern==2 && c=='0'){
			//this is a new 0 in the pattern
			pattern=3;
			n=2;
			result += "*";
		} else
		if(pattern==2 && c!='0'){
			pattern=0;
			n=0;
			result += Char_End_Boundary; result += c;
		} else
		if(pattern==3 && c==Char_End_Boundary){
			pattern=4;
		} else
		if(pattern==3 && c!=Char_End_Boundary){	//end of the pattern
			pattern=0;
			result += (QString("%1").arg(n, 0, 10)).toStdString();	//print the number of zeros after "*"
			n=0;
			result += c;
		} else
		if(pattern==4 && c=='0'){
			pattern=3;
			n++;
		} else
		if(pattern==4 && c!='0'){	//end of the pattern
			pattern=0;
			result += (QString("%1").arg(n, 0, 10)).toStdString();	//print the number of zeros after "*"
			result += Char_End_Boundary; result += c;
		}
	}
	
	if(pattern==3) result += (QString("%1").arg(n, 0, 10)).toStdString();	//print the number of zeros after "*"
	
	return result;
}


//convert all "0*n" in a list of n zeros : "0.0.(...).0"
string expand_zero(const string& stringA) {
	string result;
	unsigned char c;
	
	//indicate where we are in the pattern "0*n"
	//0 means we are not in the pattern
	//1 means we found the first "0"
	//2 means we found the "*"
	int pattern=0;
	int n=0;	//n is the value appearing in the pattern

	int i;
	int lg=stringA.length();
	for(i=0;i<lg;i++) {
		c=stringA[i];

		if(pattern==0 && c=='0') {
			pattern=1;
		} else 
		if(pattern==0 && c!='0') {
			result += c;
		} else 
		if(pattern==1 && c!='*'){
			//it means it was not the pattern but simply a "0."
			result += '0';	//'0' wasn't copied because we thought to be in the pattern
			result += c;
			pattern=0;
		} else
		if(pattern==1 && c=='*'){
			pattern=2;
		} else
		if(pattern==2 && c>='0' && c<='9'){
			n = n*10 + (int) (c-'0');
		} else {	//pattern==2 && c!='0...9'
			result += list_of_zero(n);
			result += c;
			pattern=0;
			n=0;
		}
	}
	
	if(pattern==1) result += '0';
	if(pattern==2) result += list_of_zero(n);
	
	return result;
}

//-------------------------------------------------------------------------------------------------
// Handle vertices
//-------------------------------------------------------------------------------------------------

//Test if a vertex is 1-boundary
bool is_1bnd_vtx(const Vertex& s) {
	return (s>=sproutsGlobal::First_1bnd_vtx && s<=sproutsGlobal::Last_1bnd_vtx);
}

//Test if a vertex is 2-boundary (only for surfaces)
bool is_2bnd_vtx(const Vertex& s) {
	return (s>=sproutsGlobal::First_2bnd_vtx && s<=sproutsGlobal::Last_2bnd_vtx);
}

//Test if a vertex is 2-region
bool is_2reg_vtx(const Vertex& s) {
	return (s>=sproutsGlobal::First_2reg_vtx && s<=sproutsGlobal::Last_2reg_vtx);
}

//Test if a vertex is a temporary one
bool is_temp_vtx(const Vertex& s) {
	return (s>=sproutsGlobal::Child_vtx_1 && s<=sproutsGlobal::Child_vtx_3);
}

//Test if a vertex is a letter (temp ok, but neither generic nor End*)
bool is_letter_vtx(const Vertex& s) {
	return (s>=sproutsGlobal::First_vtx && s<=sproutsGlobal::Last_vtx);
}

//Test if a vertex is a real one (generic, one_life, temp : ok, but not End_* vertices)
bool is_real_vtx(const Vertex& s) {
	return (s<=sproutsGlobal::Last_vtx);
}

//test if there is a one-Boundary Vertex in a Line
bool exist_1bnd(const Line& r_vtx) {
	bool result=false;
	Line::const_iterator s;
	
	for(s=r_vtx.begin(); s!=r_vtx.end() && !result; s++) {
		result=is_1bnd_vtx(*s);
	}
	
	return result;
}

//test if there is a two-Boundaries Vertex in a Line
bool exist_2bnd(const Line& r_vtx) {
	bool result=false;
	Line::const_iterator s;
	
	for(s=r_vtx.begin(); s!=r_vtx.end() && !result; s++) {
		result=is_2bnd_vtx(*s);
	}
	
	return result;
}


//-------------------------------------------------------------------------------------------------
// Other functions
//-------------------------------------------------------------------------------------------------

//power of 2 (math function used in Region_spr::partition)
// overflow needs treatment in Region_spr::partition because it could cause false children computations
unsigned long powers_of_two(int n) {
	unsigned long result=1;
	for(int i=0; i<n; i++) {
		if (result*2<result) {
			Error(QString("powers_of_two : too big number to compute the powers of 2"), true);
		}
		
		result=result*2;
	}
	
	return result;
}

//power of a number b (math function used in Region_spr::estimate_children_number)
//note : this function is false when n is too big, but this is not a problem when used in Region_spr::estimate_children_number
unsigned long powers_estimate(int b, int n) {
	unsigned long result=1;
	for(int i=0; i<n; i++) {
		result=result*b;
	}
	
	return result;
}


//Return true if as given position contains at least one region with 6 unused spots
//check that the position begins with "0.0.0.0.0.0."
bool has6zeros(const Line & r_som) {
	if (r_som.size()<13) return false;
	
	Line::const_iterator s=r_som.begin();
	int i;
	for(i=0; i<6; i++) {
		if (*s!=0) return false;
		s++;
		if (*s!=sproutsGlobal::End_Boundary) return false;
		s++;
	}
	
	return true;
}

//delete 6 unused spots from a given position
//the result may need a canonization if there is another region with more zeros than the new one
//TODO : vérifier que l'adaptation pour la topo fonctionne
Line delete6zeros(const Line& r_som) {
	int i;
	Line L_tmp=r_som;
	
	//delete the first 12 vertex (unused spot = 0.)
	for(i=0; i<12; i++) {
		L_tmp.pop_front();
	}
	//delete the possible end of region vertex (if the region became empty)
	if (L_tmp.front()==sproutsGlobal::Topo_Region) {
		while (L_tmp.front()!=sproutsGlobal::End_Region){
			L_tmp.pop_front();
		}
	}
	if (L_tmp.front()==sproutsGlobal::End_Region) {
		L_tmp.pop_front();
	}
	//canonisation of the resulting position
	//Position P_tmp(L_tmp);
	//P_tmp.canonize();
	
	return(L_tmp);
}

/* Reminder :
const Vertex sproutsGlobal::First_1bnd_vtx=4;
const Vertex sproutsGlobal::Last_1bnd_vtx=103;
const Vertex sproutsGlobal::First_2bnd_vtx=104;
const Vertex sproutsGlobal::Last_2bnd_vtx=203;
const Vertex sproutsGlobal::First_2reg_vtx=204;
const Vertex sproutsGlobal::Last_2reg_vtx=303;
*/

//order on the Lines, used for canonization (sort)
//first, use the pseudo-order : 0<1<2<a=b=c...<z=y=x...<A=B=C...
//if the pseudo-order doesn't allow to distinguish the Lines,
//use the lexicographical order 0<1<2<a<b<c...<z<y<x...<A<B<C...
//interest : with pseudo order, we get ABC|DEF|AC|BD|EF or ...|1C2BA|...
//with classical lexicographical order, we get ABC|AC|BD|DEF|EF or ...|1AB2C|...
//(remark : this function is always called on a Line without any temp vertex)
bool lineOrder(const Line& La, const Line& Lb) {
	//first, use the pseudo-order : 0<1<2<a=b=c...<z=y=x...<A=B=C...
	Line::const_iterator Va, Vb;
	for(Va=La.begin(), Vb=Lb.begin(); Va!=La.end() && Vb !=Lb.end(); Va++, Vb++) {
		if(!is_letter_vtx(*Va) || !is_letter_vtx(*Vb)) {
			//one vertex is not a letter : we use the normal equality
			if(*Va != *Vb) return (*Va < *Vb);
		} else {
			//the two vertices are letters : we compare only the range
			if( is_1bnd_vtx(*Va) && !(is_1bnd_vtx(*Vb)) ) {
				//Va is in the first range and Vb not, it means Va < Vb
				return true;
			}
			if( is_2bnd_vtx(*Va) && !(is_2bnd_vtx(*Vb)) ) {
				//Va is in the second range and Vb not
				//it implies Va < Vb if and only if Vb is in the third range
				return is_2reg_vtx(*Vb);
			}
			if( is_2reg_vtx(*Va) && !(is_2reg_vtx(*Vb)) ) {
				//Va is in the third range and Vb is not, it means that Vb < Va
				return false;
			}
		}
	}
	
	//the two Lines are equal for the pseudo-order : use lexicographical order
	return La<Lb;
}

//--------------------------------------------------------------------------------
//Old fromString function (rev 1453), for compatibility with old databases
//Note : can be deleted when the new function is considered stable

//! Convert a string to a list of vertex
Line oldFromString(const string& stringA) {
	Line r_vtx;		//returned vertices list
	unsigned char c;
	
	Vertex genus=0;
	bool topo_info=false; bool orientable=true;
	
	bool extended_1bnd=false;
	bool extended_2bnd=false;
	bool extended_2reg=false;
	Vertex extended_vtx=0;
	
	int i;
	int n=stringA.length();
	
	for(i=0;i<n;i++) {
		c=stringA[i];
		if(topo_info){
			switch(c) {
				case '-' : orientable=false; break;
				case old_End_Region :
					if(orientable){
						r_vtx.push_back(sproutsGlobal::First_Torus-1+genus);
					} else {
						r_vtx.push_back(sproutsGlobal::First_Crosscap+1-genus);
					}
					topo_info=false; orientable=true; genus=0;
					r_vtx.push_back(sproutsGlobal::End_Region); break;
				default :
				genus *= 10;
				genus += (c-'0');
			}
		} else if (extended_1bnd || extended_2bnd || extended_2reg){
			switch(c) {
				case '(' : break;
				case ')' :	if(extended_1bnd){
							r_vtx.push_back(extended_vtx+sproutsGlobal::First_1bnd_vtx-1);
							extended_1bnd=false;
						} else if (extended_2bnd) {
							r_vtx.push_back(extended_vtx+sproutsGlobal::First_2bnd_vtx-1);
							extended_2bnd=false;
						} else {
							r_vtx.push_back(extended_vtx+sproutsGlobal::First_2reg_vtx-1);
							extended_2reg=false;
						}
						extended_vtx=0; break;
				default :
				extended_vtx *= 10;
				extended_vtx += (c-'0');
			}
		} else {
			switch(c) {
				case old_Topo_Region  : r_vtx.push_back(sproutsGlobal::Topo_Region); topo_info=true; break;
				case old_End_Boundary : r_vtx.push_back(sproutsGlobal::End_Boundary); break;
				case old_End_Region   : r_vtx.push_back(sproutsGlobal::End_Region);   break;
				case old_End_Land     : r_vtx.push_back(sproutsGlobal::End_Land);     break;
				case old_End_Position : r_vtx.push_back(sproutsGlobal::End_Position); break;
				case '0' : r_vtx.push_back(0); break;
				case '1' : r_vtx.push_back(1); break;
				case '2' : r_vtx.push_back(2); break;
				case '3' : r_vtx.push_back(3); break;
				default:
				if(c>='a' && c <='l') {
					//a->First_1bnd_vtx, b->First_1bnd_vtx+1, etc
					r_vtx.push_back(c-'a'+sproutsGlobal::First_1bnd_vtx);
				} else if(c>='o' && c <='z') {
					//z->First_2bnd_vtx, y->First_2bnd_vtx+1, etc
					r_vtx.push_back('z'-c+sproutsGlobal::First_2bnd_vtx);
				} else if(c>='A' && c <='Y') {
					//A->First_2reg_vtx, B->First_2reg_vtx+1, etc
					r_vtx.push_back(c-'A'+sproutsGlobal::First_2reg_vtx);
				} else if(c=='m') {
					//extended 1bnd vertices : m(1), m(2)... (m(1) corresponds to a,...)
					extended_1bnd=true;
				} else if(c=='n') {
					//extended 2bnd vertices : n(1), n(2)... (n(1) corresponds to z,...)
					extended_2bnd=true;
				} else if(c=='Z') {
					//extended 2reg vertices : Z(1), Z(2)... (Z(1) corresponds to A,...)
					extended_2reg=true;
				}
			}
		}
	}
	
	return r_vtx;
}
