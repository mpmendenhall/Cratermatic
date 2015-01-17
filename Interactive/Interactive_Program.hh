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

class aExec: public Action {
public:
	aExec() : Action(){
		description="Execute a <Macro>";
		addname("exec");
		addname("x");
		ninputs = 1;
		inputtypes[0] = COF_CMACRO;
	};
	
	void DoIt() {
		mystack->controller->prependCommand(((CMacro*)mystack->get())->stringval);
		mystack->drop();
	}
};

//-----------------------------

class aRem: public Action {
public:
	aRem() : Action(){
		description="Print <object> to stdout";
		addname("rem");
		ninputs = 1;
		inputtypes[0] = COF_ANYTYPE;
	};
	
	void DoIt() {
		CratersBaseObject* foo = mystack->get(0);
		if(foo->isaNum == COBJ_CRATERSTRING) printf("%s\n",((CraterString*)mystack->get())->val);
		else if(foo->isaNum == COBJ_CFLOAT) printf("%g\n",mystack->getfloat(0));
		else printf("%s\n",((CratersBaseObject*)mystack->get())->name);
		
		 if(foo->isaNum == COBJ_CRATERSTRING || foo->isaNum == COBJ_CFLOAT) mystack->drop();
	}
};

//-----------------------------

class aGreaterthan: public Action {
public:
	aGreaterthan() : Action(){
		description="Return 1 if <number2> > <number1>, 0 otherwise";
		addname("gt");
		addname(">");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		float y = mystack->getfloat(0);
		float z = mystack->getfloat(1);
		mystack->drop();
		mystack->drop();
		
		if(z > y) mystack->push((CratersBaseObject*)(new CFloat(1)));
		else mystack->push((CratersBaseObject*)(new CFloat(0)));
	}
};

//-----------------------------

class aLessthan: public Action {
public:
	aLessthan() : Action(){
		description="Return 1 if <number2> < <number1>, 0 otherwise";
		addname("lt");
		addname("<");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		float y = mystack->getfloat(0);
		float z = mystack->getfloat(1);
		mystack->drop();
		mystack->drop();
		
		if(z < y) mystack->push((CratersBaseObject*)(new CFloat(1)));
		else mystack->push((CratersBaseObject*)(new CFloat(0)));
	}
};

//-----------------------------

class aGreaterequalthan: public Action {
public:
	aGreaterequalthan() : Action(){
		description="Return 1 if <number2> >= <number1>, 0 otherwise";
		addname("get");
		addname(">=");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		float y = mystack->getfloat(0);
		float z = mystack->getfloat(1);
		mystack->drop();
		mystack->drop();
		
		if(z >= y) mystack->push((CratersBaseObject*)(new CFloat(1)));
		else mystack->push((CratersBaseObject*)(new CFloat(0)));
	}
};

//-----------------------------

class aLessequalthan: public Action {
public:
	aLessequalthan() : Action(){
		description="Return 1 if <number2> <= <number1>, 0 otherwise";
		addname("let");
		addname("<=");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		float y = mystack->getfloat(0);
		float z = mystack->getfloat(1);
		mystack->drop();
		mystack->drop();
		
		if(z <= y) mystack->push((CratersBaseObject*)(new CFloat(1)));
		else mystack->push((CratersBaseObject*)(new CFloat(0)));
	}
};

//-----------------------------

class aEqualto: public Action {
public:
	aEqualto() : Action(){
		description="Return 1 if <number2> == <number1>, 0 otherwise";
		addname("et");
		addname("==");
		ninputs = 2;
		inputtypes[0] = COF_CFLOAT;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		float y = mystack->getfloat(0);
		float z = mystack->getfloat(1);
		mystack->drop();
		mystack->drop();
		
		if(z == y) mystack->push((CratersBaseObject*)(new CFloat(1)));
		else mystack->push((CratersBaseObject*)(new CFloat(0)));
	}
};

//-----------------------------

class aIf: public Action {
public:
	aIf() : Action(){
		description="If execute <Macro> if <number> nozero";
		addname("if");
		ninputs = 2;
		inputtypes[0] = COF_CMACRO;
		inputtypes[1] = COF_CFLOAT;
	};
	
	void DoIt() {
		mystack->swap();
		float z = mystack->getfloat(0);
		mystack->drop();
		if(z != 0)
		{
			mystack->controller->prependCommand("exec");
		} else mystack->drop();
	}
};

//-----------------------------

class aIfelse: public Action {
public:
	aIfelse() : Action(){
		description="Execute <Macro 2>, else execute <Macro 1>, depending on if <number> is nonzero";
		addname("ifelse");
		ninputs = 3;
		inputtypes[0] = COF_CMACRO;
		inputtypes[1] = COF_CMACRO;
		inputtypes[2] = COF_CFLOAT;
	};
	
	void DoIt() {
		mystack->rot(3);
		float z = mystack->getfloat(0);
		mystack->drop();
		if(z == 0)
		{
			mystack->swap();
			mystack->drop();
			mystack->controller->prependCommand("exec");
		} else {
			mystack->drop();
			mystack->controller->prependCommand("exec");
		}
	}
};

//-----------------------------

class aUserDefined: public Action {
public:
	CMacro* mymacro;
	
	aUserDefined(char* name, CMacro* m) : Action(){
		description="User defined macro";
		addname(name);
		ninputs = 0;
		mymacro = m->copy();
	};
	
	~aUserDefined()
	{
		delete(mymacro);
	}
	
	void DoIt() {
		mystack->push(mymacro->copy());
		mystack->controller->prependCommand("exec");
	}
};

//-----------------------------

class aName: public Action {
public:
	aName() : Action(){
		description="Give a calling <name> to a <Macro> and add to user-defined macro list";
		addname("name");
		ninputs = 2;
		inputtypes[0] = COF_CRATERSTRING;
		inputtypes[1] = COF_CMACRO;
	};
	
	void DoIt() {
		aUserDefined* a = new aUserDefined(((CraterString*)mystack->get(0))->val,(CMacro*)mystack->get(1));
		myinteractor->registerAction(a);
		mystack->drop();
		mystack->drop();
	}
};

//-----------------------------
