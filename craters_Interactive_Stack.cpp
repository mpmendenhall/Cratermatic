#include "craters_Interactive.h"

bool Stack::checkReadable()
{
	int foo = COF_CRATERSTRING;
	if(!validateinput(&foo,1)) return false;
	char* c = ((CraterString*)get())->val;
	FILE* f = fopen(c,"r");
	if(f==NULL) {
		sprintf(tempchar,"File '%s' is unreadable",c);
		push(new CError(tempchar,5));
		return false;
	}
	fclose(f);
	return true;
}

bool Stack::checkWritable()
{
	int foo = COF_CRATERSTRING;
	if(!validateinput(&foo,1)) return false;
	char* c = ((CraterString*)get())->val;
	FILE* f = fopen(c,"a");
	if(f==NULL) {
		sprintf(tempchar,"File '%s' is unwritable",c);
		push(new CError(tempchar,6));
		return false;
	}
	fclose(f);
	return true;
}

bool Stack::checkFolder()
{
	int foo = COF_CRATERSTRING;
	if(!validateinput(&foo,1)) return false;
	char* c = ((CraterString*)get())->val;
	//sprintf(tempchar,"ls %s/",c);
	//if(!system(tempchar)) return true; //folder exists already
	sprintf(tempchar,"mkdir %s",c);
	system(tempchar);
	return true;
	if(!system(tempchar)) return true; //folder can be created
	sprintf(tempchar,"Folder '%s' is unwritable",c);
	push(new CError(tempchar,6));
	return false;
}

Stack::Stack() {
	nitems = 0;
	ntotal = 0;
	controller = NULL;
	ptrs = (CratersBaseObject**)malloc(STACK_MAX_ITEMS*sizeof(CratersBaseObject*));
	entrynum = (int*)malloc(STACK_MAX_ITEMS*sizeof(int));
	tempchar = (char*)malloc(512*sizeof(char));
};

Stack::~Stack() {
	//while(nitems) drop();
	free(ptrs);
	free(entrynum);
	free(tempchar);
};

bool Stack::validateinput(int* t, unsigned int n) { //check type number flags t for n objects
	if(n>nitems) {
		push(new CError("Too few items in stack for requested operation",0));
		return false;
	}
	for(int i=0; i<n; i++) {
		if(!(t[i] & (1 << get(i)->isaNum))) {
			sprintf(tempchar,"Stack item #%i is of the wrong type for this operation",i+1);
			push(new CError(tempchar,1));
			return false;
		}
	}
	return true;
}

void Stack::disp() {
	printf("+---------------+\n");
	for(int i=0; i<nitems; i++) {
		CratersBaseObject* foo = get(nitems-1-i);
		printf("|%i (%i): %s ",nitems-i,entrynum[i],foo->isaName);
		printf("'%s' ",foo->name); //item name
		printf("\n");
	}
	if(nitems==0) printf("| <stack empty> |\n");
	printf("+---------------+\n");
}

void Stack::push(CratersBaseObject* ptr) {
	if(nitems > STACK_MAX_ITEMS - 5)
	{
		//properly dispose of extra item
		ntotal++;
		entrynum[nitems]=ntotal;
		ptrs[nitems++]=ptr;
		drop();
		
		if(nitems < STACK_MAX_ITEMS - 2)
		{
			ntotal++;
			entrynum[nitems]=ntotal;
			ptrs[nitems++] = new CError("Too many stack items!",7);
		}
		return;
	}
	ntotal++;
	entrynum[nitems]=ntotal;
	ptrs[nitems++]=ptr;
};

CratersBaseObject* Stack::pop() {
	if(!nitems) {
		push(new CError("Too few items in stack for 'pop'",0));
		return NULL;
	}
	return ptrs[--nitems];
};

CratersBaseObject* Stack::get() { //get without removing from stack
	if(!nitems) {
		push(new CError("Too few items in stack for'get'",0));
		return NULL;
	}
	return ptrs[nitems-1];
}

CratersBaseObject* Stack::get(unsigned int n) { //get nth item without removing from stack
	if(nitems < n) {
		fprintf(stderr,"\n** Stack error: out of items! **\n\n");
		return NULL;
	}
	return ptrs[nitems-n-1];
}

float Stack::getfloat(unsigned int n) { //get nth item as a float
	if(nitems < n) return 0;
	if(get(n)->isaNum == COBJ_CFLOAT) return (float)*(CFloat*)get(n);
	push(new CError("Requested number from non-numeric stack item",1));
	return 0;
}

int Stack::getint(unsigned int n) { //get nth item as an int
	if(nitems < n) return 0;
	if(get(n)->isaNum == COBJ_CFLOAT)
	{
		float z = (float)*(CFloat*)get(n);
		if(z != (int)z) printf("Warning: truncating Float %g to Int %i\n",z,(int)z);
		return (int)z;
	}
	push(new CError("Requested number from non-numeric stack item",1));
	return 0;
}

char* Stack::getstring(unsigned int n) { //get nth item as an int
	if(nitems < n) return NULL;
	if(get(n)->isaNum == COBJ_CRATERSTRING) return (char*)*(CraterString*)get(n);
	return NULL;
}

void Stack::rot(int n) { //swap 2 items on the stack
	if(n<2) {
		fprintf(stderr,"\n** Stack error: you don't rotate <2 items, silly! **\n\n");
		return;
	}
	
	if(nitems<n) {
		push(new CError("Too few items in stack for requested operation",0));
		return;
	}
	
	CratersBaseObject* tempptr = ptrs[nitems-n];
	int tempentry = entrynum[nitems-n];
	
	for(int i=n; i>1; i--) {
		ptrs[nitems-i] = ptrs[nitems-i+1];
		entrynum[nitems-i] = entrynum[nitems-i+1];
	}
	ptrs[nitems-1] = tempptr;
	entrynum[nitems-1] = tempentry;
}

void Stack::swap() { //swap 2 items on the stack
	if(nitems<2) {
		push(new CError("Too few items in stack for requested operation",0));
		return;
	}
	rot(2);
}

void Stack::drop()
{
	if(nitems == 0) {
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
	if(!nitems) return false;
	if(get()->isaNum != t) return false;
	return true;
};

bool Stack::istypef(int typeflag) {
	if(!nitems) return false;
	if((1 << get()->isaNum) & typeflag) return true;
	return false;
};
