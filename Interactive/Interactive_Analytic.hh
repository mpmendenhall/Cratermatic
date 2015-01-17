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

class aCraterTransform: public Action {
public:
	aCraterTransform() : Action(){
		description="Apply a craterfinding transform of the specified <radius> and <gamma> to an <Image>";
		addname("ct");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)(mystack->get(1))) -> craterFindingTransform(mystack->getfloat(0),NULL);
		mystack->drop();
		mystack->push(J);
	}
	
};

//-----------------------------

class aMaskedCraterFindingTransform: public Action {
public:
	aMaskedCraterFindingTransform() : Action(){
		description="Apply a craterfinding transform of the specified <radius> to an <Image>, masking the slope by <Image>";
		addname("mct");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
		inputtypes[2] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)(mystack->get(1))) -> craterFindingTransform(mystack->getfloat(0),(Image*)mystack->get(2));
		mystack->drop();
		mystack->drop();
		mystack->push(J);
	}
	
};

//-----------------------------

class aDx: public Action {
public:
	aDx() : Action(){
		description="Take derivative d/dx of <Image> data";
		addname("dx");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get())->deriv(true);
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aDy: public Action {
public:
	aDy() : Action(){
		description="Take derivative d/dy of <Image> data";
		addname("dy");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get())->deriv(false);
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aCurv: public Action {
public:
	aCurv() : Action(){
		description="calculate the (fake) profile curvature of an <Image>";
		addname("curv");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get())->pseudo_profile_curvature();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aSlope: public Action {
public:
	aSlope() : Action(){
		description="calculate the slope of an <Image> (difference between highest and lowest neighbor of each point)";
		addname("slope");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get())->slope();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aDirection: public Action {
public:
	aDirection() : Action(){
		description="calculate the slope direction of an <Image> by using 'atan2' on dx,dy";
		addname("direction");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* dx = ((Image*)mystack->get())->deriv(true);
		Image* dy = ((Image*)mystack->get())->deriv(false);
		Image* J = Image::directionfield(dx,dy);
		delete(dx); delete(dy);
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aHtransform: public Action {
public:
	aHtransform() : Action(){
		description="Apply an H-Transform with the specified <window radius> to an <Image>";
		addname("ht");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get(1))->htransform(mystack->getint(0));
		mystack->drop();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aSHtransform: public Action {
public:
	aSHtransform() : Action(){
		description="Apply a smoothe-edged H-Transform with the specified <window radius> to an <Image>";
		addname("sht");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get(1))->smoothehtransform(mystack->getfloat(0));
		mystack->drop();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class ablur: public Action {
public:
	ablur() : Action(){
		description="Apply gaussian blur with specified <radius> to <Image>";
		addname("blur");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* J = ((Image*)mystack->get(1))->gaussianblur(mystack->getfloat(0));
		mystack->drop();
		mystack->drop();
		mystack->push(J);
	}
};