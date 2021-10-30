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

#include "dotsboxes.h"

//=====================================Compression=======================

//map of frequent strings that we replace with one character only
void initializeReplaceMap(map<string, unsigned char> &replaceMap) {
	string s;
	s="ALB*E";
	replaceMap[s] = 'p';
	s="BLB*E";
	replaceMap[s] = 'q';
	s="BLCLB*E";
	replaceMap[s] = 'r';
	s="B*E";
	replaceMap[s] = 's';
	s="A*E";
	replaceMap[s] = 't';
	s="BLC*LGLCLCE";
	replaceMap[s] = 'u';
	s="CLC*LGLCLCE";
	replaceMap[s] = 'v';
}

//add the repetition of a given character
//note : the character has already been added onte time, and this function is called only if repeatLength>2
void addRepeatedCharacter(string &output, unsigned char r, int repeatLength) {
	if(repeatLength>2) {
		output += (QString("%1").arg(repeatLength, 0, 10)).toStdString();
	} else {
		if(r=='j') {
			output[output.size()-1] = 'x';
		} else
		if(r=='e') {
			output[output.size()-1] = 'y';
		} else
		if(r=='i') {
			output[output.size()-1] = 'w';
		} else
		if(r=='c') {
			output[output.size()-1] = 'v';
		} else
		if(r=='o') {
			output[output.size()-1] = 'z';
		} else {
			output += r;
		}
	}
}

//return a compressed string of the given input
//this function is used to store shorter strings in the database
string DotsAndBoxes::compress(const string &input) {
	if(!Parameter::useCompression) return input;
	
	/*
	static map<string, unsigned char> replaceMap;
	map<string, unsigned char>::iterator Si;
	if(replaceMap.empty()) initializeReplaceMap(replaceMap);
	*/
	
	string output;
	unsigned char c;
	int state=0;
	//bool found;
	int i;
	int length=input.length();
	for(i=0;i<length;i++) {
		c=input[i];
		
		//test if some element of the replace map appears
		//Note : too slow
		/*
		found = false;
		for(Si=replaceMap.begin(); Si!=replaceMap.end() && !found; Si++) {
			if(input.compare(i, Si->first.size(), Si->first)==0) {
				//Si appears, starting at the i position
				if(state!=0) output += input[i-1];
				output += Si->second;
				state=0;
				found = true;
				i += Si->first.size()-1;
			}
		}
		
		if(!found) {
		*/
			switch(state) {
			case 0: default:
				if(c==DoubleArrow) {
					state=1;
				} else
				if(c==SingleArrow) {
					state=2;
				} else
				if(c==Coin) {
					state=3;
				} else
				if(c==Link) {
					state=4;
				} else
				if(c==Gray) {
					state=5;
				} else {
					output += c;
				}
				break;
			case 1: //DoubleArrow
				if(c==Link){
					output += 'a';
					state = 0;
				} else
				if(c==Gray){
					output += 'b';
					state = 0;
				} else
				if(c==SizeMark){
					state = 6;
				} else {
					//we have not found the expected pattern, copy the 2 characters as they are
					output += input[i-1];
					output += c;
					state = 0;
				}
				break;
			case 2: //SingleArrow
				if(c==Link){
					output += 'c';
					state=0;
				} else
				if(c==Gray){
					output += 'd';
					state=0;
				} else
				if(c==SizeMark){
					state = 7;
				} else {
					//we have not found the expected pattern, copy the 2 characters as they are
					output += input[i-1];
					output += c;
					state=0;
				}
				break;
			case 3: //Coin
				if(c==Link){
					output += 'e';
				} else
				if(c==Gray){
					output += 'f';
				} else
				if(c==SizeMark){
					output += 's';
				} else {
					//we have not found the expected pattern, copy the 2 characters as they are
					output += input[i-1];
					output += c;
				}
				state=0;
				break;
			case 4: //Link
				if(c==DoubleArrow) {
					output += 'g';
				} else
				if(c==SingleArrow) {
					output += 'h';
				} else
				if(c==Coin) {
					output += 'i';
				} else
				if(c==Gray) {
					output += 'j';
				} else {
					//we have not found the expected pattern, copy the 2 characters as they are
					output += input[i-1];
					output += c;
				}
				state=0;
				break;
			case 5: //Gray
				if(c==DoubleArrow) {
					output += 'k';
				} else
				if(c==SingleArrow) {
					output += 'l';
				} else
				if(c==Coin) {
					output += 'm';
				} else
				if(c==Link) {
					output += 'n';
				} else
				if(c==Gray) {
					output += 'o';
				} else {
					//we have not found the expected pattern, copy the 2 characters as they are
					output += input[i-1];
					output += c;
				}
				state=0;
				break;
			case 6: //DoubleArrow+SizeMark
				if(c==EndCanonicalBoard) {
					output += 't';
				} else {
					//we have found only a partial pattern
					output += 'p';
					i--;	//process again the current character (better for parity reasons)
				}
				state=0;
				break;				
			case 7: //SingleArrow+SizeMark
				if(c==EndCanonicalBoard) {
					output += 'q';
				} else {
					//we have found only a partial pattern
					output += 'r';
					i--;	//process again the current character (better for parity reasons)
				}
				state=0;
				break;				
			}
		//}
	}
	
	//change any remaining number 0, 1 or 2 in a letter to avoid any conflict
	length=output.length();
	if(length==0) return output;
	/*
	for(i=0;i<length;i++) {
		if(output[i]=='0') {
			output[i] = 'X';
		} else
		if(output[i]=='1') {
			output[i] = 'Y';
		} else
		if(output[i]=='2') {
			output[i] = 'Z';
		}
	}*/
	
	//replace consecutive characters by the character followed by the number of appearances
	string output2;
	unsigned char r; //repeated character
	int repeatLength = 1;
	r = output[0];
	for(i=1;i<length;i++) {
		c=output[i];
		if(c!=r) {
			//end of the repetition
			//add the repeated character followed by the number of repetitions
			output2 += r;
			if(repeatLength>1) addRepeatedCharacter(output2, r, repeatLength);
			r = c;
			repeatLength = 1;
		} else {
			repeatLength++;
		}
	}
	
	//last character
	//note : no EndBoard or EndCanonicalBoard character at the end of the string
	if(r != EndBoard && r != EndCanonicalBoard) {
		output2 += r;
		if(repeatLength>1) addRepeatedCharacter(output2, r, repeatLength);
	}
	
	return output2;
}

void DotsAndBoxes::addUncompressTo(string &output, unsigned char compressedChar) {
	if(compressedChar=='a') {
		output += DoubleArrow;
		output += Link;
	} else
	if(compressedChar=='b') {
		output += DoubleArrow;
		output += Gray;
	} else
	if(compressedChar=='c') {
		output += SingleArrow;
		output += Link;
	} else
	if(compressedChar=='d') {
		output += SingleArrow;
		output += Gray;
	} else
	if(compressedChar=='e') {
		output += Coin;
		output += Link;
	} else
	if(compressedChar=='f') {
		output += Coin;
		output += Gray;
	} else
	if(compressedChar=='g') {
		output += Link;
		output += DoubleArrow;
	} else
	if(compressedChar=='h') {
		output += Link;
		output += SingleArrow;
	} else
	if(compressedChar=='i') {
		output += Link;
		output += Coin;
	} else
	if(compressedChar=='j') {
		output += Link;
		output += Gray;
	} else
	if(compressedChar=='k') {
		output += Gray;
		output += DoubleArrow;
	} else
	if(compressedChar=='l') {
		output += Gray;
		output += SingleArrow;
	} else
	if(compressedChar=='m') {
		output += Gray;
		output += Coin;
	} else
	if(compressedChar=='n') {
		output += Gray;
		output += Link;
	} else
	if(compressedChar=='o') {
		output += Gray;
		output += Gray;
	} else
	if(compressedChar=='p') {
		output += DoubleArrow;
		output += SizeMark;
	} else
	if(compressedChar=='q') {
		output += SingleArrow;
		output += SizeMark;
		output += EndCanonicalBoard;
	} else
	if(compressedChar=='r') {
		output += SingleArrow;
		output += SizeMark;
	} else
	if(compressedChar=='s') {
		output += Coin;
		output += SizeMark;
	} else
	if(compressedChar=='t') {
		output += DoubleArrow;
		output += SizeMark;
		output += EndCanonicalBoard;
	} else
	if(compressedChar=='v') {
		output += SingleArrow;
		output += Link;
		output += SingleArrow;
		output += Link;
	} else
	if(compressedChar=='w') {
		output += Link;
		output += Coin;
		output += Link;
		output += Coin;
	} else
	if(compressedChar=='x') {
		output += Link;
		output += Gray;
		output += Link;
		output += Gray;
	} else
	if(compressedChar=='y') {
		output += Coin;
		output += Link;
		output += Coin;
		output += Link;
	} else
	if(compressedChar=='z') {
		output += Gray;
		output += Gray;
		output += Gray;
		output += Gray;
	} else
	if(compressedChar=='X') {
		output += '0';
	} else {
		/*
		bool found = false;
		
		static map<string, unsigned char> replaceMap;
		if(replaceMap.empty()) initializeReplaceMap(replaceMap);
		map<string, unsigned char>::iterator Si;
		for(Si=replaceMap.begin(); Si!=replaceMap.end() && !found; Si++) {
			if(compressedChar==Si->second) {
				output += Si->first;
				found = true;
			}
		}
		
		//last case, this is just a normal character
		if(!found) output += compressedChar;
		*/
		output += compressedChar;
	}
}

//Uncompress the string given as input
//this function is used when trying to display a compressed database
string DotsAndBoxes::uncompress(const string &input) {
	if(!Parameter::useCompression) return input;
	
	string output;
	unsigned char c;
	int state=0;
	int repeat = 0;
	unsigned char repeatedCharacter=0;
	
	int i;
	int length=input.length();
	for(i=0;i<length;i++) {
		c=input[i];
		
		switch(state) {
		case 0: default:
			if(c>='0' && c<='9') {
				state = 1;
				repeat = (c-'0');
				if(i!=0){
					repeatedCharacter = input[i-1];
				} else {
					cout << "Error in uncompress" << endl;
				}
			} else {
				addUncompressTo(output, c);
			}
			break;
		case 1: 
			if(c>='0' && c<='9') {
				repeat *= 10;
				repeat += (c-'0');
			} else {
				//end of the number of repetitions
				//note : one of the repeated character has already been added
				int j;
				for(j=0; j<repeat-1; j++) {
					addUncompressTo(output, repeatedCharacter);
				}
				state = 0;
				addUncompressTo(output, c);
			}
		}
	}
	
	//special case if the string ends with a number (repeated character)
	if(state==1) {
		int j;
		for(j=0; j<repeat-1; j++) {
			addUncompressTo(output, repeatedCharacter);
		}
	}
	
	//add a EndCanonicalBoard character at the end of the string if needed
	if(!output.empty()) {
		c = output[output.size()-1];
		if(c!=EndBoard && c!=EndCanonicalBoard) output += EndCanonicalBoard;
	}
	return output;
}
