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

#include "test.h"
#include "../board/dotsboxes.h"
#include "../stringconverter.h"

//TODO : the distinction between E anf F is not clear during the test
void DotsBoxesTest() {
	testTitle("Dots and Boxes");
	DotsAndBoxes b;
	b.setParam(); //initialize the internal variables
	
	//==========================================================================
	testBegin("Board Initialization");
	testComment("2x2 Board");
	DotsAndBoxes::dotsRows = 2;
	DotsAndBoxes::dotsColumns = 2;
	DotsAndBoxes dotsBoard;
	string s="ALA*LGLALAE";
	testOutput( dotsBoard.toString(), s );
	
	testComment("3x3 Board");
	DotsAndBoxes::dotsRows = 3;
	DotsAndBoxes::dotsColumns = 3;
	DotsAndBoxes dotsBoard2;
	s="ALBLA*LGLGLBLCLBLGLGLALBLAE";
	testOutput( dotsBoard2.toString(), s );
	
	testComment("5x5 Board");
	DotsAndBoxes::dotsRows = 5;
	DotsAndBoxes::dotsColumns = 5;
	DotsAndBoxes dotsBoard3;
	s="ALBLBLBLA*LGLGLGLGLBLCLCLCLBLGLGLGLGLBLCLCLCLBLGLGLGLGLBLCLCLCLBLGLGLGLGLALBLBLBLAE";
	testOutput( dotsBoard3.toString(), s );
	
	//==========================================================================
	testBegin("Coloring of the capturable coins");
	list<string> input;
	list<string> output;
	input.push_back("BGA*GGLALAE"); //only one capturable arrow
	output.push_back("VGA*GGLALAE");
	input.push_back("ALBLA*LGGGLBGCGBLGLGLALBLAE"); //only one capturable coin
	output.push_back("ALBLA*LGGGLBGUGBLGMGLALBLAE");
	input.push_back("AGBGA*LGGGLBGCGBLGLGLAGBGAE");  //one capturable arrow, and one capturable coin+arrow
	output.push_back("AGVGA*LGGGLBGUGBLGMGLAGVGAE");
	input.push_back("AGCGA*LGLGGBGCGCLGLGLAGCLCE");  //a long chain of capturable coins
	output.push_back("AGUGA*LGMGGBGUGULGMGMAGUMUE");
	input.push_back("CLCLC*LGGGLCLCGCE");  //bug of revision 1721
	output.push_back("UMUMU*MGGGMUMUGUE");
	input.push_back("CLC*E"); // pair of capturable coins
	output.push_back("UMU*E");
	
	list<string>::iterator Si, Sj;
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		dotsBoard.initCurrentBoard(0);
		dotsBoard.colorCapturableComponent_currentBoard();
		testOutput( dotsBoard.toString(), *Sj );
	}
	
	//==========================================================================
	testBegin("Search of a keep-sente coin pair");
	input.clear();
	output.clear();
	input.push_back("BGA*GGLALAE"); //only one capturable arrow
	output.push_back("false");
	input.push_back("ALBLA*LGGGLBGCGBLGLGLALBLAE"); //only one capturable coin
	output.push_back("false");
	input.push_back("AGBGA*LGGGLBGCGBLGLGLAGBGAE");  //one capturable arrow, and one capturable coin+arrow
	output.push_back("true");
	input.push_back("AGCGA*LGLGGBGCGCLGLGLAGCLCE");  //a long chain of capturable coins but no keep-sente coin pair
	output.push_back("false");
	input.push_back("AGCGA*LGLGGBGCGBLGLGLAGCLCE");  //a long chain of capturable coins with a keep-sente coin pair
	output.push_back("true");
	input.push_back("AGCGA*LGLGGBLCGCLGLGLAGCLCE");  //a long chain of capturable coins with a keep-sente coin pair
	output.push_back("true");
	input.push_back("CLCLC*LGGGLCLCGCE");  //bug of revision 1721
	output.push_back("false");
	
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		dotsBoard.initCurrentBoard(0);
		dotsBoard.colorCapturableComponent_currentBoard();
		dotsBoard.boardList[0] = dotsBoard.toString();
		bool sentePairExist = dotsBoard.searchCoinPair();
		string result="true";
		if(!sentePairExist) result="false";
		testOutput( result, *Sj );
	}
	
	//==========================================================================
	testBegin("Search of a double pair of coins");
	input.clear();
	output.clear();
	input.push_back("BGA*GGLALAE"); //only one capturable arrow
	output.push_back("false");
	input.push_back("ALBLA*LGGGLBGCGBLGLGLALBLAE"); //only one capturable coin
	output.push_back("false");
	input.push_back("AGBGA*LGGGLBGCGBLGLGLAGBGAE");  //one capturable arrow, and one capturable coin+arrow
	output.push_back("false");
	input.push_back("AGCGA*LGLGGBGCGCLGLGLAGCLCE");  //a long chain of capturable coins
	output.push_back("true");
	input.push_back("AGCGA*LGLGGBGCGBLGLGLAGCLCE");  //a long chain of capturable coins
	output.push_back("true");
	input.push_back("AGCGA*LGLGGBLCGCLGLGLAGCLCE");  //a long chain of capturable coins
	output.push_back("true");
	input.push_back("AGCGA*LGLGGBLCGCLGLGLAGCGCE");  //a chain with only 3 coins
	output.push_back("false");
	input.push_back("AGCGA*LGLGGBGCLCLGLGGAGCGBE");  //4 coins in a star pattern
	output.push_back("false");
	input.push_back("AGCGA*GGLGGCLCLCGGLGGAGCGAE");  //5 coins in a star pattern
	output.push_back("false");
	input.push_back("CLCGA*GGLGGCLCLCGGLGGAGCGAE");  //5 coins in a star pattern + one extension
	output.push_back("true");
	input.push_back("CLCLC*LGGGLCLCGCE");  //bug of revision 1721
	output.push_back("true");
	
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		dotsBoard.initCurrentBoard(0);
		dotsBoard.colorCapturableComponent_currentBoard();
		dotsBoard.boardList[0] = dotsBoard.toString();
		bool doublePairExist = dotsBoard.searchDoublePair();
		string result="true";
		if(!doublePairExist) result="false";
		testOutput( result, *Sj );
	}
	
	//==========================================================================
	testBegin("Test ScoreRange Object");
	testComment("Initial 0;Score_Max+1 Range");
	ScoreRange range;
	s = string("251");
	testOutput( range.toString(), s );
	
	testComment("Update to 0;52 Range");
	range.decreaseMaxTo(52);
	s = string("52");
	testOutput( range.toString(), s );
	
	testComment("Read back from the string");
	ScoreRange range3;
	range3.fromString(s);
	testOutput( range3.toString(), s );
	
	testComment("Update to 5;48 Range");
	range.increaseMinTo(5);
	range.decreaseMaxTo(48);
	s = string("5;48");
	testOutput( range.toString(), s );
	
	testComment("Update to 12;23 Range");
	range.increaseMinTo(12);
	range.decreaseMaxTo(23);
	s = string("12;23");
	testOutput( range.toString(), s );
	
	testComment("Read back from the string");
	ScoreRange range2;
	range2.fromString(s);
	testOutput( range2.toString(), s );
	
	//==========================================================================
	testBegin("Detect Linear chains");
	testComment("Length of the linear chain (-1 if it is not a linear chain)");
	input.clear();
	output.clear();
	input.push_back("BLCLC*GGGGLGGGGBE"); //chain of length 2
	output.push_back("2");
	input.push_back("BLC*GGLGGBE"); //chain of length 1
	output.push_back("1");
	input.push_back("BLC*LGLCLBE"); //not a linear chain
	output.push_back("-1");
	input.push_back("BLC*LGLCGBE"); //not a linear chain
	output.push_back("-1");
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		int length = dotsBoard.isLinearChain();
		StringConverter conv;
		conv << length;
		testOutput( conv.getString(), *Sj );
	}
	
	//==========================================================================
	testBegin("Canonize caterpillars");
	input.clear();
	output.clear();
	input.push_back("ALBLA*GGGGLGGGGBGGGGLALBLAE");		//a caterpillar
	output.push_back("ALBLALBLALBLA*F");
	input.push_back("ALBGG*GGLGGGGCLBGGGGLCLBLBE");		//a caterpillar
	output.push_back("ALBLCLBLBLBLC*F");
	input.push_back("ALBLA*GGGGLGGGGBGGGGLALBLAEALBGG*GGLGGGGCLBGGGGLCLBLBE"); //two caterpillars
	output.push_back("ALBLALBLALBLA*FALBLCLBLBLBLC*F");
	input.push_back("ALBLB*GGLGGGGCLBGGGGLCLBLBE");		//this is not a caterpillar (->we do nothing)
	output.push_back("ALBLB*GGLGGGGCLBGGGGLCLBLBE");
	
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		dotsBoard.gameSpecificCanonization();
		testOutput( dotsBoard.exportToString(), *Sj );
	}
	
	//==========================================================================
	testBegin("Canonize right angles");
	input.clear();
	output.clear();
	input.push_back("ALBGBGA*GGLGLGLGGCLCLBGGLGLGLALBLBLAE");		//only one angle
	output.push_back("AGBGGGA*LGLGGGLBLCLCLBLGLGLGGALBLBLAF");
	input.push_back("ALBGCLC*GGLGLGGGGCLCGBGGLGLGLALBLBLAE");		//only one angle
	output.push_back("AGGGCLC*LGGGLGGBLCLCGBGGLGLGLALBLBLAF");
	input.push_back("ALBGGGC*GGLGGGLGGCLCLCGGLGLGGALBLBLAE");		//two angles (one already canonized)
	output.push_back("AGGGGGC*LGGGGGLBLCLCLCGGLGLGGALBLBLAF");
	input.push_back("ALBGCLC*GGLGLGGGGCLCGGGGLGLGGALBLBLAE");		//two angles (with the same result as above)
	output.push_back("AGGGGGC*LGGGGGLBLCLCLCGGLGLGGALBLBLAF");
	input.push_back("ALBLCLC*GGLGLGGGGCLCGGGGLGLGGALBLBLAE");		//no angle
	output.push_back("ALBLCLC*GGLGLGGGGCLCGGGGLGLGGALBLBLAE");
	input.push_back("ALBLCLC*GGLGLGGGGCLCGGGGLGLGGALBGBLAE");		//two angles (other directions)
	output.push_back("AGGGGGA*LGGGGGLBLCLCLBGGLGLGGALBLCLCF");
	
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		dotsBoard.gameSpecificCanonization();
		testOutput( dotsBoard.exportToString(), *Sj );
	}
	
	//==========================================================================
	testBegin("Chain opening equivalences");
	input.clear();
	output.clear();
	input.push_back("BGA*GGLALAE");		//one caturable arrow
	output.push_back("J1KGGA*GGLALAE");
	input.push_back("AGBGA*LGGGLBGCGBLGLGLAGBGAE");  //one capturable arrow, and one capturable coin+arrow
	output.push_back("J1KAGGGA*LGGGLBGGGBLGGGLAGGGAEBLC*F");
	input.push_back("CLC*E");		//pair of capturable coins
	output.push_back("J2KGGG*E");
	
	for(Si=input.begin(), Sj=output.begin() ; Si != input.end(); Si++, Sj++) {
		testComment(*Si);
		DotsAndBoxes dotsBoard(*Si);
		dotsBoard.chainOpeningEquivalences();
		dotsBoard.boardList[0] = dotsBoard.toString();
		testOutput( dotsBoard.exportToString(), *Sj );
	}
}
