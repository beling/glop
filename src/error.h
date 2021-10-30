/***************************************************************************
 *   Copyright (C) 2006-2007 by Julien Lemoine, Simon Viennot              *
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

#ifndef ERROR_H
#define ERROR_H

//Error class is provided as a convenient way to display messages in the graphic thread
//when an error occured (in the computing thread or the graphic one).

//There should be one WidgetError object created in the graphic thread
//connect it to a timer to check errors regularly

//To launch an error and display an error message, juste create an Error class : Error("you lose");

//warnings/limitations of the current class :
//  * fatal errors in the graphic thread are handled just like non-fatal errors
//  * if the graphic thread is waiting a mutex locked by the error thread
//    the program will freeze with no message display
//  * after the kill of a non-graphic fatal error thread, program should be considered in a dangerous state
//  * if two errors occur at the same time (one in the computing thread and one in the graphic thread)
//    the program will probably just freeze

#include <QString>
#include <QWidget>

class QMutex;
class QWaitCondition;
class QThread;

class WidgetError;

class Error {
  public:
	//mutex to protect access to error parameters
	static QMutex message_lock;
	
	//error parameters
	static bool error_occured;
	static bool fatal_error;
	static QString error_message;
	static QThread* error_thread;
	
	//variables to synchronize threads
	static QWaitCondition synchronize;
	static QMutex wait_lock;
	
	//pointer to the graphic thread
	//used to know if error occured in the graphic thread or not
	static QThread* graphic_thread;
	
	//pointer to the WidgetError receiving objet
	static WidgetError* widget_error;
	
	//constructor
	Error(QString message, bool fatal);
};

//Widget to trace computing
class WidgetError: public QWidget {
	Q_OBJECT
  public :
	WidgetError( QWidget *parent=0 );
	
  public slots :
	//error reception
	void receive_error();
};

#endif

