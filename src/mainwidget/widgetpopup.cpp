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

#include "widgetpopup.h"

#include <QtGui>

WidgetPopup::WidgetPopup( QWidget* parent, QString fileName, QString windowsTitle) : QDialog( parent) {
	//-------------text display-----------------------------------
	QTextBrowser *textEditPopup = new QTextBrowser( this );
	
	//text size (it implies the windows size)
	textEditPopup->setMinimumWidth(700);
	textEditPopup->setMinimumHeight(450);
	
	//text parameters
	textEditPopup->setReadOnly( TRUE );
	textEditPopup->setOpenExternalLinks( TRUE );	//use external software to open web links
	
	if(!fileName.isEmpty()) {
		QFile htmlFile(fileName);
		htmlFile.open( QIODevice::ReadOnly );
		QTextStream text_stream( &htmlFile );
		text_stream.setCodec("ISO 8859-1");
		textEditPopup->setHtml(text_stream.readAll());	//read file
		htmlFile.close();
	}
	
	//--------------horizontal layout with "Ok" button------------
	QWidget *buttons = new QWidget( this );
	QHBoxLayout *buttonsLayout = new QHBoxLayout(buttons);
	buttonsLayout->setMargin(0);
	
	//Ok button (with focus)
	QPushButton *okPushButton = new QPushButton( "Ok", buttons );
	okPushButton->setFocus();
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okPushButton);
	buttonsLayout->addStretch(1);
	
	//------------------vertical layout --------------------------
	QVBoxLayout *vboxPopup = new QVBoxLayout( this );
	vboxPopup->setSpacing(5);
	vboxPopup->setMargin(5);
	
	vboxPopup->addWidget( textEditPopup );
	vboxPopup->addWidget( buttons );
	
	//----------------- popup ------------------
	//windows title
	if(!windowsTitle.isEmpty()) {
		setWindowTitle(windowsTitle);
	}
	
	//close popup when ok button is pushed
	connect( okPushButton, SIGNAL( clicked() ), this, SLOT( close() ) );
}
