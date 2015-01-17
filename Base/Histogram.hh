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

#ifndef CRATERS_HISTOGRAM
#define CRATERS_HISTOGRAM

#include "Basics.hh"

class Histogram
{
public:
	float* binbounds;
	float* count;
	float totalcount;
	int n;
	float wtot;
	Histogram(float *d, int size, float* weight, float min, float max, int nbins);
	~Histogram();
	void display();
	int maxbin();
	float uniformity();
	float coverage();
};

#endif