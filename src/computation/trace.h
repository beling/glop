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

#ifndef TRACE_H
#define TRACE_H

#include "nodestore.h"

#include <QMutex>

//! Main class to trace computation progress.
//! This class is thread-safe : all public functions (interface) are protected with a QMutex
class Trace {
  private:
	//----------Explanation of the values of signal/current_level----------
	//The number of the first level, where are stored the children of the computed position, is 1.
	//The number of the second level is 2, etc. The number of a level is the same that the
	//number printed on the left of the table.
	//Be careful, row(item) is equal to "number_of_the_level(item)-1".
	//When we click on the level n for classical zapping, signal is set to n.
	//----------End of explanation----------
	
	static int signal;                 //!< signal to change studied child on the specified level (0 if it has no meaning)
	static bool stop_computing;        //!< for interruption because of an error, or by the player
	static bool reverse_zapping;       //!< true if we are zapping back
	
	static void signal_received();     //!< executed when signal is received

  public:
	//! @name objects used to store the data
	//@{	
	static int current_level;	//!< current level in the game tree
	//@}

	static QMutex trace_lock;
	static QMutex pause_lock;  //!< mutex used only to pause the computation
	
	Trace() {};
	~Trace() {};
	
	//! @name Interface (content)
	//@{
	static void initialize();                       //!< initialize the trace object (purge it)
	static void create_level();      //!< create a new level
	static void delete_level();                     //!< delete the bottom-level
	//@}
	
	//! @name Interface (signals)
	//@{
	static bool stop_signal();                      //!< trace->compute : return true when user stops computation
	static bool user_interrupt(bool for_cn);        //!< trace->compute : return true until the computation goes back to the correct level
	static int user_request();                      //!< trace->compute : return int value describing the current user request
	static void set_stop_value(bool value);         //!< user interface->trace : set value of stop_computing (used to stop a computation)
	static void request_level(int index, bool backward);  //!< user interface->trace : user requested next child on the specified level
	static void pauseStart();                              //!< user interface : pause in the computation
	static void pauseEnd();                                //!< user interface : end the pause in the computation
	//@}
	
	//! @name Traversal related signals
	static storeId traversalRequestedId;
	static void requestTraversalId(storeId id);		//! interface->trace : user request for a given id
	static storeId checkRequestedId();					//! trace->compute : check if the user has requested a given id
};

#endif
