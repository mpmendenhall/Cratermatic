//-----------------------------------------------------------------------
//
// CRATERMATIC Topography Analysis Toolkit
// Copyright (C) 2006 Michael Mendenhall
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
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

#include "craters_Interactive.h"

//Error codes:
// 0  too few stack items
// 1  wrong type for operation
// 2  unrecognized command
// 3  missing " or % on string
// 4  input overrun
// 5  unreadable file
// 6  unwritable file
// 7  stack overflow
// 8  bad argument value

Action::Action()
{
	description = "Null action; it does NOTHING!";
	ncommandnames=0;
	inputtypes = (int*)malloc((STACK_MAX_ITEMS+5)*sizeof(int));
	ninputs = 0; //stack inputs
	mystack = NULL;
	myinteractor = NULL;
};

void Action::addname(char* c)
{
	if(ncommandnames > 14) return;
	if(ncommandnames==0) sprintf(name,"%s",c);
	commandnames[ncommandnames] = (char*)malloc((strlen(c)+1)*sizeof(char));
	strcpy(commandnames[ncommandnames],c);
	ncommandnames++;
}

bool Action::validateinput() {
	return mystack->validateinput(inputtypes,ninputs);
}

bool Action::checkngo() {	
	if(validateinput()){
		DoIt();
		return true;
	}
	return false;
}

void Action::DoIt() {
	printf("The command, it does NOTHING!\n");
}

void Action::printinfo(int depth) {
	for(int d=0; d<depth; d++) printf("\t");
	printf("- %s",commandnames[0]);
	for(int i=1; i < ncommandnames; i++) printf(", %s",commandnames[i]);
	printf(": %s",description);
	if(ninputs == 1) printf(" [1 argument]");
	if(ninputs > 1) printf(" [%i arguments]",ninputs);
	
	//{
	//	for(int d=0; d<depth; d++) printf("\t");
	//	printf("\tInputs: ");
	//	printf("%s",argtypenames[argtypes[0]]);
	//	for(int i=1; i<nargs; i++) printf(", %s",argtypenames[argtypes[i]]);
	//	printf("\n");
	//}
		
	printf("\n");
}



//--------------------------------------------------------------------------



Interactor::Interactor(Stack* s) {
	tempchar = (char*)malloc(256*sizeof(char));
	name = NULL;
	description = NULL;
	istop = false;
	mystack = s;
	nactions=0;
	evallevel = 0;
	recordingmacro = NULL;
	actions = (Action**)malloc(0*sizeof(Action*));
	ncategories=0;
	categories = (Interactor**)malloc(0*sizeof(Interactor*));
}

void Interactor::registerAction(Action* A)
{
	nactions++;
	actions = (Action**)realloc(actions,nactions*sizeof(Action*));
	actions[nactions-1]=A;
	A->mystack = mystack;
	A->myinteractor = this;
}

void Interactor::registerCategory(Interactor* I)
{
	ncategories++;
	categories = (Interactor**)realloc(categories,ncategories*sizeof(Interactor*));
	categories[ncategories-1]=I;
}

bool Interactor::knowstopic(char* c)
{
	if(!strcmp(c,name)) return true;
	for(int i=0; i<nactions; i++) {
		Action* a = actions[i];
		for(int j=0; j<a->ncommandnames; j++) {
			if(!strcmp(c,a->commandnames[j])) return true;
		}
	}
	for(int i=0; i<ncategories; i++) {
		if(categories[i]->knowstopic(c)) return true;
	}
	return false;
}

int Interactor::nSubActions() //return the number of registered subactions in category
{
	int n = nactions;
	for(int i=0; i<ncategories; i++) n+=categories[i]->nSubActions();
	return n;
}

void Interactor::printinfo()
{
	printf("+ %s [%i]: %s\n",name,nSubActions(),description);
}

void Interactor::printhelp(char* topic, int depth)
{
	if(!strcmp(topic,"brief"))
	{
		for(int i=0; i<depth; i++) printf("\t");
		printinfo();
		return;
	}
	
	if(!strcmp(topic,name))
	{
		for(int i=0; i<depth; i++) printf("\t");
		printinfo();
		for(int i=0; i<ncategories; i++) categories[i]->printhelp("brief",depth+1);
		for(int i=0; i<nactions; i++) actions[i]->printinfo(depth+1);
		return;
	}
	
	if(!knowstopic(topic)) return;
	
	for(int i=0; i<depth; i++) printf("\t");
	printinfo();
	for(int i=0; i<ncategories; i++) categories[i]->printhelp(topic,depth+1);
	for(int i=0; i<nactions; i++) {
		Action* a = actions[i];
		for(int j=0; j<a->ncommandnames; j++) {
			if(!strcmp(topic,a->commandnames[j])) a->printinfo(depth+1);
		}
	}
}

void Interactor::rprinthelp(char* topic, int depth)
{
	if(!knowstopic(topic)) return;
	
	for(int i=0; i<depth; i++) printf("\t");
	printinfo();
	
	if(!strcmp(topic,name))
	{
		for(int i=0; i<nactions; i++) actions[i]->printinfo(depth+1);
		for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(categories[i]->name,depth+1);
		return;
	}
	
	for(int i=0; i<nactions; i++)
	{
		Action* a = actions[i];
		for(int j=0; j<a->ncommandnames; j++) if(!strcmp(a->commandnames[j],topic)) a->printinfo(depth+1);
	}
	for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(topic,depth+1);
}

Action* Interactor::findaction(char* c)
{
	for(int i=0; i<nactions; i++) {
		Action* a = actions[i];
		for(int j=0; j<a->ncommandnames; j++)
		{
			if(!strcmp(c,a->commandnames[j])) return a;
		}
	}
	for(int i=0; i<ncategories; i++) {
		Action* a = categories[i]->findaction(c);
		if(a) return a;
	}
	return NULL;
}

void Interactor::processCommand(){ //iterate through commandstream
	char* opt = NULL;
	
	while(commandstream.size())
	{
		if(opt) free(opt);
		opt = (char*)commandstream.front();
		commandstream.erase(commandstream.begin());
		
		//macro commands
		if(!strcmp(opt,"["))
		{
			if(evallevel == 0) recordingmacro = new CMacro();
			else recordingmacro->addtoken(opt);
			++evallevel;
			continue;
		}
		
		if(!strcmp(opt,"]"))
		{
			if(!evallevel)
			{
				printf("\n** Unmatched ']' **\n** Aborting further input evaluation! **\n");
				break;
			}
			--evallevel;
			if(!evallevel)
			{
				mystack->push(recordingmacro);
				recordingmacro = NULL;
			} else recordingmacro->addtoken(opt);
			continue;
		}
		
		if(evallevel) {recordingmacro->addtoken(opt); continue;}
		
		//special commands
		if(istop && (!strcmp(opt,"quit") || !strcmp(opt,"q") || !strcmp(opt,"abort")
					 || !strcmp(opt,"exit") || !strcmp(opt,"stop") || !strcmp(opt,"die") 
					 || !strcmp(opt,"decease") || !strcmp(opt,"desist") || !strcmp(opt,"halt"))){
			haltinteract=true;
			printf("\n Goodbye.\n");
			break;
		}
		
		if(istop && (!strcmp(opt,"help") || !strcmp(opt,"?") || !strcmp(opt,"commands")
					 || !strcmp(opt,"about") || !strcmp(opt,"info")))
		{
			if(mystack->nitems && mystack->get()->isaNum == COBJ_CRATERSTRING && knowstopic(mystack->getstring(0)))
			{
				printf("\n");
				for(int i=0; i<ncategories; i++) categories[i]->printhelp(mystack->getstring(0),0);
				printf("\n");
				mystack->drop();
			} else {
				printf("\nUse '.TOPIC help' for help on a particular command or category\n");
				printf("Use '.TOPIC rhelp' for recursive listing of all commands in TOPIC\n");
				printf("Use '.all rhelp' for the complete list of available commands\n\n");
				printf("Command categories [%i commands total]:\n\n",nSubActions());
				for(int i=0; i<ncategories; i++) categories[i]->printhelp("brief",1);
				printf("\n");
			}
			continue;
		}
		
		if(istop && !strcmp(opt,"rhelp"))
		{
			if(mystack->nitems && mystack->get()->isaNum == COBJ_CRATERSTRING && knowstopic(mystack->getstring(0)))
			{
				printf("\n");
				for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(mystack->getstring(0),0);
				printf("\n");
				mystack->drop();
			} 
			else if(mystack->nitems && mystack->get()->isaNum == COBJ_CRATERSTRING && !strcmp(mystack->getstring(0),"all") )
			{
				printf("\n");
				for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(categories[i]->name,0);
				printf("\n");
				mystack->drop();
			}
			continue;
		}
		
		if(istop && (!strcmp(opt,"license") || !strcmp(opt,"warranty")))
		{
			printf("//-----------------------------------------------------------------------\n");
			printf("//\n");
			printf("// CRATERMATIC Topography Analysis Toolkit\n");
			printf("// Copyright (C) 2006 Michael Mendenhall\n");
			printf("//\n");
			printf("// This program is free software; you can redistribute it and/or\n");
			printf("// modify it under the terms of the GNU General Public License\n");
			printf("// as published by the Free Software Foundation; either version 2\n");
			printf("// of the License, or (at your option) any later version.\n");
			printf("//\n");
			printf("// This program is distributed in the hope that it will be useful,\n");
			printf("// but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
			printf("// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
			printf("// GNU General Public License for more details.\n");
			printf("//\n");
			printf("// You should have received a copy of the GNU General Public License\n");
			printf("// along with this program; if not, write to the Free Software\n");
			printf("// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1\n");
			printf("//\n");
			printf("//-----------------------------------------------------------------------\n\n");
			continue;
		}
		
		if(istop && !strcmp(opt,"interactive")) {
			interactiveMode();
			continue;
		}
		
		if(!strcmp(opt,"")) { //null command
			continue;
		}
		
		//check for string types
		if(opt[0] == '.')
		{
			mystack->push(new CraterString(opt+1));
			continue;
		}
		
		if(opt[0] == '"' || opt[0] == '%') {
			char* foo = (char*)malloc(1024*sizeof(char));
			char* bar = opt;
			foo[0]='\0'; //start with null termination
			int i=0;
			
			while(commandstream.size())
			{
				if(i>0) {
					if(i>1) free(bar);
					bar = commandstream.front();
					commandstream.erase(commandstream.begin());
				}
				
				if(strlen(foo)+strlen(bar)<1000) {
					if(i>0) strcat(foo," ");
					strcat(foo,bar);
				}
				else {
					mystack->push(new CError("String input too long",4));
					free(foo);
					break;
				}
				
				if((bar[strlen(bar)-1] == '"' || bar[strlen(bar)-1] == '%') && !(i==0 && strlen(bar)==1)) break;
				
				++i;
			}
			if(i>1) free(bar);
			
			if(!(foo[strlen(foo)-1] == '"' || foo[strlen(foo)-1] == '%'))
			{
				mystack->push(new CError("Missing closing \" or % on string",3));
				free(foo);
				break;
			}
			   
			foo[strlen(foo)-1]='\0';
			mystack->push(new CraterString(foo+1));
			free(foo);
			continue;
		}
		
		//check for numerical types
		if(strspn(opt,"-0123456789.eE+") == strlen(opt) && strpbrk(opt,"0123456789")) {
			mystack->push(new CFloat(atof(opt)));
			continue;
		}
		
		
		//check for action types
		Action* a = findaction(opt);
		if(!a) {
			sprintf(tempchar,"Unrecognized command: %s",opt);
			mystack->push(new CError(tempchar,2));
			break;
		}
		
		if(!a->checkngo()) break;
	}
	if(opt) free(opt);
	
	//error handling
	int n=0;
	while(mystack->nitems > 0 && mystack->get()->isaNum == COBJ_CERROR)
	{
		CError* E = (CError*)mystack->get();
		printf("*** Error %i: %s\n",E->errnum,E->errname);
		mystack->drop();
		n++;
	}
	if(n)
	{
		printf("*** Aborting further input evaluation.\n");
		while(commandstream.size())
		{
			opt = (char*)commandstream.back();
			free(opt);
			commandstream.pop_back();
		}
	}
	
}

void Interactor::parseCommand(char* ib) //tokenize string and add to end of command stream; call processCommand
{
	char* foo;
	char* tkn = strtok(ib," \t\n\r,");
	while(tkn != NULL) {
		foo = (char*)malloc((strlen(tkn)+5)*sizeof(char));
		strcpy(foo,tkn);
		commandstream.push_back(foo);
		tkn = strtok(NULL," \t\n\r,");
	}
	processCommand();
}

void Interactor::prependCommand(char* ib) //tokenize string and prepend to command stream
{
	char* tkn = strtok(ib," \t\n\r,");
	char* foo;
	int i=0;
	while(tkn != NULL) {
		foo = (char*)malloc((strlen(tkn)+5)*sizeof(char));
		strcpy(foo,tkn);
		commandstream.insert(commandstream.begin()+i,foo);
		tkn = strtok(NULL," \t\n\r,");
		i++;
	}
}

void Interactor::commandLineToCommandstream(int argc, char** argv) //add command line arguments to command stream; call processCommand
{
	char* foo;
	for(int i=1; i<argc; i++)
	{
		foo = (char*)malloc((strlen(argv[i])+5)*sizeof(char));
		strcpy(foo,argv[i]);
		commandstream.push_back(foo);
	}
	processCommand();
}

void Interactor::interactiveMode() {
	printf(" >> Entering interactive mode <<\n");
	haltinteract = false;
	char* ib = (char*)malloc(1000*sizeof(char));
	while(!haltinteract) {
		printf("\n");
		if(!evallevel)
		{
			mystack->disp();
			printf("command> ");
		} else {
			printf("  ...  > ");
		}
		fgets(ib,900,stdin);
		printf("\n");
		parseCommand(ib);
	}
	free(ib);
}