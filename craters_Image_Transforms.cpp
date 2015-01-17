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
#include "craters_Utils.h"
#include "craters_Classify.h"
#include "craters_ComplexImage.h"

int Image::linedata(int p0, int p1, float* d) //point d towards data along line from p0 to p1
{
	if(!inrange(p0) || !inrange(p1)) return 0; //line out of range
	d = (float*)malloc((width+height)*sizeof(float));
	LineIterator l(p0%width, p0/width, p1%width, p1/width);
	int i=0;
	do {
		d[i++] = data[l.px()+width*l.py()];
	} while(l.step());
	d = (float*)realloc(d,i*sizeof(float));
	return i;
}


Image* Image::linedilation(int l, bool xdirection) //as per Soille pg. 89-90
{
	Image* foo = new Image((RectRegion*)this);
	if(xdirection) {
		for(int y=0; y<height; y++) {
			float* d = (float*)malloc(width*sizeof(float));
			for(int i=0; i<width; i++) d[i]=data[i+width*y];
			lindilate(l,d,width);
			for(int i=0; i<width; i++) foo->data[i+width*y]+=d[i];
			free(d);
		}
	} else {
		for(int x=0; x<width; x++) {
			float* d = (float*)malloc(height*sizeof(float));
			for(int i=0; i<height; i++) d[i]=data[x+width*i];
			lindilate(l,d,height);
			for(int i=0; i<height; i++) foo->data[x+width*i]+=d[i];
			free(d);
		}
	}
	
	return foo;
}

Image* Image::linedilation(int l, int x, int y) //line dilation in arbitrary direction
{
	Image* foo = copy();
	float* d;
	ImageDataScanner* DS = new ImageDataScanner(foo,x,y,&d);
	int n=1;
	while(n)
	{
		n = DS->nextline();
		lindilate(20,d,n);
		DS->replacedata();
	}
	delete(DS);
	return foo;
}

float* Image::convolve1d(float* d, int w, float* k, int s) { //return convolution of d and k
	float* f = (float*)calloc(w+s-1,sizeof(float));
	for(int i=0; i<w; i++)
	{
		for(int j=0; j<s; j++){
			f[i+j] += d[i]*k[j];
		}
	}
	return f;
}

Image* Image::lineconvolve(float* k, int s, bool xdirection) //as per Soille pg. 89-90
{
	Image* foo;
	
	if(xdirection) {
		foo = new Image(width+s-1, height);
		for(int y=0; y<height; y++) {
			float* d = (float*)malloc(width*sizeof(float));
			for(int i=0; i<width; i++) d[i]=data[i+width*y];
			float* ds = convolve1d(d, width, k, s);
			for(int i=0; i<width+s-1; i++) foo->data[i+(width+s-1)*y]=ds[i];
			free(d);
			free(ds);
		}
	} else {
		foo = new Image(width, height+s-1);
		for(int x=0; x<width; x++) {
			float* d = (float*)malloc(height*sizeof(float));
			for(int i=0; i<height; i++) d[i]=data[x+width*i];
			float* ds = convolve1d(d, height, k, s);
			for(int i=0; i<height+s-1; i++) foo->data[x+width*i]=ds[i];
			free(d);
			free(ds);
		}
	}
	
	return foo;
}

float* Image::gausskernel(int s, float r)
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

Image* Image::gaussianblur(float r)
{
	Image* pdd = mirrorpadded((int)(2*r));
	Image* k = new Image((RectRegion*)pdd);
	int nw = pdd->width;
	int nh = pdd->height;
	int ns = pdd->size;
	
	float x,y,r2;
	float n = 1/(2.0*PI*r*r*ns);
	
	for(int i=0; i<ns; i++) {
		x=i%(nw);
		if(x>nw/2) x = x-nw;
		y=i/(nw);
		if(y>nh/2) y = y-nh;
		r2 = x*x+y*y;
		k->data[i] = n*exp(-r2/(2*r*r));
	}

	ComplexImage* kk = ComplexImage::fftreal(k);
	delete(k);

	ComplexImage* kdat = ComplexImage::fftreal(pdd);
	delete(pdd);
	kdat->mult(kk);
	delete(kk);

	Image* rdat = kdat->inversefftreal();
	delete(kdat);
	
	rdat->trim_inplace((int)(2*r),(int)(2*r),(int)(2*r),(int)(2*r));
	rdat->copyfromrr((RectRegion*)this);
	sprintf(rdat->name,"%s blurred",name);
	return rdat;
};

Image* Image::edgefinder(float r)
{
	Image* n = normalized(0,1);
	Image* g = n->gaussianblur(r);
	n->add(g->mult(-1.0))->absval();
	delete(g);
	return n;
}

Image* Image::deriv(bool xdirection) {
	float* ckern = (float*)malloc(3*sizeof(float));
	ckern[0]=-0.5; ckern[1]=0; ckern[2]=0.5;
	Image* mp = mirrorpadded(1);
	Image* foo = mp->lineconvolve(ckern,3,xdirection);
	free(ckern);
	delete(mp);
	if(xdirection) {
		sprintf(foo->name,"d/dx[%s]",name);
		return foo->trim_inplace(2,1,2,1);
	}
	sprintf(foo->name,"d/dy[%s]",name);
	return foo->trim_inplace(1,2,1,2);
}

Image* Image::diagDeriv(bool posdirection)
{
	Image* foo = new Image((RectRegion*)this);
	if(posdirection)
	{
		for(int x=1; x<width-1; x++)
		{
			for(int y=1; y<height-1; y++)
			{
				foo->data[x+width*y] = 0.5*(data[x+1+width*(y+1)] - data[x-1+width*(y-1)]);
			}
		}
	} else {
		for(int x=1; x<width-1; x++)
		{
			for(int y=1; y<height-1; y++)
			{
				foo->data[x+width*y] = 0.5*(data[x-1+width*(y+1)] - data[x+1+width*(y-1)]);
			}
		}
		
	}
	
	if(posdirection) {
		sprintf(foo->name,"d/dx'[%s]",name);
		return foo;
	}
	sprintf(foo->name,"d/dy'[%s]",name);
	return foo;
}

Image* Image::directionfield(Image* dx, Image* dy)
{
	Image* foo  = new Image((RectRegion*)dx);
	for(int i=0; i<dx->size; i++) {
		foo->data[i] = atan2(dx->data[i],dy->data[i]);
	}
	return foo;
}

Image* Image::maggrad2() //mag^2 of gradient, dx^2+dy^2
{
	Image *foo = new Image(width-1,height-1);
	Image *dx=deriv(true);
	Image *dy=deriv(false);
	for(int x=0; x<width-1; x++){
		for(int y=0; y<height-1; y++){
			foo->data[x+(width-1)*y]=(dx->data[x+(width-1)*y])*(dx->data[x+(width-1)*y])+(dy->data[x+width*y])*(dy->data[x+width*y]);
		}
	}
	delete(dx);
	delete(dy);
	return foo;
};

Image* Image::slope() {
	Image* foo = new Image((RectRegion*)this);
	
	float zhi;
	float zlo;
	float r2i = 1.0/sqrt(2.0);
	
	for(int x=0; x<width; x++) {
		for(int y=0; y<height; y++){
			zhi=0;
			zlo=0;
			float z0=data[x+width*y];
			for(int dx=-1; dx<=1; dx++){
				for(int dy=-1; dy<=1; dy++){
					if(x+dx<0 || x+dx>=width || y+dy<0 || y+dy>=height || (dx==0 && dy==0)) continue; //edge, or center point
					float dz=data[x+dx+width*(y+dy)]-z0;
					if(dx*dx+dy*dy==2) dz*=r2i;
					if(dz < zlo) zlo=dz;
					if(dz > zhi) zhi=dz;
				}
			}
			foo->data[x+width*y]=zhi-zlo;
		}
	}
	sprintf(foo->name,"%s Slope",name);
	return foo;
}


Image* Image::filtered(Image* f) //apply a general filter matrix (needs odd x odd kernel dimensions)
{
	//printf("Convolving image...\n");
	int kw = (int) ((f->width-1) * 0.5);
	int kh = (int) ((f->height-1) * 0.5);
	Image* foo = new Image(width+2*kw,height+2*kh);
	for(int x=0;x<width;x++){
		for(int y=0; y<height; y++){
			for(int m=-kw; m<kw+1; m++) {
				for(int n=-kh; n<kh+1; n++) {
					foo->data[x+kw+m+(y+kh+n)*(width+2*kw)]+=data[x+width*y]*(f->data[m+kw+(2*kw+1)*(n+kh)]);
				}
			}
		}
	}
	//printf("Done convolving.\n");
	return foo;
};

Image* Image::htransform(int k)
{
	printf("Calculating Htransform by convolution..."); fflush(stdout);
	
	Image* xkernel = new Image(width, height);
	Image* ykernel = new Image(width, height);
	
	float x,y,r;
	for(int i=0; i<size; i++) {
		x=i%(width);
		if(x>width/2) x = x-width;
		y=i/(width);
		if(y>height/2) y = y-height;
		if(x>k || x<-k || y>k || y<-k) continue;
		r = sqrt(x*x+y*y);
		if(r==0) r=1;
		xkernel->data[i] = x/r;
		ykernel->data[i] = y/r;
	}
	
	ComplexImage* kx = ComplexImage::fftreal(xkernel);
	delete(xkernel);
	ComplexImage* ky = ComplexImage::fftreal(ykernel);
	delete(ykernel);
	
	ComplexImage* kdat1 = ComplexImage::fftreal(this);
	ComplexImage* kdat2 = kdat1->copy();
	
	kdat1->mult(kx);
	delete(kx);
	kdat2->mult(ky);
	delete(ky);
	
	Image* rdatx = kdat1->inversefftreal();
	delete(kdat1);
	
	Image* rdaty = kdat2->inversefftreal();
	delete(kdat2);
	
	rdatx->quadratureadd(rdaty);
	delete(rdaty);
	
	rdatx->copyfromrr((RectRegion*)this);
	sprintf(rdatx->name,"%s HTransformed",name);
	printf(" Done.\n");
	return rdatx;
	
	/* printf("Calculating H-Transform by convolution... ");
	fflush(stdout);
	Image* hkx = new HKernelImage(k);
	Image* hky = hkx->rotate();
	Image* p = mirrorpadded(k);
	Image* hx = p->filtered(hkx);
	Image* hy = p->filtered(hky);
	hx->mult(hx);
	hy->mult(hy)->add(hx)->sq_rt()->trim_inplace(2*k,2*k,2*k,2*k);
	delete(hkx); delete(hky);
	delete(hx); delete(p);
	printf("Done.\n");
	sprintf(hy->name,"%s HTransformed",name);
	return hy; */
};

Image* Image::smoothehtransform(float r0)
{
	printf("Calculating smoothe Htransform by convolution..."); fflush(stdout);
	
	Image* xkernel = new Image(width, height);
	Image* ykernel = new Image(width, height);
	
	float x,y,r2,z;
	
	for(int i=0; i<size; i++) {
		x=i%(width);
		if(x>width/2) x = x-width;
		y=i/(width);
		if(y>height/2) y = y-height;
		r2 = x*x+y*y;
		if(r2==0) r2=1;
		z = exp(-r2/(2*r0*r0))/sqrt(r2);
		xkernel->data[i] = x*z;
		ykernel->data[i] = y*z;
	}
	
	ComplexImage* kx = ComplexImage::fftreal(xkernel);
	delete(xkernel);
	ComplexImage* ky = ComplexImage::fftreal(ykernel);
	delete(ykernel);
	
	ComplexImage* kdat1 = ComplexImage::fftreal(this);
	ComplexImage* kdat2 = kdat1->copy();
	
	kdat1->mult(kx);
	delete(kx);
	kdat2->mult(ky);
	delete(ky);
	
	Image* rdatx = kdat1->inversefftreal();
	delete(kdat1);
	
	Image* rdaty = kdat2->inversefftreal();
	delete(kdat2);
	
	rdatx->quadratureadd(rdaty);
	delete(rdaty);
	
	rdatx->copyfromrr((RectRegion*)this);
	sprintf(rdatx->name,"%s HTransformed",name);
	printf(" Done.\n");
	return rdatx;
};

Image* Image::pseudo_profile_curvature()
{
	printf("Calculating image profile pseudocurvature...\n");
	Image *dx = deriv(true);
	Image *dy = deriv(false);
	Image *dx2 = dx->copy()->mult(dx);
	Image *dy2 = dy->copy()->mult(dy);
	Image *dxx = dx->deriv(true);
	Image *dyy = dy->deriv(false);
	Image *dxy = dx->deriv(false);
	
	Image *p = dx2->copy()->add(dy2); // == fx^2 + fy^2
	Image *q = p->copy()->add(1.0); // == p+1
	
	Image *foo=dxx->copy()->mult(dx2);
	Image *tmp = dxy->copy()->mult(dx)->mult(dy)->mult(2.0);
	foo->add(tmp);
	tmp->copyfrom(dyy)->mult(dy2);
	foo->add(tmp);
	tmp->copyfrom(q)->mult(q)->mult(q)->sq_rt()->mult(p)->add(1000*FLT_MIN);
	//foo->divide(tmp);
	
	delete(dx); delete(dy);
	delete(dxx); delete(dyy);
	delete(dx2); delete(dy2);
	delete(p); delete(q);
	delete(dxy); delete(tmp);
	
	sprintf(foo->name,"%s Curvature",name);
	return foo;
};

Image* Image::pseudo_tangent_curvature()
{
	printf("Calculating image profile pseudotangentcurvature...\n");
	Image *dx = deriv(true);
	Image *dy = deriv(false);
	Image *dx2 = dx->copy()->mult(dx);
	Image *dy2 = dy->copy()->mult(dy);
	Image *dxx = dx->deriv(true);
	Image *dyy = dy->deriv(false);
	Image *dxy = dx->deriv(false);
	
	Image *p = dx2->copy()->add(dy2); // == fx^2 + fy^2
	Image *q = p->copy()->add(1.0); // == p+1
	
	Image *foo=dxx->copy()->mult(dy2);
	Image *tmp = dxy->copy()->mult(dx)->mult(dy)->mult(-2.0);
	foo->add(tmp);
	tmp->copyfrom(dyy)->mult(dx2);
	foo->add(tmp);
	tmp->copyfrom(q)->sq_rt()->mult(p)->add(1000*FLT_MIN);
	foo->divide(tmp);
	
	delete(dx); delete(dy);
	delete(dxx); delete(dyy);
	delete(dx2); delete(dy2);
	delete(p); delete(q);
	delete(dxy); delete(tmp);
	
	sprintf(foo->name,"%s Curvature",name);
	return foo;
};

Image* Image::squaredilation(int n) //dilate image by a square of the specified size
{
	Image* J = linedilation(n,true);
	Image* K = J->linedilation(n,false);
	delete(J);
	sprintf(K->name,"%s Square Dilated",name);
	K->copyfromrr((RectRegion*)this);
	return K;
};

Image* Image::generaldilation(Image* k) //slow and painful method
{
	Image* foo  = new Image((RectRegion*)this);
	for(int x=0; x<width; x++)
	{
		for(int y=0; y<height; y++)
		{
			float zmax = -FLT_MAX;
			for(int p=-(k->width/2); p<k->width-(k->width/2); p++)
			{
				if(x+p<0 || x+p>=width) continue;
				for(int q=-(k->height/2); q<k->height-(k->height/2); q++)
				{
					if(y+q<0 || y+q>=height) continue;
					if(k->data[p+k->width/2+(k->width)*(q+k->height/2)] && data[x+p+width*(y+q)] > zmax) zmax  = data[x+p+width*(y+q)];
				}
			}
			foo->data[x+width*y]=zmax;
		}
	}
	return foo;
};

Image* Image::circledilation(int r)
{
	Image* k = filledcircleimage(r);
	Image* foo = generaldilation(k);
	delete(k);
	foo->copyfromrr(this);
	sprintf(foo->name,"%s Dilation",name);
	return foo;
}

Image* Image::circleerosion(int r)
{
	Image* k = filledcircleimage(r);
	mult(-1.0);
	Image* foo = generaldilation(k);
	mult(-1.0); foo->mult(-1.0);
	delete(k);
	foo->copyfromrr(this);
	sprintf(foo->name,"%s Erosion",name);
	return foo;
}

Image* Image::circleclosing(int r)
{
	Image* d = circledilation(r)->mult(-1.0);
	Image* e = d->circledilation(r)->mult(-1.0);
	delete(d);
	e->copyfromrr(this);
	sprintf(e->name,"%s Closed",name);
	return e;
}

Image* Image::circleopening(int r)
{
	Image* c = copy()->mult(-1.0);
	Image* d = c->circledilation(r)->mult(-1.0);
	delete(c);
	Image* e = d->circledilation(r);
	delete(d);
	e->copyfromrr(this);
	sprintf(e->name,"%s Opened",name);
	return e;
}

Image* Image::annulusdilation(int r0, int r1)
{
	Image* k = annulusimage(r0,r1);
	Image* foo = generaldilation(k);
	delete(k);
	return foo;
}

Image* Image::removespikes() {
	Image* N = normalized(0,1);
	Image* bluri = N->gaussianblur(3.0)->mult(-1.0);
	Image* diff = N->copy()->add(bluri);
	Image* Nb = N->complement();
	Image* Ns = N->squaredilation(3); //fixed downspikes
	Image* Nbs = Nb->squaredilation(3); //fixed upspikes
	for(int i=0; i<size; i++) {
		if(diff->data[i] > 0.2) N->data[i] = 1-Nbs->data[i];
		if(diff->data[i] < -0.2) N->data[i] = Ns->data[i];
	}
	delete(bluri); delete(diff);
	delete(Nb); delete(Ns); delete(Nbs);
	sprintf(N->name,"%s Despiked",name);
	return N;
};