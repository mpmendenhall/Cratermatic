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
#include <string.h>
#include <cassert>

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
}

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
	for(size_t i=1; i < commandnames.size(); i++) printf(", %s",commandnames[i].c_str());
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
	evallevel = 0;
	recordingmacro = NULL;
}

void Interactor::registerAction(Action* A) {
	actions.push_back(A);
	A->mystack = mystack;
	A->myinteractor = this;
}

void Interactor::registerCategory(Interactor* I) {
	categories.push_back(I);
}

bool Interactor::knowstopic(const string& c)
{
	if(c==name) return true;
	for(auto ita = actions.begin(); ita != actions.end(); ita++)
		for(auto it = (*ita)->commandnames.begin(); it != (*ita)->commandnames.end(); it++)
            if(*it == c) return true;
	for(auto itc=categories.begin(); itc!=categories.end(); itc++) if((*itc)->knowstopic(c)) return true;
	return false;
}

int Interactor::nSubActions() {
	int n = actions.size();
	for(auto itc=categories.begin(); itc!=categories.end(); itc++) n += (*itc)->nSubActions();
	return n;
}

void Interactor::printinfo() {
	printf("+ %s [%i]: %s\n", name.c_str(), nSubActions(), description.c_str());
}

void Interactor::printhelp(const string& topic, int depth) {
	if(topic == "brief") {
		for(int i=0; i<depth; i++) printf("\t");
		printinfo();
		return;
	}
	
	if(topic == name) {
		for(int i=0; i<depth; i++) printf("\t");
		printinfo();
		for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->printhelp("brief",depth+1);
		for(auto ita = actions.begin(); ita != actions.end(); ita++) (*ita)->printinfo(depth+1);
		return;
	}
	
	if(!knowstopic(topic)) return;
	
	for(int i=0; i<depth; i++) printf("\t");
	printinfo();
	for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->printhelp(topic,depth+1);
	for(auto ita = actions.begin(); ita != actions.end(); ita++)
		for(auto it = (*ita)->commandnames.begin(); it != (*ita)->commandnames.end(); it++)
            if(*it == topic) (*ita)->printinfo(depth+1);
}

void Interactor::rprinthelp(const string& topic, int depth) {
	if(!knowstopic(topic)) return;
	
	for(int i=0; i<depth; i++) printf("\t");
	printinfo();
	
	if(topic == name) {
		for(auto ita = actions.begin(); ita != actions.end(); ita++) (*ita)->printinfo(depth+1);
		for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->rprinthelp((*itc)->name,depth+1);
		return;
	}
	
	for(auto ita = actions.begin(); ita != actions.end(); ita++)
            for(size_t j=0; j<(*ita)->commandnames.size(); j++)
            if((*ita)->commandnames[j] == topic) (*ita)->printinfo(depth+1);
	for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->rprinthelp(topic,depth+1);
}

Action* Interactor::findaction(const string& c)
{
	for(auto ita = actions.begin(); ita != actions.end(); ita++)
		for(auto it = (*ita)->commandnames.begin(); it != (*ita)->commandnames.end(); it++)
            if(*it == c) return *ita;
	for(auto itc=categories.begin(); itc!=categories.end(); itc++) {
		Action* a = (*itc)->findaction(c);
		if(a) return a;
	}
	return NULL;
}

void Interactor::processCommand() {
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
				for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->printhelp(mystack->getstring(0),0);
				printf("\n");
				mystack->drop();
			} else {
				printf("\nUse '.TOPIC help' for help on a particular command or category\n");
				printf("Use '.TOPIC rhelp' for recursive listing of all commands in TOPIC\n");
				printf("Use '.all rhelp' for the complete list of available commands\n\n");
				printf("Command categories [%i commands total]:\n\n",nSubActions());
				for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->printhelp("brief",1);
				printf("\n");
			}
			continue;
		}
		
		if(istop && opt=="rhelp") {
			if(mystack->items.size() && mystack->get()->isaNum == COBJ_CRATERSTRING && knowstopic(mystack->getstring(0))) {
				printf("\n");
				for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->rprinthelp(mystack->getstring(0),0);
				printf("\n");
				mystack->drop();
			} 
			else if(mystack->items.size() && mystack->get()->isaNum == COBJ_CRATERSTRING && mystack->getstring(0) == "all") {
				printf("\n");
				for(auto itc=categories.begin(); itc!=categories.end(); itc++) (*itc)->rprinthelp((*itc)->name,0);
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

void Interactor::parseCommand(const string& ib) {
    vector<string> cmds = split(ib," \t\n\r,");
    for(auto it = cmds.begin(); it != cmds.end(); it++) commandstream.push_back(*it);
    processCommand();
}

void Interactor::prependCommand(const string& ib) {
    vector<string> cmds = split(ib," \t\n\r,");
    for(auto it = cmds.rbegin(); it != cmds.rend(); it++) commandstream.push_front(*it);
    processCommand();
    
}

void Interactor::commandLineToCommandstream(int argc, char** argv) {
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
		assert(fgets(ib,900,stdin));
		printf("\n");
		parseCommand(ib);
	}
	free(ib);
}