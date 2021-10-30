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

#ifndef GLOBAL_H
#define GLOBAL_H

//global types and constant values (used in the whole project)

//the following files are included in the whole project
#include <list>
#include <set>
#include <map>
#include <vector>
#include <algorithm>

#include <iostream>
#include <sstream>

using namespace std;

//error handling
#include "error.h"

//----- Compilation flag --------
//The following define enables some features not supported in the official release version 
//#define UNSUPPORTED_FEATURE

//----- general definitions ------

//vertex type
typedef unsigned int Vertex;

//line type : representation of an object with a list of vertex
typedef list<Vertex> Line;

//nimber type
typedef unsigned char nimber;

//win-loss type
typedef nimber win_loss;

//extended position type : (position, nimber)
typedef pair<Line, nimber> Pos_nim;

//standard type for representing positions
typedef string PositionString;

#endif
