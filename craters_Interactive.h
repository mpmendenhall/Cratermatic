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

#ifndef CRATERS_INTERACTIVE
#define CRATERS_INTERACTIVE

#include <vector>
#include "craters_Basics.h"
#include "craters_Image.h"
#include "craters_Classify.h"
#include "craters_RGBImage.h"
#include "craters_ComplexImage.h"
#include "craters_Merger.h"
#include "craters_RasterRegion.h"

#define STACK_MAX_ITEMS 256

extern bool domovie;
extern char* moviebase;
extern int movieframeadvance;
extern int mergemethod;

class Interactor;

class Stack
{
public:
	Stack();
	~Stack();
	char* tempchar;
	void push(CratersBaseObject* ptr);
	CratersBaseObject* pop();
	CratersBaseObject* get();
	CratersBaseObject* get(unsigned int);
	float getfloat(unsigned int);
	int getint(unsigned int);
	char* getstring(unsigned int);
	bool istype(int t);
	bool istypef(int t);
	bool istype(char*);
	void disp();
	bool validateinput(int*,unsigned int);
	bool checkReadable();
	bool checkWritable();
	bool checkFolder();
	void rot(int);
	void swap();
	void drop();
	int* bitmask;
	int* entrynum;
	int nitems;
	int ntotal;
	CratersBaseObject** ptrs;
	Interactor* controller;
};

class Action : public CratersBaseObject
{
public:
	char* description;
	char* commandnames[8];
	int ncommandnames;
	void addname(char* c);
	
	int ninputs;
	int* inputtypes;
	
	Stack* mystack;
	Interactor* myinteractor;
	
	Action();
	virtual void DoIt();
	
	virtual bool checkngo();
	virtual bool validateinput();
	virtual void printinfo(int depth);
};

class Interactor
{
public:
	char* tempchar;
	char* name;
	char* description;
	bool haltinteract;
	bool istop;
	unsigned int evallevel;
	CMacro* recordingmacro;
	Action** actions;
	int nactions;
	Interactor** categories;
	int ncategories;
	Stack* mystack;
	std::vector<char*> commandstream;
	
	Interactor(Stack* s);
	void processCommand();
	void parseCommand(char* ib);
	void prependCommand(char* ib);
	void commandLineToCommandstream(int argc, char** argv);
	bool knowstopic(char* c);
	int nSubActions();
	void printinfo();
	void printhelp(char* topic, int depth);
	void rprinthelp(char* topic, int depth);
	void registerAction(Action*);
	void registerCategory(Interactor*);
	void interactiveMode();
	Action* findaction(char* c);	
};

class TopInteractor : public Interactor
{
public:
	TopInteractor();
};

#include "craters_Interactive_Analytic.h"
#include "craters_Interactive_FFT.h"
#include "craters_Interactive_FileIO.h"
#include "craters_Interactive_Geo.h"
#include "craters_Interactive_IGen.h"
#include "craters_Interactive_IMath.h"
#include "craters_Interactive_IRec.h"
#include "craters_Interactive_ISeg.h"
#include "craters_Interactive_Morpho.h"
#include "craters_Interactive_Program.h"
#include "craters_Interactive_Stackops.h"
#include "craters_Interactive_Vis.h"

#endif