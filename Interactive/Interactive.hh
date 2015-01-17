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

#ifndef CRATERS_INTERACTIVE
#define CRATERS_INTERACTIVE

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <deque>

#include "Basics.hh"
#include "Image.hh"
#include "Classify.hh"
#include "RGBImage.hh"
#include "ComplexImage.hh"
#include "Merger.hh"
#include "RasterRegion.hh"

#define STACK_MAX_ITEMS 256

extern bool domovie;
extern string moviebase;
extern int movieframeadvance;
extern int mergemethod;

class Interactor;

class Stack {
public:
    /// Constructor
	Stack();
    /// Destructor
	~Stack();
    
	char* tempchar;
	void push(CratersBaseObject* ptr);
	CratersBaseObject* pop();
	CratersBaseObject* get();
	CratersBaseObject* get(unsigned int);
	float getfloat(unsigned int);
	int getint(unsigned int);
	string getstring(unsigned int);
	bool istype(int t);
	bool istypef(int t);
	bool istype(string);
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
	string description;
	vector<string> commandnames;
	void addname(const string& c);
	
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
	string name;
	string description;
	bool haltinteract;
	bool istop;
	unsigned int evallevel;
	CMacro* recordingmacro;
	Action** actions;
	int nactions;
	Interactor** categories;
	int ncategories;
	Stack* mystack;
	std::deque<string> commandstream;
	
	Interactor(Stack* s);
	void processCommand();
	void parseCommand(const string& ib);
	void prependCommand(const string& ib);
	void commandLineToCommandstream(int argc, char** argv);
	bool knowstopic(const string& c);
	int nSubActions();
	void printinfo();
	void printhelp(const string& topic, int depth);
	void rprinthelp(const string& topic, int depth);
	void registerAction(Action*);
	void registerCategory(Interactor*);
	void interactiveMode();
	Action* findaction(const string& c);
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