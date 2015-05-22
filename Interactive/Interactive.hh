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

/// maximum number of items allowed on stack
#define STACK_MAX_ITEMS 4096

extern bool domovie;
extern string moviebase;
extern int movieframeadvance;
extern int mergemethod;

class Interactor;

/// LIFO stack of objects for manipulation
class Stack {
public:
    /// Constructor
    Stack(): ntotal(0), controller(NULL) { }
    
    /// push object onto top of stack
    void push(CratersBaseObject* ptr);
    /// get and remove top object
    CratersBaseObject* pop();
    /// get top object without removal
    CratersBaseObject* get();
    /// get n^th-from-top object
    CratersBaseObject* get(unsigned int);
    /// get float value of n^th-from-top object
    float getfloat(unsigned int);
    /// get int value of n^th-from-top object
    int getint(unsigned int);
    /// get string value of n^th-from-top object
    string getstring(unsigned int);
    /// check whether top object matches specified type
    bool istype(int t);
    /// check whether top object has type flags attribute
    bool istypef(int t);
    /// display stack contents
    void disp();
    
    bool validateinput(int*,unsigned int);
    /// check whether named file is readable
    bool checkReadable();
    /// check whether named file is writeable
    bool checkWritable();
    /// check whether named folder exists
    bool checkFolder();
    /// rotate top n items
    void rot(int n);
    /// swap top 2 items
    void swap();
    /// drop top item
    void drop();
    
    int ntotal;                            ///< total number of items ever created on stack
    vector<CratersBaseObject*> items;    ///< items on stack
    vector<int> entrynum;                ///< entry numbers for items on stack
    Interactor* controller;                ///< Interactor manipulating stack
};

/// Action for manipulating stack items
class Action : public CratersBaseObject {
public:
    /// Constructor
    Action();
    /// subclass this action!
    virtual void DoIt();
    
    /// check inputs are available, then run
    virtual bool checkngo();
    /// check whether proper inputs are available
    virtual bool validateinput();
    /// print help info
    virtual void printinfo(int depth);
    
    string description;                ///< help description of command
    vector<string> commandnames;    ///< list of names to call command
    void addname(const string& c);    ///< add name for command
    
    int ninputs;                    ///< number of inputs required
    int* inputtypes;                ///< types of each required input
    
    Stack* mystack;                    ///< stack being manipulated
    Interactor* myinteractor;        ///< Interactor providing commands
};

/// Class for handling interactive command streams
class Interactor {
public:
    /// Constructor
    Interactor(Stack* s);
    
    /// process each command in command stream
    void processCommand();
    /// tokenize string and add to end of command stream; call processCommand
    void parseCommand(const string& ib);
    /// tokenize string and prepend to command stream
    void prependCommand(const string& ib);
    /// add command line arguments to command stream; call processCommand
    void commandLineToCommandstream(int argc, char** argv);
    /// check whether named topic is available in hierarchy
    bool knowstopic(const string& c);
    /// return the number of registered subactions in category
    int nSubActions();
    /// print one-line information
    void printinfo();
    /// print help on specified topic
    void printhelp(const string& topic, int depth);
    /// recursively print help on topic and its sub-topics
    void rprinthelp(const string& topic, int depth);
    /// add an action to available commands list
    void registerAction(Action*);
    /// add a category for sub-commands
    void registerCategory(Interactor*);
    /// enter interactive processing mode
    void interactiveMode();
    /// locate action matching given name
    Action* findaction(const string& c);
    
    string name;                        ///< display name of interactor
    string description;                    ///< text help description
    bool haltinteract;                    ///< whether to break interactive processing cycle
    bool istop;                            ///< whether this is the top-level interaction process
    unsigned int evallevel;
    CMacro* recordingmacro;                ///< macro being recorded
    vector<Action*> actions;            ///< available actions
    vector<Interactor*> categories;        ///< available categories (sub-menus with several actions)
    Stack* mystack;                        ///< stack being manipulated
    std::deque<string> commandstream;    ///< commands to process
    
};

/// Top-level Interactor
class TopInteractor : public Interactor {
public:
    /// Constructor
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