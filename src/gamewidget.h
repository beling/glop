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

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "gameglobal.h"
#include <QWidget>
#include <QDialog>

class QPainter;
class QPaintEvent;
class QKeyEvent;
class QLineEdit;
class QTableWidget;

class DisplayGameWidget : public QWidget {
  Q_OBJECT
	
  public:
	DisplayGameWidget( QWidget* parent = 0);
	PositionString paintPosition;
	int cellSize;
	bool displayIsActivated;
	
	int neededHeight();
	void activate(bool newIsActivated);  //activate or disactivate the drawing
	void changePaintPosition(const PositionString &newPaintPosition);
	
  protected:
	void paintEvent(QPaintEvent *event);
};

class GameWidget : public QWidget {
  Q_OBJECT
	
  public:
	GameWidget( QWidget* parent = 0);
	DisplayGameWidget* displayWidget;
	QLineEdit* box_given_position_2;
	
  public slots:
	void positionConfirmed();
};

class DisplayBaseWidget : public QDialog {
  Q_OBJECT
	
  public:
	DisplayBaseWidget( QWidget* parent = 0, int numBase=0);
	QLineEdit* givenIndex;
	QTableWidget *baseDisplayTable;
	int baseSelectionIndex;
	
  public slots:
	void indexConfirmed();
	void randomClicked();
	
  protected:
	void keyPressEvent(QKeyEvent *event);
};

#endif
