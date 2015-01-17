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

class aMerge: public Action {
public:
	aMerge() : Action(){
		description="Merge similar (to specified <threshold>; try 1e-3 to start) regions, based on the specified <number> of underlying images, in a <ClassifyImage>";
		addname("merge");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_CLASSIFYIMAGE;
	};
	
	bool validateinput() {
		if(!Action::validateinput()) return false;
		if(mystack->getint(1) > 12) {
			printf("That's a crazy big number of original images...\n\tI'm not even going to try processing that.\n");
			return false;
		}
		ninputs = mystack->getint(1)+3;
		for(int i=3; i<ninputs; i++) inputtypes[i] = COF_IMAGE;
		bool isok = Action::validateinput();
		ninputs = 3;
		return isok;
	}
	
	void DoIt() {
		Image** c = (Image**)malloc((mystack->getint(1))*sizeof(Image*));
		for(int i=0; i<mystack->getint(1); i++) c[i]=(Image*)mystack->get(i+3);
		Merger *M = new Merger((ClassifyImage*)mystack->get(2),c,mystack->getint(1));
		M->domerge(mystack->getfloat(0));
		delete(M);
		free(c);
		mystack->drop();
		mystack->drop();
	}
};

//-----------------------------

class aSetmovie: public Action {
public:
	aSetmovie() : Action(){
		description=(char*)malloc(256*sizeof(char));
		if(domovie) sprintf(description,"Toggle whether to save .bmp images of intermediate merge steps (default enabled)");
		else sprintf(description,"Toggle whether to save .bmp images of intermediate merge steps (default disabled)");
		addname("setmovie");
	};
	
	void DoIt() {
		domovie = !domovie;
		if(domovie) printf("Merge movie saving is now enabled.\n");
		else printf("Merge movie saving is now disabled.\n");
	}
};

//-----------------------------

class aSetmovieframerate: public Action {
public:
	aSetmovieframerate() : Action(){
		description=(char*)malloc(256*sizeof(char));
		sprintf(description,"Set <interval> between intermediate merge stage saves (default %i)",movieframeadvance);
		addname("setmovieframerate");
		ninputs = 1;
		inputtypes[0] = COF_CFLOAT;
	};
	
	void DoIt() {
		movieframeadvance = mystack->getint(0);
	}
};

//-----------------------------

class aSetmoviebase: public Action {
public:
	aSetmoviebase() : Action(){
		description=(char*)malloc(256*sizeof(char));
		sprintf(description,"Set <filename> for saving merge movies (default '%s')",moviebase);
		addname("setmoviebase");
		ninputs = 1;
		inputtypes[0] = COF_CRATERSTRING;
	};
	
	void DoIt() {
		char* c = mystack->getstring(0);
		if(moviebase) free(moviebase);
		moviebase = (char*)malloc(512*sizeof(char));
		sprintf(moviebase,"%s",c);
		mystack->drop();
	}
};

//-----------------------------

class aExtractchunk: public Action {
public:
	aExtractchunk() : Action(){
		description="Extract the <specified> <ClassifyImage> region from an <Image>";
		addname("extractChunk");
		addname("exch");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CLASSIFYIMAGE;
		inputtypes[2] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* a = ((ClassifyImage*)(mystack->get(1))) -> extractChunkMask(mystack->getint(0));
		Image* b = ((ClassifyImage*)(mystack->get(1))) -> extractChunk(mystack->getint(0),(Image*)(mystack->get(2)));
		mystack->drop();
		mystack->push(a);
		mystack->push(b);
	}
};

//-----------------------------

class aWSavg: public Action {
public:
	aWSavg() : Action(){
		description="Set <ClassifyImage> 'temp' stat to basin average of underlying <Image>";
		addname("wsavg");
		ninputs = 2;
		inputtypes[0] = COF_CLASSIFYIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((ClassifyImage*)(mystack->get(0)))->underlyingavg((Image*)mystack->get(1));
	}
};

//-----------------------------

class aWSmin: public Action {
public:
	aWSmin() : Action(){
		description="Set <ClassifyImage> 'temp' stat to basin minimum of underlying <Image>";
		addname("wsmin");
		ninputs = 2;
		inputtypes[0] = COF_CLASSIFYIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((ClassifyImage*)(mystack->get(0)))->underlyingmin((Image*)mystack->get(1));
	}
};

//-----------------------------

class aWSmax: public Action {
public:
	aWSmax() : Action(){
		description="Set <ClassifyImage> 'temp' stat to basin maximum of underlying <Image>";
		addname("wsmax");
		ninputs = 2;
		inputtypes[0] = COF_CLASSIFYIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((ClassifyImage*)(mystack->get(0)))->underlyingmax((Image*)mystack->get(1));
	}
};

//-----------------------------

class aWSmed: public Action {
public:
	aWSmed() : Action(){
		description="Set <ClassifyImage> 'temp' stat to basin median of underlying <Image>";
		addname("wsmed");
		ninputs = 2;
		inputtypes[0] = COF_CLASSIFYIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((ClassifyImage*)(mystack->get(0)))->underlyingmedian((Image*)mystack->get(1));
	}
};

//-----------------------------

class aWScirc: public Action {
public:
	aWScirc() : Action(){
		description="Set <ClassifyImage> 'temp' stat to basin circularity";
		addname("wscirc");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		((ClassifyImage*)(mystack->get()))->circularity();
	}
};

//-----------------------------

class aWSangl: public Action {
public:
	aWSangl() : Action(){
		description="Set <ClassifyImage> 'temp' stat to basin angularity";
		addname("wsangl");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		//((ClassifyImage*)(mystack->get()))->angularity();
	}
};

//-----------------------------

class aWSrand: public Action {
public:
	aWSrand() : Action(){
		description="Set <ClassifyImage> 'temp' stat to a random number in [0,1]";
		addname("wsrand");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		((ClassifyImage*)(mystack->get()))->random();
	}
};

//----------------------------

class aWatershed: public Action {
public:
	aWatershed() : Action(){
		description="Calculate the watershed transform of an <Image>";
		addname("watershed");
		addname("ws");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		ClassifyImage* w = ClassifyImage::watershed((Image*)mystack->get());
		mystack->drop();
		mystack->push(w);
	}
};

//----------------------------

class aGradseg: public Action {
public:
	aGradseg() : Action(){
		description="Segment an <Image> based on its gradient";
		addname("gradseg");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		mystack->push(ClassifyImage::gradSeg((Image*)mystack->get(1),mystack->getfloat(0)));
		mystack->swap(); mystack->drop();
	}
};

//-----------------------------

class aToImage: public Action {
public:
	aToImage() : Action(){
		description="Turn a <ClassifyImage> into an Image by classification number";
		addname("toimage");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		mystack->push(((ClassifyImage*)(mystack->get()))->dataToImage());
	}
};

//-----------------------------
class aBoundaryimage: public Action {
public:
	aBoundaryimage() : Action(){
		description="Create an Image of the boundaries of a <ClassifyImage>";
		addname("boundaryimage");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		mystack->push(((ClassifyImage*)(mystack->get()))->boundaryimage());
	}
};

//-----------------------------

class aPrettyimage: public Action {
public:
	aPrettyimage() : Action(){
		description="Create a nicely colored RGBImage of the regions and boundaries of a <ClassifyImage>";
		addname("prettyimage");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		mystack->push(((ClassifyImage*)(mystack->get()))->prettyImage());
	}
};

//-----------------------------

class aPrettyoverlay: public Action {
public:
	aPrettyoverlay() : Action(){
		description="Create a nicely colored RGBImage of the regions and boundaries of a <ClassifyImage> overlayed on an <Image>";
		addname("prettyoverlay");
		ninputs = 2;
		inputtypes[0] = COF_CLASSIFYIMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		mystack->push(((ClassifyImage*)(mystack->get(0)))->prettyoverlayimage((Image*)mystack->get(1)));
	}
};

//-----------------------------

class aStatimg: public Action {
public:
	aStatimg() : Action(){
		description="Extract an Image from a <ClassifyImage> colored by the 'temp' stat";
		addname("statimg");
		ninputs = 1;
		inputtypes[0] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		mystack->push(((ClassifyImage*)(mystack->get(0)))->tempstatimg());
	}
};

//----------------------------

class aGetsubregion: public Action {
public:
	aGetsubregion() : Action(){
		description="Split out subregion <number n> from an <Image> by catalog entry";
		addname("getsub");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	void DoIt() {
		Image* foo = ((Image*)(mystack->get(1)))->getsubregion(mystack->getint(0),2.0);
		if(foo) mystack->push(foo);
		else printf("\n** Error loading subregion **\n\n");
	}
};

//----------------------------

class aFourierShape: public Action {
public:
	aFourierShape() : Action(){
		description="Calculate and mark the <n> term series for the shape of region <r> of a <ClassifyImage>";
		addname("fouriershape");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_CLASSIFYIMAGE;
	};
	void DoIt() {
		int nterms = mystack->getint(0);
		int rgn = mystack->getint(1);
		mystack->drop();
		mystack->drop();
		ClassifyImage* C = (ClassifyImage*)mystack->get();
		if(nterms < 0 || nterms > 100 || rgn < 0 || rgn >= C->nbasins) return;
		float* xsft;
		float* ysft;
		float xc = C->xcenter(C->pic[rgn], C->npic[rgn], NULL);
		float yc = C->ycenter(C->pic[rgn], C->npic[rgn], NULL);
		C->radialFourier(xc,yc,C->pic[rgn], C->npic[rgn], (float*)NULL, &xsft, &ysft, nterms);
		C->fouriermark(xc,yc,xsft,ysft,nterms,50/min(nterms,50));
		xsft[0]*=0.95;
		C->fouriermark(xc,yc,xsft,ysft,nterms,50/min(nterms,50));
		xsft[0]*=1.11;
		C->fouriermark(xc,yc,xsft,ysft,nterms,50/min(nterms,50));
		for(int i=0; i<nterms; i++) printf("%+.3e\t%+.3e",xsft[i],ysft[i]);
		printf("\n");
		free(xsft);
		free(ysft);
	}
};

//-----------------------------

class aZeros: public Action {
public:
	aZeros() : Action(){
		description="Return a ClassifyImage with regions corresponding to the zeros of an <Image>";
		addname("zeros");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		mystack->push(ClassifyImage::fromzeros((Image*)mystack->get()));
	}
};

//-----------------------------

class aUpcurve: public Action {
public:
	aUpcurve() : Action(){
		description="Return a ClassifyImage with regions corresponding to the positive curvature areas of <Image>";
		addname("upcurve");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		mystack->push(ClassifyImage::fromCurvature((Image*)mystack->get()));
	}
};

//-----------------------------

class aSizeconstraint: public Action {
public:
	aSizeconstraint() : Action(){
		description="Constrain sizes to between <high> and <low> number of pixels in binary <ClassifyImage>";
		addname("sizecon");
		ninputs = 3;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
		inputtypes[2] = COF_CLASSIFYIMAGE;
	};
	
	void DoIt() {
		int a =mystack->getint(0); mystack->drop();
		int b =mystack->getint(0); mystack->drop();
		((ClassifyImage*)mystack->get(0))->constrainSize(b,a);
	}
};
