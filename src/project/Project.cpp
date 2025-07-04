///////////////////////////////////////////////////////////////////////////////
// Name               : Project.cpp
// Purpose            :
// Thread Safe        : No
// Platform dependent : No
// Compiler Options   :
// Author             : Tobias Schaefer
// Created            : 15.06.2011
// Copyright          : (C) 2011 Tobias Schaefer <tobiassch@users.sourceforge.net>
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

#include "Project.h"

//#include "../StdInclude.h"

#include "../Config.h"
#include "../gui/IDs.h"

#include "../3D/BooleanBox.h"
#include "../3D/FileSTL.h"
#include "ToolBox.h"
#include "../math/JSON.h"

#include <wx/zipstrm.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/cmdproc.h>

#include <sstream>
#include <iostream>
#include <float.h>
#include <wx/dir.h>
#include <algorithm>

#include "../gui/FrameMain.h"
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif
#include <math.h>

IMPLEMENT_DYNAMIC_CLASS(Project, wxDocument);

#include "../3D/OpenGL.h"

Project::Project() :
		wxDocument()
{
	minResolution = 0.5e-3; // = 0.5 mm
	maxCells = 1e5;

	SetTitle(_("Untitled"));
	maxObjectID = 20000;
	maxRunID = 30000;
	maxGeneratorID = 40000;

//	LoadDefaultTools(
//			wxFileName(_T("/home/toby/Pojekte/genericcam/toolbox/local.json")));

}

Project::~Project()
{
	if(DEBUG) printf("Project::Destructor\n");
}

bool Project::Has(const Selection& sel) const
{
	std::set <size_t> IDs = sel.GetSet();
	if(sel.IsType(Selection::Object)){
		for(std::list <Object>::const_iterator itobj = objects.begin();
				itobj != objects.end(); ++itobj)
			IDs.erase(itobj->GetID());
	}
	if(sel.IsType(Selection::Run)){
		for(std::list <Run>::const_iterator itrun = run.begin();
				itrun != run.end(); ++itrun)
			IDs.erase(itrun->GetID());
	}
	if(sel.IsType(Selection::Generator) && sel.IsBaseType(Selection::BaseRun)){
		const size_t baseID = sel.GetBaseID();
		std::list <Run>::const_iterator itRun = std::find(run.begin(),
				run.end(), baseID);
		if(itRun == run.end()) return false;
		for(std::vector <Generator*>::const_iterator itgenerator =
				itRun->generators.begin();
				itgenerator != itRun->generators.end(); ++itgenerator)
			IDs.erase((*itgenerator)->GetID());
	}
	return IDs.empty();
}

bool Project::Has(const Selection::Type type, const size_t ID) const
{
	if(type == Selection::Object){
		if(std::find(objects.begin(), objects.end(), ID) != objects.end()) return true;
		return false;
	}
	if(type == Selection::Run){
		if(std::find(run.begin(), run.end(), ID) != run.end()) return true;
		return false;
	}
	return false;
}

size_t Project::GetNextObjectID(void)
{
	maxObjectID++;
	return maxObjectID;
}

size_t Project::GetNextRunID(void)
{
	maxRunID++;
	return maxRunID;
}

size_t Project::GetToolCount(void) const
{
	return tools.tools.size();
}

const std::vector <Tool> * Project::GetTools(void) const
{
	return &(tools.tools);
}

std::vector <size_t> Project::GetAllObjectIDs(void) const
{
	std::vector <size_t> temp;
	for(std::list <Object>::const_iterator it = objects.begin();
			it != objects.end(); ++it)
		temp.push_back(it->GetID());
	return temp;
}

std::vector <size_t> Project::GetAllRunIDs(void) const
{
	std::vector <size_t> temp;
	for(std::list <Run>::const_iterator it = run.begin(); it != run.end(); ++it)
		temp.push_back(it->GetID());
	return temp;
}

std::vector <size_t> Project::GetAllGeneratorIDs(size_t runID) const
{
	std::vector <size_t> temp;
	std::list <Run>::const_iterator itRun;
	itRun = std::find(run.begin(), run.end(), runID);
	if(itRun != run.end()){
		for(std::vector <Generator*>::const_iterator it =
				itRun->generators.begin(); it != itRun->generators.end(); ++it)
			temp.push_back((*it)->GetID());
	}
	return temp;
}

const Object * Project::Get3DObject(size_t ID) const
{
	std::list <Object>::const_iterator it;
	it = std::find(objects.begin(), objects.end(), ID);
	if(it == objects.end()) throw(std::range_error(
			"Project::GetObject - Object not found."));
	return &(*it);
}

const std::list <Object> * Project::GetObjects(void) const
{
	return &objects;
}

const Run * Project::GetRun(size_t ID) const
{
	std::list <Run>::const_iterator it;
	it = std::find(run.begin(), run.end(), ID);
	if(it == run.end()) throw(std::range_error(
			"Project::GetRun - Run not found."));
	return &(*it);
}

const Generator * Project::GetGenerator(size_t runID, size_t ID)
{
	std::list <Run>::const_iterator itRun;
	itRun = std::find(run.begin(), run.end(), runID);
	if(itRun == run.end()) throw(std::range_error(
			"Project::GetGenerator - Run not found."));

	std::vector <Generator *>::const_iterator itGenerator;
	itGenerator = itRun->generators.begin();
	while(itGenerator != itRun->generators.end()){
		if((*itGenerator)->GetID() == ID) break;
		++itGenerator;
	}
	if(itGenerator == itRun->generators.end()) throw(std::range_error(
			"Project::GetGenerator - Generator not found."));
	return *itGenerator;
}

const Tool * Project::GetTool(size_t index) const
{
	if(index >= tools.tools.size()) throw(std::range_error(
			"Project::GetTools - index out of range."));
	return &(tools[index]);
}

size_t Project::GetNextGeneratorID(void)
{
	maxGeneratorID++;
	return maxGeneratorID;
}

BoundingBox Project::GetBBox(const Selection &selected) const
{
	BoundingBox temp;
	if(selected.IsType(Selection::Object)){
		std::set <size_t> set = selected.GetSet();
		if(selected.IsInverted()){

		}else{
			for(std::set <size_t>::const_iterator it = set.begin();
					it != set.end(); ++it){
				std::list <Object>::const_iterator obj;
				obj = std::find(objects.begin(), objects.end(), *it);
				if(obj != objects.end()) temp.Insert(obj->bbox);
			}
		}
	}
	return temp;
}

bool Project::GenerateToolpaths(void)
{
	Update();

//	const AffineTransformMatrix rotx0 = AffineTransformMatrix::Identity();
//	const AffineTransformMatrix rotx180 = AffineTransformMatrix::RotationXYZ(
//	M_PI, 0, 0);

	for(std::vector <Tool>::iterator it = tools.tools.begin();
			it != tools.tools.end(); ++it)
		it->Update();

	for(std::list <Run>::iterator run = this->run.begin();
			run != this->run.end(); ++run){
		run->GenerateToolpaths();
	}

	return true;
//#if(_GENERICCAM_USEMULTITHREADING == 1)
//
//	// Prevent the toolpath generation from being started in more than
//	// one thread.
//	if(mtx_generator.TryLock() != wxMUTEX_NO_ERROR) return false;
//
//	// Wait for the last command to finish. (For these
//	// are the only other function locking the project.)
//	mtx_project.Lock();
//
//	PropagateChanges();
//

//	// Select and start the next generators to have a go
//	// The selection is done workpiece-wise, even if spread across multiple runs.
//	for(std::map<size_t, Run>::iterator run = this->run.begin();run!=this->run.end();++run){
////		if(workpieces[workpieceNr].hasRunningGenerator) continue;
//		for(size_t runNr = 0; runNr < run.GetCount(); runNr++){
//			if(run[runNr].workpieceNr != workpieceNr) continue;
//
//			// Find the first generator, that has not generated its toolpath.
//			for(size_t toolpathNr = 0;
//					toolpathNr < run[runNr].toolpaths.GetCount(); toolpathNr++){
//				if(run[runNr].toolpaths[toolpathNr].generator->toolpathGenerated) continue;
//
//				// Generate a detached thread. on exit it signals the workpiece to be free for
//				// other generators.
//				workpieces[workpieceNr].hasRunningGenerator = true;
//				wxThread * thread = new ToolpathGeneratorThread(this, runNr,
//						toolpathNr);
//				if(thread->Create() != wxTHREAD_NO_ERROR){
//					workpieces[workpieceNr].hasRunningGenerator = false;
//					wxLogError(
//							_(
//									"Could not create new thread for toolpath generation."));
//				} else{
//					thread->Run();
//				}
//				break;
//			}
//			if(workpieces[workpieceNr].hasRunningGenerator) break;
//		}
//	}
//
//	mtx_project.Unlock();
//
//	mtx_generator.Unlock();
//	return true;
//#else
//	return false;
//#endif

}

void Project::Update(void)
{
	for(std::list <Object>::iterator it = objects.begin();
			it != objects.end(); ++it)
		it->Update();

	for(std::list <Run>::iterator it = run.begin(); it != run.end(); ++it)
		it->Update(this);

	UpdateAllViews();
}

bool Project::DoSaveDocument(const wxString& filename)
{
	setlocale(LC_ALL, "C");

	wxFFileOutputStream out(filename);
	wxZipOutputStream zip(out);
	wxTextOutputStream txt(zip);

	zip.PutNextEntry(_T("project.json"));
	JSON js;
	js.SetObject();
	js["Name"].SetString(this->GetTitle().ToStdString());
	{
		JSON &objs = js["Objects"];
		objs.SetArray(objects.size());
		size_t n = 0;
		for(std::list <Object>::const_iterator obj = objects.begin();
				obj != objects.end(); ++obj)
			obj->ToJSON(objs[n++]);
	}
	{
		JSON &r = js["Run"];
		r.SetArray(run.size());
		size_t n = 0;
		for(std::list <Run>::const_iterator it = run.begin(); it != run.end();
				++it)
			it->ToJSON(r[n++]);
	}
	{
		std::ostringstream out;
		js.Save(out);
		txt << out.str();
	}

	zip.PutNextEntry(_T("tools.json"));
	{
		std::ostringstream out;
		tools.Save(out);
		txt << out.str();
	}

	for(std::list <Object>::const_iterator obj = objects.begin();
			obj != objects.end(); ++obj){

		wxString tempName = wxString::Format(_T("object_%zu.obj"),
				obj->GetID());
		zip.PutNextEntry(tempName);
		std::ostringstream out;
		out << obj->geometry;
		txt << out.str();
	}

	setlocale(LC_ALL, "");
	this->SetDocumentSaved(true);
	GetCommandProcessor()->MarkAsSaved();
	this->Modify(false);
	this->SetFilename(filename, true);
	if(DEBUG) std::cout << "Project::Save(" << filename << ") - saved.\n";

	return true;
}

bool Project::DoOpenDocument(const wxString& filename)
{
	if(DEBUG) std::cout << "Project::DoOpenDocument(" << filename.ToStdString()
			<< ")\n";
	setlocale(LC_ALL, "C");
	wxFFileInputStream in(filename);

	if(!in.IsOk()){
		std::cout << "File is not OK: ";
		std::cout << filename.ToStdString();
		std::cout << "\n";
		return false;
	}

	wxZipInputStream zip(in);
	wxTextInputStream txt(zip);

	tools.tools.clear();
	objects.clear();
	run.clear();

	wxZipEntry* entry;
	in.SeekI(0, wxFromStart);

	while((entry = zip.GetNextEntry()))
		if(entry->GetName().Cmp(_T("project.json")) == 0) break;
	if(entry == NULL){
		setlocale(LC_ALL, "");
		if(DEBUG) std::cout << "Entry 'project.json' not found.\n";
		return false;
	}
	{
		zip.OpenEntry(*entry);
		size_t N = entry->GetSize();
		char* buffer = new char[N + 1];
		zip.Read(buffer, N);
		size_t L = zip.LastRead();
		assert(L <= N);
		std::istringstream in(std::string(buffer, L));

		JSON js = JSON::Load(in);

		this->SetTitle(js["Name"].GetString());
		{
			JSON &objs = js["Objects"];
			objects.resize(objs.Size(), Object(0));
			size_t n = 0;
			for(std::list <Object>::iterator obj = objects.begin();
					obj != objects.end(); ++obj)
				obj->FromJSON(objs[n++]);
		}

		FrameMain* frame = wxStaticCast(GetFirstView()->GetFrame(), FrameMain);
		wxDir machinedirectory(frame->GetFilePaths()->lastMachineDirectory);

		JSON &r = js["Run"];
		for(size_t n = 0; n < r.Size(); ++n){
			try{
				std::list <Run>::iterator it = run.emplace(run.end());
				it->FromJSON(r[n]);
				if(it->GetID() > maxRunID) maxRunID = it->GetID();
				if(it->GetMaxGeneratorID() > maxGeneratorID) maxGeneratorID =
						it->GetMaxGeneratorID();
				it->machinefile.SetPath(machinedirectory.GetName());
				it->machinefile.SetExt(_T("zip"));
				if(!it->machinefile.IsFileReadable()){
					std::cout << "File: "
							<< it->machinefile.GetFullName().ToStdString()
							<< "\n";
					std::cout
							<< "Project::DoOpenDocument - Machine file not found.\n";
				}
			}
			catch(std::exception &e){
				std::cout << "Project::DoOpenDocument - While loading run: "
						<< e.what() << "\n";
			}
		}
		if(DEBUG) std::cout << "Project::DoOpenDocument - All Run loaded.\n";
		zip.CloseEntry();
		delete[] buffer;
	}
	// Load objects
	//TODO: Rewind zip, because there may be object models before the main project file.

	while((entry = zip.GetNextEntry())){
		wxString temp = entry->GetName();
		if(temp.compare(_T("tools.json")) == 0){
			zip.OpenEntry(*entry);
			size_t N = entry->GetSize();
			char* buffer = new char[N + 1];
			zip.Read(buffer, N);
			size_t L = zip.LastRead();
			assert(L <= N);
			std::istringstream in(std::string(buffer, L));
			tools.Load(in);
			zip.CloseEntry();
			delete[] buffer;
			if(DEBUG) std::cout << "Tools loaded\n";
		}
		if(temp.StartsWith(wxT("object_"), &temp)){
			long p;
			temp.BeforeFirst('.').ToLong(&p);
			size_t objID = p;
			if(objID > maxObjectID) maxObjectID = objID;
			temp = temp.AfterFirst('.');
			if(!temp.StartsWith(wxT("obj"))) continue;
			std::list <Object>::iterator obj = std::find(objects.begin(),
					objects.end(), objID);
			if(obj == objects.end()) continue;

			zip.OpenEntry(*entry);
			size_t N = entry->GetSize();
			if(DEBUG) std::cout << "Expected object size: " << N << "bytes.\n";
			char* buffer = new char[N + 1];
			zip.Read(buffer, N);
			size_t L = zip.LastRead();
			assert(L <= N);
			std::istringstream in(std::string(buffer, L));
			Hull temphull;
			in >> temphull;
			obj->geometry = temphull;
			obj->geometry.CalcNormals();
			obj->geometry.CalcGroups();
			obj->geometry.ApplyTransformation();
			zip.CloseEntry();
			delete[] buffer;
		}
	}
	setlocale(LC_ALL, "");
	this->SetFilename(filename, true);
	this->SetDocumentSaved(true);
	GetCommandProcessor()->MarkAsSaved();
	this->Modify(false);
	this->Update();
	return true;
}

//void Project::PropagateChanges(void)
//{
//	// Testing preconditions.
//	// Propagate modifcation flag from object to workpiece
//	for(size_t workpieceNr = 0; workpieceNr < workpieces.GetCount();
//			workpieceNr++){
//		for(size_t placementNr = 0;
//				placementNr < workpieces[workpieceNr].placements.GetCount();
//				placementNr++){
//			int objectNr =
//					workpieces[workpieceNr].placements[placementNr].objectNr;
//			if(objectNr >= 0){
//				if(objects[objectNr].modified) workpieces[workpieceNr].modified =
//						true;
//			}
//		}
//	}
//
//	// Propagate modification flag from workpiece to run to the generators
//	for(size_t runNr = 0; runNr < run.GetCount(); runNr++){
//		int workpieceNr = run[runNr].workpieceNr;
//		if(workpieceNr >= 0){
//			if(workpieces[workpieceNr].modified) run[runNr].modified = true;
//		}
//		if(run[runNr].modified){
//			for(size_t toolpathNr = 0;
//					toolpathNr < run[runNr].toolpaths.GetCount(); toolpathNr++){
//				run[runNr].toolpaths[toolpathNr].generator->toolpathGenerated =
//						false;
//			}
//		}
//	}
//
//}

//size_t Project::ToolpathToGenerate(void)
//{
//	size_t maxNr = 0;
//	for(size_t workpieceNr = 0; workpieceNr < workpieces.GetCount();
//			workpieceNr++){
//		if(workpieces[workpieceNr].hasRunningGenerator) continue;
//		for(size_t runNr = 0; runNr < run.GetCount(); runNr++){
//			if(run[runNr].workpieceNr != workpieceNr) continue;
//			maxNr += run[runNr].toolpaths.GetCount();
//		}
//	}
//
//	generator_workpieceNr = 0;
//	generator_runNr = 0;
//	generator_toolpathNr = 0;
//
//	return maxNr;
//}
//
//bool Project::ToolpathGenerate(void)
//{
//	if(generator_workpieceNr >= workpieces.GetCount()) return false;
//
//	bool flag = true;
//	if(flag && workpieces[generator_workpieceNr].hasRunningGenerator) flag =
//			false;
//
//	if(flag && run[generator_runNr].workpieceNr != generator_workpieceNr) flag =
//			false;
//
//	if(flag
//			&& run[generator_runNr].toolpaths[generator_toolpathNr].generator->toolpathGenerated) flag =
//			false;
//
//	if(flag){
//		run[generator_runNr].toolpaths[generator_toolpathNr].generator->GenerateToolpath();
//		workpieces[generator_workpieceNr].hasRunningGenerator = false;
//	}
//
//	generator_toolpathNr++;
//	if(generator_toolpathNr >= run[generator_runNr].toolpaths.GetCount()){
//		generator_toolpathNr = 0;
//		generator_runNr++;
//	}
//	if(generator_runNr >= run.GetCount()){
//		generator_runNr = 0;
//		generator_workpieceNr++;
//	}
//	if(generator_workpieceNr >= workpieces.GetCount()) return false;
//	return true;
//}

//wxString Project::ToolPathGenerateCurrent(void)
//{
//	if(generator_workpieceNr >= workpieces.GetCount()) return _T("");
//	if(generator_runNr >= run.GetCount()) return _T("");
//	if(generator_workpieceNr >= run[generator_runNr].toolpaths.GetCount()) return _T(
//			"");
//	return (workpieces[generator_workpieceNr].name + _T(" - ")
//			+ run[generator_runNr].toolpaths[generator_toolpathNr].generator->GetName());
//}

//bool Project::SaveToolpath(wxFileName fileName, int runNr)
//{
//	if(runNr < 0 || runNr > run.GetCount()) return false;
//	return run[runNr].SaveToolpaths(fileName);
//	return false;
//}

bool Project::LoadDefaultTools(wxString fileName, bool loadAll)
{
	ToolBox localtools;
	if(loadAll) tools.tools.clear();

	// Only load the default tools into an empty project.
	if(!tools.tools.empty()) return true;

	if(!localtools.Load(fileName.ToStdString())) return false;
	for(size_t n = 0; n < localtools.Size(); ++n){
		if(!loadAll && !localtools[n].addtonewprojects) continue;
		tools.tools.push_back(localtools[n]);
	}

//	tools.Save("/tmp/tools.json");
//	ToolBox test;
//	bool success = test.Load("/tmp/tools.json");
//	if(success) std::cout << "File could be read back.\n";
	return true;
}

bool Project::IsModified() const
{
	return wxDocument::IsModified() && GetCommandProcessor()->IsDirty();
}

//void Project::FlipRun(void)
//{
//	size_t n = run[activeRun].placements.GetCount();
//	if(n < 4) return;
//	size_t i, j, m, p;
//	Vector3 temp;
//	double dx, dy;
//	for(i = 1; i < n; i++){
//		m = run[activeRun].placements[i].outLine.elements.GetCount();
//
//		if(run[activeRun].placements[i].isKeepout){
//			for(j = 0; j < m; j++)
//				run[activeRun].placements[i].outLine.elements[j].y =
//						-run[activeRun].placements[i].outLine.elements[j].y;
//
//			temp = run[activeRun].placements[i].matrix.GetCenter();
//			run[activeRun].placements[i].matrix.TranslateGlobal(0.0,
//					(middleY - temp.y) * 2, 0.0);
//
//		}else{
//			p = run[activeRun].placements[i].targetNumber;
//			dx = targets[p].GetSizeX() / 2;
//			dy = targets[p].GetSizeY() / 2;
//
//			for(j = 0; j < m; j++)
//				run[activeRun].placements[i].outLine.elements[j].y = dy * 2
//						- run[activeRun].placements[i].outLine.elements[j].y;
//
//			run[activeRun].placements[i].matrix.TranslateLocal(+dx, +dy, 0.0);
//
//			temp = run[activeRun].placements[i].matrix.GetCenter();
//			double alpha = atan2(run[activeRun].placements[i].matrix.a[1],
//					run[activeRun].placements[i].matrix.a[0]);
//
//			AffineTransformMatrix tr;
//			tr = AffineTransformMatrix::RotateXYZ(0.0, 0.0, -2 * alpha);
//			run[activeRun].placements[i].matrix =
//					run[activeRun].placements[i].matrix * tr;
//
//			run[activeRun].placements[i].matrix.TranslateGlobal(0.0,
//					2 * (middleY - temp.y), 0.0);
//
//			run[activeRun].placements[i].matrix.TranslateLocal(-dx, -dy, 0.0);
//
//		}
//
//	}
//	for(i = 0; i < targets.GetCount(); i++){
//		targets[i].FlipX();
//		targets[i].toolpathFlipped = targets[i].toolpath;
//		targets[i].toolpath.Clear();
//		targets[i].refresh = true;
//	}
//}

