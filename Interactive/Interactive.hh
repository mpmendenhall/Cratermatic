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
#include "Basics.hh"
#include "Image.hh"
#include "Classify.hh"
#include "RGBImage.hh"
#include "ComplexImage.hh"
#include "Merger.hh"
#include "RasterRegion.hh"

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

#include "Interactive_Analytic.hh"
#include "Interactive_FFT.hh"
#include "Interactive_FileIO.hh"
#include "Interactive_Geo.hh"
#include "Interactive_IGen.hh"
#include "Interactive_IMath.hh"
#include "Interactive_IRec.hh"
#include "Interactive_ISeg.hh"
#include "Interactive_Morpho.hh"
#include "Interactive_Program.hh"
#include "Interactive_Stackops.hh"
#include "Interactive_Vis.hh"

#endif