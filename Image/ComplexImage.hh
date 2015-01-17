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


#ifndef CRATERS_COMPLEXIMAGE
#define CRATERS_COMPLEXIMAGE

#include "RectRegion.hh"

#ifdef WITHFFTW

#include <fftw3.h>
class Image;

class ComplexImage : public RectRegion
{
public:
	int origwidth;
	int origheight;
	fftw_complex* data;
	ComplexImage(int w, int h);
	~ComplexImage();
	ComplexImage* copy();
	Image* inversefftreal();
	static ComplexImage* fftreal(Image* I);
	Image* real();
	Image* imag();
	Image* magv();
	ComplexImage* mult(ComplexImage*);
};
#endif

#endif