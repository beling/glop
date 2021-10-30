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

#ifndef GAME_H
#define GAME_H

#include <qthread.h>
#include <qmutex.h>

#include "../gameglobal.h"
#include "../baseclass/basenode.h"

//! Class to launch a computation in a separate thread
class ComputationThread : public QThread {
	
  public:
	//----------QThreads-----------------------------------------------------------------------
	virtual void run();                 //this function is called as a Qt thread by start()
	
	//result of a computation and mutex of protection
	QMutex compthread_mutex;
	bool computationRunning; //true if a computation is runnning
	
	//----------computation parameters---------------------------------------------------------
	static BaseNode BaseNodeIni;
	
	//variable to choose the Node structure (recursive structure or NodeStore)
	bool recursiveStructure;
	void startOneComputation();				//function to start one computation
	void recursiveLoop(BaseNode& node);		//simple recursive algorithm	
};

#endif
