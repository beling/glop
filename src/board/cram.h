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

#ifndef CRAM_H
#define CRAM_H

#include "crambase.h"

//! Game of Cram
//! @ingroup BoardGroup
class Cram : public CramBase {
  public:
	Cram();
	Cram(const Line& board_ini);
	Cram(const string& board_ini);
	
	GAME_REGISTER_MACRO(Cram)
	
	//inherited from BaseGame
	virtual int gameCategory();
	virtual void computeOptionsSet();
	virtual win_loss Victory(); //indicate which player has won
	virtual void computeOrderParameters(MoveOrdering& result);
	
	bool is_terminal_position(); //indicate whether it is a terminal position or not
	
	//parameter interface
	virtual Interface getParamDef();
	virtual void setParam();
	virtual string getPositionName();
};

#endif
