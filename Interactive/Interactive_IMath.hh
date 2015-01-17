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

class aAdd: public Action {
public:
	aAdd() : Action(){
		description="Add <Image1> to <Image2>";
		addname("add");
		addname("+");
		ninputs = 2;
		inputtypes[0] = COF_IMAGE | COF_CFLOAT;
		inputtypes[1] = COF_IMAGE | COF_CFLOAT;
	};
	
	void DoIt() {
		if(mystack->istypef(COF_CFLOAT))
		{
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0)+mystack->getfloat(1);
				mystack->drop(); mystack->drop();
				mystack->push(new CFloat(z));
				return;
			} else {
				float z = mystack->getfloat(1);
				mystack->swap(); mystack->drop();
				((Image*)mystack->get(0))->add(z);
				return;
			}
		} else {
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0);
				mystack->drop();
				((Image*)mystack->get(0))->add(z);
				return;
			} else {
				((Image*)mystack->get(1))->add((Image*)mystack->get(0));
				mystack->drop();
				return;
			}
		}
	}
};

//-----------------------------

class aQadd: public Action {
public:
	aQadd() : Action(){
		description="Add <Image1> and <Image2> in quadrature (i.e. sqrt(I1^2+I2^2) )";
		addname("qadd");
		ninputs = 2;
		inputtypes[0] = COF_IMAGE;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((Image*)mystack->get(1))->quadratureadd((Image*)mystack->get(0));
		mystack->drop();
	}
};

//-----------------------------

class aSubtr: public Action {
public:
	aSubtr() : Action(){
		description="Subtract <Image1> from <Image2>";
		addname("subtr");
		addname("-");
		ninputs = 2;
		inputtypes[0] = COF_IMAGE | COF_CFLOAT;
		inputtypes[1] = COF_IMAGE | COF_CFLOAT;
	};
	
	void DoIt() {
		if(mystack->istypef(COF_CFLOAT))
		{
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0)-mystack->getfloat(1);
				mystack->drop(); mystack->drop();
				mystack->push(new CFloat(z));
				return;
			} else {
				float z = mystack->getfloat(1);
				mystack->swap(); mystack->drop();
				((Image*)mystack->get(0))->add(-z);
				return;
			}
		} else {
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0);
				mystack->drop();
				((Image*)mystack->get(0))->mult(-1)->add(z);
				return;
			} else {
				((Image*)mystack->get(0))->add(((Image*)mystack->get(1))->mult(-1.0));
				mystack->swap(); mystack->drop();
				return;
			}
		}
	}
};

//-----------------------------

class aMult: public Action {
public:
	aMult() : Action(){
		description="Multiply <Image1> by <Image2>";
		addname("mult");
		addname("*");
		ninputs=2;
		inputtypes[0] = COF_IMAGE | COF_CFLOAT;
		inputtypes[1] = COF_IMAGE | COF_CFLOAT;
	};
	
	void DoIt() {
		if(mystack->istypef(COF_CFLOAT))
		{
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0)*mystack->getfloat(1);
				mystack->drop(); mystack->drop();
				mystack->push(new CFloat(z));
				return;
			} else {
				float z = mystack->getfloat(1);
				mystack->swap(); mystack->drop();
				((Image*)mystack->get(0))->mult(z);
				return;
			}
		} else {
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0);
				mystack->drop();
				((Image*)mystack->get(0))->mult(z);
				return;
			} else {
				((Image*)mystack->get(1))->mult((Image*)mystack->get(0));
				mystack->drop();
				return;
			}
		}
	}
};

//-----------------------------

class aDiv: public Action {
public:
	aDiv() : Action(){
		description="Produce quotient by <Image1> of <Image2>";
		addname("divide");
		addname("/");
		ninputs = 2;
		inputtypes[0] = COF_IMAGE | COF_CFLOAT;
		inputtypes[1] = COF_IMAGE | COF_CFLOAT;
	};
	
	void DoIt() {
		if(mystack->istypef(COF_CFLOAT))
		{
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0)/mystack->getfloat(1);
				mystack->drop(); mystack->drop();
				mystack->push(new CFloat(z));
				return;
			} else {
				float z = mystack->getfloat(1);
				mystack->swap(); mystack->drop();
				((Image*)mystack->get(0))->mult(1.0/z);
				return;
			}
		} else {
			mystack->swap();
			if(mystack->istypef(COF_CFLOAT))
			{
				float z = mystack->getfloat(0);
				mystack->drop();
				((Image*)mystack->get(0))->reciprocal()->mult(z);
				return;
			} else {
				((Image*)mystack->get(0))->divide(((Image*)mystack->get(1)));
				mystack->swap(); mystack->drop();
				return;
			}
		}
	}
};

//-----------------------------

class aNormalize: public Action {
public:
	aNormalize() : Action(){
		description="Normalize <Image> data to [0,1]";
		addname("normalize");
		addname("n");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		Image* I =(Image*)mystack->pop();
		Image* J =I->normalized(0,1);
		delete(I);
		mystack->push(J);
	}
};

//-----------------------------

class aComplement: public Action {
public:
	aComplement() : Action(){
		description="<Image> -> 1-Image";
		addname("complement");
		addname("c");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		((Image*)mystack->get())->icomplement();
	}
};

//-----------------------------

class aNegate: public Action {
public:
	aNegate() : Action(){
		description="<Image> -> -Image";
		addname("negate");
		addname("neg");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE | COF_CFLOAT;
	};
	
	void DoIt() {
		if(mystack->istypef(COF_IMAGE)) ((Image*)mystack->get())->mult(-1.0);
		else {
			float z = mystack->getfloat(0);
			mystack->drop();
			mystack->push(new CFloat(-z));
		}
	}
};

//-----------------------------

class aThreshold: public Action {
public:
	aThreshold() : Action(){
		description="Take the maximum of a specified <threshold> and the <Image> data";
		addname("threshold");
		addname("th");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	
	void DoIt() {
		((Image*)mystack->get(1))->threshold(mystack->getfloat(0));
		mystack->drop();
	}
};

//-----------------------------

class aRange: public Action {
public:
	aRange() : Action(){
		description="Return the min and max data values of <Image> data";
		addname("range");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		float* mnmx = ((Image*)mystack->get(0))->minmax();
		mystack->push(new CFloat(mnmx[0]));
		mystack->push(new CFloat(mnmx[1]));
		free(mnmx);
	}
};

//-----------------------------

class aGamma: public Action {
public:
	aGamma() : Action(){
		description="apply rec709 gamma curve to <Image>";
		addname("gamma");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	void DoIt() {
		mystack->push(((Image*)mystack->get())->rec709gamma());
	}
};

//----------------------------

class aSGamma: public Action {
public:
	aSGamma() : Action(){
		description="Apply signed <gamma> to <Image>";
		addname("sgamma");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_IMAGE;
	};
	void DoIt() {
		float c = mystack->getfloat(0);
		mystack->drop();
		((Image*)mystack->get(0))->signedgamma(c);
	}
};

//----------------------------

class aAbs: public Action {
public:
	aAbs() : Action(){
		description="Take absolute value of <Image> data";
		addname("abs");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		((Image*)mystack->get())->absval();
	}
};

//----------------------------

class aFlatHisto: public Action {
public:
	aFlatHisto() : Action(){
		description="Flatten <image> value histogram";
		addname("flathisto");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE;
	};
	
	void DoIt() {
		((Image*)mystack->get())->flatHisto();
	}
};