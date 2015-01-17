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

class aLoad: public Action {
public:
	aLoad() : Action(){
		description="Load an <ASCII data file>";
		addname("load");
		addname("l");
		ninputs = 1;
		inputtypes[0] = COF_CRATERSTRING;
	};
	void DoIt() {
		if(!mystack->checkReadable()) return;
		string c = mystack->getstring(0);
		Image* I = new Image(0,0);
		I->loadarcgis(c);
		mystack->drop();
		mystack->push(I);
	}
};

//----------------------------

class aLoadppm: public Action {
public:
	aLoadppm() : Action(){
		description="Load a <PPM data file> as 3 color channels";
		addname("loadppm");
		addname("lppm");
		ninputs = 1;
		inputtypes[0] = COF_CRATERSTRING;
	};
	void DoIt() {
		if(!mystack->checkReadable()) return;
		string c = mystack->getstring(0);
		mystack->push(Image::loadppm(c,0));
		mystack->push(Image::loadppm(c,1));
		mystack->push(Image::loadppm(c,2));
		mystack->rot(4);
		mystack->drop();
	}
};

//----------------------------

class aLoadrawdat: public Action {
public:
	aLoadrawdat() : Action(){
		description="Load <raw binary data file> of dimensions <width>*<height> at specified <bit depth>";
		addname("loadraw");
		ninputs = 4;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_CFLOAT;
		inputtypes[3] = COF_CFLOAT;
	};
	void DoIt() {
		if(!mystack->checkReadable()) return;
		string c = mystack->getstring(0);
		Image* J = Image::loadrawbinary(c,mystack->getint(1),mystack->getint(2),mystack->getint(3));
		mystack->drop();
		mystack->drop();
		mystack->drop();
		mystack->push(J);
		
	}
};

//----------------------------


class aSave: public Action {
public:
	aSave() : Action(){
		description="Save an <ASCII data file> from <Image>";
		addname("save");
		addname("s");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_IMAGE | COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		if(!mystack->checkWritable()) return;
		string c = mystack->getstring(0);
		printf("Saving image data to ascii file '%s'\n",c.c_str());
		CratersBaseObject* foo = mystack->get(1);
		if(foo->isaNum == COBJ_IMAGE) ((Image*)foo)->writeArcGIS(c);
		else if(foo->isaNum == COBJ_CLASSIFYIMAGE) ((ClassifyImage*)foo)->writeArcGIS(c);
		mystack->drop();
	};
};

//----------------------------

class aSavepgm: public Action {
public:
	aSavepgm() : Action(){
		description="Save a <PGM format file> from <Image>";
		addname("savepgm");
		addname("spgm");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		if(!mystack->checkReadable()) return;
		string c = mystack->getstring(0);
		printf("Saving image data to PGM file '%s'\n",c.c_str());
		((Image*)mystack->get(1))->writePGM1(c);
		mystack->drop();
	};
};

//----------------------------

class aSaverawbinary: public Action {
public:
	aSaverawbinary() : Action(){
		description="Save a <raw binary file> with the specified <bit depth> from <Image>";
		addname("saveraw");
		ninputs = 3;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_IMAGE;
	};
	
	void DoIt() {
		if(!mystack->checkWritable()) return;
		string c = mystack->getstring(0);
		printf("Saving image data to raw binary format file '%s'\n",c.c_str());
		((Image*)mystack->get(2))->writerawbinary(c,mystack->getint(1));
		mystack->drop();
		mystack->drop();
	};
};

//----------------------------

class aDraw: public Action {
public:
	aDraw() : Action(){
		description="Draw to the specified <BMP format graphics file> a picture of <Image | RGBImage>";
		addname("draw");
		addname("d");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_IMAGE | COF_RGBIMAGE;
	};
	
	void DoIt() {
		if(!mystack->checkWritable()) return;
		string c = mystack->getstring(0);
		CratersBaseObject* foo = mystack->get(1);
		if(foo->isaNum == COBJ_IMAGE){
			((Image*)foo)->writeBMP(c);
		} else if(foo->isaNum == COBJ_RGBIMAGE){
			((RGBImage*)foo)->writeBMP(c);
		}
		mystack->drop();
	};
};

//-----------------------------

class aLoadcatalog: public Action {
public:
	aLoadcatalog() : Action(){
		description="Load a <catalog file> of subregions for an <Image>";
		addname("loadcat");
		addname("lcat");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_IMAGE;
	};
	void DoIt() {
		if(!mystack->checkReadable()) return;
		string c = mystack->getstring(0);
		((Image*)(mystack->get()))->loadcatalog(c);
		mystack->drop();
	}
};

//----------------------------

class aDumpcatalog: public Action {
public:
	aDumpcatalog() : Action(){
		description="Save each region in catalog to a series of <file>s (filename should have %i for sprintf) from an <Image>";
		addname("dumpcatalog");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_IMAGE;
	};
	void DoIt() {
		if(!mystack->checkWritable()) return;
		string c = mystack->getstring(0);
		((Image*)(mystack->get()))->dumpcatalog(c);
		mystack->drop();
	}
};

//----------------------------

class aLoadClassification: public Action {
public:
	aLoadClassification() : Action(){
		description="Load an <ASCII data file> as a ClassifyImage";
		addname("loadc");
		addname("lc");
		ninputs = 1;
		inputtypes[0] = COF_CRATERSTRING;
	};
	void DoIt() {
		if(!mystack->checkReadable()) return;
		string c = mystack->getstring(0);
		ClassifyImage* I = new ClassifyImage(0,0);
		I->loadarcgis(c);
		I->renumerate();
		mystack->drop();
		mystack->push(I);
	}
};

