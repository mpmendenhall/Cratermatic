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

#ifndef CRATERS_RASTERREGION
#define CRATERS_RASTERREGION

#include "craters_Basics.h"

class Image;
class ClassifyImage;
class RectRegion;

class RasterRegion : public CratersBaseObject
{
public:
	int maxwidth;
	int maxtotal;
	int* widths;
	int* offsets;
	int height;
	int size;
	
	int myx;
	int myy;
	int origwidth;
	int origheight;
	
	//can use same structure over float or int data
	float* data;
	int* cdata;
	float** lines;
	int** clines;
	
	void init();
	RasterRegion();
	RasterRegion(RectRegion* R, int x, int y);
	~RasterRegion();
	
	void initcdata();
		
	static RasterRegion* scanFromImage(Image* I, int x, int y);
	static RasterRegion* scanFromClassify(ClassifyImage* C, int x, int y);
	
	RasterRegion* updateImage(Image* I);
	RasterRegion* updateClassify(ClassifyImage* C);
	void correctoffsets();
	Image* putIntoImage(Image* I);
	ClassifyImage* putIntoClassify(ClassifyImage* C);
	Image* makeImage();
	ClassifyImage* makeClassifyImage();
	//static RasterRegion* craterLineScanner(Image* I,int x, int y);
	RasterRegion* craterLineScanner(Image* u);
	RasterRegion* midPointer();
	RasterRegion* peakSegmenter();
	RasterRegion* expandBy(RasterRegion*);
	
	RasterRegion* rimExpand();
	RasterRegion* cLineMerge(float ol, int andkey, int xorkey, int modkey);
	static void linearCraterMerger(int* cd, int npts, int ncls, int* ul);
	RasterRegion* orthoblur(float r);
	RasterRegion* dx();
	static float* convolve1d(float* d, int w, float* k, int s);
	static float* gausskernel(int s, float r);
};

class OrthoIterator
{
public:
	float** d;
	int pad;
	int x0;
	int lasth;
	int npts;
	RasterRegion* R;
	OrthoIterator(RasterRegion* RR, float** dd, int p);
	~OrthoIterator();
	int OrthoIterator::getoffset();
	int nextline();
	void replacedata();
	void replacedata(float* rdat);
};


#endif