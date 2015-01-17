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

//compile: g++ -I../include -L../lib -w craters_*.cpp -O3 -o craters craters.cpp -lfftw3
//winblows compile: g++ -I/cygdrive/c/fftw/ -L. -static -mno-cygwin -w craters_*.cpp -o craters.o craters.cpp -lfftw3-3
//dll-to-library: dlltool --def /cygdrive/c/fftw/libfftw3-3.def --dllname /cygdrive/c/fftw/libfftw3-3.dll --output-lib /cygdrive/c/fftw/libfftw3-3

//run example: ./craters l ut3gridA findcraters ut3a.txt lc ut3a.txt d ut3a.bmp


extern bool domovie;
extern char* moviebase;
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
};
