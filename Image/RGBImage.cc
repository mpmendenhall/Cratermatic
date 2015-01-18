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

#include "RGBImage.hh"
#include "Image.hh"
#include "Classify.hh"

RGBImage::RGBImage(int w, int h) : RectRegion(w,h) {
	isaName = "RGBImage";
	isaNum = COBJ_RGBIMAGE;
	R = new Image(w,h);
	G = new Image(w,h);
	B = new Image(w,h);
}

RGBImage::RGBImage(Image* img) : RectRegion(img->width, img->height)
{
	isaName = "RGBImage";
	isaNum = COBJ_RGBIMAGE;
	copyfromrr((RectRegion*)img);
	R = new Image(width,height); R->copyfromrr((RectRegion*)img);
	G = new Image(width,height); G->copyfromrr((RectRegion*)img);
	B = new Image(width,height); B->copyfromrr((RectRegion*)img);
	colorby(img);
}

RGBImage::RGBImage(Image* x, Image* y) : RectRegion(x->width, x->height)
{
	isaName = "RGBImage";
	isaNum = COBJ_RGBIMAGE;
	copyfromrr((RectRegion*)x);
	R = new Image((RectRegion*)x);
	G = new Image((RectRegion*)x);
	B = new Image((RectRegion*)x);
	Image* m = x->copy()->quadratureadd(y)->flatHisto()->inormalized();
	for(int i=0; i<size; i++)
	{
		hsv2rgb(atan2(y->data[i],x->data[i])+M_PI,1,m->data[i],R->data + i,G->data + i,B->data + i);
	}
	delete(m);
}

RGBImage::RGBImage(RectRegion* RR) : RectRegion(RR->width,RR->height)
{
	isaName = "RGBImage";
	isaNum = COBJ_RGBIMAGE;
	copyfromrr(RR);
	R = new Image(width,height); R->copyfromrr(RR);
	G = new Image(width,height); G->copyfromrr(RR);
	B = new Image(width,height); B->copyfromrr(RR);
}

RGBImage::~RGBImage(){
	delete(R);
	delete(G);
	delete(B);
}

RGBImage* RGBImage::redzeros(Image* I)
{
	RGBImage* C = new RGBImage((RectRegion*)I);
	Image* foo = I->normalized(0,1.0);
	for(int i=0; i<I->size; i++) {
		C->R->data[i] = foo->data[i];
		C->G->data[i] = foo->data[i];
		C->B->data[i] = foo->data[i];
		if(!I->data[i]) C->R->data[i] = 1;
	}
	delete(foo);
	return C;
}


RGBImage* RGBImage::drawmarks() {
	R->drawmarks(marks,1.0);
	B->drawmarks(marks,1.0);
	G->drawmarks(marks,0.0);
	return this;
}

void RGBImage::solidcolor(float r, float g, float b)
{
	for(int i=0; i<size; i++)
	{
		R->data[i]=r;
		G->data[i]=g;
		B->data[i]=b;
	}
}

RGBImage* RGBImage::maskby(Image* img) {
	Image* foo = img->normalized(0,1.0,50.0);
	for(int i=0; i<size; i++) {
		R->data[i]*=1-foo->data[i];
		R->data[i]+=foo->data[i];
		B->data[i]*=1-foo->data[i];
		B->data[i]+=foo->data[i];
	}
	delete(foo);
	return this;
}

RGBImage* RGBImage::emboss(Image* img) {
	Image* dx = img->diagDeriv(true)->flatHisto();
	Image* n = dx->normalized(-1.0,1.0);
	delete(dx);
	float z,f;
	for(int i=0; i<size; i++)
	{
		z = n->data[i];
		f = 0.4 + 0.6*(z+4*z*z*z*z*z + 5)/10.0;
		R->data[i] *= f;
		G->data[i] *= f;
		B->data[i] *= f;
	}
	delete(n);
	return this;
}

RGBImage* RGBImage::renderTopo(Image* t)
{
	Image* tf = t->copy()->flatHisto();
	RGBImage* C = new RGBImage(tf);
	delete(tf);
	C->emboss(t);
	return C;
}


RGBImage* RGBImage::colorby(Image* img) {
	Image* foo = img->normalized(0,1.0);
	for(int i=0; i<size; i++) {
		float z = foo->data[i];
		hsv2rgb(19*M_PI/12*(1-z),1,1,R->data+i,G->data+i,B->data+i);
	}
	delete(foo);
	return this;
}

void RGBImage::desaturate(float z) {
	float rb,gb,bb;
	for(int i=0; i<size; i++) {
		rb = z/3*(G->data[i]+B->data[i]);
		gb = z/3*(R->data[i]+B->data[i]);
		bb = z/3*(R->data[i]+G->data[i]);
		R->data[i] *= (1-2*z/3); R->data[i]+=rb;
		G->data[i] *= (1-2*z/3); G->data[i]+=gb;
		B->data[i] *= (1-2*z/3); B->data[i]+=bb;
	}
}

RGBImage* RGBImage::grayby(Image* img, float bp) {
	RGBImage* C = new RGBImage((RectRegion*)img);
	Image* foo = img->normalized(0,1.0);
	for(int i=0; i<C->size; i++) {
		C->R->data[i] = bp+(1-bp)*foo->data[i];
		C->G->data[i] = bp+(1-bp)*foo->data[i];
		C->B->data[i] = bp+(1-bp)*foo->data[i];
	}
	delete(foo);
	return C;
}

RGBImage* RGBImage::classifyoverlay(ClassifyImage* C, int andkey, int xorkey, float r, float g, float b)
{
	float z = 1/(1+std::max(std::max(r,g),b));
	for(int i=0; i<size; i++)
	{
		if( (C->data[i] ^ xorkey) & andkey )
		{
			R->data[i] += r; R->data[i] *= z; 
			G->data[i] += g; G->data[i] *= z;
			B->data[i] += b; B->data[i] *= z;
		}
	}
	return this;
}
									
void RGBImage::speckfield() {
	for(int i=0; i<size; i++) {
		R->data[i] = (float)rand()/((float)RAND_MAX);
		G->data[i] = (float)rand()/((float)RAND_MAX);
		B->data[i] = (float)rand()/((float)RAND_MAX);
	}
}

void RGBImage::pointfield(int n) {
	for(int i=0; i<size; i++) {
		if((i%width)%n == 0 && (i/width)%n == 0)
		{
			R->data[i] = 1;
			G->data[i] = 1;
			B->data[i] = 1;
		}
	}
}

void RGBImage::gridfield(int n) {
	for(int i=0; i<size; i++) {
		if((i%width)%n == 0 || (i/width)%n == 0)
		{
			R->data[i] = 1;
			G->data[i] = 1;
			B->data[i] = 1;
		}
	}
}

RGBImage* RGBImage::qrbs(Image* img, float z) { //red-blue stereoize according to elevation image
	Image* foo = img->normalized(0,1.0)->icomplement();
	
	Image* Rb = new Image(this);
	Image* Gb = new Image(this);
	Image* Bb = new Image(this);
	
	Image* Rn = new Image(this);
	Image* Gn = new Image(this);
	Image* Bn = new Image(this);
	
	for(int i=0; i<size; i++)
	{
		float x0 = i%width;
		int y = i/width;
		float delta = z*(foo->data[i]-0.5);
		Rb->interpladd(x0+delta,y,R->data[i]);
		Gb->interpladd(x0-delta,y,G->data[i]);
		Bb->interpladd(x0-delta,y,B->data[i]);
		Rn->interpladd(x0+delta,y,1);
		Gn->interpladd(x0-delta,y,1);
		Bn->interpladd(x0-delta,y,1);
	}
	
	//quick patch for holes
	for(int i=1; i<size; i++)
	{
		if(Rn->data[i]<0.0001) {
			Rn->data[i]=Rn->data[i-1];
			Rb->data[i]=Rb->data[i-1];
		}
		
		if(Gn->data[i]<0.0001) {
			Gn->data[i]=Gn->data[i-1];
			Bn->data[i]=Bn->data[i-1];
			Gb->data[i]=Gb->data[i-1];
			Bb->data[i]=Bb->data[i-1];
		}
	}
	
	delete(R); R=Rb->divide(Rn);
	delete(G); G=Gb->divide(Gn);
	delete(B); B=Bb->divide(Bn);
	//R->rec709gamma();
	//G->rec709gamma();
	//B->rec709gamma();
	delete(foo);
	return this;
}

RGBImage* RGBImage::shadeby(Image* img) {
	Image* foo = img->normalized(0,1.0);
	for(int i=0; i<size; i++) {
		R->data[i] *= foo->data[i];
		G->data[i] *= foo->data[i];
		B->data[i] *= foo->data[i];
	}
	delete(foo);
	return this;
}

RGBImage* RGBImage::overlay(Image* img, float r, float g, float b, float t) {
	Image* foo = img->normalized(0,t);
	for(int i=0; i<size; i++) {
		R->data[i]*=1-foo->data[i];
		R->data[i]+=r*foo->data[i];
		G->data[i]*=1-foo->data[i];
		G->data[i]+=g*foo->data[i];
		B->data[i]*=1-foo->data[i];
		B->data[i]+=b*foo->data[i];
	}
	delete(foo);
	return this;
}

void RGBImage::writeBMP(const string& ofname) {
	fprintf (stdout, "Saving RGB image (%i x %i) to %s \n",width,height,ofname.c_str());
	drawmarks();
	
	//interleave color data
	float* inter = (float*)malloc(3*size*sizeof(float));
	float* padzer = (float*)calloc(3*width,sizeof(float));
	
	for(int i=0; i<size; i++)
	{
		inter[3*i+0] = B->data[i];
		inter[3*i+1] = G->data[i];
		inter[3*i+2] = R->data[i];
	}
	
	FILE* ofp = fopen(ofname.c_str(), "wb");
	CratersBaseObject::writeBMPHeaders(ofp,24,width,height);
	for(int i=height-1; i>=0; i--) //pad each line to 32-bit boundaries
	{
		CratersBaseObject::writeBinaryFromNormalizedFloat(inter+3*i*width,3*width,ofp,8);
		int npad = (4 - ((3*width)%4))%4;
		CratersBaseObject::writeBinaryFromNormalizedFloat(padzer,npad,ofp,8);
	}
	fclose(ofp);
	free(inter);
	free(padzer);
};

RGBImage* RGBImage::colorwheel(int r)
{
	int w = 2*r+1;
	RGBImage* C = new RGBImage(w,w);
	float x,y;
	for(int i=0; i<w*w; i++)
	{
		x = (i%w)-r;
		y = (i/w)-r;
		if(x*x+y*y > r*r) continue;
		hsv2rgb(atan2(y,x)+M_PI,(x*x+y*y)/(r*r),1,C->R->data+i,C->G->data+i,C->B->data+i);
	}
	return C;
}

RGBImage* RGBImage::fourierSpinner(int r, int k, float theta)
{
	int w = 2*r+1;
	RGBImage* C = new RGBImage(w,w);
	Image* S = new Image(w,w);
	float x,y;
	for(int i=0; i<w*w; i++)
	{
		x = (i%w)-r;
		y = (i/w)-r;
		float s = sin(theta + k*atan2(y,x));
		if(s>0) C->R->data[i] = 1;
		else C->B->data[i] = 1;
		if(x*x+y*y >= r*r*s*s)
			S->data[i] = 0;
		else
			S->data[i] = s*s - (x*x+y*y)/(r*r);
	}

	C->shadeby(S);
	delete(S);
	return C;
}


