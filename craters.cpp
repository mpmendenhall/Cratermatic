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

//run example: ./craters l ut3gridA findcraters ut3a.txt lc ut3a.txt d ut3a.bmp

extern bool domovie;
extern string moviebase;
extern int movieframeadvance;
extern int mergemethod;

int main (int argc, char **argv)
{
	//systemwide globals
	domovie = false;
	moviebase = "movieframe_%04i.bmp";
	movieframeadvance = 20;
	mergemethod=2;
	//------------------
	
	TopInteractor* I = new TopInteractor();
	printf("\n");
	if(argc>1) I->commandLineToCommandstream(argc,argv);
	else I->interactiveMode();
	printf("\n");
	
	return 0;
}
