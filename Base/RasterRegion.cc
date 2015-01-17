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

#include "RasterRegion.hh"
#include "Image.hh"
#include "Classify.hh"
#include "Utils.hh"

RasterRegion::RasterRegion()
{
	init();
}

RasterRegion::RasterRegion(RectRegion* R, int x, int y)
{
	init();
	size = R->size;
	origwidth = R->width;
	origheight = R->height;
	myx = x;
	myy = y;
	widths = (int*)malloc((R->width+R->height) * sizeof(int));
	offsets = (int*)malloc((R->width+R->height) * sizeof(int));
}

void RasterRegion::init()
{
	sprintf(isaName,"RasterRegion");
	isaNum = COBJ_RASTERREGION;
	widths = NULL;
	offsets = NULL;
	height = 0;
	size=0;
	maxwidth=0;
	maxtotal=0;
	myx = 1;
	myy = 0;
	origwidth=0;
	origheight=0;
	data = NULL;
	cdata = NULL;
	lines = NULL;
	clines = NULL;
}

RasterRegion::~RasterRegion()
{
	if(widths) free(widths);
	if(offsets) free(offsets);
	if(data) free(data);
	if(cdata) free(cdata);
	if(lines) free(lines);
	if(clines) free(clines);
	widths = NULL;
	offsets = NULL;
	data = NULL;
	cdata = NULL;
	lines = NULL;
	clines = NULL;
}




RasterRegion* RasterRegion::scanFromImage(Image* I, int x, int y)
{
	RasterRegion* R = new RasterRegion((RectRegion*)I,x,y); 
	R->data = (float*)malloc(I->size * sizeof(float));
	R->lines = (float**)malloc((I->width+I->height) * sizeof(float*));
	
	float* d;
	ImageDataScanner ids(I,x,y,&d);
	int n;
	int ntot=0;
	R->maxwidth = 0;
	
	while(n=ids.nextline(R->data+ntot)) {
		
		if(n>R->maxwidth) R->maxwidth = n;
		
		R->lines[R->height] = R->data+ntot;
		R->widths[R->height] = n;
		R->offsets[R->height] = ids.getoffset();
		
		ntot+=n;
		R->height++;
	}
	
	R->widths = (int*)realloc(R->widths,R->height*sizeof(int));
	R->offsets = (int*)realloc(R->offsets,R->height*sizeof(int));
	R->lines = (float**)realloc(R->lines,R->height*sizeof(float*));
	R->correctoffsets();
	
	return R;
}

RasterRegion* RasterRegion::scanFromClassify(ClassifyImage* C, int x, int y)
{
	RasterRegion* R = new RasterRegion((RectRegion*)C,x,y); 
	R->cdata = (int*)malloc(C->size * sizeof(int));
	R->clines = (int**)malloc((C->width+C->height) * sizeof(int*));
	
	int* d;
	ClassifyDataScanner cds(C,x,y,&d);
	int n;
	int ntot=0;
	R->maxwidth = 0;
	
	while(n=cds.nextline(R->cdata+ntot)) {
		
		if(n>R->maxwidth) R->maxwidth = n;
		
		R->clines[R->height] = R->cdata+ntot;
		R->widths[R->height] = n;
		R->offsets[R->height] = cds.getoffset();
		
		ntot+=n;
		R->height++;
	}
	
	R->widths = (int*)realloc(R->widths,R->height*sizeof(int));
	R->offsets = (int*)realloc(R->offsets,R->height*sizeof(int));
	R->clines = (int**)realloc(R->clines,R->height*sizeof(int*));
	R->correctoffsets();
	return R;
}

RasterRegion* RasterRegion::updateImage(Image* I)
{
	float* d;
	ImageDataScanner ids(I,myx,myy,&d);
	int ntot=0;
	int n;
	while(n=ids.nextline(data+ntot)) ntot += n;
	return this;
}

RasterRegion* RasterRegion::updateClassify(ClassifyImage* C)
{
	int* d;
	ScanIterator si((RectRegion*)C, myx, myy, &d);
	int ntot=0;
	int n;
	while(n=si.nextline()) 
	{
		for(int i=0; i<n; i++) cdata[ntot+i] = C->data[d[i]];
		ntot += n;
	}
	return this;
}

void RasterRegion::correctoffsets()
{	
	int x = myx; int y = myy;
	bool flipx = x<0;
	bool flipy = y<0;
	x=abs(x); y=abs(y);
	bool swp = x<y;
	LineIterator* li;
	if(swp) {int t=x; x=y; y=t;}
	if(swp ^ flipx ^ flipy) li = new LineIterator(0,0,x*x+y*y,-x*y);
	else li = new LineIterator(0,0,x*x+y*y,x*y);
	int offsetmin = INT_MAX;
	maxtotal = -INT_MAX;
	for(int i=0; i<height; i++)
	{
		offsets[i] += li->py();
		li->step();
		if(offsets[i]<offsetmin) offsetmin = offsets[i];
		if(offsets[i]+widths[i] > maxtotal) maxtotal = offsets[i]+widths[i];
	}
	delete(li);

	for(int i=0; i<height; i++) offsets[i]-=offsetmin;
	maxtotal -= offsetmin;	
}

Image* RasterRegion::putIntoImage(Image* I)
{
	if(!I) I = new Image(origwidth, origheight);
	
	float* d;
	ImageDataScanner ids(I,myx,myy,&d);
	int n;
	int ntot=0;
	
	while(n=ids.nextline()) {
		ids.replacedata(data + ntot);
		ntot+=n;
	}
	
	return I;
}

ClassifyImage* RasterRegion::putIntoClassify(ClassifyImage* C)
{
	if(!C) C = new ClassifyImage(origwidth,origheight);
	int* d;
	ScanIterator si(C,myx,myy,&d);
	int n;
	int ntot=0;
	
	while(n=si.nextline()) {
		for(int i=0; i<n; i++) C->data[d[i]] = cdata[ntot+i];
		ntot+=n;
	}
	
	return C;
}

void RasterRegion::initcdata()
{
	if(!widths) return;
	
	if(cdata) cdata = (int*)realloc(cdata,size * sizeof(int));
	else cdata = (int*)calloc(size, sizeof(int));
		
	if(clines) clines = (int**)realloc(clines,height*sizeof(int*));
	else clines = (int**)calloc(height,sizeof(int*));
		
	clines[0]=cdata;
	for(int i=1; i<height; i++) clines[i] = clines[i-1]+widths[i-1];
}

Image* RasterRegion::makeImage()
{
	Image* I = new Image(maxtotal,height);
	for(int h=0; h<height; h++) {
		for(int x=0; x<widths[h]; x++) I->data[x+offsets[h]+maxtotal*h] = lines[h][x];
	}
	return I;
}

ClassifyImage* RasterRegion::makeClassifyImage()
{
	ClassifyImage* C = new ClassifyImage(maxtotal,height);
	for(int h=0; h<height; h++) {
		for(int x=0; x<widths[h]; x++) C->data[x+offsets[h]+maxtotal*h] = clines[h][x];
	}
	return C;
}

RasterRegion* RasterRegion::cLineMerge(float ol, int andkey, int xorkey, int modkey)
{
	int* segs1 = (int*)malloc(maxwidth*sizeof(int));
	int* segs2 = (int*)malloc(maxwidth*sizeof(int));
	int* tip;
	int nsegs1=0;
	int nsegs2=0;
	
	int cc;
	int nuclaim = 0;
	for(int h=0; h<height; h++)
	{
		cc=clines[h][0]/modkey;
		segs2[0] = offsets[h];
		nsegs2 = 1;
		
		//classify line
		for(int i=0; i<widths[h]; i++)
		{
			if(clines[h][i]/modkey == cc) { clines[h][i] = nuclaim + clines[h][i]%modkey; continue; } //same as previous point
			else{
				segs2[nsegs2++] = i+offsets[h];
				cc = clines[h][i]/modkey;
				nuclaim += modkey; 
				clines[h][i] = nuclaim + clines[h][i]%modkey;
			}
		}
		
		//compare segments
		int i=0; int j=0;
		bool wasmerged;
		
		while(i<nsegs1-1 && j<nsegs2-1)
		{
			if(segs1[i] >= segs2[j+1])
			{
				++j;
				continue;
			}
			
			if(segs2[j] > segs1[i+1])
			{
				++i;
				continue;
			}
			
			int oldclass = clines[h-1][segs1[i]-offsets[h-1]];
			int newclass = clines[h][segs2[j]-offsets[h]];
			wasmerged=false;
			
			while(((newclass ^ xorkey) & andkey) && ((oldclass ^ xorkey) & andkey))
			{
				int l = min(segs1[i+1],segs2[j+1]) - max(segs1[i],segs2[j]);
				int sl1 = segs1[i+1]-segs1[i];
				int sl2 = segs2[j+1]-segs2[j];
				if(sl1>6 || sl2>6) { if(sl1*ol > l || sl2*ol > l) break; }//not overlapping enough
				else if(sl1 >= 2*l || sl2 >= 2*l) break; //small regions not overlapping enough
				oldclass = modkey*(oldclass/modkey); //strip off extra modifiers
				for(int k = segs2[j]; k<segs2[j+1]; k++) {
					clines[h][k-offsets[h]] = oldclass + clines[h][k-offsets[h]]%modkey; //do the merge
					clines[h][k-offsets[h]] |= 0xF0000000; //mark as merged
				}
				for(int k = segs1[i]; k<segs1[i+1]; k++) clines[h-1][k-offsets[h-1]] |= 0xF0000000; //mark as merged				
				wasmerged = true;
				break;
			}
			
			if(wasmerged)
			{
				++j;
				++i;
				continue;
			}
			
			if(segs1[i+1] > segs2[j+1])
			{
				++j; continue;
			} else {
				++i; continue;
			}
			
		}
		
		//swap segments list
		tip = segs1; segs1=segs2; segs2 = tip;
		nsegs1 = nsegs2;
	}
	
	//clear unmerged segments
	for(int k = 0; k<size; k++) {
		if(cdata[k] & 0xF0000000) cdata[k] ^= 0xF0000000;
		else cdata[k] = xorkey;
	}
	
	free(segs1);
	free(segs2);
	return this;
}

RasterRegion* RasterRegion::dx()
{
	float* d = (float*)malloc(maxwidth*sizeof(float));
	for(int h=0; h<height; h++)
	{
		for(int i=0; i<widths[h]; i++) d[i]=lines[h][i];
		for(int i=1; i<widths[h]-1; i++) lines[h][i] = 0.5*(d[i+1]-d[i-1]);
		lines[h][0]=0; lines[h][widths[h]-1]=0;
	}
	free(d);
	return this;
}

void RasterRegion::linearCraterMerger(int* cd, int npts, int ncls, int* ul)
{
	int* lcg = (int*)malloc(ncls*sizeof(int));
	int* rcg = (int*)malloc(ncls*sizeof(int));
	int* typ = (int*)calloc(ncls,sizeof(int));
	int* sum = (int*)calloc(ncls,sizeof(int));
	int* npic = (int*)calloc(ncls,sizeof(int));

	for(int i=0; i<ncls; i++) { lcg[i]=-1; rcg[i]=-1; }
	
	for(int i=0; i<npts; i++) 
	{
		++npic[cd[i]/0x100];
		if(ul[i] & 0x02) sum[cd[i]/0x100] += 1; //overlaps with Up region
		typ[cd[i]/0x100] = cd[i] & 0xFF;
		if(i<npts-1 && cd[i]/0x100 != cd[i+1]/0x100)
		{
			rcg[cd[i]/0x100] = cd[i+1]/0x100;
			lcg[cd[i+1]/0x100] = cd[i]/0x100;
		}
	}
	
	//do merging
	int nmerged = 1;
	while(nmerged)
	{
		nmerged = 0;
		for(int i=0; i<ncls; i++)
		{
			if(!(typ[i] & 0x02)) continue; //not a local maximum
			if(rcg[i]==-1 || lcg[i]==-1) continue; //missing neighbors (e.g. edge)
			if(sum[i]>0) continue; //overlaps with Up region

			nmerged++;
			typ[i] = 0x101 + (lcg[i]*0x100);
			typ[rcg[i]] = 0x101 + (lcg[i]*0x100);
			sum[lcg[i]]+=sum[i]+sum[rcg[i]];
			npic[lcg[i]] += npic[i]+npic[rcg[i]];
			if(rcg[rcg[i]] != -1) lcg[rcg[rcg[i]]] = lcg[i];
			rcg[lcg[i]] = rcg[rcg[i]];
			
		}
		
		for(int i=0; i<ncls; i++)
		{
			if(!(typ[i] & 0x01)) continue; //not a local minimum
			if(rcg[i]==-1 || lcg[i]==-1) continue; //missing neighbors (e.g. edge)
			if(sum[i]<npic[i]) continue; //not completely whited over
			
			nmerged++;
			typ[i] = 0x102 + (lcg[i]*0x100);
			typ[rcg[i]] = 0x102 + (lcg[i]*0x100);
			sum[lcg[i]] += sum[i]+sum[rcg[i]];
			npic[lcg[i]] += npic[i]+npic[rcg[i]];
			if(rcg[rcg[i]] != -1) lcg[rcg[rcg[i]]] = lcg[i];
			rcg[lcg[i]] = rcg[rcg[i]];
		}
		
	}
	
	//re-mark merged segments
	for(int i=0; i<npts; i++)
	{
		if(typ[cd[i]/0x100] >= 0x100) cd[i] = typ[cd[i]/0x100] - 0x100;
	}
	
	free(typ);
	free(lcg); free(rcg);
	free(sum); free(npic);
}


void linePairer(float* d, int* c, int n, int np)
{
	int* edges = (int*)calloc(2*np+1,sizeof(int));
	float matchlvl = 8.0;
	
	//find region edges
	np = 1;
	for(int i=1; i<n; i++) if(c[i] != c[i-1]) edges[np++]=i;
	edges[np] = n;
	int nmatches = 1;
	
	//clear claims
	for(int i=0; i<n; i++) c[i] = 0; //c[i]%0x100;
	
	//look for Down,Up matching pairs
	float h0;
	for(int r=0; r<np-1; r++)
	{
		h0 = d[edges[r]];
		if(h0 >= 0) continue;
		for(int s = r+1; s<np-1; s++)
		{
			if(d[edges[s]] < h0) break; //oops! another big edge in same direction
			if(d[edges[s]] < -h0/matchlvl) continue; //too small
			if(d[edges[s]] > -h0*matchlvl) break; //too big
		
			//just right?
			for(int i = edges[r]; i<edges[r+1]; i++) c[i] = 0x100*nmatches + 0x02 + 0x01;
			for(int i = edges[s]; i<edges[s+1]; i++) c[i] = 0x100*nmatches + 0x04 + 0x01;
			for(int i = edges[r+1]; i<edges[s]; i++) c[i] = 0x100*nmatches + 0x00 + 0x01;
		
			nmatches++;
			r=s;
			break;
		}
	}
	
	free(edges);
}

int samesignsegclass(float* d, int* c, int n, int peakn)
{
	int np=0;
	int* peakpos = (int*)malloc(n*sizeof(int));
	bool sgn = d[0] >= 0;
	
	//initial peaks count
	for(int i=1; i<n-1; i++) if( (d[i] < d[i-1] xor sgn) && (d[i] <= d[i+1] xor sgn) ) peakpos[np++] = i;

	if(!np)
	{ //no appropriate extrema in segment
		for(int i=0; i<n; i++) d[i]=0;
		free(peakpos);
		return 0;
	} 
	
	if(np>1)
	{
		//find lowest relative pt and split region
		float reldepth = 1.1;
		int lowestrel;
		for(int p=0; p<np-1; p++)
		{
			float ht;
			if(sgn) ht=max(d[peakpos[p]],d[peakpos[p+1]]);
			else ht=min(d[peakpos[p]],d[peakpos[p+1]]);
			
			for(int q=peakpos[p]; q<peakpos[p+1]; q++)
			{
				if(d[q]/ht < reldepth) { reldepth = d[q]/ht; lowestrel = q; }
			}
		}
		
		if(reldepth<0.9)
		{
			np = samesignsegclass(d,c,lowestrel,peakn);
			np += samesignsegclass(d+lowestrel,c+lowestrel,n-lowestrel,peakn+np);
			free(peakpos);
			return np;
		}
		
		//divide is too shallow to worry about; just go from highest peak
		for(int p=1; p<np; p++) if( (d[peakpos[p]] < d[peakpos[0]]) xor sgn ) peakpos[0]=peakpos[p];
	}
	
	//else, claim the 'SINGLE' peak and continuate.
	float pheight = d[peakpos[0]];
	int ctyp;
	if(sgn) ctyp = 0x02; else ctyp = 0x04;
	float accum = 0;
	float naccum = 0;
	
	for(int q=peakpos[0]; q>=0; q--)
	{
		if( d[q] > 0.02*pheight xor sgn ) break;
		c[q] = peakn*0x100 + ctyp;
		accum += d[q]; naccum++;
	}
	for(int q=peakpos[0]+1; q<n; q++) //claim back
	{
		if( d[q] > 0.02*pheight xor sgn ) break;
		c[q] = peakn*0x100 + ctyp;
		accum += d[q]; naccum++;
	}
	
	for(int i=0; i<n; i++) 
	{
		if(c[i]) d[i] = accum/naccum;
		else d[i]=0;
	}
	
	//posion edges
	if(sgn) d[n-1]=0;
	else d[0]=0;
	
	free(peakpos);
	return 1;
}

RasterRegion* RasterRegion::peakSegmenter()
{
	initcdata();
	
	for(int h=0; h<height; h++)
	{
		int n = widths[h];
		float* dx = lines[h];
		int* claimer = clines[h];
		for(int i=0; i<n; i++) claimer[i]=0;
		
		// find zero crossings and CLAIMIZE!!!
		int peakn = 0;
		int lastp = 0;
		bool sgn = dx[0] >= 0;
		
		for(int i=0; i<n; i++)
		{
			if((dx[i] >= 0 xor sgn) || i == n-1) //just crossed 0!
			{
				peakn += samesignsegclass(dx+lastp,claimer+lastp,i-lastp,peakn); //do claimation
				sgn = !sgn;
				lastp = i;
			}
		}
	}
	
	return this;
}

RasterRegion* RasterRegion::midPointer() //divide into UPWARD and DOWNWARD regions at inflection pts
{
	initcdata();
	dx();
	
	for(int h=0; h<height; h++)
	{
		int n = widths[h];
		float* dx = lines[h];
		int* claimer = clines[h];
		for(int i=0; i<n; i++) claimer[i]=0;
		int peakn = 0;
		int curvtyp = 0x00;
		
		for(int i=0; i<n; i++)
		{
			claimer[i] = peakn*0x100 + curvtyp;
			if(i>0 && i<n-1 && (dx[i] > dx[i-1] xor dx[i] < dx[i+1]))
			{
				if(dx[i] > dx[i-1]) curvtyp = 0x02; //local maximum => upcoming peak
				else curvtyp = 0x01; //local minimum => upcoming basin
				peakn++;
			}
		}
	}
	return this;
}


RasterRegion* RasterRegion::rimExpand()
{
	for(int h=0; h<height; h++)
	{
		int n = widths[h];
		float* dx = lines[h];
		int* claimer = clines[h];
		int startpt = 0;
		int x;
		int w;
		int startcrat;
		float startdx;
		
		while(startpt < n)
		{
			if(!(claimer[startpt] & 0x01)) {startpt++; continue;} //not in a crater
			
			startcrat = claimer[startpt]/0x100; //number of crater we are in
			
			//find segment width
			w=0;
			while(claimer[startpt+w]/0x100 == startcrat) w++;
				
			//scan left to rim edge
			x=startpt;
			while(x>1 && dx[x-1] <= dx[x]) x--; //make sure we are at least up to peak ... next goes back up
			startdx = dx[x];
			if(startpt - x < 3 && startdx < 0)
			{
				for(int q = x; q<=startpt; q++) if(claimer[q]/0x100 != startcrat) claimer[q] = 0x100*startcrat + 0x01 + 0x04;
				while(x>1 && dx[x-1] > dx[x] && dx[x-1]<0.2*startdx && !((claimer[x] & 0x01) && (claimer[x]/0x100 != startcrat))) //&& startpt-x < w
				{
					x--;
					if(claimer[x]/0x100 != startcrat) claimer[x] = 0x100*startcrat + 0x01 + 0x04;
				}
			}
			
			//move to other edge
			startpt += w;

			//scan right to rim edge
			x=startpt;
			while(x<n-1 && dx[x+1] >= dx[x]) x++; //make sure we are at least up to peak ... next goes back down
			startdx = dx[x];
			if(x-startpt < 3 && startdx > 0)
			{
				for(int q = startpt; q<=x; q++) if(claimer[q]/0x100 != startcrat) claimer[q] = 0x100*startcrat + 0x01 + 0x04;
				while(x<n-1 && dx[x+1] < dx[x] && dx[x+1]>0.2*startdx && !((claimer[x] & 0x01) && (claimer[x]/0x100 != startcrat)))
				{
					x++;
					if(claimer[x]/0x100 != startcrat) claimer[x] = 0x100*startcrat + 0x01 + 0x04;
				}
			}
			
			startpt = max(x+1,startpt);
		}
	}
	
	return this;
}


RasterRegion* RasterRegion::expandBy(RasterRegion* R)
{
	for(int h=0; h<height; h++)
	{
		int n = widths[h];
		int* c = clines[h];
		int* c2 = R->clines[h];
		int uc; int j;
		
		for(int i=1; i<n; i++)
		{
			if( (!(c[i-1] & 0x01) || (c[i-1] & 0x08)) && (c[i] & 0x01) && !(c[i] & 0x08) ) //just moved into original region
			{
				uc = c2[i];
				if(!(uc & 0x04)) continue; //not a downslope
				j=i-1;
				while(j>0 && j>i-3 && c2[j] == uc  && !(c[j] & 0x01 && !(c[j] & 0x08)))
				{
					c[j] = c[i] | 0x08;
					j--;
				}
			} 
			else if( (c[i-1] & 0x01) && (!(c[i] & 0x01) || (c[i] & 0x08))  && !(c[i-1] & 0x08) ) //just moved out of original region
			{
				uc = c2[i-1];
				if(!(uc & 0x02)) continue; //not an upslope
				j=i;
				while(j<n && j<i+3 && c2[j] == uc && !(c[j] & 0x01 && !(c[j] & 0x08)))
				{
					c[j] = c[i-1] | 0x08;
					j++;
				}
			}
		
		}
	
	}
	
	return this;
}





float* RasterRegion::convolve1d(float* d, int w, float* k, int s)
{ //return convolution of d and k
	float* f = (float*)calloc(w+s-1,sizeof(float));
	for(int i=0; i<w; i++)
	{
		for(int j=0; j<s; j++){
			f[i+j] += d[i]*k[j];
		}
	}
	return f;
}

RasterRegion* RasterRegion::orthoblur(float r)
{
	float* d;
	int w = (int)(2.5*r);
	OrthoIterator oi(this,&d,w);
	float* gk = RasterRegion::gausskernel(w,r);
	float* cnvd;
	int n;
	while(n=oi.nextline())
	{
		cnvd = RasterRegion::convolve1d(d,n,gk,2*w+1);
		oi.replacedata(cnvd + 2*w);
		free(cnvd);
	}
	free(gk);
	return this;
}

float* RasterRegion::gausskernel(int s, float r)
{
	float* d = (float*)malloc((2*s+1)*sizeof(float));
	float n=0;
	for(int x=-s; x<s+1; x++){
		d[x+s] = exp(-((float)(x*x))/(2.0*r*r));
		n+=d[x+s];
	}
	//normalize
	for(int i=0; i<2*s+1; i++) d[i]/=n;
	return d;
};

OrthoIterator::OrthoIterator(RasterRegion* RR, float** dd, int p)
{
	d=dd;
	pad = p;
	R = RR;
	(*d) = (float*)malloc((R->height+2*pad)*sizeof(float));
	x0 = 0;
	lasth = 0;
	npts=0;
}

OrthoIterator::~OrthoIterator()
{
	if(*d) free(*d);
}

int OrthoIterator::getoffset()
{
	return lasth;
}

int OrthoIterator::nextline()
{
	if(x0 >= R->maxtotal) return 0;
	
	while(lasth>0 && (x0 >= R->offsets[lasth-1] && x0 < R->offsets[lasth-1] + R->widths[lasth-1])) lasth--;
	while(lasth<R->height && (x0 < R->offsets[lasth] || x0 >= R->offsets[lasth] + R->widths[lasth])) lasth++;
	
	npts=0;
	int h = lasth;
	while(h<R->height && x0 >= R->offsets[h] && x0 < R->offsets[h] + R->widths[h])
	{
		(*d)[pad+npts] = R->lines[h][x0-R->offsets[h]];
		++npts;
		++h;
	}
	if(!npts) return 0;

	for(int i=0; i<pad; i++) { (*d)[pad-1-i] = (*d)[pad+i]; (*d)[2*pad+npts+i] = (*d)[2*pad+npts-1-i]; } //mirror padding
		  
	++x0;
	return(npts+2*pad);
}

void OrthoIterator::replacedata()
{
	for(int i=0; i<npts; i++)  R->lines[i+lasth][x0-R->offsets[i+lasth]] = (*d)[pad+i];
}

void OrthoIterator::replacedata(float* rdat)
{
	for(int i=0; i<npts; i++)  R->lines[i+lasth][x0-1-R->offsets[i+lasth]] = rdat[i];
}