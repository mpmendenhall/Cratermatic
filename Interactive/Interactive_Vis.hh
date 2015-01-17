//-----------------------------------------------------------------------
//
// CRATERMATIC Topography Analysis Toolkit
// Copyright (C) 2006 Michael Mendenhall
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

class aColorview: public Action {
public:
	aColorview() : Action(){
		description="Turn an <Image> into a colorized RGBImage";
		addname("colorview");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		RGBImage* C = new RGBImage((Image*)mystack->get());
		mystack->drop();
		mystack->push(C);
	}
};

//-----------------------------

class aColorvec: public Action {
public:
	aColorvec() : Action(){
		description="Create an RGBImage of the vector field represented by <X Image>, <Y Image>";
		addname("colorvec");
		ninputs = 2;
		inputtypes[0] = COF_IMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		mystack->push(new RGBImage((Image*)mystack->get(0),(Image*)mystack->get(1)));
	}
};

//-----------------------------

class aGrayview: public Action {
public:
	aGrayview() : Action(){
		description="Turn an <Image> into a greyscale RGBImage";
		addname("grayview");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* I = (Image*)mystack->get();
		RGBImage* C = RGBImage::grayby(I,0.05);
		mystack->drop();
		mystack->push(C);
	}
};

//-----------------------------

class aShadeby: public Action {
public:
	aShadeby() : Action(){
		description="Set the luminance channel of an <RGBImage> by <Image> data";
		addname("shadeby");
		ninputs = 2;
		inputtypes[0] = COF_RGBIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((RGBImage*)mystack->get(0))->shadeby((Image*)mystack->get(1));
		mystack->swap();
		mystack->drop();
	}
};

//-----------------------------

class aEmboss: public Action {
public:
	aEmboss() : Action(){
		description="\"Emboss\" an <RGBImage> by an <Image>";
		addname("emboss");
		ninputs = 2;
		inputtypes[0] = COF_RGBIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((RGBImage*)mystack->get(0))->emboss((Image*)mystack->get(1));
		mystack->swap();
		mystack->drop();
	}
};

//-----------------------------

class aRbs: public Action {
public:
	aRbs() : Action(){
		description="Red-Cyan stereo-ize (with specified <depth>) an <RGBImage> by an <Image>";
		addname("rbs");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_RGBIMAGE;
		inputtypes[2] = COF_IMAGE;
	};
	
	void DoIt() {
		((RGBImage*)mystack->get(1))->qrbs((Image*)mystack->get(2),mystack->getfloat(0));
		mystack->drop();
		mystack->swap();
		mystack->drop();
	}
};

//-----------------------------

class aOverlay: public Action {
public:
	aOverlay() : Action(){
		description="Overlay an <RGBImage> with an <Image>";
		addname("overlay");
		ninputs = 2;
		inputtypes[0] = COF_RGBIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((RGBImage*)mystack->get(0))->overlay((Image*)mystack->get(1),0,0,0,1);
		mystack->swap();
		mystack->drop();
	}
};

//-----------------------------

class aRenderTopo: public Action {
public:
	aRenderTopo() : Action(){
		description="Create a nicely colored and shaded topography RGBImage from <Image>";
		addname("rendertopo");
		addname("topo");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		mystack->push(RGBImage::renderTopo((Image*)mystack->get(0)));
	}
};

//-----------------------------
