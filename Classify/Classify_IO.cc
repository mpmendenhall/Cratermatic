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
#include "RGBImage.hh"
#include <string.h>
#include <cassert>

Image* ClassifyImage::markimage() {
	Image* foo = new Image(width,height);
	for(int i=0; i<nbasins; i++){
		if(markedregion[i]){
			for(int j=0; j<npic[i]; j++) {
				foo->data[pic[i][j]]=1.0;
			}
		}
	}
	return foo;
}

RGBImage* ClassifyImage::prettyImage() {
	Image* CI = recolorize(1,13);
	RGBImage* C = new RGBImage(CI);
	delete(CI);
	Image* BI = fourboundaryimage();
	C->overlay(BI,0,0,0,0.3);
	delete(BI);
	return C;
}

RGBImage* ClassifyImage::colorbytemp() {
	Image* foo = tempstatimg();
	RGBImage* C = new RGBImage(foo);
	Image* BI = fourboundaryimage();
	C->overlay(BI,0,0,0,0.3);
	delete(foo);
	delete(BI);	
	return C;
}


RGBImage* ClassifyImage::prettyoverlayimage(Image* under) {
	RGBImage* C = new RGBImage(under);
	C->copyfromrr((RectRegion*)this);
	Image* BI = fourboundaryimage();
	Image* MI = markimage();
	Image* NI = new Image(width,height);
	C->overlay(MI,1,0,1,0.5);
	C->overlay(BI,0,0,0,0.6);
	C->overlay(NI,1,1,1,1.0);
	if(badimg) C->shadeby(BI);
	delete(BI);
	delete(MI);
	delete(NI);
	return C;
}

RGBImage* ClassifyImage::prettyoverlayimage(RGBImage* C) {
	Image* BI = fourboundaryimage();
	Image* MI = markimage();
	Image* NI = new Image(width,height);
	C->overlay(MI,1,0,1,0.5);
	C->overlay(BI,0,0,0,0.6);
	C->overlay(NI,1,1,1,1.0);
	if(badimg) C->shadeby(BI);
	delete(BI);
	delete(MI);
	delete(NI);
	return C;
}

void ClassifyImage::loadarcgis(const string& ifname) {
	
	FILE *ifp = fopen(ifname.c_str(),"rb");
	char* linebuffer = (char*)malloc(50005*sizeof(char));
	char* wordptr;
	
	//get width,height
	assert(fgets(linebuffer,500,ifp));
	wordptr = strtok(linebuffer," ,\t=");
	wordptr = strtok(NULL," ,\t=");
	width=atoi(wordptr);
	assert(fgets(linebuffer,500,ifp));
	wordptr = strtok(linebuffer," ,\t=");
	wordptr = strtok(NULL," ,\t=");
	height=atoi(wordptr);
	size=width*height;
	printf("Importing ArcGIS ASCII file of size %i x %i (%i total)...",width,height,size);
	fflush(stdout);
	
	int ndata=0;
	int nlines=0;
	data = (int*)malloc(size*sizeof(int));
	
	while(fgets(linebuffer,50000,ifp) && ndata<size){
		nlines++;
		int* tbuf = (int*)malloc(4*sizeof(int));
		int ntbuf = 0;
		wordptr = strtok(linebuffer," ,\t\n\r");
		while(wordptr) {
			if(ntbuf<4)
			{
				tbuf[ntbuf++] = atoi(wordptr);
				wordptr = strtok(NULL," ,\t\n\r");
				continue;
			}
			if(ntbuf==4)
			{
				for(int q=0; q<4; q++) {
					if(ndata >= size) break;
					data[ndata++]=tbuf[q];
				}
				ntbuf++;
			}
			if(ndata >= size) break;
			data[ndata++] = atoi(wordptr);
			wordptr = strtok(NULL," ,\t\n\r");
		}
		if(wordptr) printf("### Warning: more data than indicated in file! ###\n");
		free(tbuf);
	}
	free(linebuffer);
	fclose(ifp);
	
	if(ndata<size) printf("### Warning: less data than indicated in file! ###\n");
	printf("Done.\n");
	name = ifname;
	coords.lx=0; coords.ux=width-1;
	coords.ly=0; coords.uy=height-1;
}

void ClassifyImage::writeArcGIS(const string& ofname) { //write text data to file
	FILE *ofp = fopen(ofname.c_str(), "w");
	fprintf(ofp,"ncols %i\nrows %i\nxllcorner 500\nyllcorner 500\ncellsize 500\nNODATA_value -9999",width,height);
	for(int y=0; y<height; y++) {
		fprintf(ofp,"\n");
		for(int x=0; x<width; x++) fprintf(ofp,"%i ",data[x+width*y]);
	}
	fclose(ofp);
}

void ClassifyImage::writeLowBitBMP(const string& ofname) //write BMP with low bits of data
{
	
	fprintf (stdout, "Saving binary image (%i x %i) to %s \n",width,height,ofname.c_str());
	bool* poodle = (bool*)malloc(width*sizeof(bool));
	bool* padzer = (bool*)calloc(64,sizeof(bool));
	FILE* ofp = fopen(ofname.c_str(),"wb");
	CratersBaseObject::writeBMPHeaders(ofp,1,width,height);
	CratersBaseObject::writeBMPmonoColorPalette(ofp);
	
	for(int i=height-1; i>=0; i--) //pad each line to 32-bit boundaries
	{
		for(int j=0; j<width; j++) poodle[j] = (bool)(data[j+width*i] & 0x01);
		CratersBaseObject::writeBinaryFromBool(poodle,width,ofp);
		int xtrabit = (8-width%8)%8;
		int npad = (4 - ((width+xtrabit)/8)%4)%4;
		CratersBaseObject::writeBinaryFromBool(padzer,8*npad,ofp);
	}
	fclose(ofp);
	free(padzer);
	free(poodle);
}

//////////////////////////////////////////////////

ClassifyDataScanner::ClassifyDataScanner(ClassifyImage* img, int x, int y):
myImg(img), si(img,x,y) { }

void ClassifyDataScanner::replacedata() {
	if(dat.size()==si.datp.size()) { for(size_t i=0; i<si.datp.size(); i++) myImg->data[si.datp[i]] = dat[i]; }
}

void ClassifyDataScanner::replacedata(int* d) {
	for(size_t i=0; i<si.datp.size(); i++) myImg->data[si.datp[i]] = d[i];
}

int ClassifyDataScanner::nextline() {
	dat.resize(si.nextline());
	for(size_t i=0; i<si.datp.size(); i++) dat[i]=myImg->data[si.datp[i]];
	return si.datp.size();
}

int ClassifyDataScanner::nextline(int* dput) {
	si.nextline();
	for(size_t i=0; i<si.datp.size(); i++) dput[i]=myImg->data[si.datp[i]];
	return si.datp.size();
}
