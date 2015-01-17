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

void Image::load(char *ifname) { //load raw float data
	FILE *ifp = fopen(ifname, "rb");
	fread (data, size, sizeof(float), ifp);
	coords.lx=0; coords.ux=width-1;
	coords.ly=0; coords.uy=height-1;
};

Image* Image::loadrawbinary(char *ifname, int w, int h, int nbits) { //load raw arbitrary bit depth (short) raster data
	FILE *ifp = fopen(ifname, "rb");
	Image* foo = Image::loadrawbinary(ifp,w,h,nbits,0,1);
	fclose(ifp);
	return foo;
};

Image* Image::loadrawbinary(FILE* ifp, int w, int h, int nbits, int offset, int every) {
	
	Image* foo = new Image(w,h);
	
	char abl=8;
	char dbl=nbits;
	unsigned long int accum=0;
	unsigned char fin=fgetc(ifp);
	int c=0;
	
	while(c<foo->size*every)
	{
		if(dbl >= abl) //add in and fetch next data chunk
		{
			accum += (long int)((unsigned char)(fin << 8-abl) >> 8-abl) << dbl-abl;
			dbl -= abl;
			fin = fgetc(ifp); abl=8;
			if(!dbl) {
				if((c-offset)%every == 0) foo->data[(c-offset)/every] = accum;
				++c;
				accum=0;
				dbl = nbits;
			}
		} else { //add in partial data chunk and save to image
			accum += ((unsigned char)(fin << 8-abl) >> 8-dbl);
			if((c-offset)%every == 0) foo->data[(c-offset)/every] = accum;
			accum=0; ++c;
			abl -= dbl;
			dbl = nbits;
		}
	}

	if(c<foo->size) printf("\n***Warning: file contained less data than indicated***\n");
	if(fread(&fin,1,1,ifp)) printf("\n***Warning: file contained more data than indicated***\n");
	return foo;
}

void Image::writeBMP(char *ofname) {
	
	fprintf (stdout, "Saving greyscale image (%i x %i) to %s \n",width,height,ofname);
	Image* foo;
	if(nmarks)
	{
		foo = normalized(0.2,1);
		foo->drawmarks(0.0);
	} else {
		foo = normalized(0,1);
	}
	
	float* padzer = (float*)calloc(3*width,sizeof(float));
	FILE* ofp = fopen(ofname,"wb");
	CratersBaseObject::writeBMPHeaders(ofp,8,width,height);
	CratersBaseObject::writeBMPgreyColorPalette(ofp);
	for(int i=height-1; i>=0; i--) //pad each line to 32-bit boundaries
	{
		CratersBaseObject::writeBinaryFromNormalizedFloat(foo->data+i*width,width,ofp,8);
		int npad = (4 - (width%4))%4;
		CratersBaseObject::writeBinaryFromNormalizedFloat(padzer,npad,ofp,8);
	}
	fclose(ofp);
	free(padzer);
	delete(foo);
};

void Image::writerawbinary(char* ofname, int nbits)
{
	FILE* ofp = fopen(ofname,"wb");
	writerawbinary(ofp, nbits);
	fclose(ofp);
}

void Image::writerawbinary(FILE* ofp, int nbits) {
	Image* foo = normalized(0,1.0);
	CratersBaseObject::writeBinaryFromNormalizedFloat(foo->data, size, ofp, nbits);
	delete(foo);
}


Image* Image::loadppm(char* ifname, int n) { //import channel n of a 3-channel ppm file
	
	FILE *ifp = fopen(ifname,"rb");
	
	char* linebuffer = (char*)malloc(1024*sizeof(char));
	char* wordptr;
	int itm = 0;
	int maxval;
	bool abort = false;
	int w=0;
	int h=0;
	
	while(itm < 4 && !abort && fgets(linebuffer,1000,ifp))
	{
		wordptr = strtok(linebuffer," ,\t\n\r");
		while(wordptr && !abort) {
			if(wordptr[0] == '#') {
				wordptr = strtok(NULL," ,\t\n\r");
				continue;
			}
			
			switch(itm)
			{
				case 0:
					if(strcmp(wordptr,"P6"))
					{
						printf("Aack! Not a PPM format file!\n");
						abort = true;
					}
					printf("Importing from PPM ");
					itm++;
					break;
				case 1:
					w = atoi(wordptr); itm++; printf("(%i x",w); break;
				case 2:
					h = atoi(wordptr); itm++; printf(" %i)",h); fflush(stdout); break;
				case 3:
					{
						maxval = atoi(wordptr);
						if(maxval > 255) maxval = 16;
						else maxval = 8;
						printf(" [%i-bit]",maxval);
						fflush(stdout);
						itm++; 
						break;
					}
			}
			wordptr = strtok(NULL," ,\t\n\r");
		}
	}
	
	free(linebuffer);
	if(abort) {
		fclose(ifp);
		return new Image(0,0);
	}
	
	
	Image* foo;
	
	
	switch(n)
	{
		case 0: foo = Image::loadrawbinary(ifp,w,h,maxval,0,3); sprintf(foo->name,"%s R",ifname); break;
		case 1: foo = Image::loadrawbinary(ifp,w,h,maxval,1,3); sprintf(foo->name,"%s G",ifname); break;
		case 2: foo = Image::loadrawbinary(ifp,w,h,maxval,2,3); sprintf(foo->name,"%s B",ifname); break;
	}
	
	fclose(ifp);
	printf(" Done.\n");
	return foo;
};

void Image::loadtexttable(char* ifname) {
	
	FILE *ifp = fopen(ifname,"rb");
	int nlines=0;
	int maxnlines=1;
	int* nitems = (int*)malloc(maxnlines*sizeof(int));
	float** itms = (float**)malloc(maxnlines*sizeof(float*));
	
	
	nlines=0;
	height=0;
	width=0;
	char* linebuffer = (char*)malloc(50005*sizeof(char));
	char* wordptr;
	
	while(fgets(linebuffer,50000,ifp)){
		
		//keep buffer up to size
		if(nlines>=maxnlines) {
			maxnlines+=50;
			itms = (float**)realloc(itms,maxnlines*sizeof(float*));
			nitems = (int*)realloc(nitems,maxnlines*sizeof(int*));
		}
		
		nitems[nlines]=0;
		int maxnwords=5;
		itms[nlines] = (float*)malloc(maxnwords*sizeof(float));
		wordptr = strtok(linebuffer," ,\t");
		while(wordptr) {
			//keep buffer up to size
			if(nitems[nlines]>=maxnwords) {
				maxnwords+=100;
				itms[nlines]=(float*)realloc(itms[nlines],maxnwords*sizeof(float));
			}
			
			itms[nlines][nitems[nlines]] = atof(wordptr);
			wordptr = strtok(NULL," ,\t");
			nitems[nlines]++;
		}
		
		if(nitems[nlines]>width) {
			width=nitems[nlines];
			height=0;
		}
		if(nitems[nlines]==width) height++;
		itms[nlines]=(float*)realloc(itms[nlines],nitems[nlines]*sizeof(float));
		nlines++;
	}
	free(linebuffer);
	
	fclose(ifp);
	size=width*height;
	printf("Imported image file of size %i x %i (%i total).\n",width,height,size);
	data = (float*)malloc(size*sizeof(float));
	float* ptr = data;
	for(int i=0; i<nlines; i++){
		if(nitems[i] == width) {
			memcpy(ptr,itms[i],width*sizeof(float));
			ptr += width;
		}
		free(itms[i]);
	}
	free(nitems);
	free(itms);
	sprintf(name,ifname);
	coords.lx=0; coords.ux=width-1;
	coords.ly=0; coords.uy=height-1;
};

void Image::loadarcgis(char* ifname) {
	
	FILE *ifp = fopen(ifname,"rb");
	char* linebuffer = (char*)malloc(50005*sizeof(char));
	char* wordptr;
	
	//get width,height
	fgets(linebuffer,500,ifp);
	wordptr = strtok(linebuffer," ,\t=");
	wordptr = strtok(NULL," ,\t=");
	width=atoi(wordptr);
	fgets(linebuffer,500,ifp);
	wordptr = strtok(linebuffer," ,\t=");
	wordptr = strtok(NULL," ,\t=");
	height=atoi(wordptr);
	size=width*height;
	printf("Importing ArcGIS ASCII file of size %i x %i (%i total)...",width,height,size);
	fflush(stdout);
	
	int ndata=0;
	int nlines=0;
	data = (float*)malloc(size*sizeof(float));
	
	while(fgets(linebuffer,50000,ifp) && ndata<size){
		nlines++;
		float* tbuf = (float*)malloc(4*sizeof(float));
		int ntbuf = 0;
		wordptr = strtok(linebuffer," ,\t\n\r");
		while(wordptr) {
			if(ntbuf<4)
			{
				tbuf[ntbuf++] = atof(wordptr);
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
			data[ndata++] = atof(wordptr);
			wordptr = strtok(NULL," ,\t\n\r");
		}
		if(wordptr) printf("### Warning: more data than indicated in file! ###\n");
		free(tbuf);
	}
	free(linebuffer);
	fclose(ifp);
	
	if(ndata<size) printf("### Warning: less data than indicated in file! ###\n");
	printf("Done.\n",width,height,size);
	sprintf(name,ifname);
	coords.lx=0; coords.ux=width-1;
	coords.ly=0; coords.uy=height-1;
};

void Image::write(char *ofname) { //write raw float data
	FILE *ofp = fopen (ofname, "wb");
	fwrite (data, size, sizeof(float), ofp);
};

void Image::writeArcGIS(char *ofname) { //write text data to file
	FILE *ofp = fopen (ofname, "w");
	fprintf(ofp,"ncols %i\nrows %i\nxllcorner 500\nyllcorner 500\ncellsize 500\nNODATA_value -9999",width,height);
	for(int y=0; y<height; y++) {
		fprintf(ofp,"\n");
		for(int x=0; x<width; x++) fprintf(ofp,"%f ",data[x+width*y]);
	}
	fclose(ofp);
};

//write data to a raw PGM format file (integers in range 0-65535)
void Image::writePGM2(char *ofname) 
{
	Image* foo = rec709gamma()->mult(65535);
	FILE *ofp = fopen (ofname, "w");
	fprintf(ofp,"P5\n%i\n%i\n65535\n",width,height);
	char* idat = (char*)malloc(2*size);
	for(int i=0; i<size; i++) {
		idat[2*i] = (char)((int)foo->data[i]/256);
		idat[2*i+1] = (char)((int)foo->data[i]%256);
	}
	fwrite(idat,2,size,ofp);
	delete(foo);
	free(idat);
	fclose(ofp);
};

//write data to a raw PGM format file (integers in range 0-255)
void Image::writePGM1(char *ofname) 
{
	Image* foo = rec709gamma()->mult(255);
	FILE *ofp = fopen (ofname, "w");
	fprintf(ofp,"P5\n%i\t%i\n255\n",width,height);
	char* idat = (char*)malloc(size);
	for(int i=0; i<size; i++) {
		idat[i] = (char)((int)foo->data[i]);
	}
	fwrite(idat,1,size,ofp);
	delete(foo);
	free(idat);
	fclose(ofp);
};

void Image::dumpcatalog(char *outpath) //dump sepatate image files for each item in catalog
{
	if(!mycatalog) return;
	char* fname = (char*)malloc(1024*sizeof(char));
	for(int i=0; i<mycatalog->ncraters; i++) {
		Image* foo = getsubregion(i,2.0);
		sprintf(fname,outpath,i);
		printf("Saving %s ...\n",fname);
		foo->writeArcGIS(fname);
		delete(foo);
	}
}