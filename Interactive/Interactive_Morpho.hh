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

class aUnspike: public Action {
public:
    aUnspike() : Action(){
        description="Remove spikes in <Image> data";
        addname("unspike");
        ninputs = 1;
        inputtypes[0] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(0))->removespikes();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aAnndil: public Action {
public:
    aAnndil() : Action(){
        description="Apply an annular dilation with specified <outer>, <inner> radii to an <Image> (slow!)";
        addname("anndil");
        ninputs = 3;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_CFLOAT;
        inputtypes[2] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(2))->annulusdilation(mystack->getint(0),mystack->getint(1));
        mystack->drop();
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aSqdil: public Action {
public:
    aSqdil() : Action(){
        description="Apply a square dilation of the specified <size> to an <Image>";
        addname("sqdil");
        ninputs = 2;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(1))->squaredilation(mystack->getint(0));
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aCdil: public Action {
public:
    aCdil() : Action(){
        description="Apply a circular dilation of the specified <size> to an <Image> (slow!)";
        addname("cdil");
        ninputs = 2;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(1))->circledilation(mystack->getint(0));
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aCero: public Action {
public:
    aCero() : Action(){
        description="Apply a circular erosion of the specified <size> to an <Image> (slow!)";
        addname("cero");
        ninputs = 2;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(1))->circleerosion(mystack->getint(0));
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aCopen: public Action {
public:
    aCopen() : Action(){
        description="Apply a circular opening of the specified <size> to an <Image>";
        addname("copen");
        ninputs = 2;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(1))->circleopening(mystack->getint(0));
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aCclose: public Action {
public:
    aCclose() : Action(){
        description="Apply a circular closing of the specified <size> to an <Image>";
        addname("cclose");
        ninputs = 2;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(1))->circleclosing(mystack->getint(0));
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};

//-----------------------------

class aEdgefind: public Action {
public:
    aEdgefind() : Action(){
        description="Apply gaussian blur of specified <radius> to copy of <Image>, subtract from original image, and return absolute value of result";
        addname("edgefind");
        ninputs = 2;
        inputtypes[0] = COF_CFLOAT;
        inputtypes[1] = COF_IMAGE;
    };
    
    void DoIt() {
        Image* J = ((Image*)mystack->get(1))->edgefinder(mystack->getfloat(0));
        mystack->drop();
        mystack->drop();
        mystack->push(J);
    }
};
