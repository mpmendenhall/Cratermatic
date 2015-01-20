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
#include "Utils.hh"
//Pointwise math operations on Image float data


Image* Image::add(Image* I) //add image data of the same dimensions to this image's data
{
	if (width != I->width || height != I->height) { fprintf(stderr,"** Error: mismatched image sizes in add! (%i,%i) and (%i,%i) \n",width,height,I->width,I->height); exit(1); }
	for(int z=0; z<size; z++) data[z]+=I->data[z];
	name = "["+name+"]+["+I->name+"]";
	return this;
}

Image* Image::lesser(Image* I)
{
	if (width != I->width || height != I->height) { fprintf(stderr,"** Error: mismatched image sizes for 'lesser'! (%i,%i) and (%i,%i) \n",width,height,I->width,I->height); exit(1); }
	for(int z=0; z<size; z++) if(I->data[z] < data[z]) data[z] = I->data[z];
	name = "min(["+name+"],["+I->name+"])";
	return this;
}

Image* Image::greater(Image* I)
{
	if (width != I->width || height != I->height) { fprintf(stderr,"** Error: mismatched image sizes for 'greater'! (%i,%i) and (%i,%i) \n",width,height,I->width,I->height); exit(1); }
	for(int z=0; z<size; z++) if(I->data[z] > data[z]) data[z] = I->data[z];
	name = "max(["+name+"],["+I->name+"])";
	return this;
}

Image* Image::add(float c) //add a constant to this image's data
{
	for(int z=0; z<size; z++) data[z]+=c;
	name = "["+name+"]"+to_str(c);
	return this;
}

Image* Image::quadratureadd(Image* I) //RMS add image data of the same dimensions to this image's data
{
	if (width != I->width || height != I->height) { fprintf(stderr,"** Error: mismatched image sizes in qadd!\n"); exit(1); }
	for(int z=0; z<size; z++) data[z]= sqrt(I->data[z]*I->data[z]+data[z]*data[z]);
    name = "sqrt(["+name+"]^2+["+I->name+"]^2)";
	return this;
}

Image* Image::mult(Image* I) //multiply this image's data by image data of the same dimensions
{
	if (width != I->width || height != I->height) { fprintf(stderr,"** Error: mismatched image sizes in multiply!\n"); exit(1); }
	for(int z=0; z<size; z++) data[z] *= I->data[z];
    name = "["+name+"]*["+I->name+"]";
	return this;
}


Image* Image::mult(float c) //multiply this image's data by a constant
{
	for(int z=0; z<size; z++) data[z] *= c;
	name = "["+name+"]*"+to_str(c);
    return this;
}

Image* Image::divide(Image* I) //divide this image's data by image data of the same dimensions
{
	if(!(size==I->size)) fprintf(stderr,"** Error: mismatched image sizes in divide!\n");
	for(int z=0; z<size; z++) {
		if(fabs(I->data[z])>FLT_MIN*1e6) data[z] /= I->data[z];
		else data[z]=0;
	}
    name = "["+name+"]/["+I->name+"]";
	return this;
}

Image* Image::sq_rt() //replace this image's data by its square root
{
	for(int z=0; z<size; z++) data[z] = sqrt(data[z]);
	name = "sqrt(["+name+"])";
    return this;
}

Image* Image::absval() //replace this image's data by its abs()
{
	for(int z=0; z<size; z++) data[z] = fabs(data[z]);
	name = "|["+name+"]|";
    return this;
}

Image* Image::reciprocal() //replace this image's data 1/data
{
	for(int z=0; z<size; z++) data[z] = 1/data[z];
	name = "1/["+name+"]";
    return this;
}

Image* Image::normalized(float mn, float mx) //return an image normalized to range [mn,mx]
{
	float* mnmx = minmax();
	//printf("Normalizing [%.3g,%.3g]->[%.3g,%.3g]\n",mnmx[0],mnmx[1],mn,mx);
	Image* foo = new Image((RectRegion*)this);
	foo->name = name+" Normalized";
	if(mnmx[1]-mnmx[0] != 0) {for (int i=0; i<size; i++) foo->data[i]=mn+(data[i]-mnmx[0])*(mx-mn)/(mnmx[1]-mnmx[0]);}
	free(mnmx);
	return foo;
}

Image* Image::normalized(float mn, float mx, float sd) //return an image normalized to range [mn,mx] after kill_outliers
{
	Image* foo = kill_outliers(sd);
	Image* bar = foo->kill_outliers(sd);
	Image* baz = bar->normalized(mn,mx);
	delete(foo); delete(bar);
	baz->name = name+" Normalized";
	return baz;
}

Image* Image::inormalized() //normalize an image in place to [0,1]
{
	float* mnmx = minmax();
	if(mnmx[1]-mnmx[0]) {for (int i=0; i<size; i++) data[i] = (data[i]-mnmx[0])/(mnmx[1]-mnmx[0]);}
	free(mnmx);
	return this;
}

Image* Image::above(float x) //black-and-white image for above/below threshold
{
	Image* foo = new Image(this);
	for(int i=0; i<size; i++) foo->data[i] = (float)(foo->data[i]>x);
	return foo;
}

Image* Image::below(float x) //black-and-white image for above/below threshold
{
	Image* foo = new Image(this);
	for(int i=0; i<size; i++) foo->data[i] = (float)(foo->data[i] < x);
	return foo;
}

Image* Image::complement() //return the complement of a normalized image
{
	Image* foo = new Image((RectRegion*)this);
	foo->name = name+" Complement";
	for (int i=0; i<size; i++) foo->data[i]=1-data[i];
	return foo;
}

Image* Image::icomplement() //return the inplace complement of a normalized image
{
	for (int i=0; i<size; i++) data[i]=1-data[i];
	return this;
}

Image* Image::threshold(float t) //threshold this image's data
{
	for (int i=0; i<size; i++) {if(data[i]<t) data[i]=t;}
    name = name+" threshold "+to_str(t);
	return this;
}

Image* Image::kill_outliers(float sd) //chop data down to \pm sd standard deviations
{
	float* ms = stats();
	Image* foo = this->copy();
	for(int z=0; z<size; z++) {
		if (data[z]-ms[0] > sd*ms[1]) foo->data[z]=sd*ms[1];
		if (data[z]-ms[0] < -sd*ms[1]) foo->data[z]=-sd*ms[1];
	}
	free(ms);
	return foo;
}

Image* Image::signedgamma(float g)
{
	float sm = pow(1e-9,g-1);
	for(int i=0; i<size; i++)
	{
		if(data[i]<0) {
			if(data[i]<-1e-9) data[i]=-pow(-data[i],g);
			else data[i] = sm*data[i];
		} else {
			if(data[i]>1e-15) data[i] = pow(data[i],g);
			else data[i] = sm*data[i];
		}
	}
	return this;
}

Image* Image::rec709gamma() //return an image normalized to range [0,1] with ITU-R BT.709 gamma transfer function
{
	float* mnmx = minmax();
	Image* foo = new Image((RectRegion*)this);
    foo->name = name+" Normalized";
	if(mnmx[1]-mnmx[0] != 0) {
		for (int i=0; i<size; i++) {
			foo->data[i]=(data[i]-mnmx[0])/(mnmx[1]-mnmx[0]);
			if(foo->data[i]<=0.018) foo->data[i] *= 4.5;
			else foo->data[i] = 1.099*pow(foo->data[i],0.45)-0.099;
		}
	}
	free(mnmx);
	return foo;
}

Image* Image::rec709gammaInverse() //return an image normalized to range [0,1] with ITU-R BT.709 gamma transfer function
{
	float* mnmx = minmax();
	Image* foo = new Image((RectRegion*)this);
	foo->name = name+" Normalized";
	if(mnmx[1]-mnmx[0] != 0) {
		for (int i=0; i<size; i++) {
			foo->data[i]=(data[i]-mnmx[0])/(mnmx[1]-mnmx[0]);
			if(foo->data[i]<=0.081) foo->data[i] /= 4.5;
			else foo->data[i] = pow((foo->data[i]+.099)/1.099,1/0.45);
		}
	}
	free(mnmx);
	return foo;
}

int* Image::sortImage()
{
	int* srt = (int*)calloc(size,sizeof(int));
	float** dp = (float**)malloc(size*sizeof(float*));
	for(int i=0; i<size; i++) dp[i] = data+i;
	qsort(dp,size,sizeof(float*),CratersBaseObject::compareFloatP);
	for(int i=0; i<size; i++) srt[(int)(dp[i] - data)] = i;
	free(dp);
	return srt;
}

Image* Image::flatHisto()
{
	int* s = sortImage();
	float* newdat = (float*)malloc(size*sizeof(float));
	for(int i=0; i<size; i++) newdat[i] = (float)s[i];
	free(data); data = newdat;
	free(s);
	return this;
}