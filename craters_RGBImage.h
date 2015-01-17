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

#ifndef CRATERS_RGBIMAGE
#define CRATERS_RGBIMAGE

#include "craters_RectRegion.h"
class Image;
class ClassifyImage;

class RGBImage : public RectRegion
{
public:
	Image* R;
	Image* G;
	Image* B;
	RGBImage(int w, int h);
	RGBImage(Image* img);
	RGBImage(Image* x, Image* y);
	RGBImage(RectRegion* R);
	~RGBImage();
	
	void solidcolor(float r, float g, float b);
	void desaturate(float z);
	void speckfield();
	void pointfield(int);
	void gridfield(int);
	static RGBImage* redzeros(Image* I);
	RGBImage* colorby(Image* img);
	static RGBImage* grayby(Image* img, float bp);
	static RGBImage* renderTopo(Image* t);
	RGBImage* shadeby(Image* img);
	RGBImage* maskby(Image* img);
	RGBImage* emboss(Image* img);
	RGBImage* qrbs(Image* img, float z);
	RGBImage* overlay(Image* img, float r, float g, float b, float t);
	RGBImage* drawmarks();
	RGBImage* classifyoverlay(ClassifyImage* C, int andkey, int xorkey, float r, float g, float b);
	void writeBMP(char* ofname);
	
	static RGBImage* colorwheel(int r);
	static RGBImage* fourierSpinner(int r, int k, float theta);
};

#endif