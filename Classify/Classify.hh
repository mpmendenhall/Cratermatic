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

#ifndef CRATERS_CLASSIFY
#define CRATERS_CLASSIFY

#include "RectRegion.hh"

class Image;
class RGBImage;
class Pointset;
class Histogram;

class SparseInt
{
public:
	SparseInt(int w, int h);
	int width;
	int height;
	~SparseInt();
	int get(int,int);
	void set(int,int, int);
	std::vector<int>** rows;
	std::vector<int>** rowdat;
	int columnlist(int row, int*& output);
	int columnvals(int row, int*& output);
	void disprow(int row);
};

struct BasinStat {
	int idnum;			//1
	int npic;			//2
	float basinmin;		//3
	float basinmax;		//4
	float boundsmin;	//5
	float boundsmax;	//6
	float boundsavg;	//7
	float xsum;
	float ysum;
	float xxsum;		//10
	float yysum;		//11
	float zsum;			//12
	float zzsum;		//13
	float temp;			//14
	int minloc;
};

class CraterPoints {
public:
	int nfloorpts;
	int* floorpts;
	int nrimpts;
	int* rimpts;
	
	CraterPoints();
	~CraterPoints();
	
	void addfloor(int* d, int n);
	void addrim(int* d, int n);
};

class ClassifyImage: public RectRegion
{
public:
	int* npic;
	int** pic;
	int* data;
	int shift;
	int nbasins;
	int* nbounds;
	int** bounds;
	Image* underlying;
	bool hasboundaries;
	SparseInt* cg;
	bool hasconnectivity;
	bool badimg;
	bool isclassified;
	
	BasinStat** stats;
	bool *markedregion;
	int* poi; //"points of interest"
	int npoi;
	void addregiontopoi(int n);
	void markedregionstopoi();
	Image* markedregionstoimage();
	
	ClassifyImage(RectRegion* R);
	ClassifyImage(int w,int h);
	~ClassifyImage();
	
	void initialize();
	void renumerate();
	void renumerateWithKey(int andkey);
	void cleardata();
	
	ClassifyImage* copy();
	void loadarcgis(char* ifname);
	void writeArcGIS(char *ofname);
	void writeLowBitBMP(char *ofname);
	Image* lowBitsToImage(int nbits);
	Image* dataToImage();
	
	void calcstats();
	void underlyingstats(Image* foo);
	void settempstat(unsigned int n);
	void normalizebasins(Image*);
	Histogram* regionhisto(Image* I, Image* wt, unsigned int n, float mn, float mx, int nbins);
	void underlyingavg(Image *);
	void underlyingmedian(Image *);
	void underlyingmin(Image *);
	void underlyingmax(Image *);
	void underlyingRadialCorrelation(Image* u);
	void circularity();
	void angularity();
	void random();
	
	void findboundaries();
	void connectivitygraph();
	void joinregions(int a, int b, bool dobounds);
	Pointset* boundswho(int p);
	Pointset* mutualboundary(int a, int b);
	void getjoinpoints(int,int,int,Pointset*,Pointset*);
	
	Image* recolorize(int base, int modkey);
	Image* recolorize(float* c);
	Image* scatterColor(unsigned int nbits);
	Image* boundaryimage();
	Image* tempstatimg();
	Image* fourboundaryimage();
	Image* markimage();
	RGBImage* prettyImage();
	RGBImage* prettyoverlayimage(Image*);
	RGBImage* prettyoverlayimage(RGBImage* C);
	RGBImage* colorbytemp();
	void labelboundaries(int c);
	
	Image* extractChunk(unsigned int n, Image* img, int l);
	Image* extractChunk(unsigned int n, Image* img);
	Image* extractMaskedChunk(unsigned int n, Image* img);
	Image* extractChunkMask(unsigned int n, int l);
	Image* extractChunkMask(unsigned int n);
	void cutoutChunkMask(Image* msk, unsigned int n);
		
	ClassifyImage* dat_and(ClassifyImage* b);
	ClassifyImage* dat_or(ClassifyImage* b);
	ClassifyImage* dat_xor(ClassifyImage* b);
	ClassifyImage* dat_and(int q);
	ClassifyImage* dat_or(int q);
	ClassifyImage* dat_xor(int q);
	void xorPoints(int*, unsigned int, unsigned int);
	void xorRegion(unsigned int, unsigned int);
	void andPoints(int*, unsigned int, unsigned int);
	void andRegion(unsigned int, unsigned int);
	
	static ClassifyImage* fromzeros(Image* I);
	static ClassifyImage* fromCurvature(Image* I);
	static const bool* is8simple;
	static Image* neatopic(int n);
	static Image* simplepic();
	void findObjectsByLowBits(int nbits);
	void findObjectsByHigherBits(int nbits);
	int seedFillByMaskedBits(int startp, int** pout, int searchmask, int setmask, int setnum, int* nudata);
	ClassifyImage* upShift(int nbits);
	ClassifyImage* getPoints(int* d, unsigned int n, unsigned int pad);
	ClassifyImage* getObject(unsigned int n, unsigned int pad);
	Image* getImageObject(Image* u, unsigned int n, unsigned int pad);
	Image* getImageMaskObject(unsigned int n, unsigned int pad);
	ClassifyImage* putObject(unsigned int n, ClassifyImage* bi, unsigned int pad);
	ClassifyImage* fillHoles(int flagmark);
	ClassifyImage* removeSmall(int s);
	ClassifyImage* constrainSize(int s1, int s2);
	ClassifyImage* removeUncircular(float th);
	
	void watershedFloodplainFill(int* fd, int* distance);
	void watershedMinimaRecursor(int x, int y, int x0, int y0, int* fd);
	int watershedFlowRecursor(int x, int y, int* fd);
	Pointset* watershedDownBoundsWho(int p);
	static ClassifyImage* watershed(Image* u);
	static ClassifyImage* gradSeg(Image* u, float r);
	
	void radialization(unsigned int n, Image* u);
		
	/* //old ClassifyImage stuff
	void calcstats();
	Image* suppressminima(float t);
	float* averagelocation();
	float averageradius(float x0, float y0);
	float singleregionaverageradius(float x0, float y0, int n);
	float radialvariance(float* d, int n);
	float singleregionradialvariance(float* d, float ravg, int n);
	void chainmergeslope(Image* u, float r);
	static float radialvariance_wrapper(float* d, int n, void* parentobj); // for callbacks by minimizer; implemented as in www.newty.de/fpt/callback.html
	float* find_equidistant(); */
};

/*
 //old BinaryImage stuffs
	 ClassifyImage* homotopicthin(ClassifyImage* mask, bool prune);
	 ClassifyImage* halfplanedilation(int x, int y);
	 ClassifyImage* convexhull(int n);
	 ClassifyImage* boundaries();
	 ClassifyImage* generaldilation(ClassifyImage* k);
	 ClassifyImage* boundarydilation(ClassifyImage* k);
	 ClassifyImage* circledilation(int r);
	 ClassifyImage* circleerosion(int r);
	 ClassifyImage* circleclosing(int r);
	 ClassifyImage* circleopening(int r);
	 
	 static ClassifyImage* filledcircleimage(int r);
	 static int connectivity(RectRegion* R, int* d, unsigned int n);
	 static bool* binlindil(int l, bool* d, int w);
	 ClassifyImage* linedilation(int x, int y, int l);
	 static bool* binlinero(int l, bool* d, int w);
	 ClassifyImage* lineerosion(int x, int y, int l);
	 ClassifyImage* lineopening(int x, int y, int l);
	 ClassifyImage* lineclosing(int x, int y, int l);
	 ClassifyImage* slineopening(int x, int y, int l);
	 ClassifyImage* slineclosing(int x, int y, int l);
	 ClassifyImage* srectopening(int x, int y, int lx, int ly);
	 ClassifyImage* srectclosing(int x, int y, int lx, int ly);
*/ 

class ClassifyDataScanner
{
public:
	ClassifyDataScanner(ClassifyImage*, int, int, int**);
	~ClassifyDataScanner();
	ClassifyImage* myImg;
	ScanIterator* si;
	int** dat;
	int* datp;
	int npts;
	int nextline();
	int nextline(int* dput);
	int* getpositions();
	void replacedata();
	void replacedata(int* d);
	int getoffset();
};

#endif