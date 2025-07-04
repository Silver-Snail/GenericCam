///////////////////////////////////////////////////////////////////////////////
// Name               : Run.h
// Purpose            : Defines a machine run.
// Thread Safe        : Yes
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

#ifndef RUN_H_
#define RUN_H_

/*!\class Run
 * \ingroup document
 * \brief Single run of a machine
 *
 * The workpiecePlacement matrix herein is for rotating the workpiece.
 *
 * The machine itself contains another matrix used for placing the
 * workpiece in the machine.
 *
 * Every operation run is centered around one Workpiece. (Please not that a
 * workpiece may exist over the course several machine Run%s.)
 *
 *
 *
 * \dot
 * digraph classes {
 *    rankdir=LR;
 *	  node [shape=record, fontname=Helvetica, fontsize=10];
 *    ini [ label="Initialisation"];
 *    tp1 [ label="Generated Toolpath"];
 *    tp2 [ label="Generated Toolpath"];
 *    cont [ label="..." shape=none];
 *
 *    ini -> tp1;
 *    tp1 -> tp2;
 *    tp2 -> cont;
 *  }
 *  \enddot
 *
 */

#include <stddef.h>
#include <wx/filename.h>
#include <wx/string.h>
#include <vector>

#include "../3D/AffineTransformMatrix.h"
#include "../3D/BoundingBox.h"
#include "../3D/OpenGLImage.h"
#include "../3D/Vector3.h"
#include "generator/DexelTarget.h"
#include "generator/Generator.h"
#include "Selection.h"

class Project;
class JSON;

class Run {
	friend class Project;
	friend class CommandRunAdd;

public:
	Run();
	virtual ~Run();
//	Run& operator=(const Run&);
//private:
//	Run(const Run&);

public:
	Project * parent; ///< Pointer back to the Project this Run belongs to.
	wxString name;

	// Coordinate system for run
	Selection coordX;
	Selection coordY;
	Selection coordZ;

	// Object(s) to work on
	Selection object;

	// Stock around object
	enum StockType {
		sObject, BoxTop, BoxCenter, BoxBottom
	} stocktype;
	Selection stockobject; //<! used, if the stocktype is Object

	Vector3 stocksize; //<! Size of a Box in x,y,z used as stock
	Vector3 stockorigin; //<! Point where in the stock the origin lies

	std::vector <Generator*> generators; ///< List of Generator%s applied to the workpiece in this run

	wxFileName machinefile;

	// Set up by Update()
	BoundingBox stock;
	AffineTransformMatrix origin; //!< Origin and coordinate-system

	DexelTarget base;
	DexelTarget start;

	std::vector <CNCPosition> toolpath;

private:
	size_t ID; //!< Internal ID number
	OpenGLImage touchpoint;

public:
	bool operator ==(const size_t ID) const;
	size_t GetID(void) const;
	size_t GetMaxGeneratorID() const;
	void Update(Project * project);

	void GenerateToolpaths(void);
	static double RecalculateTiming(std::vector <CNCPosition> * toolpath,
			double t0 = 0.0);
//	bool SaveToolpaths(wxFileName fileName);

	void Paint(void) const;
	void PaintVertices(void) const;

	void ToJSON(JSON &js) const;
	bool FromJSON(const JSON &js);

//	void ToolpathToStream(wxTextOutputStream & stream);
//	Workpiece* GetWorkpiece(void);
//	const Workpiece* GetWorkpiece(void) const;
//	ToolPath* GetFirstSelectedToolpath(void);
//	const ToolPath* GetFirstSelectedToolpath(void) const;

	friend bool operator<(const Run& a, const Run& b)
	{
		return a.name.Cmp(b.name) < 0;
	}

wxDECLARE_NO_COPY_CLASS(Run);
};

#endif /* RUN_H_ */
