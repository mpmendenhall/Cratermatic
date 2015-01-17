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

#include "RectRegion.hh"
#include "Utils.hh"
#include "Image.hh"
#include <climits>

int comparebyradius(const void* entry1, const void* entry2) {
	return  (int)( (*(CatalogEntry**)entry2)->radius - (*(CatalogEntry**)entry1)->radius );
};

bool* RectRegion::gennocds(int n) //table of nonnegative integer pairs with no common divisors
{
	bool* b = (bool*)malloc(n*n*sizeof(bool));
	for(int i=0; i<n*n; i++) b[i]=true;
	
	for(int j=2; j<n; j++)
	{
		for(int k=0; k<n; k+=j)
		{
			for(int l=0; l<n; l+=j) b[k+n*l]=false;
		}
	}
	return b;
}

CraterCatalog::CraterCatalog(const string& infile) {
	FILE* ifp = fopen(infile.c_str(),"r");
	char* buf = (char*)malloc(1024*sizeof(char));
	ncraters=0;
	entries = (CatalogEntry**)malloc(sizeof(CatalogEntry*));
	while(fgets(buf,300,ifp) && strlen(buf)>10) {
		ncraters++;
		entries = (CatalogEntry**)realloc(entries,ncraters*sizeof(CatalogEntry*));
		entries[ncraters-1]=new CatalogEntry;
		sscanf(buf,"%i %i %i %*f %*f\n",&(entries[ncraters-1]->radius),&(entries[ncraters-1]->centery),&(entries[ncraters-1]->centerx));
	}
	free(buf);
	fclose(ifp);
	qsort(entries,ncraters,sizeof(CatalogEntry*),comparebyradius);
}

CraterCatalog::~CraterCatalog() {
	for(int i=0; i<ncraters; i++) free(entries[i]);
	free(entries);
}


bool* RectRegion::nocomdivs = RectRegion::gennocds(40); //static member

int* makedx()
{
	int* connectdx = (int*)malloc(13*sizeof(int));
	connectdx[0]=-1; connectdx[1]=1; connectdx[2]=0; connectdx[3]=0;
	connectdx[4]=0;
	connectdx[5]=-1; connectdx[6]=-1; connectdx[7]=1; connectdx[8]=1;
	connectdx[9]=0; connectdx[10]=2; connectdx[11]=0; connectdx[12]=-2;
	return connectdx;
}

int* makedy()
{
	int* connectdy = (int*)malloc(13*sizeof(int));
	connectdy[0]=0; connectdy[1]=0; connectdy[2]=-1; connectdy[3]=1;
	connectdy[4]=0;
	connectdy[5]=-1; connectdy[6]=1; connectdy[7]=-1; connectdy[8]=1;
	connectdy[9]=2; connectdy[10]=0; connectdy[11]=-2; connectdy[12]=0;
	return connectdy;
}

int* RectRegion::connectdx = makedx();
int* RectRegion::connectdy = makedy();

RectRegion::RectRegion(int w,int h)
{
	isaName = "RectRegion";
	isaNum = COBJ_RECTREGION;
	
	width=w;
	height=h;
	size=w*h;
	marks = NULL;
	nmarks = 0;
	mycatalog = NULL;
	
	coords.lx = 0; coords.ux = width-1;
	coords.ly = 0; coords.uy = height-1;
	
	//define connectedness
	connectn=9;

	connectr2 = (int*)malloc(13*sizeof(int));
	
	connectr2[0]=1; connectr2[1]=1; connectr2[2]=1; connectr2[3]=1;
	connectr2[4]=0;
	connectr2[5]=2; connectr2[6]=2; connectr2[7]=2; connectr2[8]=2;
	connectr2[9]=9; connectr2[10]=9; connectr2[11]=9; connectr2[12]=9;
}

RectRegion::~RectRegion() {
	if(marks) {free(marks); marks=NULL;}
	free(connectr2);
	if(mycatalog) {free(mycatalog); mycatalog=NULL;}
}

void RectRegion::copyfromrr(RectRegion* R)
{
	nmarks=R->nmarks;
	if(nmarks) {
		marks = (ImageMark*)malloc(nmarks*sizeof(ImageMark));
		for(int i=0; i<nmarks; i++) marks[i]=R->marks[i];
	}
	width = R->width;
	height = R->height;
	size = width*height;
	coords = R->coords;
	name = R->name;
}

bool RectRegion::inrange(int p)
{
	if(p<0 || p>=size) return false;
	return true;
}
bool RectRegion::inrange(int x, int y)
{
	if(x<0 || y<0 || x>=width || y>=height) return false;
	return true;
}

void RectRegion::addmark(int t, int x, int y, int r){
	nmarks++;
	if(!marks) marks=(ImageMark*)malloc(sizeof(ImageMark));
	else marks=(ImageMark*)realloc(marks,nmarks*sizeof(ImageMark));
	marks[nmarks-1].type=t;
	marks[nmarks-1].x0=x;
	marks[nmarks-1].y0=y;
	marks[nmarks-1].r=r;
}

void RectRegion::addmarkline(float x0, float y0, float x1, float y1)
{
	nmarks++;
	if(!marks) marks=(ImageMark*)malloc(sizeof(ImageMark));
	else marks=(ImageMark*)realloc(marks,nmarks*sizeof(ImageMark));
	marks[nmarks-1].type=3;
	marks[nmarks-1].x0=x0;
	marks[nmarks-1].y0=y0;
	marks[nmarks-1].x1=x1;
	marks[nmarks-1].y1=y1;
}

void RectRegion::fouriermark(float x0, float y0, float* xs, float* ys, unsigned int nterms, unsigned int ndivisions)
{
	float angl = atan2(ys[nterms],xs[nterms])/nterms;
	float r = invRadialFourier(angl,xs,ys,nterms);
	float angl2;
	float r2;
	for(int j=0; j<2*nterms*ndivisions; j++)
	{
		angl2 = angl + M_PI/(nterms*ndivisions);
		r2 = invRadialFourier(angl2,xs,ys,nterms);
		addmarkline(x0+r2*cos(angl2),y0+r2*sin(angl2),x0+r*cos(angl),y0+r*sin(angl));
		angl = angl2;
		r=r2;
	}
}

void RectRegion::clearmarks() //clear all marks from image
{
	if(marks) {free(marks); marks=NULL; nmarks=0;}
}

float RectRegion::reallength(float l)
{
	return l*(coords.ux-coords.lx)/((float)width-1.0);
}

float RectRegion::realdx(float l)
{
	return l*(coords.ux-coords.lx)/(width - 1.0);
}

float RectRegion::realdy(float l)
{
	return l*(coords.uy-coords.ly)/(height - 1.0);
}

float RectRegion::realx(float l)
{
	return coords.lx + l*(coords.ux-coords.lx)/(width - 1.0);
}

float RectRegion::realy(float l)
{
	return coords.ly + l*(coords.uy-coords.ly)/(height - 1.0);
}

BoundingBox RectRegion::findboundingbox(int* p, int n)
{
	//find bounding box
	BoundingBox b;
	b.lx=INT_MAX; b.ly=INT_MAX;
	b.ux=0; b.uy=0;
	for(int i=0; i<n; i++) {
		if(p[i]%width < b.lx) b.lx = p[i]%width;
		if(p[i]%width > b.ux) b.ux = p[i]%width;
		if(p[i]/width < b.ly) b.ly = p[i]/width;
		if(p[i]/width > b.uy) b.uy = p[i]/width;
	}
	return b;
}

//distance squared between two points
int RectRegion::dist2(int p, int q)
{
	int dx = (p%width)-(q%width);
	int dy = (p/width)-(q/width);
	return dx*dx+dy*dy;
}

void RectRegion::radialFourier(float x0, float y0, int* ps, unsigned int nps, float* wt, float** xs, float** ys, unsigned int nmoms)
{
	if(nmoms == 0) return;
	
	*xs = (float*)malloc(nmoms*sizeof(float));
	*ys = (float*)malloc(nmoms*sizeof(float));
	
	float* w = wt;
	if(!wt)
	{
		w = (float*)malloc(nps*sizeof(float));
		for(int i=0; i<nps; i++) w[i] = 1.0;
	}
	
	//calculate 0th order term
	float wsum = 0;
	for(int i=0; i<nps; i++) wsum += w[i];
	(*xs)[0] = wsum;
	(*ys)[0] = 0;
	
	//calculate higher order terms
	float xterm;
	float yterm;
	float angl;
	for(int t=1; t<nmoms; t++)
	{
		xterm = 0;
		yterm = 0;
		for(int i=0; i<nps; i++)
		{
			angl = atan2((ps[i]/width)-y0, (ps[i]%width)-x0);
			xterm += w[i]*cos(t*angl);
			yterm += w[i]*sin(t*angl);
		}
		(*xs)[t] = xterm;
		(*ys)[t] = yterm;
	}
	
	if(!wt) free(w);
}

void RectRegion::radialFourier(float x0, float y0, int* ps, unsigned int nps, Image* wt, float** xs, float** ys, unsigned int nmoms)
{
	float* w = (float*)malloc(nps*sizeof(float));
	for(int i=0; i<nps; i++) w[i] = wt->data[ps[i]];
	radialFourier(x0, y0, ps, nps, w, xs, ys, nmoms);
	free(w);
}

float RectRegion::invRadialFourier(float angl, float* xs, float* ys, unsigned int nmoms)
{
	float r0 = sqrt(xs[0]/M_PI);
	float rexp = r0;
	for(int t=1; t<nmoms; t++)
	{
		rexp += (xs[t]*cos(t*angl) + ys[t]*sin(t*angl))/(M_PI*r0);
	}
	return rexp;
}

float RectRegion::xcenter(int* pts, unsigned int npts, float* wt)
{
	float accum = 0;
	float w = 0;
	if(wt) {
		for(int i=0; i<npts; i++) { accum += wt[i] * (pts[i]%width); w+=wt[i]; }
		return accum / w;
	} else {
		for(int i=0; i<npts; i++) accum += pts[i]%width;
		return accum/npts;
	}
	
	return (float)accum/npts;
}

float RectRegion::ycenter(int* pts, unsigned int npts, float* wt)
{
	float accum = 0;
	float w = 0;
	if(wt) {
		for(int i=0; i<npts; i++) { accum += wt[i] * (pts[i]/width); w+=wt[i]; }
		return accum / w;
	} else {
		for(int i=0; i<npts; i++) accum += pts[i]/width;
		return accum/npts;
	}
	
	return (float)accum/npts;
}

int RectRegion::fourierPoints(float x0, float y0, float* xs, float* ys, int nterms, int** pout) //return list of points enclosed by Fourier boundaries
{
	//find maximum radius and bounding box
	float rmax = sqrt(xs[0]/M_PI);
	for(int i=1; i<nterms; i++) rmax += sqrt(xs[i]*xs[i]+ys[i]*ys[i]);
	BoundingBox bb;
	bb.lx = (int)std::max((float)0., x0-rmax-2);
	bb.ly = (int)std::max((float)0., y0-rmax-2);
	bb.ux = (int)std::min((float)width,x0+rmax+2);
	bb.uy = (int)std::min((float)height,y0+rmax+2);
	
	*pout = (int*)malloc((bb.ux-bb.lx+1)*(bb.uy-bb.ly+1)*sizeof(int));
	int npts = 0;
	
	float delx, dely;
	float rexp, ract;
	float angl;
	for(int x=bb.lx; x<=bb.ux; x++)
	{
		for(int y=bb.ly; y<=bb.uy; y++)
		{
			delx = x - x0;
			dely = y - y0;
			angl = atan2(dely,delx);
			ract = sqrt(delx*delx+dely*dely);
			rexp = invRadialFourier(angl,xs,ys,nterms);
			if(ract<=rexp) (*pout)[npts++] = x + width*y;
		}
	}
	
	return npts;
}

void RectRegion::fourierDeviations(float x0, float y0, int* pts, unsigned int npts, float* xs, float* ys, float** ds, int nterms)
{
	(*ds) = (float*)malloc(nterms*sizeof(float));
	
	BoundingBox bb = findboundingbox(pts,npts);
	
	float delx, dely;
	float rexp, ract;
	float angl;
	
	for(int k=1; k<=nterms; k++)
	{
		float nwrongsided = 0;
		for(int x=bb.lx; x<=bb.ux; x++)
		{
			for(int y=bb.ly; y<=bb.uy; y++)
			{
				if(!inrange(x,y)) continue;
				delx = x - x0;
				dely = y - y0;
				angl = atan2(dely,delx);
				ract = sqrt(delx*delx+dely*dely);
				rexp = invRadialFourier(angl,xs,ys,k);
				if(ract<rexp) nwrongsided += (ract-rexp)*(ract-rexp);
			}
		}
		
		for(int i=0; i<npts; i++)
		{
			delx = pts[i]%width - x0;
			dely = pts[i]/width - y0;
			angl = atan2(dely,delx);
			ract = sqrt(delx*delx+dely*dely);
			rexp = invRadialFourier(angl,xs,ys,k);
			if(ract<rexp) nwrongsided -= (ract-rexp)*(ract-rexp);
			else nwrongsided += (ract-rexp)*(ract-rexp);
		}
		
		(*ds)[k-1] = sqrt(nwrongsided)/npts;
	}
}


Circle RectRegion::findboundingcirc(int* p, unsigned int n)
{
	Circle c;
	c.r = -1;
	
	//center-of-mass
	c.x=0;
	c.y=0;
	for(int i=0; i<n; i++) {
		c.x += p[i]%width;
		c.y += p[i]/width;
	}
	c.x /= n;
	c.y /= n;
	
	//find radius from center-of-mass
	float r=0;
	for(int i=0; i<n; i++) {
		float x = (float)(p[i]%width);
		float y = (float)(p[i]/width);
		float d = (x-c.x)*(x-c.x)+(y-c.y)*(y-c.y);
		if(d > r) r=d;
	}
	
	c.r = sqrt(r);
	return c;
}

BoundingBox RectRegion::expandbb(BoundingBox b, int l)
{
	if(b.lx-l>0) b.lx-=l; else b.lx=0;
	if(b.ly-l>0) b.ly-=l; else b.ly=0;
	if(b.ux+l<width) b.ux+=l; else b.ux = width-1;
	if(b.uy+l<height) b.uy+=l; else b.uy = height-1;
	return b;
}

void RectRegion::loadcatalog(const string& f){
	mycatalog = new CraterCatalog(f);
	for(int i=0; i<mycatalog->ncraters; i++) addmark(1,(int)mycatalog->entries[i]->centerx,(int)mycatalog->entries[i]->centery,(int)mycatalog->entries[i]->radius);
}

ScanIterator::ScanIterator(RectRegion* R, int xa, int ya, int** dd)
{
	steep = abs(ya)>abs(xa);
	if(steep) {
		if(ya>=0) {w = R->height; h = R->width; x=ya; y=-xa; flipx = false;}
		else {w = R->height; h = R->width; x=-ya; y=xa; flipx = true;}
	}
	else {
		if(xa>=0) {w=R->width; h=R->height; x=xa; y=ya; flipx = false;}
		else {w=R->width; h=R->height; x=-xa; y=-ya; flipx = true;}
	}
	
	buildout = y<0;
	d=dd;
	(*d) = (int*)malloc(w*sizeof(int));
	ys = (int*)malloc(w*sizeof(int));
	bps = (int*)malloc(w*sizeof(int));
	
	LineIterator l(0,0,x,y);
	for(int i=0; i<w; i++) {ys[i]=l.py(); l.step();}
	
	//determine in/out entry points
	int k=0;
	if(buildout)
	{
		for(int i=1; i<w; i++)
		{
			if(ys[i]<ys[i-1]) bps[k++]=i;
		}
		bps[k] = -1;
	} else {
		for(int i=1; i<w; i++)
		{
			if(ys[i]>ys[i-1]) bps[k++]=i;
		}
		bps[k] = -1;
	}
	
	if(buildout) y0 = 0;
	else y0 = -k;
}

ScanIterator::~ScanIterator()
{
	free(ys); ys=NULL;
	free(*d); d=NULL;
}

int ScanIterator::getoffset()
{
	return offset;
}

int ScanIterator::nextline()
{
	if(buildout) {
		if(y0 < h) x0=0; 
		else {
			x0 = bps[y0-h];
			if(x0==-1) return 0;
		}
	} else {
		if(y0<0) x0=bps[-y0-1];
		else x0=0;
		if(y0>=h) return 0;
	}
	
	offset  = x0;
	
	int npts = 0;
	int rx,ry;
	while(x0<w && y0+ys[x0] >= 0 && y0+ys[x0] < h)
	{
		if(!flipx) { rx=x0; ry=y0+ys[x0]; }
		else {rx = w-1-x0; ry = h-1-y0-ys[x0]; }
		
		if(!steep) (*d)[npts] = rx+w*ry;
		else (*d)[npts] = (h-1-ry)+h*rx;
		
		++npts;
		++x0;
	}
	
	++y0;
	return(npts);
}

