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

#include "Interactive.hh"
#include "Utils.hh"
#include <iostream>
#include <sstream>

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

Action::Action(): description("Null action; it does NOTHING!") {
	inputtypes = (int*)malloc((STACK_MAX_ITEMS+5)*sizeof(int));
	ninputs = 0; //stack inputs
	mystack = NULL;
	myinteractor = NULL;
};

void Action::addname(const string& c) {
	if(!commandnames.size()) name = c;
	commandnames.push_back(c);
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
	printf("- %s",commandnames[0].c_str());
	for(int i=1; i < commandnames.size(); i++) printf(", %s",commandnames[i].c_str());
	printf(": %s",description.c_str());
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

bool Interactor::knowstopic(const string& c)
{
	if(c==name) return true;
	for(int i=0; i<nactions; i++) {
		Action* a = actions[i];
		for(auto it = a->commandnames.begin(); it != a->commandnames.end(); it++) if(*it == c) return true;
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

void Interactor::printinfo() {
	printf("+ %s [%i]: %s\n", name.c_str(), nSubActions(), description.c_str());
}

void Interactor::printhelp(const string& topic, int depth)
{
	if(topic == "brief") {
		for(int i=0; i<depth; i++) printf("\t");
		printinfo();
		return;
	}
	
	if(topic == name) {
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
		for(auto it = a->commandnames.begin(); it != a->commandnames.end(); it++) if(*it== topic) a->printinfo(depth+1);
	}
}

void Interactor::rprinthelp(const string& topic, int depth)
{
	if(!knowstopic(topic)) return;
	
	for(int i=0; i<depth; i++) printf("\t");
	printinfo();
	
	if(topic == name) {
		for(int i=0; i<nactions; i++) actions[i]->printinfo(depth+1);
		for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(categories[i]->name,depth+1);
		return;
	}
	
	for(int i=0; i<nactions; i++)
	{
		Action* a = actions[i];
		for(int j=0; j<a->commandnames.size(); j++) if(a->commandnames[j] == topic) a->printinfo(depth+1);
	}
	for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(topic,depth+1);
}

Action* Interactor::findaction(const string& c)
{
	for(int i=0; i<nactions; i++) {
		Action* a = actions[i];
		for(auto it = a->commandnames.begin(); it != a->commandnames.end(); it++) if(*it == c) return a;
	}
	for(int i=0; i<ncategories; i++) {
		Action* a = categories[i]->findaction(c);
		if(a) return a;
	}
	return NULL;
}

void Interactor::processCommand(){ //iterate through commandstream
	string opt;
	
	while(commandstream.size())
	{
		opt = commandstream.front();
		commandstream.erase(commandstream.begin());
		
		//macro commands
		if(opt == "[") {
			if(evallevel == 0) recordingmacro = new CMacro();
			else recordingmacro->addtoken(opt);
			++evallevel;
			continue;
		}
		
		if(opt == "]") {
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
		if(istop && (opt=="quit" || opt=="q" || opt=="abort" || opt=="exit" || opt=="stop"
					 || opt=="die" || opt=="decease" || opt=="desist" || opt=="halt")){
			haltinteract=true;
			printf("\n Goodbye.\n");
			break;
		}
		
		if(istop && (opt=="help" || opt=="?" || opt=="commands" || opt=="about" || opt=="info")) {
			if(mystack->items.size() && mystack->get()->isaNum == COBJ_CRATERSTRING && knowstopic(mystack->getstring(0)))
			{
				printf("\n");
				for(int i=0; i<ncategories; i++) categories[i]->printhelp(mystack->getstring(0),0);
				printf("\n");
				mystack->drop();
			} else {
				printf("\nUse '.TOM_PIC help' for help on a particular command or category\n");
				printf("Use '.TOM_PIC rhelp' for recursive listing of all commands in TOM_PIC\n");
				printf("Use '.all rhelp' for the complete list of available commands\n\n");
				printf("Command categories [%i commands total]:\n\n",nSubActions());
				for(int i=0; i<ncategories; i++) categories[i]->printhelp("brief",1);
				printf("\n");
			}
			continue;
		}
		
		if(istop && opt=="rhelp") {
			if(mystack->items.size() && mystack->get()->isaNum == COBJ_CRATERSTRING && knowstopic(mystack->getstring(0))) {
				printf("\n");
				for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(mystack->getstring(0),0);
				printf("\n");
				mystack->drop();
			} 
			else if(mystack->items.size() && mystack->get()->isaNum == COBJ_CRATERSTRING && mystack->getstring(0) == "all") {
				printf("\n");
				for(int i=0; i<ncategories; i++) categories[i]->rprinthelp(categories[i]->name,0);
				printf("\n");
				mystack->drop();
			}
			continue;
		}
		
		if(istop && (opt=="license" || opt=="warranty"))
		{
			printf("//-----------------------------------------------------------------------\n");
			printf("//\n");
			printf("// CRATERMATIC Topography Analysis Toolkit\n");
			printf("// Copyright (C) 2006-2015 Michael Mendenhall\n");
			printf("//\n");
			printf("// This program is free software; you can redistribute it and/or\n");
			printf("// modify it under the terms of the GNU General Public License\n");
			printf("// as published by the Free Software Foundation; either version 3\n");
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
		
		if(istop && opt=="interactive") {
			interactiveMode();
			continue;
		}
		
		if(opt=="") { //null command
			continue;
		}
		
		//check for string types
		if(opt[0] == '.') {
			mystack->push(new CraterString(opt.c_str()+1));
			continue;
		}
		
		if(opt[0] == '"' || opt[0] == '%') {
			string foo;
			string bar = opt;
			foo[0]='\0'; //start with null termination
			int i=0;
			
			while(commandstream.size()) {
				if(i>0) {
					bar = commandstream.front();
					commandstream.erase(commandstream.begin());
				}
				
				if(foo.size()+bar.size()<1000) {
					if(i>0) foo += " ";
					foo += bar;
				}
				else {
					mystack->push(new CError("String input too long",4));
					break;
				}
				
				if((bar[bar.size()-1] == '"' || bar[bar.size()-1] == '%') && !(i==0 && bar.size()==1)) break;
				
				++i;
			}
			
			if(!(foo[foo.size()-1] == '"' || foo[foo.size()-1] == '%')) {
				mystack->push(new CError("Missing closing \" or % on string",3));
				break;
			}
			   
			foo.resize(foo.size()-1);
			mystack->push(new CraterString(foo.c_str()+1));
			continue;
		}
		
		//check for numerical types
		if(strspn(opt.c_str(),"-0123456789.eE+") == opt.size() && strpbrk(opt.c_str(),"0123456789")) {
			mystack->push(new CFloat(atof(opt.c_str())));
			continue;
		}
		
		
		//check for action types
		Action* a = findaction(opt);
		if(!a) {
			mystack->push(new CError("Unrecognized command: '"+opt+"'",2));
			break;
		}
		
		if(!a->checkngo()) break;
	}
	
	//error handling
	int n=0;
	while(mystack->items.size() > 0 && mystack->get()->isaNum == COBJ_CERROR)
	{
		CError* E = (CError*)mystack->get();
		printf("*** Error %i: %s\n",E->errnum,E->errname.c_str());
		mystack->drop();
		n++;
	}
	if(n)
	{
		printf("*** Aborting further input evaluation.\n");
		while(commandstream.size()) {
			opt = commandstream.back();
			commandstream.pop_back();
		}
	}
	
}

void Interactor::parseCommand(const string& ib) //tokenize string and add to end of command stream; call processCommand
{
    vector<string> cmds = split(ib," \t\n\r,");
    for(auto it = cmds.begin(); it != cmds.end(); it++) commandstream.push_back(*it);
    processCommand();
}

void Interactor::prependCommand(const string& ib) //tokenize string and prepend to command stream
{
    vector<string> cmds = split(ib," \t\n\r,");
    for(auto it = cmds.rbegin(); it != cmds.rend(); it++) commandstream.push_front(*it);
    processCommand();
    
}

void Interactor::commandLineToCommandstream(int argc, char** argv) //add command line arguments to command stream; call processCommand
{
	for(int i=1; i<argc; i++) commandstream.push_back(argv[i]);
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