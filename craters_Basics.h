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

#ifndef CRATERS_BASICS
#define CRATERS_BASICS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <cmath>
#include <vector>
#include <time.h>

#define WITHFFTW
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define PI 3.141592653589796

enum CraterObjectType
{
	COBJ_CRATERSBASEOBJECT,
	COBJ_RECTREGION,
	COBJ_IMAGE,
	COBJ_CLASSIFYIMAGE,
	COBJ_COMPLEXIMAGE,
	COBJ_RGBIMAGE,
	COBJ_CFLOAT,
	COBJ_CRATERSTRING,
	COBJ_CMACRO,
	COBJ_RASTERREGION,
	COBJ_CERROR
};

enum CraterObjectTypeFlag
{
	COF_CRATERSBASEOBJECT	= 1 << COBJ_CRATERSBASEOBJECT,
	COF_RECTREGION			= 1 << COBJ_RECTREGION,
	COF_IMAGE				= 1 << COBJ_IMAGE,
	COF_CLASSIFYIMAGE		= 1 << COBJ_CLASSIFYIMAGE,
	COF_COMPLEXIMAGE		= 1 << COBJ_COMPLEXIMAGE,
	COF_RGBIMAGE			= 1 << COBJ_RGBIMAGE,
	COF_RASTERREGION		= 1 << COBJ_RASTERREGION,
	COF_CFLOAT				= 1 << COBJ_CFLOAT,
	COF_CRATERSTRING		= 1 << COBJ_CRATERSTRING,
	COF_CMACRO				= 1 << COBJ_CMACRO,
	COF_CERROR				= 1 << COBJ_CERROR,
	COF_ANYTYPE = COF_CRATERSBASEOBJECT | COF_RECTREGION | COF_IMAGE |
		COF_CLASSIFYIMAGE | COF_COMPLEXIMAGE | COF_RGBIMAGE | COF_RASTERREGION |
		COF_CFLOAT | COF_CRATERSTRING | COF_CMACRO | COF_CERROR
};

class CratersBaseObject
{
public:
	char* isaName;
	CraterObjectType isaNum;
	char* name;
	
	static const int bytelength;
	
	CratersBaseObject();
	~CratersBaseObject();
	
	static void writeBinaryFromNormalizedFloat(float* fdat, int len, FILE* ofp, int nbits);
	static void writeBinaryFromBool(bool* bdat, int len, FILE* ofp);
	static void writeBMPHeaders(FILE* ofp, int bitdepth, int width, int height);
	static void rev2byte(FILE* ofp, unsigned short foo);
	static void rev4byte(FILE* ofp, unsigned int foo);
	static void writeBMPgreyColorPalette(FILE* ofp);
	static void writeBMPmonoColorPalette(FILE* ofp);
	static int* scatterNumberBase(unsigned int n);
	static int* scatterNumber(unsigned int n);
	static void lsrl(float* x, float* y, float* w, int n, float* a, float* b, float* r);
	
	static void hsv2rgb(float h, float s, float v, float* r, float* g, float* b);
	static void rgb2hsv(float r, float g, float b, float* h, float* s, float* v);
	
	static int compareFloat(const void * a, const void * b);
	static int compareFloatP(const void * a, const void * b);
};

class CFloat : public CratersBaseObject
{
public:
	float val;
	CFloat(float f);
	CFloat* copy();
	operator float() const;
};


class CraterString : public CratersBaseObject
{
public:
	char* val;
	CraterString(char* c);
	~CraterString();
	CraterString* copy();
	operator char*() const;
};

class CError : public CratersBaseObject
{
public:
	char* errname;
	int errnum;
	CError(char* c, int i);
	~CError();
	CError* copy();
};

class CMacro : public CratersBaseObject
{
public:
	signed int maxlen;
	char* stringval;
	CMacro();
	~CMacro();
	CMacro* copy();
	void addtoken(char* t);
	operator char*() const;
};

class CraterSpec
{
public:
	
	CraterSpec(int idn);
	~CraterSpec();
	
	int idnum;
	float x;
	float y;
	float r;
	float area;
	float volume;
	float hipt;
	float lowpt;
	float depth;
	float steepness;
	
	//shape Fourier terms
	float* xsft;
	float* ysft;
	float* deviation;
	
	//gradient fourier terms
	float* grxxsft;
	float* grxysft;
	float* gryxsft;
	float* gryysft;
	
	void writeToFile(FILE* ofp);
	static void writeHeaders(FILE* ofp);
	void writeShapeFourierToFile(FILE* ofp);
	void writeGradFourierToFile(FILE* ofp);
};

#endif