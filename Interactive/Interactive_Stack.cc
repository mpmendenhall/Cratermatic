#include "Interactive.hh"
#include "Utils.hh"
#include <cassert>

bool Stack::checkReadable() {
    int foo = COF_CRATERSTRING;
    if(!validateinput(&foo,1)) return false;
    string c = ((CraterString*)get())->val;
    FILE* f = fopen(c.c_str(),"r");
    if(f==NULL) {
        push(new CError("File '"+c+"' is unreadable",5));
        return false;
    }
    fclose(f);
    return true;
}

bool Stack::checkWritable() {
    int foo = COF_CRATERSTRING;
    if(!validateinput(&foo,1)) return false;
    string c = ((CraterString*)get())->val;
    FILE* f = fopen(c.c_str(), "a");
    if(f==NULL) {
        push(new CError("File '"+c+"' is unwriteable",6));
        return false;
    }
    fclose(f);
    return true;
}

bool Stack::checkFolder()
{
    int foo = COF_CRATERSTRING;
    if(!validateinput(&foo,1)) return false;
    string c = ((CraterString*)get())->val;
    //sprintf(tempchar,"ls %s/",c);
    //if(!system(tempchar)) return true; //folder exists already
    assert(!system(("mkdir -p "+c).c_str()));
    return true;
    //if(!system(tempchar)) return true; //folder can be created
    //sprintf(tempchar,"Folder '%s' is unwritable",c.c_str());
    //push(new CError(tempchar,6));
    //return false;
}

bool Stack::validateinput(int* t, unsigned int n) { //check type number flags t for n objects
    if(n>items.size()) {
        push(new CError("Too few items in stack for requested operation",0));
        return false;
    }
    for(unsigned int i=0; i<n; i++) {
        if(!(t[i] & (1 << get(i)->isaNum))) {
            push(new CError("Stack item #"+to_str(i+1)+" is of the wrong type for this operation",1));
            return false;
        }
    }
    return true;
}

void Stack::disp() {
    printf("+---------------+\n");
    for(size_t i=0; i<items.size(); i++) {
        CratersBaseObject* foo = get(items.size()-1-i);
        printf("|%zu (%i): %s ",items.size()-i,entrynum[i],foo->isaName.c_str());
        printf("'%s' ",foo->name.c_str()); //item name
        printf("\n");
    }
    if(items.size()==0) printf("| <stack empty> |\n");
    printf("+---------------+\n");
}

void Stack::push(CratersBaseObject* ptr) {
    if(items.size() > STACK_MAX_ITEMS - 5) {
        //properly dispose of extra item
        entrynum.push_back(++ntotal);
        items.push_back(ptr);
        drop();
        
        if(items.size() < STACK_MAX_ITEMS - 2) {
            entrynum.push_back(++ntotal);
            items.push_back(new CError("Too many stack items!",7));
        }
        return;
    }
    entrynum.push_back(++ntotal);
    items.push_back(ptr);
}

CratersBaseObject* Stack::pop() {
    if(!items.size()) {
        push(new CError("Too few items in stack for 'pop'",0));
        return NULL;
    }
    CratersBaseObject* o = items.back();
    items.pop_back();
    return o;
}

CratersBaseObject* Stack::get() { //get without removing from stack
    if(!items.size()) {
        push(new CError("Too few items in stack for'get'",0));
        return NULL;
    }
    return items.back();
}

CratersBaseObject* Stack::get(unsigned int n) { //get nth item without removing from stack
    if(items.size() < n) {
        fprintf(stderr,"\n** Stack error: out of items! **\n\n");
        return NULL;
    }
    return items[items.size()-n-1];
}

float Stack::getfloat(unsigned int n) { //get nth item as a float
    if(items.size() < n) return 0;
    if(get(n)->isaNum == COBJ_CFLOAT) return (float)*(CFloat*)get(n);
    push(new CError("Requested number from non-numeric stack item",1));
    return 0;
}

int Stack::getint(unsigned int n) { //get nth item as an int
    if(items.size() < n) return 0;
    if(get(n)->isaNum == COBJ_CFLOAT)
    {
        float z = (float)*(CFloat*)get(n);
        if(z != (int)z) printf("Warning: truncating Float %g to Int %i\n",z,(int)z);
        return (int)z;
    }
    push(new CError("Requested number from non-numeric stack item",1));
    return 0;
}

string Stack::getstring(unsigned int n) { //get nth item as an int
    if(items.size() < n) return "";
    if(get(n)->isaNum == COBJ_CRATERSTRING) return ((CraterString*)get(n))->val;
    return "";
}

void Stack::rot(int n) { //swap 2 items on the stack
    if(n<2) {
        fprintf(stderr,"\n** Stack error: you don't rotate <2 items, silly! **\n\n");
        return;
    }
    
    if((int)items.size()<n) {
        push(new CError("Too few items in stack for requested operation",0));
        return;
    }
    
    CratersBaseObject* tempptr = items[items.size()-n];
    int tempentry = entrynum[items.size()-n];
    
    for(int i=n; i>1; i--) {
        items[items.size()-i] = items[items.size()-i+1];
        entrynum[items.size()-i] = entrynum[items.size()-i+1];
    }
    items.back() = tempptr;
    entrynum.back() = tempentry;
}

void Stack::swap() { //swap 2 items on the stack
    if(items.size()<2) {
        push(new CError("Too few items in stack for requested operation",0));
        return;
    }
    rot(2);
}

void Stack::drop()
{
    if(items.size() == 0) {
        push(new CError("No item in stack to drop",0));
        return;
    }
    CratersBaseObject* foo = pop();
    if(foo->isaNum == COBJ_IMAGE) delete((Image*)foo);
    else if(foo->isaNum == COBJ_RGBIMAGE) delete((RGBImage*)foo);
    else if(foo->isaNum == COBJ_COMPLEXIMAGE) delete((ComplexImage*)foo);
    else if(foo->isaNum == COBJ_CLASSIFYIMAGE) delete((ClassifyImage*)foo);
    else if(foo->isaNum == COBJ_RECTREGION) delete((RectRegion*)foo);
    else if(foo->isaNum == COBJ_CRATERSBASEOBJECT) delete((CratersBaseObject*)foo);
    else if(foo->isaNum == COBJ_CFLOAT) delete((CFloat*)foo);
    else if(foo->isaNum == COBJ_CRATERSTRING) delete((CraterString*)foo);
    else if(foo->isaNum == COBJ_CMACRO) delete((CMacro*)foo);
    else if(foo->isaNum == COBJ_CERROR) delete((CError*)foo);
    else printf("*** Warning: dropped unknown object type (memory not freed) ***\n");
}

bool Stack::istype(int t) {
    if(!items.size()) return false;
    if(get()->isaNum != t) return false;
    return true;
}

bool Stack::istypef(int typeflag) {
    if(!items.size()) return false;
    if((1 << get()->isaNum) & typeflag) return true;
    return false;
}
