///////////////////////////////////////////////////////////////////////////////
// Name               : GeneratorSurfaceDexel.h
// Purpose            :
// Thread Safe        : Yes
// Platform dependent : No
// Compiler Options   :
// Author             : Tobias Schaefer
// Created            : 23.12.2017
// Copyright          : (C) 2017 Tobias Schaefer <tobiassch@users.sourceforge.net>
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

#ifndef GENERATORSURFACEDEXEL_H_
#define GENERATORSURFACEDEXEL_H_

/*!\class GeneratorSurfaceDexel
 * \ingroup Generator
 * \brief ...
 *
 * ...
 */

#include "GeneratorFactory.h"
#include "GeneratorDexel.h"

#include <wx/stattext.h>
#include <wx/textctrl.h>

class GeneratorSurfaceDexel:public GeneratorDexel {
public:
	GeneratorSurfaceDexel(size_t ID);
	virtual ~GeneratorSurfaceDexel();

	virtual void CopyParameterFrom(const Generator * other);
	virtual size_t GetType(void) const
				{
					return TYPE_GENERATORSURFACE;
				}
	virtual wxString GetTypeName(void) const;
	virtual wxSizer * AddToPanel(wxPanel * panel, CollectionUnits* settings) const;
	virtual void TransferDataToPanel(wxPanel* panel,
			CollectionUnits* settings) const;
	virtual void TransferDataFromPanel(CollectionUnits* settings);
	virtual bool operator==(const Generator &b) const;
	virtual void GenerateToolpath(const Run &run,
			const std::list <Object> &objects, const Tool &tool,
			const DexelTarget &base);

public:

private:
	mutable wxStaticText* m_staticTextTwiddleFactor;
	mutable wxTextCtrl* m_textCtrlTwiddleFactor;
	mutable wxStaticText* m_staticTextUnit;

};

#endif /* GENERATORSURFACEDEXEL_H_ */
