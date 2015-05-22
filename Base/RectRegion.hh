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

#ifndef CRATERS_RECTREGION
#define CRATERS_RECTREGION

#include "Basics.hh"
#include <vector>
using std::vector;

class Image;

/// Specification for a mark on an image
struct ImageMark {
    /// mark type options
	enum markType {
        MARK_CROSS,		///< cross mark
        MARK_CIRCLE,	///< circle mark
        MARK_LINE		///< line mark
    } type;				///< type of mark to produce
    
	float x0;			///< start or center x
	float y0;			///< start or center y
	float x1;			///< end x
	float y1;			///< end y
	float r;			///< radius
};

/// Specification for a bounding box
struct BoundingBox {
    int lx;				///< lower x
	int ly; 			///< lower y
	int ux; 			///< upper x
	int uy; 			///< upper y
};

/// Specification for a circle
struct Circle {
	float x;			///< center x
	float y;			///< center y
	float r;			///< radius
};

/// catalog of subcraters in larger region
class CraterCatalog {
public:
	/// Constructor from file name to read
	CraterCatalog(const string& infile);
    vector<Circle> entries;		///< craters
};

/// Base class with metadata for rectangular raster data region
class RectRegion : public CratersBaseObject {
public:
    
    /// Constructor from width, height
    RectRegion(int w, int h);
    /// Destructor
    ~RectRegion();
    /// copy from another RectRegion
    void copyfromrr(RectRegion* R);
    
	int width;					///< region width
	int height;					///< region height
	int size;					///< region size = width*height
	vector<ImageMark> marks;	///< list of marks to draw
	
	int connectn;				///< connectivity number (4 or 8)
	static int* connectdx;		///< dx[] array for connectivity offsets
	static int* connectdy;		///< dy[] array for connectivity offsets
	int* connectr2;				///< r^2 = dx[i]^2+dy[1]^2 connectivity radius
	BoundingBox coords;			///< "real space" coordinates for region
	CraterCatalog* mycatalog;	///< catalog of identified craters
    
    /// load craters catalog
	void loadcatalog(const string& fname);
    
    /// add specified mark to marks list
	void addmark(ImageMark::markType t, int x, int y, int r);
    /// add specified line mark to marks list
	void addmarkline(float x0, float y0, float x1, float y1);
    /// add specified Fourier series mark to marks list
	void fouriermark(float x0, float y0, const vector<float>& xs, const vector<float>& ys, unsigned int ndivisions = 4);
	
    /// pixel distance squared between two points
    int dist2(int p, int q) const;
    /// physical length represented by pixel length l
	float reallength(float l) const { return l*(coords.ux-coords.lx)/((float)width-1.0); }
    /// physical dx represented by pixel dx l
	float realdx(float l) const { return l*(coords.ux-coords.lx)/(width - 1.0); }
    /// physical dy represented by pixel dy l
	float realdy(float l) const { return l*(coords.uy-coords.ly)/(height - 1.0); }
    /// physical coordinate represented by pixel x
	float realx(float x) const { return coords.lx + x*(coords.ux-coords.lx)/(width - 1.0); }
    /// physical coordinate represented by pixel y
	float realy(float y) const { return coords.ly + y*(coords.uy-coords.ly)/(height - 1.0); }
	bool inrange(int);
	bool inrange(int,int);
    /// find bounding box for specified array of points
	BoundingBox findboundingbox(unsigned int* p, int n);
    /// find bounding circle for specified array of points
	Circle findboundingcirc(unsigned int* p, unsigned int n);
    /// expand a bounding box by specified margin
	BoundingBox expandbb(BoundingBox, int);

	/// find x center-of-mass of specified (weighted) points
	float xcenter(unsigned int* pts, unsigned int npts, float* wt);
    /// find y center-of-mass of specified (weighted) points
	float ycenter(unsigned int* pts, unsigned int npts, float* wt);
    /// determine radial Fourier series for shape of points set
	void radialFourier(float x0, float y0, const vector<unsigned int>& ps, const vector<float>& wt, vector<float>& xs, vector<float>& ys, unsigned int nmoms);
    /// determine radial Fourier series for shape of points set
	void radialFourier(float x0, float y0, const vector<unsigned int>& ps, Image* wtimg, vector<float>& xs, vector<float>& ys, unsigned int nmoms);
    /// sum radial Fourier series at specified angle
	float invRadialFourier(float angl, const vector<float>& xs, const vector<float>& ys);
    /// fourier series ds of RMS deviation from shape
	void fourierDeviations(float x0, float y0, const vector<unsigned int>& pts, const vector<float>& xs, const vector<float>& ys, vector<float>& ds, size_t nterms);
    /// return list of points enclosed by Fourier boundaries
	void fourierPoints(float x0, float y0, const vector<float>& xs, const vector<float>& ys, vector<unsigned int>& pout);
};

/// Class for scanning across arbitrarily-oriented pixel lines
class ScanIterator {
public:
    /// Constructor
    ScanIterator(RectRegion* R, int xa, int ya);
    /// Destructor
    ~ScanIterator();
    
    /// get next line; return number of points in line
    int nextline();
    /// get offset of current line
    int getoffset() const { return offset; }
    
	int x,y,x0,y0;
	int w, h;
	int offset;
	bool steep;
	bool flipx;
	bool buildout;
	int* ys;			///< y coordinates of each scan point
	int* bps;
    vector<int> datp;	///< point locations on current line
};

#endif
