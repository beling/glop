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

#include "displaypainter.h"
#include <QtGui>

DisplayPainter::DisplayPainter() {
	cellSize = 10;
	painter = new QPainter;
}

DisplayPainter::~DisplayPainter() {
	//DisplayPainter is not a Qt class, so we need to delete ourselves the QPainter object to avoid a memory leak
	if(painter != 0) {
		delete painter;
		painter = 0;
	}
}

void DisplayPainter::setClipRect(int x, int y, int width, int height) {
	painter->setClipRect(x, y, width, height);
}

void DisplayPainter::fillRect(int x, int y, int width, int height, string color) {
	painter->setPen(Qt::NoPen);
	QColor qcol(color.c_str());
	painter->fillRect(QRect(x, y, width, height), QBrush(qcol) );
}

void DisplayPainter::drawLine(int x0, int y0, int x1, int y1, string color, int size) {
	QColor qcol(color.c_str());
	painter->setPen( QPen(QBrush(qcol), size) );
	painter->drawLine(x0, y0, x1, y1);
}

