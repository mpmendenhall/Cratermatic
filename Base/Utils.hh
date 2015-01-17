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

#ifndef CRATERS_UTILS
#define CRATERS_UTILS

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using std::string;
using std::vector;
using std::ifstream;

/// utility function for converting to string
template<typename T>
string to_str(T x) {
    std::stringstream ss;
    ss << x;
    return ss.str();
}

/// split string into components
vector<string> split(const string& s, const string splitchars = " \t\n\r");

#include "Basics.hh"

float* lindilate(int l, float* d, int w);

class Minimizer {
public:
	Minimizer(float* params, float* lbound, float* ubound, int nparams, float (*evalfunc)(float* params, int n, void* p), void* po);
	~Minimizer();
	bool fitstep();
	int n;
	float* dv;
	float* p;
	float* lb;
	float* ub;
	void* parentobject;
	float (*func)(float* params, int n, void* po);
	float* derivs;
	float** sderivs;
	void calcderivs();
	void calcsderivs();
};

class Pointset
{
public:
	int nitems;
	Pointset();
	~Pointset();
	int getitem(int);
	void additem(int);
	void additem_unchecked(int);
	void additems(int*,int);
	void removeitem(int);
	void merge(Pointset*);
	bool checkitem(int);
	void intersect(Pointset*);
	void addset(Pointset*);
	void addset_unchecked(Pointset*);
	int whereis(int);
	int maxitems;
	int* items;
};

class LineIterator
{
public:
	int x0,y0,x1,y1,x,y;
	bool steep;
	bool reverse;
	int n;
	int deltax;
	int deltay;
	int error;
	int ystep;
	LineIterator(int xa,int ya,int xb,int yb);
	//LineIterator(x0,y0,a,b);
	bool step();
	int px();
	int py();
};

class Matrix
{
public:
	int rows;
	int cols;
	float* data;
	Matrix(int r, int c);
	Matrix* cp(); //copy
	~Matrix();
	float& operator() (unsigned i, unsigned j);
	Matrix* mult(float c);
	Matrix* imult(float c);
	Matrix* mult(Matrix* M);
	Matrix* add(Matrix* M);
	Matrix* iadd(Matrix* M);
	Matrix* transpose();
	Matrix* itranspose();
	Matrix* comat(unsigned, unsigned);
	Matrix* invert();
	float det();
	void disp();
};

class Vector: public Matrix
{
public:
	Vector(int l);
};

class ProgressBar
{
public:
	float p;
	int npts;
	int length;
	
	ProgressBar(int l);
	~ProgressBar();
	void update(float u);
};

#endif