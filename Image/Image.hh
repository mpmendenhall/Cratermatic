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

#ifndef CRATERS_IMAGE
#define CRATERS_IMAGE

#include "RectRegion.hh"
#include <string>
using std::string;

/// Class for Image (2D float array) data
class Image: public RectRegion {
public:
    /// Constructor
    Image(int w, int h);
    /// Constructor from RectRegion
    Image(RectRegion*);
    /// Destructor
    ~Image();
    
    /// load raw float data
    virtual void load(const string& ifname);
    /// load channel n of a 3-channel ppm file
    static Image* loadppm(const string& ifname, int n);
    
    virtual void loadtexttable(const string& ifname);
    virtual void loadarcgis(const string& ifname);
    /// load raw arbitrary bit depth (short) raster data
    static Image* loadrawbinary(const string& ifname, int w, int h, int nbits);
    static Image* loadrawbinary(FILE* ifp, int w, int h, int nbits, int ltoss, int rtoss);
    void writerawbinary(const string& ofname, int nbits);
    void writerawbinary(FILE* ofp, int nbits);
    /// write raw float data
    virtual void write(const string& ofname);
    /// write ArcGIS text data format
    virtual void writeArcGIS(const string& ofname);
    /// write BMP image
    virtual void writeBMP(const string& ofname);
    /// write data to a raw PGM format file (integers in range 0-65535)
    virtual void writePGM2(const string& ofname);
    /// write data to a raw PGM format file (integers in range 0-255)
    virtual void writePGM1(const string& ofname);
    /// dump sepatate image files for each item in catalog
    void dumpcatalog(const string&);
    
    /// produce copy of this image
    Image* copy();
    /// copy data from another image
    Image* copyfrom(Image *img);
    
    /// produce new image removing specified number of left, top, right, bottom pixel rows
    Image* trimmed(int l, int t, int r, int b);
    /// add specified number of left, top, right, bottom pixel rows
    Image* padded(int l, int t, int r, int b);
    /// pad data with r rows mirrored across edges
    Image* mirrorpadded(int r);
    /// trim this image in-place
    Image* trim_inplace(int l, int t, int r, int b);
    /// rotate image 90 degrees
    Image* rotate();
    
    int linedata(int p0, int p1, float* d);
    
    Image* add(Image* I);
    Image* lesser(Image* I);
    Image* greater(Image* I);
    Image* add(float c);
    void interpladd(float x, float y, float z);
    Image* quadratureadd(Image* I);
    Image* mult(Image* I);
    Image* mult(float c);
    Image* divide(Image* I);
    Image* sq_rt();
    Image* absval();
    Image* reciprocal();
    Image* deriv(bool);
    Image* diagDeriv(bool posdirection);
    static Image* directionfield(Image*, Image*);
    Image* maggrad2();
    
    Image* filtered(Image* f);
    Image* linedilation(int l, bool xdirection);
    Image* linedilation(int l, int x, int y);
    Image* halfplanedilation(int x, int y);
    Image* convexhull(int n);
    Image* reduce();
    Image* xlinearscale(float s);
    Image* bilinearscale(float s);
    Image* lanczos2decimate(bool xdirection);
    
    Image* lineconvolve(float* k, int s, bool xdirection);
    float* convolve1d(float* d, int w, float* k, int s);
    Image* gaussianblur(float r);
    Image* generaldilation(Image* k);
    Image* circledilation(int r);
    Image* circleerosion(int r);
    Image* circleclosing(int r);
    Image* circleopening(int r);
    Image* annulusdilation(int r0, int r1);
    static Image* filledcircleimage(unsigned int r);
    static Image* annulusimage(unsigned int r0, unsigned int r1);
    float* gausskernel(int s, float r);
    Image* removespikes();
    Image* squaredilation(int);
    Image* complement();
    Image* icomplement();
    
    static bool characterizecrater(Image* topo, Image* mask, float r, int idnum, float x0, float y0);
    void characterizesmallcrater(float r, Image* mask, int rgn);
    static float fit_wrapper(float* d, int n, void* parentobj);
    float fitdev(float* d);
    float (*fitterfunc)(float* d, int i, int width, int height);
    
    Image* htransform(int k);
    Image* smoothehtransform(float r0);
    Image* edgefinder(float r);
    Image* craterFindingTransform(float r0, Image* mask);
    static Image* craterFindingTransform(float r0, Image* gx, Image* gy);
    /// Find craters, with optional mask to exclude previously found craters. Record results to cspecs.
    int findcraters(const string& basefolder, Image* msk, vector<CraterSpec>& cspecs, bool makeoutimg, float k1, float k2, float k3, float k4, float k5, float k6, float k7);
    
    Image* pseudo_profile_curvature();
    Image* pseudo_tangent_curvature();
    //Image* plan_curvature();
    //Image* tangential_curvature();
    Image* slope();
    
    float* minmax();
    float* stats();
    
    Image* getsubregion(unsigned int,float);
    Image* getregion(BoundingBox b);
    void putregion(Image*, BoundingBox b);
    
    Image* normalized(float mn, float mx);
    Image* signedgamma(float g);
    Image* rec709gamma();
    Image* rec709gammaInverse();
    Image* threshold(float t);
    Image* kill_outliers(float sd);
    Image* normalized(float mn, float mx, float sd);
    Image* inormalized();
    Image* above(float);
    Image* below(float);
    
    /// draw marks to image in color c
    Image* drawmarks(float c);
    /// draw specified set of marks to image in color c
    Image* drawmarks(const vector<ImageMark>& m, float c);
    /// draw cross on image
    Image* crossmark(int x0, int y0, int l, float c);
    /// draw circle on image
    Image* circle(int x0, int y0, int r, float c);
    /// draw line from (x0,y0) to (x1,y1) on image
    Image* line(int x0, int y0, int x1, int y1, float c);
    /// draw line from point index p0 to p1 on image
    Image* line(int p0, int p1, float c);
    //Image* star(int,int,int);
    void safeset(int,int,float);
    void draweightsymm(int,int,int,int,float);
    
    int* sortImage();
    Image* flatHisto();
    
    float *data;        ///< data array
};

/// Grouped set of multiple images
class MultiImage : public RectRegion {
public:
    MultiImage(int,int);
    ~MultiImage();
    void addimg(Image*);
    void** apply(void* (Image::*)());
    void apply(void (Image::*)());
    void apply(void (Image::*)(float),float);
    void apply(void (Image::*)(int),int);
    int nimgs;
    Image** imgs;
};

/// Scanner for processing lines of pixels in image
class ImageDataScanner {
public:
    /// Constructor
    ImageDataScanner(Image*, int, int);
    
    /// Load next line of image into *dat. Return number of points.
    int nextline();
    /// load next line of image data into dput; return number of points.
    int nextline(float* dput);
    /// get pixel positions array
    int* getpositions() { return si.datp.data(); }
    /// place modified line data back into image
    void replacedata();
    /// place modified line data at d back into image
    void replacedata(float* d);
    /// get current line starting offset
    int getoffset() { return si.getoffset(); }
    
    Image* myImg;                    ///< image being scanned
    ScanIterator si;                ///< ScanIterator for selecting points
    vector<float> dat;                ///< data values along current line
};

#endif