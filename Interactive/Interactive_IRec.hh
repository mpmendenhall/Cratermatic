//-----------------------------------------------------------------------
//
// CRATERMATIC Topography Analysis Toolkit
// Copyright (C) 2006 Michael Mendenhall
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1
//
//-----------------------------------------------------------------------

class aFindcraters: public Action {
public:
	aFindcraters() : Action(){
		description="Place results in <output directory> from finding craters in an (elevation) <Image>";
		addname("findcraters");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		if(!mystack->checkFolder()) return;
		char* c = mystack->getstring(0);
		((Image*)mystack->get(1))->findcraters(c, NULL, NULL, 0.25, 0.10, 0.06, 0.02, 0.5, 0.33, 0.33);
		mystack->drop();
	}
};

class aFindcratersParams: public Action {
public:
	aFindcratersParams() : Action(){
		description="Use 7 tuning parameters <<k1...k7>> and specified <products directory> for finding craters in an (elevation) <Image>";
		addname("findcratersP");
		ninputs = 9;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_CFLOAT;
		inputtypes[3] = COF_CFLOAT;
		inputtypes[4] = COF_CFLOAT;
		inputtypes[5] = COF_CFLOAT;
		inputtypes[6] = COF_CFLOAT;
		inputtypes[7] = COF_CRATERSTRING;
		inputtypes[8] = COF_IMAGE;
	};
	
	void DoIt() {
		if(!mystack->checkFolder()) return;
		char* c = mystack->getstring(7);
		((Image*)mystack->get(8))->findcraters(c, NULL, NULL, mystack->getfloat(6),
											   mystack->getfloat(5), mystack->getfloat(4), mystack->getfloat(3), 
											   mystack->getfloat(2), mystack->getfloat(1), mystack->getfloat(0));
	}
};