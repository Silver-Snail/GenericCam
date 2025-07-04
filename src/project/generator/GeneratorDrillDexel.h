///////////////////////////////////////////////////////////////////////////////
// Name               : GeneratorDrillDexel.h
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

#ifndef GENERATORDRILLDEXEL_H_
#define GENERATORDRILLDEXEL_H_

/*!\class GeneratorDrillDexel
 * \ingroup Generator
 * \brief ...
 *
 * ...
 */

#include "GeneratorFactory.h"
#include "GeneratorDexel.h"

#include <wx/stattext.h>
#include <wx/textctrl.h>

class GeneratorDrillDexel:public GeneratorDexel {
public:
	GeneratorDrillDexel(size_t ID);
	virtual ~GeneratorDrillDexel();

	virtual void CopyParameterFrom(const Generator * other);
	virtual bool operator==(const Generator &b) const;
	virtual size_t GetType(void) const
	{
		return TYPE_GENERATORDRILLDEXEL;
	}
	virtual wxString GetTypeName(void) const
	{
		return _("Drillpattern (dexel-based)");
	}
	virtual wxSizer * AddToPanel(wxPanel * panel,
			CollectionUnits * settings) const;
	virtual void TransferDataToPanel(wxPanel* panel,
			CollectionUnits* settings) const;
	virtual void TransferDataFromPanel(CollectionUnits* settings);
	virtual void GenerateToolpath(const Run &run,
			const std::list <Object> &objects, const Tool &tool,
			const DexelTarget &base);

public:

private:
	mutable wxStaticText* m_staticTextTwiddleFactor;
	mutable wxTextCtrl* m_textCtrlTwiddleFactor;
	mutable wxStaticText* m_staticTextUnit;

};

#endif /* GENERATORDRILLDEXEL_H_ */
