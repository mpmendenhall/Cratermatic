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

#include "Classify.hh"
#include "Image.hh"
#include "Histogram.hh"
#include "Utils.hh"

void ClassifyImage::underlyingstats(Image* foo)
{
	if(underlying == foo) return; //already done this!
	if(stats.size() != pic.size()) calcstats();
	underlying = foo;
	
	for(int i=0; i<pic.size(); i++) {
		
		stats[i].basinmin = FLT_MAX;
		stats[i].minloc = 0;
		stats[i].basinmax = -FLT_MAX;
		stats[i].boundsmin=FLT_MAX;
		stats[i].zsum = 0;
		stats[i].zzsum = 0;
		stats[i].boundsavg=0;
		
		float z;
		
		
		for(int j=0; j<pic[i].size(); j++) {
			z = foo->data[pic[i][j]];
			stats[i].zsum += z;
			stats[i].zzsum += z*z;
			if(foo->data[pic[i][j]]<stats[i].basinmin) {
				stats[i].basinmin=foo->data[pic[i][j]];
				stats[i].minloc = pic[i][j];
			}
			if(foo->data[pic[i][j]]>stats[i].basinmax) stats[i].basinmax=foo->data[pic[i][j]];
		}
		
		if(hasboundaries)
		{
			for(int j=0; j<bounds[i].size(); j++) {
				if(foo->data[bounds[i][j]] < stats[i].boundsmin) stats[i].boundsmin = foo->data[bounds[i][j]];
				stats[i].boundsavg += foo->data[bounds[i][j]];
			}
			stats[i].boundsavg /= bounds[i].size();
	
		}
	}
}


void ClassifyImage::underlyingavg(Image* u){ //calculate region average over underlying image
	if(stats.size() != pic.size()) calcstats();
	if(underlying != u) underlyingstats(u);
	for(int i=0; i<pic.size(); i++) {
		stats[i].temp = stats[i].zsum/pic[i].size();
	}
}

void ClassifyImage::underlyingmin(Image* u){ //calculate region average over underlying image
	if(stats.size() != pic.size()) calcstats();
	if(underlying != u) underlyingstats(u);
	settempstat(3);
}

void ClassifyImage::underlyingmax(Image* u){ //calculate region average over underlying image
	if(stats.size() != pic.size()) calcstats();
	if(underlying != u) underlyingstats(u);
	settempstat(4);
}


int floatcompare(const void* a, const void* b) { //for qsort by average z
	float at = *(float*)a;
	float bt = *(float*)b;
	return (int)(at > bt) - (int)(at < bt);
};

void ClassifyImage::underlyingmedian(Image* u){ //calculate region median over underlying image
	if(stats.size() != pic.size()) calcstats();
	for(int i=0; i<pic.size(); i++) {
		float* ur = (float*)malloc(pic[i].size()*sizeof(float));
		for(int j=0; j<pic[i].size(); j++) ur[j]=u->data[pic[i][j]];
		qsort(ur,pic[i].size(),sizeof(float),floatcompare);
		stats[i].temp = ur[pic[i].size()/2];
		free(ur);
	}
}

void ClassifyImage::underlyingRadialCorrelation(Image* u)
{
	if(stats.size() != pic.size()) calcstats();
	float x0,y0,x1,y1;
	int q;
	float* r;
	float* s;
	float a,b,corr;
	
	for(int i=0; i<pic.size(); i++)
	{
		r = (float*)malloc(pic[i].size()*sizeof(float));
		s = (float*)malloc(pic[i].size()*sizeof(float));
		x0 = ((float)stats[i].xsum)/((float)pic[i].size());
		y0 = ((float)stats[i].ysum)/((float)pic[i].size());
		
		for(int p=0; p<pic[i].size(); p++)
		{
			q=pic[i][p];
			x1 = (float)(q%width) - x0;
			y1 = (float)(q/width) - y0;
			r[p] = sqrt(x1*x1+y1*y1);
			s[p] = u->data[q];
		}
		
		CratersBaseObject::lsrl(r, s, NULL, pic[i].size(), &a, &b, &corr);
		stats[i].temp = corr;
		free(r); free(s);
		//printf(" >> %i %i %g\n",i,pic[i].size(),corr);
	}
}

void ClassifyImage::normalizebasins(Image* foo) {
	printf("Normalizing image by basin...\n");
	underlyingstats(foo);
	for(int i=0; i<pic.size(); i++) {
		for(int j=0; j<pic[i].size(); j++) {
			foo->data[pic[i][j]] = (foo->data[pic[i][j]]-stats[i].basinmin)/(stats[i].basinmax - stats[i].basinmin);
			if(foo->data[pic[i][j]]>1) foo->data[pic[i][j]]=1;
		}
	}
}

Histogram* ClassifyImage::regionhisto(Image* I, Image* wt, unsigned int n, float mn, float mx, int nbins)
{
	if(n>=pic.size()) return NULL;
	
	float* d = (float*)malloc(pic[n].size()*sizeof(float));
	for(int i=0; i<pic[n].size(); i++) d[i]=I->data[pic[n][i]];
	float* w=NULL;
	if(wt){
		w = (float*)malloc(pic[n].size()*sizeof(float));
		for(int i=0; i<pic[n].size(); i++) w[i]=wt->data[pic[n][i]];
	}
	
	if(mn==0 && mx==0) //autorange
	{
		underlyingstats(I);
		mn = stats[n].basinmin;
		mx = stats[n].basinmax;
	}
	
	if(nbins==0)
	{
		if(pic[n].size()<49) nbins = (int)sqrt((float)pic[n].size());
		else if(pic[n].size() < 140) nbins = pic[n].size()/7;
		else nbins = 20;
	}
	
	Histogram* foo = new Histogram(d,pic[n].size(),w,mn,mx,nbins);
	return foo;
}

Image* ClassifyImage::dataToImage()
{
	Image* foo = new Image((RectRegion*)this);
	for(int i=0; i<size; i++) foo->data[i] = data[i];
	return foo;
};

Image* ClassifyImage::lowBitsToImage(int nbits)
{
	int bitm = (1<<nbits)-1;
	Image* foo = new Image((RectRegion*)this);
	for(int i=0; i<size; i++) foo->data[i] = (data[i] & bitm);
	return foo;
};

Image* ClassifyImage::recolorize(int base, int modkey) {
	Image* foo = new Image(width,height);
	for(int i=0; i<size; i++) foo->data[i] = ((base*(data[i]/base))%modkey);
	return foo;
};

Image* ClassifyImage::recolorize(float* c) {
	Image* foo = new Image(width,height);
	for(int i=0; i<size; i++) foo->data[i]= c[data[i] >> shift];
	return foo;
};

Image* ClassifyImage::scatterColor(unsigned int nbits)
{
	Image* foo = new Image((RectRegion*)this);
	int* sn = CratersBaseObject::scatterNumber(nbits);
	for(int i=0; i<size; i++) foo->data[i] = (float)sn[data[i]%(1<<nbits)];
	free(sn);
	return foo;
};

Image* ClassifyImage::boundaryimage() {
	Image* foo = new Image(width,height);
	if(hasboundaries)
	{
		for(int i=0; i<pic.size(); i++){
			for(int j=0; j<bounds[i].size(); j++) {
				foo->data[bounds[i][j]]= 1.0;
			}
		}
	} else {
		for(int i=0; i<size; i++)
		{
			int baseown = data[i] >> shift;
			for(int k=0; k<connectn; k++)
			{
				if(k==4) continue;
				int dx = connectdx[k];
				int dy = connectdy[k];
				if(!inrange(i+dx+width*dy)) continue;
				if(data[i+dx+width*dy] >> shift != baseown) { foo->data[i]=1.0; break; }
			}
		}
	}
	return foo;
};

Image* ClassifyImage::tempstatimg() {
	if(stats.size() != pic.size()) calcstats();
	Image* foo = new Image(this);
	for(int i=0; i<pic.size(); i++){
		for(int j=0; j<pic[i].size(); j++) {
			foo->data[pic[i][j]] = stats[i].temp;
		}
	}
	return foo;
}

Image* ClassifyImage::fourboundaryimage()
{
	Image* foo = boundaryimage();
	for(int j=0; j<size; j++)
	{
		if(!foo->data[j]) continue;
		foo->data[j]=0;
		int x = j%width;
		int y = j/width;
		for(int i=0; i<4; i++) //only consider 4-connected boundary
		{
			int dx = connectdx[i];
			int dy = connectdy[i];
			if(x+dx<0 || x+dx>=width || y+dy<0 || y+dy>=height) continue; //off edge
			if(data[x+dx+width*(y+dy)] >> shift == data[x+width*y] >> shift) continue; //in same region
			foo->data[j] = 1;
			break;
		}
	}
	return foo;
}

//extract basin n chunk from img
Image* ClassifyImage::extractChunk(unsigned int n, Image* img, int l){
	if(n>=pic.size()) return new Image(0,0);
	
	BoundingBox bb = findboundingbox(pic[n].data(), pic[n].size());
	bb = expandbb(bb,l);
	int w=1+bb.ux-bb.lx; int h=1+bb.uy-bb.ly;
	Image* foo = new Image(w, h);
	for(int x=0; x<w; x++) {
		for(int y=0; y<h; y++) {
			foo->data[x+w*y] = img->data[x+bb.lx+width*(y+bb.ly)];
		}
	}
	
	foo->name = "Classification region "+to_str(n);
	
	return foo;
}

Image* ClassifyImage::extractChunk(unsigned int n, Image* img){
	return extractChunk(n,img,0);
}

//extract basin n chunk from img
Image* ClassifyImage::extractMaskedChunk(unsigned int n, Image* img){
	if(n>=pic.size()) return new Image(0,0);
	if(stats.size() != pic.size()) calcstats();
	BoundingBox bb = findboundingbox(pic[n].data(), pic[n].size());
	int w=1+bb.ux-bb.lx; int h=1+bb.uy-bb.ly;
	
	float zmax=img->data[pic[n][0]];
	for(int i=0; i<pic[n].size(); i++)
	{
		if(img->data[pic[n][i]] > zmax) zmax  = img->data[pic[n][i]];
	}
	
	Image* foo = new Image(w, h);
	for(int x=0; x<w; x++) {
		for(int y=0; y<h; y++) {
			if(data[x+bb.lx+width*(y+bb.ly)] >> shift == n) foo->data[x+w*y]=img->data[x+bb.lx+width*(y+bb.ly)];
			else foo->data[x+w*y] = zmax;
		}
	}
	
	foo->name = "Classification region "+to_str(n);
	
	return foo;
}

Image* ClassifyImage::extractChunkMask(unsigned int n, int l){
	if(n>=pic.size()) return new Image(0,0);
	
	BoundingBox bb = findboundingbox(pic[n].data(), pic[n].size());
	bb=expandbb(bb,l);
	int w=1+bb.ux-bb.lx; int h=1+bb.uy-bb.ly;
	Image* foo = new Image(w, h);
	for(int i=0; i<pic[n].size(); i++) {
		int p = pic[n][i];
		int nx = p%width-bb.lx;
		int ny = p/width-bb.ly;
		foo->data[nx+w*ny]=1;
	}
	
	foo->name = "Classification region "+to_str(n)+" mask";
	
	return foo;
}

void ClassifyImage::cutoutChunkMask(Image* msk, unsigned int n)
{
	if(n>=pic.size()) return;
	for(int i=0; i<pic[n].size(); i++) msk->data[pic[n][i]] = 0.0;
}

Image* ClassifyImage::extractChunkMask(unsigned int n)
{
	return extractChunkMask(n, 0);
}


