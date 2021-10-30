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

#ifndef DISPLAYPAINTER_H
#define DISPLAYPAINTER_H

#include "global.h"

class QPainter;

//Note : as much as possible, this class should be used through the available functions
//if more functionality is needed, the QPainter object could be accessed directly
class DisplayPainter {
  public:
	DisplayPainter();
	~DisplayPainter();
	
	QPainter * painter;
	int cellSize;
	int neededHeight;
	bool computeHeightOnly;
	
	void setClipRect(int x, int y, int width, int height);
	void fillRect(int x, int y, int width, int height, string color);
	void drawLine(int x0, int y0, int x1, int y1, string color, int size);
};

#endif
