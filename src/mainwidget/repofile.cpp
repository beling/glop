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

#include "repofile.h"

#include <QtGui>

QString RepoFile::repositoryPath;
QString RepoFile::serverRepositoryPath;
bool RepoFile::isLocal=true;

QString RepoFile::getPath() {
	if(isLocal) {
		return repositoryPath;
	} else {
		return serverRepositoryPath;
	}
}

void RepoFile::setPath(QString path) {
	if(isLocal) {
		repositoryPath = path;
	} else {
		serverRepositoryPath = path;
	}
}

//return the absolute file name corresponding to a file in the repository
QString RepoFile::getAbsoluteFileName(QString shortFileName) {
	if(isLocal) {
		QDir directory(repositoryPath);
		return directory.absoluteFilePath(shortFileName);
	} else {
		QDir directory(serverRepositoryPath);
		return directory.absoluteFilePath(shortFileName);
	}
}

//!< get a "unique" new filename to save a database or an xml file
//the returned value is a short file name without the path to the repository
QString RepoFile::getNewFileName(const QString &identifierPart) {
	QString fileName;
	QString shortFileName;
	unsigned int a = 0;
	bool fileExist = true;
	while(fileExist) {
		shortFileName = QDate::currentDate().toString(Qt::ISODate) + QString("-%1%2.spr").arg(identifierPart).arg(a);
		fileName = getAbsoluteFileName(shortFileName);
		
		QFile file(fileName);
		if( !file.open( QIODevice::ReadOnly ) ) {
			//file doesn't exist, we can use this name
			fileExist = false;
		} else {
			//file already exists. We need to increment the value of a
			file.close();
			a++;
			if(a>=500) { //to avoid a possible infinite loop
				cout << "RepoWidget::saveCurrentComputation() : could not find an available file name." << endl;
				return shortFileName;
			}
		}
	}
	
	return shortFileName;
}
