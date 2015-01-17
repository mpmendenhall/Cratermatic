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

#include "Histogram.hh"

Histogram::Histogram(float* d, int size, float* weight, float min, float max, int nbins)
{
	n=nbins;
	wtot=0;
	binbounds = (float*)calloc(n+1,sizeof(float));
	count = (float*)calloc(n+2,sizeof(float)); //2 extra bins for above/below max/min
	int x;
	float delta=(max-min)/n;
	for(int i=0; i<n+1; i++) binbounds[i]=min+delta*(float)i;
	for(int z=0; z<size; z++) {
		x=1+(int)((d[z]-min)/delta);
		if(x<0) x=0;
		if(x>n) x=n+1;
		if(weight) { count[x]+=weight[z]; wtot+=weight[z]; }
		else { count[x]+=1; wtot+=1;}
	}
	
}

Histogram::~Histogram()
{
	free(binbounds);
	free(count);
};

int Histogram::maxbin() {
	int maxb=1;
	for(int i=0; i<n; i++) {
		if(count[i+1]>count[maxb]) maxb=i+1;
	}
	return maxb;
	
}

void Histogram::display()
{
	float maxc=count[maxbin()];
	printf("|\t\t<|\t%i\t|<%f\t|\n",(int)count[0],binbounds[0]);
	for(int i=0; i<n; i++) {
		printf("|%f\t<|\t%i\t|<%f\t|\t",binbounds[i],(int)count[i+1],binbounds[i+1]);
		for(int j=0; j<70*count[i+1]/maxc; j++) printf("*");
		printf("\n");
	}
	printf("|%f\t<|\t%i\t|<%t\t\t|\n",binbounds[n],(int)count[n+1]);
};

float Histogram::uniformity()
{
	float z=0;
	float mu=0;
	float tsum = 0;
	//find avg bin occupancy
	for(int i=0; i<n; i++) {mu += count[i+1]; tsum+=fabs(count[i+1]); }
	mu/=n;
	//find deviation from avg
	for(int i=0; i<n; i++) z += fabs(count[i+1]-mu);
	z /= tsum;
	return -z;
};

float Histogram::coverage()
{
	float z=0;
	float mu=0;
	//find avg bin occupancy
	for(int i=0; i<n; i++) mu += count[i+1];
	mu/=n;
	for(int i=0; i<n; i++) if(count[i+1] >= 0.2*mu) z += 1.0;
	z /= n;
	return z;
};