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

class aDup: public Action {
public:
	aDup() : Action(){
		description="Duplicate the <object> on the stack";
		addname("dup");
		ninputs = 1;
		inputtypes[0] = COF_IMAGE | COF_CLASSIFYIMAGE
			| COF_CFLOAT | COF_CMACRO | COF_CRATERSTRING;
	};
	
	void DoIt() {
		CratersBaseObject* foo = mystack->get();
		if(foo->isaNum == COBJ_IMAGE) mystack->push(((Image*)foo)->copy());
		else if(foo->isaNum == COBJ_CLASSIFYIMAGE) mystack->push(((ClassifyImage*)foo)->copy());
		else if(foo->isaNum == COBJ_CFLOAT) mystack->push(((CFloat*)foo)->copy());
		else if(foo->isaNum == COBJ_CMACRO) mystack->push(((CMacro*)foo)->copy());
		else if(foo->isaNum == COBJ_CRATERSTRING) mystack->push(((CraterString*)foo)->copy());
	}
};

//-----------------------------

class aSwap: public Action {
public:
	aSwap() : Action(){
		description="Swap <two> <items> on the stack";
		addname("swap");
		ninputs = 2;
		inputtypes[0] = COF_ANYTYPE;
		inputtypes[1] = COF_ANYTYPE;
	};
	
	void DoIt() {
		mystack->swap();
	}
};

//-----------------------------

class aDrop: public Action {
public:
	aDrop() : Action(){
		description="delete an <item> from the stack";
		addname("drop");
		ninputs = 1;
		inputtypes[0] = COF_ANYTYPE;
	};
	
	void DoIt() {
		mystack->drop();
	}
};

//-----------------------------

class aRot: public Action {
public:
	aRot() : Action(){
		description="rotate the order of the specified <number> of <<stack items>>";
		addname("rot");
		ninputs = 1;
		inputtypes[0] = COF_CFLOAT;
	};
	
	bool validateinput() {
		if(!Action::validateinput()) return false;
		ninputs = mystack->getint(0)+1;
		if(ninputs > 100) {ninputs = 1; return false;}
		for(int i=1; i<ninputs+1; i++) inputtypes[i] = COF_ANYTYPE;
		bool isok = Action::validateinput();
		ninputs = 1;
		return isok;
	}
	
	void DoIt() {
		ninputs = mystack->getint(0);
		mystack->drop();
		mystack->rot(ninputs);
		ninputs = 1;
	}
};

//-----------------------------

class aDropn: public Action {
public:
	aDropn() : Action(){
		description="Drop <n> <<stack items>>";
		addname("dropn");
		ninputs = 1;
		inputtypes[0] = COF_CFLOAT;
	};
	
	bool validateinput() {
		if(!Action::validateinput()) return false;
		ninputs = mystack->getint(0)+1;
		if(ninputs > STACK_MAX_ITEMS) {ninputs = 1; mystack->push(new CError("Oversize argument value for 'dropn'",8)); return false;}
		for(int i=1; i<ninputs+1; i++) inputtypes[i] = COF_ANYTYPE;
		bool isok = Action::validateinput();
		ninputs = 1;
		return isok;
	}
	
	void DoIt() {
		ninputs = mystack->getint(0);
		mystack->drop();
		for(int i=0; i<ninputs; i++) mystack->drop();
		ninputs = 1;
	}
};

//-----------------------------

class aClear: public Action {
public:
	aClear() : Action(){
		description="Drop ALL stack items";
		addname("clear");
		ninputs = 0;
	};
	
	void DoIt() {
		while(mystack->nitems > 0) mystack->drop();
	}
};

//-----------------------------