//-----------------------------------------------------------------------
//
// CRATERMATIC Topography Analysis Toolkit
// Copyright (C) 2006-2015 Michael Mendenhall
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
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

class aReduce: public Action {
public:
	aReduce() : Action(){
		description="Downsample <Image> by factor of 2 in each direction";
		addname("reduce");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get())->reduce();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aBilinear: public Action {
public:
	aBilinear() : Action(){
		description="Use bilinear interpolation to resize (by specified <scale factor> in each direction) an <Image> (Hint: sqrt(2) ~ 1.41421, 1/sqrt(2) ~ .707107)";
		addname("bilinear");
		addname("bl");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get(1))->bilinearscale(mystack->getfloat(0));
		mystack->drop();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aReraster: public Action {
public:
	aReraster() : Action(){
		description="\"Rotate\" by rerastering along Bresenham lines oriented in the (<y>,<x>) direction an <Image>";
		addname("reraster");
		addname("rr");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_IMAGE;
	};
	
	void DoIt() {
		RasterRegion* R = RasterRegion::scanFromImage((Image*)mystack->get(2),mystack->getint(1),mystack->getint(0));
		mystack->drop();
		mystack->drop();
		mystack->drop();
		mystack->push(R->makeImage());
		delete(R);
	}
};

//-----------------------------
