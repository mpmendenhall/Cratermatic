//-----------------------------------------------------------------------
//
// CRATERMATIC Topography Analysis Toolkit
// Copyright (C) 2006 Michael Mendenhall
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

#include "Image.hh"
#include "Utils.hh"

Image* Image::drawmarks(ImageMark* m, unsigned int nm, float c) {
	for(int i=0; i<nm; i++) {
		if(m[i].type==0) crossmark((int)m[i].x0,(int)m[i].y0,(int)m[i].r,c);
		if(m[i].type==1) circle((int)m[i].x0,(int)m[i].y0,(int)m[i].r,c);
		if(m[i].type==3) line((int)m[i].x0,(int)m[i].y0,(int)m[i].x1,(int)m[i].y1,c);
	}
	return this;
}

Image* Image::drawmarks(float c) {
	if(!marks) return this;
	return drawmarks(marks,nmarks,c);
}

Image* Image::crossmark(int x0, int y0, int l, float c){
	for(int d=0; d<l; d++){
		if(y0>=0 && y0<height) {
			if(x0+d<width) data[x0+d+width*y0]=c;
			if(y0+d<height) data[x0+width*(y0+d)]=c;
		}
		if(x0>=0 && x0<width) {
			if(x0-d>=0) data[x0-d+width*y0]=c;
			if(y0-d>=0) data[x0+width*(y0-d)]=c;
		}
	}
	return this;
}

Image* Image::circle(int x0, int y0, int r, float c) {
	if(r>2000) return this;
	int d=-r;
	int x=1;
	int y=r;
	draweightsymm(x0,y0,0,r,c);
	while(x<y)
	{
		d+=2*x-1;
		if(d>=0) {
			y--;
			d -= 2*y;
		}
		draweightsymm(x0,y0,x,y,c);
		x++;
	}
	return this;
}

Image* Image::line(int x0,int y0,int x1,int y1, float c)
{
	if(!inrange(x0,y0) || !inrange(x1,y1)) return this;
	LineIterator l(x0,y0,x1,y1);
	do {
		data[l.px()+width*l.py()] = c;
	} while(l.step());
	return this;
}

Image* Image::line(int p0, int p1, float c)
{
	return line(p0%width, p0/width, p1%width, p1/width, c);
}

void Image::safeset(int x,int y,float z) {
	if(!inrange(x,y)) return;
	data[x+width*y]=z;
}

void Image::draweightsymm(int x0, int y0, int dx, int dy, float c){
	safeset(x0 + dx, y0 + dy, c);
	safeset(x0 + dx, y0 - dy, c);
	safeset(x0 - dx, y0 + dy, c);
	safeset(x0 - dx, y0 - dy, c);
	safeset(x0 + dy, y0 + dx, c);
	safeset(x0 + dy, y0 - dx, c);
	safeset(x0 - dy, y0 + dx, c);
	safeset(x0 - dy, y0 - dx, c);
}

Image* Image::filledcircleimage(unsigned int r)
{
	Image* foo = new Image(2*r+1,2*r+1);
	for(int x=0; x<2*r+1; x++)
	{
		for(int y=0; y<2*r+1; y++)
		{
			if((x-r)*(x-r)+(y-r)*(y-r) <= (r+0.5)*(r+0.5)) foo->data[x+(2*r+1)*y]=1;
		}
	}
	return foo;
};

Image* Image::annulusimage(unsigned int r0, unsigned int r1)
{
	Image* foo = new Image(2*r0+1,2*r0+1);
	for(int x=0; x<2*r0+1; x++)
	{
		for(int y=0; y<2*r0+1; y++)
		{
			int r = (x-r0)*(x-r0)+(y-r0)*(y-r0);
			if(r <= (r0+0.5)*(r0+0.5) && r >= (r1+0.5)*(r1+0.5)) foo->data[x+(2*r0+1)*y]=1;
		}
	}
	return foo;
};

void Image::interpladd(float x, float y, float z)
{
	float px = x-(int)x;
	float py = y-(int)y;
	if(x>=width || y>=height) return;
	if(x>=0) {
		if(y>0) data[(int)x+width*(int)y] += z*(1-px)*(1-py);
		if(y+1>0 && y+1<height) data[(int)x+width*((int)y+1)] += z*(1-px)*py;
	}
	if(x+1 >= 0 && x+1<width) {
		if(y>0) data[(int)x+1+width*(int)y] += z*px*(1-py);
		if(y+1>0 && y+1<height) data[(int)x+1+width*((int)y+1)] += z*px*py;
	}
};