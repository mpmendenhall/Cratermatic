//-----------------------------------------------------------------------
//
// CRATERMATIC Topography Analysis Toolkit
// Copyright (C) 2006 Michael Mendenhall
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

#ifndef CRATERS_MERGER
#define CRATERS_MERGER

#include "Basics.hh"

class ClassifyImage;
class Matrix;
class Image;

class Merger
{
public:
	Merger(ClassifyImage* ws, Image** ch, int nch);
	void basinstats();
	void mainloop();
	void domerge(float);
	void snapshot();
	void genfcritvals(float);
	float getfcritval(int);
	float calcweight(int,int);
	float** linkweights;
	//~Merger();
	ClassifyImage* w;
	Image** ch;
	Matrix** reg_sum;
	Matrix** reg_sscp;
	float* fcritvals;
	int* bestj;
	float* bestdv;
	int nch;
	void merge(int i, int j);
};

#endif