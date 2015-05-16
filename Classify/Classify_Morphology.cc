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
#include <cassert>

// connectivity:
//
// 426	16	4	64
// 0 1	1		2
// 537	32	8	128
//

bool* determinesimple()
{
	bool* s = (bool*)malloc(256*sizeof(bool));
	for(int i=0; i<256; i++)
	{
		
		s[i]=false;
		
		int n=0; //number of pts;
		for(int k=0; k<8; k++) n+=((i>>k) & 1);
		
		if(!n) {continue;} //test foreground nonempty
		if(!((~i) & 15)) {continue;} //test 4-connected background nonempty
		
		if(n==1) {s[i]=true; continue;}
		
		//test foreground connectivity
		if((!(i & 3)) && (i & 84) && (i & 168)) {continue;}
		if((!(i & 12)) && (i & 49) && (i & 194)) {continue;}
		if((!(i & 5)) && (i & 16)) {continue;}
		if((!(i & 6)) && (i & 64)) {continue;}
		if((!(i & 9)) && (i & 32)) {continue;}
		if((!(i & 10)) && (i & 128)) {continue;}
		
		s[i]=true;
	}
	return s;
}

// connectivity:
//
// 426	16	4	64
// 0 1	1		2
// 537	32	8	128
//

bool* determinenoprune()
{
	bool* s = determinesimple();
	for(int i=0; i<256; i++)
	{
		
		if(!s[i]) continue;
		s[i]=false;
		
		if(!(i & 1+16+4+64+2)) continue;
		if(!(i & 4+64+2+128+8)) continue;
		if(!(i & 2+128+8+32+1)) continue;
		if(!(i & 8+32+1+16+4)) continue;
		
		s[i]=true;
	}
	return s;
}

const bool* ClassifyImage::is8simple = determinesimple();

Image* ClassifyImage::simplepic()
{
	Image* b = new Image(97,97);
	
	for(int i=0; i<256; i++)
	{
		int x0 = 3+6*(i%16);
		int y0 = 3+6*(i/16);
		
		for(int dx=-3; dx<=3; dx++) {
			for(int dy=-3; dy<=3; dy++) {
				if(dx*dx == 9 || dy*dy == 9) { b->data[x0+dx+97*(y0+dy)] = 1.0; continue; }
				if(dx*dx == 4 || dy*dy == 4) { b->data[x0+dx+97*(y0+dy)] = 0.7 - 0.4*(is8simple[i]); continue; }
			}
		}
		
		for(int q=0; q<9; q++)
		{
			if(q==4) continue;
			int dx = connectdx[q];
			int dy = connectdy[q];
			b->data[x0+dx+97*(y0+dy)] = 1.0;
			if(q<4 && (i & 1<<q)) b->data[x0+dx+97*(y0+dy)]=0;
			if(q>4 && (i & 1<<(q-1))) b->data[x0+dx+97*(y0+dy)]=0;
		}
		
	}
	return b;
}


ClassifyImage* ClassifyImage::fromzeros(Image* I){
	ClassifyImage* foo = new ClassifyImage((RectRegion*)I);
	foo->shift = 1;
	for(int i=0; i<I->size; i++) foo->data[i] = (int)(I->data[i] == 0);
	foo->findObjectsByLowBits(1);
	return foo;
}


ClassifyImage* ClassifyImage::fromCurvature(Image* I) {
	ClassifyImage* combined = new ClassifyImage((RectRegion*)I);
	Image* ctdx = I->deriv(true); Image* ctdxdx = ctdx->deriv(true); delete(ctdx);
	Image* ctdy = I->deriv(false); Image* ctdydy = ctdy->deriv(false); delete(ctdy);
	Image* ctddy = I->diagDeriv(false); Image* ctddydy = ctddy->diagDeriv(false); delete(ctddy);
	Image* ctddx = I->diagDeriv(true); Image* ctddxdx = ctddx->diagDeriv(true); delete(ctddx);
	for(int i=0; i<I->size; i++) if(ctdxdx->data[i]>0 && ctdydy->data[i]>0 && ctddxdx->data[i]>0 && ctddydy->data[i]>0) combined->data[i] = 0x1;
	delete(ctdxdx); delete(ctdydy); delete(ctddxdx); delete(ctddydy);
	combined->shift = 1;
	return combined;
}


ClassifyImage* ClassifyImage::dat_and(ClassifyImage* b)
{
	for(int i=0; i<size; i++) data[i] &= b->data[i];
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::dat_or(ClassifyImage* b)
{
	for(int i=0; i<size; i++) data[i] |= b->data[i];
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::dat_xor(ClassifyImage* b)
{
	for(int i=0; i<size; i++) data[i] ^= b->data[i];
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::dat_and(int q)
{
	for(int i=0; i<size; i++) data[i] &= q;
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::dat_or(int q)
{
	for(int i=0; i<size; i++) data[i] |= q;
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::dat_xor(int q)
{
	for(int i=0; i<size; i++) data[i] ^= q;
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::getPoints(int* d, unsigned int n, unsigned int pad) {
	BoundingBox bb = findboundingbox(d,n);
	ClassifyImage* bi = new ClassifyImage(bb.ux-bb.lx + 2*pad + 1, bb.uy-bb.ly + 2*pad + 1);
	bi->shift = shift;
	for(int i=0; i<n; i++) {
		int p = d[i];
		int nx = p%width - bb.lx + pad;
		int ny = p/width - bb.ly + pad;
		bi->data[nx+(bi->width)*ny] = data[p];
	}
	return bi;
}

ClassifyImage* ClassifyImage::getObject(unsigned int n, unsigned int pad) {
    assert(n < pic.size());
	return getPoints(pic[n].data(), pic[n].size(), pad);
}

Image* ClassifyImage::getImageObject(Image* u, unsigned int n, unsigned int pad)
{
	if(n>=pic.size()) return NULL;
	BoundingBox bb = findboundingbox(pic[n].data(),pic[n].size());
	int w = bb.ux-bb.lx+2*pad+1;
	int h = bb.uy-bb.ly+2*pad+1;
	Image* bi = new Image(w,h);
	for(int i=0; i<w*h; i++) {
		if(inrange(bb.lx-pad+i%w,bb.ly-pad+i/w)) bi->data[i] = u->data[bb.lx-pad+i%w + (bb.ly-pad+i/w)*width];
	}
	return bi;
}

Image* ClassifyImage::getImageMaskObject(unsigned int n, unsigned int pad)
{
	if(n>=pic.size()) return NULL;
	BoundingBox bb = findboundingbox(pic[n].data(),pic[n].size());
	Image* bi = new Image(bb.ux-bb.lx+2*pad+1,bb.uy-bb.ly+2*pad+1);
	for(int i=0; i<pic[n].size(); i++) {
		int p = pic[n][i];
		int nx = p%width - bb.lx + pad;
		int ny = p/width - bb.ly + pad;
		bi->data[nx+(bi->width)*ny] = 1.0;
	}
	return bi;
}

void ClassifyImage::radialization(unsigned int n, Image* u)
{
	if(n>=pic.size()) return;
	int p0 = stats[n].minloc;
	int* rn = (int*)calloc(100,sizeof(int));
	float* rs = (float*)calloc(100,sizeof(float));
	
	int r;
	for(int i=0; i<pic[n].size(); i++)
	{
		r = (int)(2*sqrt((float)dist2(p0,pic[n][i])));
		if(r>=100) continue;
		rn[r]++;
		rs[r] += u->data[pic[n][i]];
	}
	
	for(int i=0; i<pic[n].size(); i++)
	{
		r = (int)(2*sqrt((float)dist2(p0,pic[n][i])));
		if(r>=100) continue;
		u->data[pic[n][i]] = rs[r]/rn[r];
	}
	
	//take derivative
	float* radii = (float*)calloc(100,sizeof(float));
	float* drv = (float*)calloc(100,sizeof(float));
	int npts = 0;
	for(int i=0; i<100; i++)
	{
		if(rn[i]*rn[i+1] == 0) continue;
		radii[npts] = (i + 0.5)/2.0;
		drv[npts++] = rs[i+1]/rn[i+1] - rs[i]/rn[i];
	}
	
	//find "radius"
	int imx = 0;
	float dmx = 0;
	for(int i=0; i<npts; i++)
	{
		if(drv[i] >= dmx) { dmx = drv[i]; imx = i; }
		else if(drv[i] < dmx) break;
	}
	u->addmark(ImageMark::MARK_CIRCLE, p0%width,p0/width,(int)radii[imx]);
	
	for(int i=imx+1; i<npts; i++)
	{
		if(drv[i] <= dmx) { dmx = drv[i]; imx = i; }
		else if(drv[i] > dmx) break;
	}
	u->addmark(ImageMark::MARK_CIRCLE, p0%width,p0/width,(int)radii[imx]);
	

	
	free(rn);
	free(rs);
}

ClassifyImage* ClassifyImage::putObject(unsigned int n, ClassifyImage* bi, unsigned int pad)
{
	BoundingBox bb = findboundingbox(pic[n].data(),pic[n].size());
	for(int i=0; i<bi->size; i++) {
		int nx = bb.lx + i%bi->width - pad;
		int ny = bb.ly + i/bi->width - pad;
		if(!inrange(nx,ny)) continue;
		if(bi->data[i] & 1<<shift) data[nx+width*ny] = (n<<shift) + (bi->data[i] & ((1 << shift) -1));
	}
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::fillHoles(int flagmark) {
    printf("Filling holes in %zu regions with mark %i... ", pic.size(), flagmark); fflush(stdout);
	for(int b = 1; b < pic.size(); b++) {
		ClassifyImage* foo = getObject(b,1);
        //printf("%zu/%i\t", pic[b].size(), foo->size); fflush(stdout);
		foo->findObjectsByHigherBits(shift);
        assert(foo->pic.size() >= 2);
		for(auto it = foo->pic[0].begin(); it != foo->pic[0].end(); it++) foo->data[*it] = 0;
		for(auto it = foo->pic[1].begin(); it != foo->pic[1].end(); it++) foo->data[*it] = (1<<shift) | (foo->data[*it] & ((1<<shift)-1));
		for(int j=2; j<foo->pic.size(); j++) for(auto it = foo->pic[j].begin(); it != foo->pic[j].end(); it++) foo->data[*it] = (1<<shift) | flagmark;
		putObject(b,foo,1);
		delete(foo);
	}
	isclassified = false;
    printf(" Done.\n");
	return this;
}

ClassifyImage* ClassifyImage::removeSmall(int s) {
	if(!isclassified) findObjectsByLowBits(1);
	for(int i=0; i<pic.size(); i++)
	{
		if(data[pic[i][0]] & 0x1 && pic[i].size()<s) xorRegion(i,0x1);
	}
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::constrainSize(int s1, int s2)
{
	for(int i=1; i<pic.size(); i++)
	{
		if(pic[i].size()<s1 || pic[i].size()>s2) andRegion(i,0x0);
	}
	isclassified = false;
	return this;
}

ClassifyImage* ClassifyImage::removeUncircular(float th)
{
	for(int i=1; i<pic.size(); i++)
	{
		Circle c = findboundingcirc(pic[i].data(), pic[i].size());
		if(pic[i].size() < 3.14159*th*c.r*c.r) andRegion(i,0x0);
	}
	isclassified = false;
	return this;
}

Image* ClassifyImage::neatopic(int n)
{
	//Image* foo = new Image(n,n);
	//bool* b = RectRegion::gennocds(n);
	//for(int i=0; i<n*n; i++) foo->data[i]=b[i];
	//free(b);
	return NULL;
}

/* int ClassifyImage::connectivity(RectRegion* R, int* d, unsigned int n) { //find the connectivity number (# of holes) in region
	BoundingBox bb = R->findboundingbox(d,n);
	ClassifyImage* bi = new ClassifyImage(bb.ux-bb.lx+3,bb.uy-bb.ly+3);
	for(int i=0; i<n; i++) {
		int p = d[i];
		int nx = p%R->width - bb.lx + 1;
		int ny = p/R->width - bb.ly + 1;
		bi->bdata[nx+(bi->width)*ny] = true;
	}
	bi->findobjects();
	int cn = bi->pic.size() - 2; //additional objects are holes
	delete(bi);
	return cn;
}

ClassifyImage* ClassifyImage::halfplanedilation(int x, int y)
{
	ClassifyImage* foo = copy();
	int* d;
	ScanIterator* si = new ScanIterator((RectRegion*)this,x,y,&d);
	
	int n;
	while(n = si->nextline())
	{
		bool databool = false;
		for(int x=0; x<n; x++)
		{
			if(!databool) databool = foo->bdata[d[x]];
			foo->bdata[d[x]]=databool;
		}
	}
	
	delete(si);
	return foo;
} */


/*
 
int* ClassifyImage::lindil(int l, int* d, int w, int bitn) { //overwrite a line with its dilation on specified bit
	int c = 0;
	int q = 1<<bitn;
	for(int n=0; n<w; n++){
		if(d[n] & q) {c=l-1; continue;}
		if(c) {d[n] |= q; --c;}
	}
	return d;
}

int* ClassifyImage::linero(int l, bool* d, int w, int bitn) { //overwrite a line with its erosion on specified bit
	int c = 0;
	int q = 1<<bitn;
	for(int n=0; n<w; n++){
		if(!(d[n] & q)) {c=l-1; continue;}
		if(c) {d[n] &= (~q); --c;}
	}
	return d;
}

ClassifyImage* ClassifyImage::linedilation(int x, int y, int l, int bitn)
{
	bool* bp;
	BinaryDataScanner bds(this,x,y,&bp);
	int n;
	while(n = bds.nextline()) lindil(l,bp,n,bitn);
	return this;
}

ClassifyImage* ClassifyImage::lineclosing(int x, int y, int l)
{
	bool* bp;
	BinaryDataScanner bds(this,x,y,&bp);
	int n;
	while(n = bds.nextline()) {
		binlindil(l,bp,n);
		binlinero(l,bp,n);
	}
	return this;
}

ClassifyImage* ClassifyImage::lineerosion(int x, int y, int l)
{
	bool* bp;
	BinaryDataScanner bds(this,x,y,&bp);
	int n;
	while(n = bds.nextline()) binlinero(l,bp,n);
	return this;
}

ClassifyImage* ClassifyImage::lineopening(int x, int y, int l)
{
	bool* bp;
	BinaryDataScanner bds(this,x,y,&bp);
	int n;
	while(n = bds.nextline()) {
		binlinero(l,bp,n);
		binlindil(l,bp,n);
	}
	return this;
}

ClassifyImage* ClassifyImage::slineopening(int x, int y, int l)
{
	bool* bp;
	BinaryDataScanner bds(this,x,y,&bp);
	BinaryDataScanner bds2(this,-x,-y,&bp);
	BinaryDataScanner bds3(this,x,y,&bp);
	BinaryDataScanner bds4(this,-x,-y,&bp);
	int n;
	while(n = bds.nextline()) binlinero(l/2,bp,n);
	while(n = bds2.nextline()) binlinero(l-l/2,bp,n);
	while(n = bds3.nextline()) binlindil(l/2,bp,n);
	while(n = bds4.nextline()) binlindil(l-l/2,bp,n);
	return this;
}

ClassifyImage* ClassifyImage::slineclosing(int x, int y, int l)
{
	bool* bp;
	BinaryDataScanner bds(this,x,y,&bp);
	BinaryDataScanner bds2(this,-x,-y,&bp);
	BinaryDataScanner bds3(this,x,y,&bp);
	BinaryDataScanner bds4(this,-x,-y,&bp);
	int n;
	while(n = bds.nextline()) binlindil(l/2,bp,n);
	while(n = bds2.nextline()) binlindil(l-l/2,bp,n);
	while(n = bds3.nextline()) binlinero(l/2,bp,n);
	while(n = bds4.nextline()) binlinero(l-l/2,bp,n);
	return this;
}

ClassifyImage* ClassifyImage::srectclosing(int x, int y, int lx, int ly)
{
	bool* bp;
	BinaryDataScanner bds1(this,x,y,&bp);
	BinaryDataScanner bds2(this,-x,-y,&bp);
	BinaryDataScanner bds3(this,y,-x,&bp);
	BinaryDataScanner bds4(this,-y,x,&bp);
	BinaryDataScanner bds5(this,x,y,&bp);
	BinaryDataScanner bds6(this,-x,-y,&bp);
	BinaryDataScanner bds7(this,y,-x,&bp);
	BinaryDataScanner bds8(this,-y,x,&bp);
	int n;
	while(n = bds1.nextline()) binlindil(lx/2,bp,n);
	while(n = bds2.nextline()) binlindil(lx-lx/2,bp,n);
	while(n = bds3.nextline()) binlindil(ly/2,bp,n);
	while(n = bds4.nextline()) binlindil(ly-ly/2,bp,n);
	
	while(n = bds5.nextline()) binlinero(lx/2,bp,n);
	while(n = bds6.nextline()) binlinero(lx-lx/2,bp,n);
	while(n = bds7.nextline()) binlinero(ly/2,bp,n);
	while(n = bds8.nextline()) binlinero(ly-ly/2,bp,n);
	return this;
}

ClassifyImage* ClassifyImage::srectopening(int x, int y, int lx, int ly)
{
	bool* bp;
	BinaryDataScanner bds1(this,x,y,&bp);
	BinaryDataScanner bds2(this,-x,-y,&bp);
	BinaryDataScanner bds3(this,y,-x,&bp);
	BinaryDataScanner bds4(this,-y,x,&bp);
	BinaryDataScanner bds5(this,x,y,&bp);
	BinaryDataScanner bds6(this,-x,-y,&bp);
	BinaryDataScanner bds7(this,y,-x,&bp);
	BinaryDataScanner bds8(this,-y,x,&bp);
	int n;
	while(n = bds5.nextline()) binlinero(lx/2,bp,n);
	while(n = bds6.nextline()) binlinero(lx-lx/2,bp,n);
	while(n = bds7.nextline()) binlinero(ly/2,bp,n);
	while(n = bds8.nextline()) binlinero(ly-ly/2,bp,n);
	
	while(n = bds1.nextline()) binlindil(lx/2,bp,n);
	while(n = bds2.nextline()) binlindil(lx-lx/2,bp,n);
	while(n = bds3.nextline()) binlindil(ly/2,bp,n);
	while(n = bds4.nextline()) binlindil(ly-ly/2,bp,n);
	return this;
}

ClassifyImage* ClassifyImage::homotopicthin(ClassifyImage* mask, bool prune)
{
	bool* exdat = (bool*)calloc((width+2)*(height+2),sizeof(bool));
	for(int y=0; y<height; y++) memcpy(exdat+1+(width+2)*(y+1),bdata+width*y,width*sizeof(bool));
	
	const bool* patterns;
	if(prune) patterns = is8simple;
	else patterns = determinenoprune();
	
	unsigned char cinv[] = {1,0,3,2,0,7,6,5,4};
	unsigned char* connec = (unsigned char*)calloc((width+2)*(height+2),sizeof(unsigned char));
	int dx,dy;
	
	for(int x=1; x<width+1; x++) {
		for(int y=1; y<height+1; y++) {
			if(!exdat[x+(width+2)*y]) continue;
			for(int q=0; q<9; q++) {
				if(q==4) continue;
				dx = connectdx[q];
				dy = connectdy[q];
				connec[x+connectdx[q]+(width+2)*(y+connectdy[q])] += (1 << cinv[q]);
			}
		}
	}
	
	if(mask)
	{
		for(int x=1; x<width+1; x++) {
			for(int y=1; y<height+1; y++) {
				if(mask->bdata[x-1+width*(y-1)]) exdat[x+(width+2)*y] = false;
			}
		}
		
	}
	
	int nthinned = 1;
	while(nthinned)
	{
		nthinned=0;
		for(int x=1; x<width+1; x++) {
			for(int y=1; y<height+1; y++) {
				if(!exdat[x+(width+2)*y] || (!patterns[connec[x+(width+2)*y]] && connec[x+(width+2)*y] != 0) ) continue;
				exdat[x+(width+2)*y] = false;
				nthinned++;
				for(int q=0; q<9; q++) {
					if(q==4) continue;
					connec[x+connectdx[q]+(width+2)*(y+connectdy[q])] -= (1 << cinv[q]);
				}
			}
		}
	}
	
	if(mask)
	{
		for(int x=1; x<width+1; x++) {
			for(int y=1; y<height+1; y++) {
				if(mask->bdata[x-1+width*(y-1)]) exdat[x+(width+2)*y] = true;
			}
		}
		
	}
	
	for(int y=0; y<height; y++) memcpy(bdata+width*y,exdat+1+(width+2)*(y+1),width*sizeof(bool));
	delete(exdat); delete(connec);
	return this;
}

ClassifyImage* ClassifyImage::convexhull(int n)
{
	if(n>15) n=15;
	if(n<=0) return copy();
	ClassifyImage* foo = NULL;
	ClassifyImage* l;
	ClassifyImage* r;
	for(int i=-n; i<=n; i++) {
		for(int j=0; j<=n; j++) {
			if(j==0 && i>=0) continue;
			if(!RectRegion::nocomdivs[abs(i)+40*abs(j)]) continue;
			l = halfplanedilation(i,j);
			r = halfplanedilation(-i,-j);
			for(int p=0; p<size; p++) r->bdata[p] = r->bdata[p] || l->bdata[p];
			if(!foo) foo = r->copy();
			else for(int p=0; p<size; p++) foo->bdata[p] = r->bdata[p] && foo->bdata[p];
			delete(l);
			delete(r);
		}
	}
	foo->findobjects();
	return foo;
}

int* ClassifyImage::whichregion(ClassifyImage* w)
{
	//count craters
	int ncrat = 0;
	for(int i=0; i<pic.size(); i++) ncrat += bdata[pic[i][0]];
	
	int* foo = (int*)malloc(ncrat*sizeof(int));
	ncrat=0;
	for(int i=0; i<pic.size(); i++)
	{
		if(!bdata[pic[i][0]]) continue;
		foo[ncrat] = (int)w->data[pic[i][0]];
		for(int j=0; j<pic[i].size(); j++)
		{
			if(foo[ncrat] != (int)w->data[pic[i][j]])
			{
				foo[ncrat]=-1;
				break;
			}
		}
		ncrat++;
	}
	return foo;
};

int ClassifyImage::writecat(char* cname) //append crater records to a catalog
{
	FILE* of = fopen(cname,"a");
	printf("Appending craters to %s...\n",cname);
	int ncr=0;
	for(int i=0; i<pic.size(); i++)
	{
		if(!bdata[pic[i][0]]) continue; //black region
		Circle c = findboundingcirc(pic[i],pic[i].size());
		printf("\tCrater found! %i @ (%i,%i)\n",(int)reallength(c.r),(int)realx(c.x),(int)realy(c.y));
		fprintf(of,"%i\t%i\t%i\t0.0\t0.0\n",(int)reallength(c.r),(int)realy(c.y+0.5),(int)realx(c.x+0.5));
		ncr++;
	}
	fclose(of);
	printf("Done.\n");
	return ncr;
}

ClassifyImage* ClassifyImage::generaldilation(ClassifyImage* k)
{
	ClassifyImage* foo  = new ClassifyImage((RectRegion*)this);
	int x,y;
	for(int i=0; i<size; i++)
	{
		if(!bdata[i]) continue;
		x=i%width; y=i/width;	
		for(int p=-(k->width/2); p<k->width-(k->width/2); p++)
		{
			if(x+p<0 || x+p>=width) continue;
			for(int q=-(k->height/2); q<k->height-(k->height/2); q++)
			{
				if(y+q<0 || y+q>=height) continue;
				if(k->bdata[p+k->width/2+(k->width)*(q+k->height/2)]) foo->bdata[x+p+width*(y+q)]=true;
			}
		}
	}
	return foo;
};

ClassifyImage* ClassifyImage::boundarydilation(ClassifyImage* k)
{
	ClassifyImage* foo  = new ClassifyImage((RectRegion*)this);
	ClassifyImage* bnds = boundaries();
	int x,y;
	for(int i=0; i<size; i++)
	{
		if(!bdata[i]) continue;
		foo->bdata[i]=true;
		if(!bnds->bdata[i]) continue;
		
		x=i%width; y=i/width;	
		for(int p=-(k->width/2); p<k->width-(k->width/2); p++)
		{
			if(x+p<0 || x+p>=width) continue;
			for(int q=-(k->height/2); q<k->height-(k->height/2); q++)
			{
				if(y+q<0 || y+q>=height) continue;
				if(k->bdata[p+k->width/2+(k->width)*(q+k->height/2)]) foo->bdata[x+p+width*(y+q)]=true;
			}
		}
	}
	delete(bnds);
	return foo;
};

ClassifyImage* ClassifyImage::filledcircleimage(int r)
{
	ClassifyImage* foo = new ClassifyImage(2*r+1,2*r+1);
	for(int x=0; x<2*r+1; x++)
	{
		for(int y=0; y<2*r+1; y++)
		{
			if((x-r)*(x-r)+(y-r)*(y-r) <= (r+0.5)*(r+0.5)) foo->bdata[x+(2*r+1)*y]=true;
		}
	}
	return foo;
};

ClassifyImage* ClassifyImage::circledilation(int r)
{
	ClassifyImage* k = filledcircleimage(r);
	ClassifyImage* foo = boundarydilation(k);
	delete(k);
	foo->copyfromrr(this);
	sprintf(foo->name,"%s Dilation",name);
	return foo;
}

ClassifyImage* ClassifyImage::circleerosion(int r)
{
	ClassifyImage* k = filledcircleimage(r);
	complement();
	ClassifyImage* foo = boundarydilation(k);
	complement();
	foo->complement();
	delete(k);
	foo->copyfromrr(this);
	sprintf(foo->name,"%s Erosion",name);
	return foo;
}

ClassifyImage* ClassifyImage::circleclosing(int r)
{
	ClassifyImage* d = circledilation(r);
	ClassifyImage* e = d->circleerosion(r);
	delete(d);
	e->copyfromrr(this);
	sprintf(e->name,"%s Closed",name);
	return e;
}

ClassifyImage* ClassifyImage::circleopening(int r)
{
	ClassifyImage* d = circleerosion(r);
	ClassifyImage* e = d->circledilation(r);
	delete(d);
	e->copyfromrr(this);
	sprintf(e->name,"%s Opened",name);
	return e;
}

*/