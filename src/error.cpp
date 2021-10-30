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

#include "error.h"
#include <QtGui>

//create static variables (global to all threads)
	QMutex Error::message_lock;
	
	//used to synchronize error release in error thread with graphic thread
	//to treat possible successive errors
	QMutex Error::wait_lock;
	QWaitCondition Error::synchronize;
	
	//error parameters
	bool Error::error_occured=false;
	bool Error::fatal_error=false;
	QString Error::error_message;
	QThread* Error::error_thread;
	
	//direct acces to WidgetError object
	WidgetError* Error::widget_error;
	
	//pointer to the graphic thread
	QThread* Error::graphic_thread;

//displays message in the graphic thread
//computing thread will be killed if error is fatal
Error::Error(QString message, bool fatal) {
	bool is_graphic_thread=false;
	
	message_lock.lock();
		//save error parameters in the static variables
		error_occured=true;
		fatal_error=fatal;
		error_message=message;
		error_thread=QThread::currentThread();	//get the current thread pointer
		
		//compare current thread pointer to graphic thread one
		if(error_thread==graphic_thread) {
			is_graphic_thread=true;
		}
	message_lock.unlock();
	
	//treat error differently in graphic thread and computing thread
	if(is_graphic_thread) {
		//this is the graphic thread : call receive_error immediatly
		widget_error->receive_error();
	} else {
		//we're not in the graphic thread : wait until error is treated
		wait_lock.lock();
		synchronize.wait(&wait_lock);
		wait_lock.unlock();
	}
}

//Widget to trace computing
WidgetError::WidgetError(QWidget *parent) : QWidget( parent) {
	//WidgetError is in the graphic thread
	//save the pointer to the graphic thread
	Error::graphic_thread=QThread::currentThread();
	
	//save pointer to itself in order to give direct access to class Error
	Error::widget_error=this;
}

//this function should be called at regular intervals to check if an error occured
void WidgetError::receive_error() {
	QMutexLocker locker(&Error::message_lock);
	
	//test if an error occured
	if(!Error::error_occured) return;
	
	//initialize again error_occured (to handle a new error)
	Error::error_occured=false;
	
	if(Error::fatal_error) {
		QMessageBox::critical( this, "Fatal error", Error::error_message);
		Error::error_thread->terminate();   //kill the error thread (it's dirty and could cause a crash or a freeze)
	} else {
		QMessageBox::information( this, "Error", Error::error_message);
		Error::synchronize.wakeAll();  //error wasn't fatal : wake the thread
	}
}
