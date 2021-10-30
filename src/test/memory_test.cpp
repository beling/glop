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

#include "../computation/nodestore.h"
#include "../baseclass/basegame.h"
#include "../baseclass/basenode.h"
#include "../computation/classicpnsearch.h"

//display the size in bytes of the main elementary objects used in Glop
//this is usefull when trying to optimize memory usage

void MemoryTest() {
	testTitle("Memory");
	testBegin("Critical for the memory consumption of the search-tree");
	MoveOrdering a;
	cout << "MoveOrdering: " << sizeof(a) << " bytes" << endl;
	cout << "MoveOrdering.param: " << sizeof(a.param) << " bytes" << endl;
	cout << "MoveOrdering.pos: " << sizeof(a.pos) << " bytes" << endl;
	BaseNode b;
	cout << "BaseNode: " << sizeof(b) << " bytes" << endl;
	BaseTraversal c;
	cout << "BaseTraversal: " << sizeof(c) << " bytes" << endl;	
	NodeData d;
	cout << "NodeData: " << sizeof(d) << " bytes" << endl;
	Node e;
	cout << "Node: " << sizeof(e) << " bytes" << endl;
	cout << "Node.childrenIdList: " << sizeof(e.childrenIdList) << " bytes" << endl;
	ClassicPN f;
	cout << "ClassicPN: " << sizeof(f) << " bytes" << endl;
	testBegin("Non-critical elements");
	BaseGame g;
	cout << "BaseGame: " << sizeof(g) << " bytes" << endl;
}
