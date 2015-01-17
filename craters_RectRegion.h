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

#ifndef CRATERS_RECTREGION
#define CRATERS_RECTREGION

#include "craters_Basics.h"

class Image;

struct ImageMark {
	int type;
	float x0;
	float y0;
	float x1;
	float y1;
	float r;
};

struct BoundingBox
{
	int lx; //lower x
	int ly; //lower y
	int ux; //upper x
	int uy; //upper y
};

struct ImageCoords {
	float lx;
	float ly;
	float ux;
	float uy;
};

struct Circle
{
	float x;
	float y;
	float r;
};

struct CatalogEntry {
	int radius;
	int centerx;
	int centery;
};

class CraterCatalog //catalog of subcraters in larger region
{
public:
	int ncraters;
	CatalogEntry** entries;
	CraterCatalog(char* infile);
	~CraterCatalog();
};

class RectRegion : public CratersBaseObject
{
public:
	int width;
	int height;
	int size;
	ImageMark* marks;
	int nmarks;
	
	int connectn;
	static int* connectdx;
	static int* connectdy;
	int* connectr2;
	static bool* nocomdivs;
	static bool* gennocds(int n);
		
	CraterCatalog* mycatalog;
	void loadcatalog(char*);
	ImageCoords coords;
	RectRegion(int, int);
	~RectRegion();
	void copyfromrr(RectRegion* R);
	
	void addmark(int t, int x, int y, int r);
	void addmarkline(float x0, float y0, float x1, float y1);
	void fouriermark(float x0, float y0, float* xs, float* ys, unsigned int nterms, unsigned int ndivisions);
	
	void clearmarks();
	float reallength(float);
	float realdx(float);
	float realdy(float);
	float realx(float);
	float realy(float);
	bool inrange(int);
	bool inrange(int,int);
	BoundingBox findboundingbox(int* p, int n);
	Circle findboundingcirc(int* p, unsigned int n);
	BoundingBox expandbb(BoundingBox, int);
	int dist2(int,int);
	
	float xcenter(int* pts, unsigned int npts, float* wt);
	float ycenter(int* pts, unsigned int npts, float* wt);
	void radialFourier(float x0, float y0, int* ps, unsigned int nps, float* wt, float** xs, float** ys, unsigned int nmoms);
	void radialFourier(float x0, float y0, int* ps, unsigned int nps, Image* wtimg, float** xs, float** ys, unsigned int nmoms);
	float invRadialFourier(float angl, float* xs, float* ys, unsigned int nmoms);
	void fourierDeviations(float x0, float y0, int* pts, unsigned int npts, float* xs, float* ys, float** ds, int nterms);
	int fourierPoints(float x0, float y0, float* xs, float* ys, int nterms, int** pout);
};

class ScanIterator
{
public:
	int x,y,x0,y0;
	int w, h;
	int offset;
	bool steep;
	bool flipx;
	bool buildout;
	ScanIterator(RectRegion* R, int xa, int ya, int** dd);
	~ScanIterator();
	int* ys;
	int* bps;
	int** d;
	int nextline();
	int getoffset();
};


#endif