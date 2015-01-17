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

#include "ComplexImage.hh"
#include "Image.hh"

#ifdef WITHFFTW

ComplexImage::ComplexImage (int w, int h) : RectRegion(w/2+1,h)
{
	sprintf(isaName,"ComplexImage");
	isaNum = COBJ_COMPLEXIMAGE;
	origwidth=w;
	origheight=h;
	data = (fftw_complex*)calloc(size+w+h,sizeof(fftw_complex));
};

ComplexImage::~ComplexImage ()
{
	free(data);
};

ComplexImage* ComplexImage::fftreal(Image* I) {
	ComplexImage* foo = new ComplexImage(I->width,I->height);
	double* datain = (double*)malloc((I->size+I->width+I->height)*sizeof(double));
	fftw_plan myplan = fftw_plan_dft_r2c_2d(I->height,I->width,datain,foo->data,FFTW_ESTIMATE);
	for(int i=0; i<I->size; i++) datain[i] = (double)(I->data[i]);
	fftw_execute(myplan);
	fftw_destroy_plan(myplan);
	free(datain);
	return foo;
}

ComplexImage* ComplexImage::copy()
{
	ComplexImage* C = new ComplexImage(origwidth, origheight);
	C->copyfromrr((RectRegion*)this);
	for(int i=0; i<size; i++) {
		C->data[i][0]=data[i][0];
		C->data[i][1]=data[i][1];
	}
	return C;
}

Image* ComplexImage::inversefftreal() {
	Image* foo = new Image(origwidth,origheight);
	double* dataout = (double*)malloc(origwidth*origheight*sizeof(double));
	fftw_plan myplan = fftw_plan_dft_c2r_2d(origheight,origwidth,data,dataout,FFTW_ESTIMATE);
	fftw_execute(myplan);
	fftw_destroy_plan(myplan);
	for(int i=0; i<origwidth*origheight; i++) foo->data[i] = (float)dataout[i];
	free(dataout);
	return foo; 
}

Image* ComplexImage::real() {
	Image* foo = new Image(width,height);
	for(int i=0; i<size; i++) foo->data[i]=(float)data[i][0];
	return foo;
}

Image* ComplexImage::imag() {
	Image* foo = new Image(width,height);
	for(int i=0; i<size; i++) foo->data[i]=(float)data[i][1];
	return foo;
}

Image* ComplexImage::magv() {
	Image* foo = new Image(width,height);
	float rp,ip;
	for(int i=0; i<size; i++) {
		rp=(float)data[i][0];
		ip=(float)data[i][1];
		foo->data[i]=sqrt(rp*rp+ip*ip);
	}
	return foo;
}

ComplexImage* ComplexImage::mult(ComplexImage* ci) {
	for(int i=0; i<size; i++) {
		double tmp = data[i][0]*ci->data[i][0] - data[i][1]*ci->data[i][1];
		data[i][1] = data[i][1]*ci->data[i][0] + data[i][0]*ci->data[i][1];
		data[i][0] = tmp;
	}
	return this;
}

#endif
