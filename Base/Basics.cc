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

#include "Basics.hh"
#include "Utils.hh"

//calculate byte length
int calcbytelength()
{
	srand(time(NULL));
	int n=0;
	while((unsigned char)(1<<n)) n++;
	return n;
}

int CratersBaseObject::compareFloat(const void * a, const void * b)
{
	if(*(float*)a > *(float*)b) return 1;
	if(*(float*)a < *(float*)b) return -1;
	return 0;
}

int CratersBaseObject::compareFloatP(const void * a, const void * b)
{
	if((**(float**)a) > (**(float**)b)) return 1;
	if((**(float**)a) < (**(float**)b)) return -1;
	return 0;
}

const int CratersBaseObject::bytelength = calcbytelength();

CratersBaseObject::CratersBaseObject() {
    isaName = "CratersBaseObject";
	name = "";
	isaNum = COBJ_CRATERSBASEOBJECT;
}

void CratersBaseObject::writeBinaryFromNormalizedFloat(float* fdat, int len, FILE* ofp, int nbits) {
	
	//test for max bit depth
	int i=0;
	while( (1<<(i+1)) > (1<<i)) i++;
	if(nbits > i)
	{
		printf("\n*** Warning: maximum bit depth exceeded; writing file at %i bits/pixel ***\n",i);
		nbits=i;
	}
	
	//make appropriately scaled long-int copy
	unsigned long int* dat = (unsigned long int*)malloc(len*sizeof(unsigned long int));
	for(int i=0; i<len; i++) dat[i] = (unsigned long int)((1<<nbits)*(0.999999*fdat[i]));
	
	unsigned char abl=8;
	unsigned char dbl=nbits;
	unsigned char accum=0;
	int c=0;
	while(c<len)
	{
		if(dbl >= abl) //finish up accum and write
		{
			accum += (unsigned char)(dat[c] >> (dbl - abl)) << (8-abl) >> (8-abl);
			dbl -= abl; abl = 8;
			fputc(accum,ofp); accum=0;
			if(!dbl) {++c; dbl = nbits;}
		} else { //finish up data and get next data point
			accum += (unsigned char)dat[c] << (8-dbl) >> (8-abl);
			abl -= dbl;
			dbl = nbits; ++c;
		}
	}
	if(accum != 0) fputc(accum,ofp);
	free(dat);
}

void CratersBaseObject::writeBinaryFromBool(bool* bdat, int len, FILE* ofp) {
	
	unsigned char accum;
	for(int i=0; i<len; i+=8)
	{
		accum = 0;
		for(int j=0; j<8; j++)
		{
			if(i+j>=len) break;
			accum |= (unsigned char)(bdat[i+j]) << (7-j);
		}
		fputc(accum,ofp);
	}
}


void CratersBaseObject::rev2byte(FILE* ofp, unsigned short foo)
{
	fputc(foo << 8 >> 8,ofp);
	fputc(foo >> 8,ofp);
}

void CratersBaseObject::rev4byte(FILE* ofp, unsigned int foo)
{
	fputc(foo << 24 >> 24,ofp);
	fputc(foo << 16 >> 24,ofp);
	fputc(foo << 8 >> 24,ofp);
	fputc(foo >> 24,ofp);
}

void CratersBaseObject::writeBMPHeaders(FILE* ofp, int bitdepth, int width, int height)
{
	if(bitdepth != 1 && bitdepth != 8 && bitdepth != 24)
	{
		printf("*** Unsupported BMP bitdepth! Headers not written. ***\n");
		return;
	}
	
	//precalc data size in bytes
	int dsize = height*(bitdepth*width + (32 - ((bitdepth*width)%32)%32))/8;
	
	//header size
	int hsize = 54;
	if(bitdepth == 1) hsize += 0x8; //extra space for color table
	if(bitdepth == 8) hsize += 0x400; //extra space for color table
	
	//file header
	fprintf(ofp,"BM"); //magic file-type numbers
	rev4byte(ofp,hsize+dsize); //filesize in bytes
	rev4byte(ofp,0); //2 shorts unused
	rev4byte(ofp,hsize); //offset to data from start of file
	
	//data header
	rev4byte(ofp,40); //header size
	rev4byte(ofp,width); rev4byte(ofp,height);; //width & height
	rev2byte(ofp,1); //1
	rev2byte(ofp,bitdepth); //bits per pixel
	rev4byte(ofp,0); //compression (none)
	rev4byte(ofp,0); //image size -- may be 0 for uncompressed
	rev4byte(ofp,2000); //x resolution, pixels/meter
	rev4byte(ofp,2000); //y resolution, pixels/meter
	
	if(bitdepth == 24)
	{
		rev4byte(ofp,0); //colors in lookup table
		rev4byte(ofp,0); //"important colors"
	}
	
	if(bitdepth == 8)
	{
		rev4byte(ofp,0x100); //colors in lookup table
		rev4byte(ofp,0x100); //"important colors"
	}
	
	if(bitdepth == 1)
	{
		rev4byte(ofp,0x2); //colors in lookup table
		rev4byte(ofp,0x2); //"important colors"
	}
}

void CratersBaseObject::writeBMPgreyColorPalette(FILE* ofp)
{
	for(int i=0; i<0x100; i++)
	{
		fputc(i,ofp);
		fputc(i,ofp);
		fputc(i,ofp);
		fputc(0,ofp);
	}
}

void CratersBaseObject::writeBMPmonoColorPalette(FILE* ofp)
{
	for(int i=0; i<0x2; i++)
	{
		fputc(0xFF*i,ofp);
		fputc(0xFF*i,ofp);
		fputc(0xFF*i,ofp);
		fputc(0,ofp);
	}
}

int* CratersBaseObject::scatterNumberBase(unsigned int n)
{
	int* sn = (int*)malloc((1<<n)*sizeof(int));
	if(n==1)
	{
		sn[0]=0;
		sn[1]=1;
		return sn;
	}
	
	int* s0 = scatterNumberBase(n-1);
	int q = 1 << (n-2);
	for(int i=0; i<q; i++)
	{
		sn[i] = s0[i] << 1;
		sn[q+i] = (s0[q+i] << 1) + 1;
		sn[2*q+i] = (s0[i] << 1) + 1;
		sn[3*q+i] = s0[q+i] << 1;
	}
	free(s0);
	return sn;
}

int* CratersBaseObject::scatterNumber(unsigned int n)
{
	int* sn = scatterNumberBase(n);
	int* snf = (int*)calloc((1<<n),sizeof(int));
	for(int i=0; i<(1<<n); i++) snf[sn[i]] = i;
	free(sn);
	return snf;
}

//y = a+bx weighted lsrl
void CratersBaseObject::lsrl(float* x, float* y, float* w, int n, float* a, float* b, float* r)
{
	float sx=0;
	float sy=0;
	float sw=0;
	float sxx=0;
	float syy=0;
	float sxy=0;
	float wt;
	for(int i=0; i<n; i++) {
		if(w) wt=w[i]; else wt=1;
		sx += x[i]*wt;
		sy += y[i]*wt;
		sxx += x[i]*x[i]*wt;
		syy += y[i]*y[i]*wt;
		sxy += x[i]*y[i]*wt;
		sw += wt;
	}
	if(b) *b = (sw*sxy - sx*sy)/(sw*sxx-sx*sx);
	if(a) *a = (sy-(*b)*sx)/sw;
	if(r) *r = (sw*sxy-sx*sy)/sqrt((sw*sxx-sx*sx)*(sw*syy-sy*sy));
}

CFloat::CFloat(float f) : CratersBaseObject() {
	isaName = "Float";
	name = to_str(f);
	isaNum = COBJ_CFLOAT;
	val = f;
}

CFloat* CFloat::copy() {
	return new CFloat(val);
}

CFloat::operator float() const {
	return val;
}

CraterString::CraterString(const string& c) : CratersBaseObject() {
	isaName = "String";
	isaNum = COBJ_CRATERSTRING;
	name = c;
	val = c;
}

CraterString* CraterString::copy() {
	return new CraterString(val);
}

CError::CError(const string& c, int i) : CratersBaseObject() {
	isaName = "Error";
	isaNum = COBJ_CERROR;
	errname = c;
	name = "["+to_str(i)+"] "+errname;
	errnum = i;
}

CError* CError::copy() {
	return new CError(errname,errnum);
}

CMacro::CMacro(): CratersBaseObject() {
	isaName = "Macro";
	isaNum = COBJ_CMACRO;
	name = "[ ]";
	maxlen = 1;
	stringval = "";
}

CMacro* CMacro::copy() {
	CMacro* M = new CMacro();
	M->name = name;
	M->maxlen = maxlen;
	M->stringval = stringval;
	return M;
}

string shortize(const string& t, int l) {
	if((int)t.size()<l) return "[ "+t+" ]";
        return "[ %s<...>%s ]"; // TODO
    
	//char* fool = (char*)malloc(((l/2)+1)*sizeof(char));
	//char* foor = (char*)malloc(((l/2)+1)*sizeof(char));
	//strncpy(fool,t,l/2-5); fool[l/2-5] = '\0';
	//strncpy(foor,t+strlen(t)-l/2+5,l/2-5); foor[l/2-5] = '\0';
	//sprintf(foo,"[ %s<...>%s ]",fool,foor);
	//free(fool); free(foor);
	//return(foo);
}

void CMacro::addtoken(const string& t) {
    if(stringval.size()) stringval += " ";
    stringval += t;	
	name = shortize(stringval,60);
}

CraterSpec::CraterSpec(int idn)
{
	idnum = idn;
	x = 0;
	y = 0;
	r = 0;
	area = 0;
	volume = 0;
	hipt = 0;
	lowpt = 0;
	depth = 0;
	steepness = 0;
	xsft = NULL;
	ysft = NULL;
	grxxsft = NULL;
	grxysft = NULL;
	gryxsft = NULL;
	gryysft = NULL;
	deviation = NULL;
}

CraterSpec::~CraterSpec()
{
	if(xsft) free(xsft);
	if(ysft) free(ysft);
	if(grxxsft) free(grxxsft);
	if(grxysft) free(grxysft);
	if(gryxsft) free(gryxsft);
	if(gryysft) free(gryysft);
	if(deviation) free(deviation);
}

void CraterSpec::writeHeaders(FILE* ofp)
{
	fprintf(ofp,"# ID\tx\ty\tr\tdepth\tarea\ta_1\tb_1\n");
}

void CraterSpec::writeToFile(FILE* ofp)
{
	fprintf(ofp,"%i\t%5.2f\t%5.2f\t%5.2f\t%6.4g\t%6.4g\t%6.4g\t%6.4g\n",idnum,x,y,r,depth,xsft[0],xsft[2]/xsft[0],ysft[2]/xsft[0]);
	fflush(ofp);
}

void CraterSpec::writeShapeFourierToFile(FILE* ofp)
{
	fprintf(ofp,"%i",idnum);
	if(xsft && ysft) {
		fprintf(ofp,"\t%+.4e",xsft[0]);
		for(int i=1; i<10; i++) fprintf(ofp,"\t%+.4e\t%+.4e",xsft[i]/xsft[0],ysft[i]/xsft[0]);
	}
	fprintf(ofp,"\n");
	fflush(ofp);
}

void CraterSpec::writeGradFourierToFile(FILE* ofp)
{
	fprintf(ofp,"%i",idnum);
	if(grxxsft && grxysft)
	{
		for(int i=0; i<8; i++) fprintf(ofp,"\t%+.4e\t%+.4e",grxxsft[i],grxysft[i]);
	}
	fprintf(ofp,"\n%i",idnum);
	if(gryxsft && gryysft)
	{
		for(int i=0; i<8; i++) fprintf(ofp,"\t%+.4e\t%+.4e",gryxsft[i],gryysft[i]);
	}	
	fprintf(ofp,"\n");
	fflush(ofp);
}

void CratersBaseObject::hsv2rgb(float h, float s, float v, float* r, float* g, float* b)
{
	h = fmod(h,2*M_PI);
	
	if(s==0)
	{
		*r=v; *g=v; *b=v;
		return;
	}
	
	float var_h = 3*h/M_PI;
	int var_i = (int)floor(var_h);
	float var_1 = v * ( 1 - s );
	float var_2 = v * ( 1 - s * ( var_h - var_i ));
	float var_3 = v * ( 1 - s * ( 1 - ( var_h - var_i )));
	switch(var_i)
	{
		case 0:
			*r = v;
			*g = var_3;
			*b = var_1;
			return;
		case 1:
			*r = var_2;
			*g = v;
			*b = var_1;
			return;
		case 2:
			*r = var_1;
			*g = v;
			*b = var_3;
			return;
		case 3:
			*r = var_1;
			*g = var_2;
			*b = v;
			return;
		case 4:
			*r = var_3;
			*g = var_1;
			*b = v;
			return;
		case 5:
			*r = v;
			*g = var_1;
			*b = var_2;
			return;
	}
}


void CratersBaseObject::rgb2hsv(float r, float g, float b, float* h, float* s, float* v)
{
	*v = std::max(r,std::max(g,b));
	float d = *v - std::min(r,std::min(g,b));
	if(d==0) { *h=0; *s=0; return; }
	*s = d/(*v);
	if( *v == r ) *h = (g - b)/d;
	else if( *v == g ) *h = 2 + (b -r)/d;
	else *h = 4 + (r - g)/d;
	*h *= M_PI/3;
}