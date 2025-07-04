///////////////////////////////////////////////////////////////////////////////
// Name               : CommandObjectLoad.cpp
// Purpose            : 
// Thread Safe        : No
// Platform dependent : No
// Compiler Options   :
// Author             : Tobias Schaefer
// Created            : 29.12.2014
// Copyright          : (C) 2014 Tobias Schaefer <tobiassch@users.sourceforge.net>
// Licence            : GNU General Public License version 3.0 (GPLv3)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include "CommandObjectLoad.h"

#include "../../Config.h"
#include "../Project.h"
#ifdef _DEBUGMODE
#include  <wx/stopwatch.h>
#endif

CommandObjectLoad::CommandObjectLoad(const wxString& name, Project * project,
		const wxString& fileName, const size_t objID) :
		wxCommand(true, name)
{
	this->project = project;
	this->fileName = fileName;
	this->ID = objID;
}

bool CommandObjectLoad::Do(void)
{
	Object temp(ID);
#ifdef _DEBUGMODE
	wxStopWatch sw;
#endif
	if(temp.LoadObject(fileName)){
		project->objects.push_back(temp);
#ifdef _DEBUGMODE
//		wxLogMessage(_T("CommandObjectLoad took %ld ms to execute."),
//				sw.Time());
#endif
		project->Modify(true);
		project->Update();
		return true;
	}
	return false;
}

bool CommandObjectLoad::Undo(void)
{
	std::list <Object>::iterator obj = project->objects.begin();
	while(obj!=project->objects.end()){
		if(*obj==ID){
			obj = project->objects.erase(obj);
		}else{
			++obj;
		}
	}
	project->Modify(true);
	project->Update();
	return true;
}
