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

#include "Image.hh"
#include <string.h>

Image::Image(int w, int h) : RectRegion(w,h) {
    isaName = "Image";
    isaNum = COBJ_IMAGE;
    data = (float*)calloc(size,sizeof(float));
}

Image::Image(RectRegion* R) : RectRegion(R->width,R->height) {
    isaName = "Image";
    isaNum = COBJ_IMAGE;
    copyfromrr(R);
    data = (float*)calloc(size,sizeof(float));
}

Image::~Image () {
    free(data);
}


Image* Image::copy() //return a copy of this image
{
    Image *foo=new Image((RectRegion*)this);
    for(int i=0; i<size; i++) foo->data[i]=data[i];
    return foo;
}


Image* Image::copyfrom(Image *img) //copy everything over from another image
{
    copyfromrr((RectRegion*)img);
    mycatalog = img->mycatalog;
    data = (float*)realloc(data,size*sizeof(float));
    memcpy(data,img->data,size*sizeof(float));
    return this;
}

/*
 //This algorithm doesn't quite work... should be ~2x faster than convolve, but needs work
 Image* Image::htransform_OLD(int k) //Homogeneity transform
 {
 printf("Calculating H-Image...\n");
 
 //calculate direction vectors cp_i/||cp_i||
 //by symmetry, only need to calculate for one coordinate/quadrant
 float *dv = (float*)calloc((2*k+1)*(2*k+1),sizeof(float));
 for(int y=0; y<k+1; y++) {
 for(int x=0; x<k+1; x++) {
 if (x==0 && y==0) continue;
 float z = (float)x/sqrt((float)x*x+(float)y*y);
 dv[(x+k)+(2*k+1)*(y+k)]=z;
 dv[(k-x)+(2*k+1)*(y+k)]=-z;
 dv[(x+k)+(2*k+1)*(k-y)]=z;
 dv[(k-x)+(2*k+1)*(k-y)]=-z;
 }
 
 }
 
 for(int y=0; y<2*k+1; y++) {
 for(int x=0; x<2*k+1; x++) {
 printf("%f\t",dv[x+(2*k+1)*y]);
 }
 printf("\n");
 }
 
 
    //x,y components of f
 float *fx = (float*)calloc(size,sizeof(float));
 float *fy = (float*)calloc(size,sizeof(float));
 
 float delta;
 for(int x=k; x<width-k; x++) {
 for(int y=0; y<height-k; y++) {
 for(int m=0; m<2*k+1; m++) {
 for(int n=0; n<k+1; n++) {
 if(n==0 && m<=k) continue;
 delta = data[x+width*y]-data[x+m-k+width*(y+n)];
 fx[x+width*y]+=delta*dv[m+(2*k+1)*(n+k)];
 fy[x+width*y]+=delta*dv[(n+k)+(2*k+1)*m];
 fx[x+m-k+width*(y+n)]+=-delta*dv[m+(2*k+1)*(n+k)];
 fy[x+m-k+width*(y+n)]+=-delta*dv[(n+k)+(2*k+1)*m];
 }
 }
 }
 }
 
 printf("Combining results...");
 Image* foo = new Image(width-2*k,height-2*k);
 for(int x=0; x<width-2*k; x++) {
 for(int y=0; y<height-2*k; y++) {
 foo->data[x+(width-2*k)*y]=sqrt(fx[x+k+width*(y+k)]*fx[x+k+width*(y+k)]+fy[x+k+width*(y+k)]*fy[x+k+width*(y+k)]);
 }
 }
 
 free(dv); free(fx); free(fy);
 return foo;
 };
 */

/* Image* Image::plan_curvature()
 {
    printf("Calculating image tangential curvature...\n");
    DerivsTable D(this);
    D.trim_matching();
    Image *dx2 = D.dx->copy()->mult(D.dx);
    Image *dy2 = D.dy->copy()->mult(D.dy);
    Image *p = dx2->copy()->add(dy2);
    Image *q = p->copy()->add(1.0);
    Image *foo=D.dxx->copy()->mult(dy2);
    Image *tmp = D.dxy->copy()->mult(D.dx)->mult(D.dy)->mult(-2.0);
    foo->add(tmp);
    tmp->copyfrom(D.dyy)->mult(dx2);
    foo->add(tmp);
    tmp->copyfrom(p)->mult(p)->mult(p)->sq_rt();
    foo->divide(tmp);
    delete(dx2); delete(dy2);
    delete(p); delete(q);
    delete(tmp);
    sprintf(hy->name,"%s Plan Curvature",name);
    return foo;
 };
 
 
 Image* Image::tangential_curvature()
 {
    printf("Calculating image tangential curvature...\n");
    DerivsTable D(this);
    D.trim_matching();
    Image *dx2 = D.dx->copy()->mult(D.dx);
    Image *dy2 = D.dy->copy()->mult(D.dy);
    Image *p = dx2->copy()->add(dy2);
    Image *q = p->copy()->add(1.0);
    Image *foo=D.dxx->copy()->mult(dy2);
    Image *tmp = D.dxy->copy()->mult(D.dx)->mult(D.dy)->mult(-2.0);
    foo->add(tmp);
    tmp->copyfrom(D.dyy)->mult(dx2);
    foo->add(tmp);
    tmp->copyfrom(q)->sq_rt()->mult(p);
    foo->divide(tmp);
    delete(dx2); delete(dy2);
    delete(p); delete(q);
    delete(tmp);
    sprintf(hy->name,"%s Tangential Curvature",name);
    return foo;
 }; */


float* Image::minmax() //get pointer to array [minfloatue, maxfloatue] for the data
{
    float* mnmx = (float*)malloc(2*sizeof(float));
    mnmx[0] = FLT_MAX;
    mnmx[1] = -FLT_MAX;
    for (int i=0; i<size; i++) {
#ifndef WIN32
        //if(__isnanf(data[i]) || __isinff(data[i])) continue; // TODO
#endif
        if (data[i]<mnmx[0]) mnmx[0]=data[i];
        if (data[i]>mnmx[1]) mnmx[1]=data[i];
    }
    return mnmx;
}


float* Image::stats() //get pointer to array [mu, sigma] for the data
{
    float sx=0;
    float sxx=0;
    for(int z=0; z<size; z++)
    {
        sx+=data[z];
        sxx+=data[z]*data[z];
    }
    sx/=(float)size;
    sxx/=(float)size;
    float* musigma = (float*)malloc(2*sizeof(float));
    musigma[0]=sx;
    musigma[1]=sqrt(sxx-sx*sx);
    return musigma;
}

//--------------------------------------------------------

MultiImage::MultiImage(int w, int h) : RectRegion(w,h)
{
    nimgs=0;
    imgs = NULL;
}

void MultiImage::addimg(Image* I)
{
    nimgs++;
    if(!imgs) imgs = (Image**)malloc(sizeof(Image*));
    else imgs = (Image**)realloc(imgs,nimgs*sizeof(Image*));
    imgs[nimgs-1]=I;
}

MultiImage::~MultiImage()
{
    if(imgs) {free(imgs); imgs=NULL;}
}

//apply variants allow an Image:: member function to be applied to each Image in the MultiImage
void** MultiImage::apply(void* (Image::*fctn)())
{
    void** v = (void**)malloc(nimgs*sizeof(void*));
    for(int i=0; i<nimgs; i++) v[i] = (*imgs[i].*fctn)();
    return v;
}

void MultiImage::apply(void (Image::*fctn)())
{
    for(int i=0; i<nimgs; i++) (*imgs[i].*fctn)();
}

void MultiImage::apply(void (Image::*fctn)(float), float x)
{
    for(int i=0; i<nimgs; i++) (*imgs[i].*fctn)(x);
}

void MultiImage::apply(void (Image::*fctn)(int), int x)
{
    for(int i=0; i<nimgs; i++) (*imgs[i].*fctn)(x);
}

