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

#ifndef CRATERS_CLASSIFY
#define CRATERS_CLASSIFY

#include "RectRegion.hh"
#include <string>
using std::string;

class Image;
class RGBImage;
class Pointset;
class Histogram;

/// Sparse integer-valued matrix (used for connectivity graph)
class SparseInt {
public:
    /// Constructor
	SparseInt(int w, int h);
    /// Destructor
    ~SparseInt();

    /// get value at row,column
	int get(int,int);
    /// set value at row,column
	void set(int,int, int);
    
    /// get list of columns filled in specified row
	int columnlist(int row, int*& output);
    /// get list of column values in specified row
	int columnvals(int row, int*& output);
    /// display specified row
	void disprow(int row);
    
    int width;					///< array width
    int height;					///< array height
    std::vector<int>** rows;	///< columns assigned in each row
    std::vector<int>** rowdat;	///< value assigned to each row's columns
};

/// Characterization information for a crater basin
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

/// Integer-valued image for point classification
class ClassifyImage: public RectRegion {
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
	SparseInt* cg;			///< connectivity graph between basins
	bool hasconnectivity;	///< whether connectivity graph is calculated
	bool badimg;
	bool isclassified;		///< whether classification is complete
	
	BasinStat** stats;		///< statistics for each crater basin
	bool *markedregion;
	int* poi; 				///< "points of interest"
	int npoi;
	void addregiontopoi(int n);
	void markedregionstopoi();
	Image* markedregionstoimage();
	
    /// Constructor from RectRegion
	ClassifyImage(RectRegion* R);
    /// Constructor from width, height
	ClassifyImage(int w, int h);
    /// Destructor
	~ClassifyImage();
	
	void initialize();
	void renumerate();
	void renumerateWithKey(int andkey);
	void cleardata();
	
    /// create copy of this ClassifyImage
	ClassifyImage* copy();
    /// load ArcGIS text file format
	void loadarcgis(const string& ifname);
    /// write ArcGIS text file format
	void writeArcGIS(const string& ofname);
    /// write least significant bits to BMP image
	void writeLowBitBMP(const string& ofname);
    /// produce Image from nbits least significant bits
	Image* lowBitsToImage(int nbits);
    /// produce Image from classify data
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
	
    /// produce "colorized" image from ((base*(data[i]/base))%modkey)
	Image* recolorize(int base, int modkey);
	Image* recolorize(float* c);
	Image* scatterColor(unsigned int nbits);
	Image* boundaryimage();
	Image* tempstatimg();
    /// produce image of four-connected boundaries between regions
	Image* fourboundaryimage();
	Image* markimage();
    /// produce color "pretty image" visualization
	RGBImage* prettyImage();
    /// produce colored overlay on Image
	RGBImage* prettyoverlayimage(Image*);
    /// produce colored overlay on RGBImage
	RGBImage* prettyoverlayimage(RGBImage* C);
	RGBImage* colorbytemp();
	void labelboundaries(int c);
	
	Image* extractChunk(unsigned int n, Image* img, int l);
	Image* extractChunk(unsigned int n, Image* img);
	Image* extractMaskedChunk(unsigned int n, Image* img);
	Image* extractChunkMask(unsigned int n, int l);
	Image* extractChunkMask(unsigned int n);
	void cutoutChunkMask(Image* msk, unsigned int n);
    
    /// bitwise `and' data with other ClassifyImage
	ClassifyImage* dat_and(ClassifyImage* b);
    /// bitwise 'or' data with other ClassifyImage
	ClassifyImage* dat_or(ClassifyImage* b);
    /// bitwise 'xor' data with other ClassifyImage
	ClassifyImage* dat_xor(ClassifyImage* b);
    /// bitwise 'and' data with specified value
	ClassifyImage* dat_and(int q);
    /// bitwise 'or' data with specified value
	ClassifyImage* dat_or(int q);
    /// bitwise 'xor' data with specified value
	ClassifyImage* dat_xor(int q);
    /// xor cdata values for specified points
	void xorPoints(int*, unsigned int, unsigned int);
    /// xor cdata values in specified region
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
};

/// Line scanner for ClassifyImage
class ClassifyDataScanner {
public:
    /// Constructor
	ClassifyDataScanner(ClassifyImage*, int, int);
    
    /// load next line of data into *dat
	int nextline();
    /// load next line of data into dput
	int nextline(int* dput);
    /// get current line positions array
	int* getpositions() { return si.datp.data(); }
    /// replace modified line data back into ClassifyImage
	void replacedata();
    /// place line data in d into ClassifyImage
	void replacedata(int* d);
    /// get current line offset
	int getoffset() { return si.getoffset(); }
    
    ClassifyImage* myImg;	///< ClassifyImage being scanned
    ScanIterator si;		///< ScanIterator for selecting lines
    vector<int> dat;		///< data points on current line
};

#endif
