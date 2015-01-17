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

#include "craters_Image.h"

Image* Image::trimmed(int l, int t, int r, int b) //return an image with trimmed borders
{
	Image *foo = new Image(width-l-r, height-t-b);
	for(int y=t; y<height-b; y++) memcpy(&foo->data[(width-t-b)*(y-t)],&data[l+width*y],(width-t-b)*sizeof(float));
	return foo;
};


Image* Image::padded(int l, int t, int r, int b) //return an image padded on sides
{
	Image *foo = new Image(width+l+r, height+t+b);
	for(int y=0; y<height; y++) memcpy(&(foo->data[l+(width+l+r)*(y+t)]),&(data[width*y]),width*sizeof(float));
	return foo;
};


Image* Image::mirrorpadded(int r) //return an image padded on sides with mirroring
{
	//printf("Mirror padding image...\n");
	Image *foo = new Image(width+2*r, height+2*r);
	int xr,yr;
	for(int x=-r; x<width+r; x++) {
		for(int y=-r; y<height+r; y++){
			xr=x;
			yr=y;
			if(x<0) xr=-x;
			if(y<0) yr=-y;
			if(x>=width) xr=2*width-x-1;
			if(y>=height) yr=2*height-y-1;
			foo->data[x+r+(width+2*r)*(y+r)]=data[xr+width*yr];
		}
	}
	//printf("Done mirror padding.\n");
	return foo;
};


Image* Image::trim_inplace(int l, int t, int r, int b) //trim dimensions of this image
{
	float *newdata = (float*)malloc((width-l-r)*(height-t-b)*sizeof(float));
	for(int y=t; y<height-b; y++) memcpy(&newdata[(width-l-r)*(y-t)],&data[l+width*y],(width-l-r)*sizeof(float));
	free(data);
	data=newdata;
	width=width-l-r;
	height=height-t-b;
	size=width*height;
	return this;
};


Image* Image::rotate() //rotate the data array (transpose for square data)
{
	Image *foo = new Image(height, width);
	foo->copyfromrr((RectRegion*)this);
	foo->height=width; foo->width=height;
	for(int x=0; x<width; x++) {
		for(int y=0; y<height; y++) {
			foo->data[y+height*x]=data[x+width*y];
		}
	}
	return foo;
};

Image* Image::reduce() { //reduce each image dimension by factor of two
	Image* foo = lanczos2decimate(true);
	Image* bar = foo->lanczos2decimate(false);
	delete(foo);
	return bar;
}

Image* Image::xlinearscale(float z) {
	int nw = (int)(width*z);
	float s = ((float)nw)/((float)width);
	Image* foo = new Image(nw,height);
	int* lpix = (int*)malloc(nw*sizeof(int));
	float* lwt = (float*)malloc(nw*sizeof(float));
	for(int i=0; i<nw; i++) {
		lpix[i] = (int)(i/s);
		lwt[i] = i/s-(float)lpix[i];
		if(lpix[i] == width-1) {
			lpix[i]=width-2;
			lwt[i]=1;
		}
	}
	
	for(int y=0; y<height; y++) {
		for(int x=0; x<nw; x++) {
			foo->data[x+nw*y] = data[lpix[x]+width*y]*(1-lwt[x])+data[lpix[x]+1+width*y]*lwt[x];
		}
	}
	free(lpix);
	free(lwt);
	foo->copyfromrr((RectRegion*)this);
	foo->width = nw;
	foo->size = nw*height;
	return foo;
};

Image* Image::bilinearscale(float s) {
	Image* foo = xlinearscale(s);
	Image* foor = foo->rotate();
	delete(foo);
	Image* bar = foor->xlinearscale(s);
	delete(foor);
	Image* barr = bar->rotate();
	delete(bar);
	sprintf(barr->name,"%s Scaled %g",name,s);
	return barr;
}

Image* Image::lanczos2decimate(bool xdirection)
{
	if(xdirection)
	{
		Image* bar = mirrorpadded(4);
		Image* foo = new Image(width/2,height);
		float l2[] = {-.009,-.042,.117,.434,.434,.117,-.042,-.009};
		for(int y=0; y<height; y++)
		{
			for(int x=0; x<width/2; x++)
			{
				for(int k=0; k<8; k++)
				{
					foo->data[x+(width/2)*y] += bar->data[2*x+k+1 + (width+8)*(y+4)]*l2[k];
				}
			}
		}
		delete(bar);
		foo->coords.lx = coords.lx; foo->coords.ux = realx(2*(width/2)-1);
		foo->coords.ly = coords.ly; foo->coords.uy = coords.uy;
		return foo;
	} else {
		Image* bar = mirrorpadded(4);
		Image* foo = new Image(width,height/2);
		float l2[] = {-.009,-.042,.117,.434,.434,.117,-.042,-.009};
		for(int y=0; y<height/2; y++)
		{
			for(int x=0; x<width; x++)
			{
				for(int k=0; k<8; k++)
				{
					foo->data[x+width*y] += bar->data[x + 4 + (width+8)*(2*y+1+k)]*l2[k];
				}
			}
		}
		delete(bar);
		foo->coords.lx = coords.lx; foo->coords.ux = coords.ux;
		foo->coords.ly = coords.ly; foo->coords.uy = realy(2*(height/2)-1); 
		return foo;
	}
}

Image* Image::getregion(BoundingBox b) 
{
	Image* foo = new Image(b.ux-b.lx+1,b.uy-b.ly+1);
	for(int x=b.lx; x<=b.ux; x++) {
		for(int y=b.ly; y<=b.uy; y++) {
			foo->data[x-b.lx+(foo->width)*(y-b.ly)] = data[x+width*y];
		}
	}
	return foo;
}

void Image::putregion(Image* foo, BoundingBox b) 
{
	for(int x=b.lx; x<=b.ux; x++) {
		for(int y=b.ly; y<=b.uy; y++) {
			data[x+width*y] = foo->data[x-b.lx+(foo->width)*(y-b.ly)];
		}
	}
}

Image* Image::getsubregion(unsigned int n, float overreach) {
	if(!mycatalog) return NULL;
	if(n>mycatalog->ncraters-1) return NULL;
	int y = height - mycatalog->entries[n]->centery;
	int r = (int)(overreach*mycatalog->entries[n]->radius);
	int xmin = max(0,mycatalog->entries[n]->centerx-r);
	int xmax = min(width-1,mycatalog->entries[n]->centerx+r);
	int ymin = max(0,y-r);
	int ymax = min(height-1,y+r);
	Image* foo = new Image(xmax-xmin+1,ymax-ymin+1);
	for(int x=xmin; x<=xmax; x++){
		for(int y=ymin; y<=ymax; y++){
			foo->data[x-xmin+(xmax-xmin+1)*(y-ymin)]=data[x+width*y];
		}
	}
	sprintf(foo->name,"%s Region %i",name,n);
	return foo;
}

ImageDataScanner::ImageDataScanner(Image* img, int x, int y, float** d)
{
	myImg = img;
	si = new ScanIterator((RectRegion*)img,x,y,&datp);
	dat = d;
	npts=0;
	(*dat) = NULL;
}

ImageDataScanner::~ImageDataScanner()
{
	if(*dat) {free(*dat); (*dat) = NULL;}
	delete(si);
}

int* ImageDataScanner::getpositions() { return datp; }
int ImageDataScanner::getoffset() {return si->getoffset();}

void ImageDataScanner::replacedata()
{
	if(*dat) { for(int i=0; i<npts; i++) myImg->data[datp[i]] = (*dat)[i]; }
}

void ImageDataScanner::replacedata(float* d)
{
	for(int i=0; i<npts; i++) myImg->data[datp[i]] = d[i];
}

int ImageDataScanner::nextline()
{
	if(!(*dat)) (*dat) = (float*)malloc((myImg->width+myImg->height)*sizeof(float));
	npts=si->nextline();
	for(int i=0; i<npts; i++) (*dat)[i]=myImg->data[datp[i]];
	return npts;
}

int ImageDataScanner::nextline(float* dput)
{
	npts=si->nextline();
	for(int i=0; i<npts; i++) dput[i]=myImg->data[datp[i]];
	return npts;
}