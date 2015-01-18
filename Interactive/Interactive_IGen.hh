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

class aSpeckles: public Action {
public:
	aSpeckles() : Action(){
		description="Created a speckled RGBImage of same size as <Image>";
		addname("speckles");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		RGBImage* C = new RGBImage((RectRegion*)mystack->get());
		C->speckfield();
		mystack->push(C);
	}
};

//-----------------------------

class aPoints: public Action {
public:
	aPoints() : Action(){
		description="Create an RGBImage of a grid with uniform <point spacing> with the same dimensions as <Image>";
		addname("points");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		RGBImage* C = new RGBImage((RectRegion*)mystack->get(1));
		C->pointfield(mystack->getint(0));
		mystack->drop();
		mystack->push(C);
	}
};

//-----------------------------

class aGrids: public Action {
public:
	aGrids() : Action(){
		description="Create an RGBImage of a grid with uniform <line spacing> with the same dimensions as <Image>";
		addname("grids");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		RGBImage* C = new RGBImage((RectRegion*)mystack->get(1));
		C->gridfield(mystack->getint(0));
		mystack->drop();
		mystack->push(C);
	}
};

//-----------------------------

class aAnnimg: public Action {
public:
	aAnnimg() : Action(){
		description="Generate an Image of an annulus with specified <outer>, <inner> radii";
		addname("annimg");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		Image* J = Image::annulusimage(mystack->getint(0),mystack->getint(1));
		mystack->drop();
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aCircimg: public Action {
public:
	aCircimg() : Action(){
		description="Generate an Image of a filled circle of the specified <radius>";
		addname("circimg");
		ninputs = 1;
		inputtypes[0] = COF_CFLOAT;
	};
	
	void DoIt() {
		Image* J = Image::filledcircleimage(mystack->getint(0));
		mystack->drop();
		mystack->push(J);
	}
};

//-----------------------------

class aNeatopic: public Action {
public:
	aNeatopic() : Action(){
		description="Generate some neato picture with specified <side length>";
		addname("neatopic");
		ninputs = 0;
	};
	
	void DoIt() {
        return;
		if(1) mystack->push(ClassifyImage::neatopic(100));
		else mystack->push(ClassifyImage::simplepic());
	}
};

//-----------------------------

class aColorwheel: public Action {
public:
	aColorwheel() : Action(){
		description="Generate an RGBImage colorwheel picture with the specified <radius>";
		addname("colorwheel");
		ninputs = 1;
		inputtypes[0] = COF_CFLOAT;
	};
	
	void DoIt() {
		int r = mystack->getint(0);
		mystack->drop();
		mystack->push(RGBImage::colorwheel(r));
	}
};

//-----------------------------

class aFourierSpinner: public Action {
public:
	aFourierSpinner() : Action(){
		description="Generate an RGBImage image of sin(<phi>+<k>*theta) of the specified <radius>";
		addname("fourierspinner");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_CFLOAT;
	};
	
	void DoIt() {
		float phi = mystack->getfloat(0);
		int k = mystack->getint(1);
		int r = mystack->getint(2);
		mystack->drop();
		mystack->drop();
		mystack->drop();
		mystack->push(RGBImage::fourierSpinner(r,k,phi));
	}
};