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

#include "trace.h"
#include <QMutexLocker>

int Trace::signal;                 //!< signal to change studied child on the specified level (0 if it has no meaning)
bool Trace::stop_computing;        //!< for interruption because of an error, or by the player
bool Trace::reverse_zapping;       //!< true if we are zapping back

int Trace::current_level;	//!< current level in the game tree
QMutex Trace::trace_lock;
QMutex Trace::pause_lock; 

storeId Trace::traversalRequestedId;

//--------------------------------------------------------------------------
//Communication with "Game" class
//--------------------------------------------------------------------------

//initialize the trace
void Trace::initialize() {
	QMutexLocker locker(&trace_lock);
	
	current_level=0;
	signal=0;		//no signal
	reverse_zapping=false;
	stop_computing=false;	//"false" allows the computing to be launched without graphical interface (from -t option)
	traversalRequestedId=-1; //it means "no requested id"
}

//create a new level
void Trace::create_level() {
	QMutexLocker locker(&trace_lock);
	
	current_level++;		//increment number of levels
}

//delete the lowest level
void Trace::delete_level() {
	QMutexLocker locker(&trace_lock);
	
	if (current_level>0) { //"normal" security to avoid pop_back of an empty list
		current_level--;
	}
}

//used to indicate a signal was received
void Trace::signal_received() {
	//no lock protection : it is always called "internally" from a protected function
	signal=0;
	reverse_zapping=false;
}

//return true if user asked to stop computation
//pause the computation if the pause is active
bool Trace::stop_signal() {
	//WARNING : the order of the mutex is very important
	//if trace is checked before pause, there is a case of dead-lock
	QMutexLocker locker2(&pause_lock);		//pause until pause_lock is available
	QMutexLocker locker(&trace_lock);
	
	//stop computation on signal stop_computing
	if (stop_computing) {
		return true;
	}
	
	return false;
}

//return true until the computation goes back to the correct level
//"for_cn" (cn means compute_nimber) allow to treat the case current_level==signal correctly,
//as this case is different in game::compute_nimber and game::computeNode
bool Trace::user_interrupt(bool for_cn) {
	QMutexLocker locker(&trace_lock);
	
	//request is for above level (=> end this level)
	if ( (signal!=0 && current_level>=signal && !(current_level==signal && for_cn)) || stop_computing) {
		return true;
	}
	
	//invalid request (too low, requested level doesn't exist or zapping on this level has no meaning)
	//=>just receive the signal without any special treatment
	if(signal!=0) {
		signal_received();	//simply indicates signal was received
	}

	return false;
}

//detect any signal for zapping, and translate it into an integer treated in Game::computeNode
int Trace::user_request() {
	QMutexLocker locker(&trace_lock);
	
	//end of computation
	if(stop_computing) {
		return (-5);			//means "stop computation"
	}
	
	//request is for above levels (-> end this level)
	if (signal!=0 &&current_level>signal) {
		return (-1);			//means "go back to above level"
	}
	
	//request is for the current level (-> change the studied child)
	if(signal!=0 && current_level==signal) {
		bool rz_tmp=reverse_zapping;
		signal_received();	//indicates signal was received
		if (!rz_tmp){
			return -2;	//means "go to next child"
		} else {
			return -3;	//means "go to previous child"
		}
	}
	
	//invalid request (too low, requested level doesn't exist)
	//->just receive the signal without any special treatment
	if(signal!=0 && current_level<signal) {
		signal_received();	//indicates signal was received
	}
	
	return (-4);				//means "do nothing"
}


//---------used in graphic thread ---------------

//set value of stop_computing
void Trace::set_stop_value(bool new_value) {
	QMutexLocker locker(&trace_lock);
	
	stop_computing=new_value;
}

//user interface->trace : user requested next child on the specified level
void Trace::request_level(int index, bool backward) {
	QMutexLocker locker(&trace_lock);
	
	//note : class Game should always receive signals - even invalid
	//if a previous signal has not yet been received, there is no emission of a new one
	if (signal==0) {
		signal=index;	//emit the signal indicating which level should compute next branch (user choice)
		reverse_zapping=backward;
	}
}

//user interface : pause in the computation
//the computation is paused simply by locking the QMutexLocker
//pause_lock is regularly checked by the computation with Trace::stop_signal
void Trace::pauseStart() {
	pause_lock.lock();
}

//user interface : end the pause in the computation
//unlock the QMutexLocker so that computation can flow again
//WARNING : a pause start should always be followed by a pause end
void Trace::pauseEnd() {
	pause_lock.unlock();
}

//--------------Traversal related signals---------------------

//interface->trace : user request for a given id
void Trace::requestTraversalId(storeId id) {
	QMutexLocker locker(&trace_lock);
	traversalRequestedId = id;
}

//trace->compute : check if the user has requested a given id
storeId Trace::checkRequestedId() {
	QMutexLocker locker(&trace_lock);
	storeId requestedIdCopy = traversalRequestedId;
	traversalRequestedId = -1; //initialize back the requestedId
	return requestedIdCopy;
}
