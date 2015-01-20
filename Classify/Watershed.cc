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
#include "Utils.hh"
#include <climits>

ClassifyImage* ClassifyImage::gradSeg(Image* u, float r)
{
	Image* foo = u->deriv(true);
	Image* foosh = foo->smoothehtransform(r);
	delete(foo);
	
	Image* bar = u->deriv(false);
	Image* barsh = bar->smoothehtransform(r);
	delete(bar);
	
	barsh->quadratureadd(foosh);
	delete(foosh);
	
	ClassifyImage* w = ClassifyImage::watershed(barsh);
	delete(barsh);
	return w;
}

ClassifyImage* ClassifyImage::watershed(Image* u)
{
	ClassifyImage* CI = new ClassifyImage((RectRegion*)u);
	CI->underlying = u;
	printf("Calculating watershed transform ");
	ProgressBar* pb = new ProgressBar(40);
	
	int width = CI->width;
	int height = CI->height;
	int size = width*height;
	
	//find flow direction and boundary of flat regions
	//printf("Finding flow direction...\n");
	float zbest;
	int besti;
	int* fd = (int*)malloc(size*sizeof(int));
	for(int i=0; i<size; i++) fd[i]=-9;
	bool* df = (bool*)calloc(CI->size,sizeof(bool));
	int dx,dy;
	
	for(int x=0; x<width; x++) {
		for(int y=0; y<height; y++){
			zbest=u->data[x+width*y];
			besti = 4;
			for(int i=0; i<CI->connectn; i++) {
				dx = connectdx[i];
				dy = connectdy[i];
				if(x+dx<0 || x+dx>=width || y+dy<0 || y+dy>=height) continue; //edge
				if(u->data[x+dx+width*(y+dy)] < zbest) {
					zbest=u->data[x+dx+width*(y+dy)];
					besti = i;
				}
			}
			fd[x+width*y]=besti;
			if(besti == 4){
				//mark LOWER boundary points
				for(int i=0; i<CI->connectn; i++) {
					dx = connectdx[i];
					dy = connectdy[i];
					if(x+dx<0 || x+dx>=width || y+dy<0 || y+dy>=height) continue; //edge
					if(u->data[x+dx+width*(y+dy)] <= u->data[x+width*y]) {
						df[x+dx+width*(y+dy)]=true;
					}
				}
			}
		}
	}	
	pb->update(0.25);
	
	//find ownership of flat regions
	//printf("Classifying flat regions...\n");
	for(int i=0; i<size; i++) CI->data[i]=-1; //initialize to unclaimed
	int* distance = (int*)malloc(size*sizeof(int)); //how close they are
	for(int i=0; i<size; i++) distance[i]=INT_MAX;
	CI->watershedFloodplainFill(fd, distance);
	pb->update(0.5);
	
	//mark unowned flat regions
	//printf("Classifying unowned flat regions...\n");
	for(int x=0; x<width; x++){
		for(int y=0; y<height; y++){
			if(fd[x+width*y]==4 && CI->data[x+width*y]==-1) CI->watershedMinimaRecursor(x, y, x, y, fd);
		}
	}
	pb->update(0.75);
	
	//follow flow to terminus, ... and we're home free!
	//printf("Going with the flow...\n");
	for(int x=0; x<width; x++){
		for(int y=0; y<height; y++){
			if(CI->data[x+width*y]!=-1) continue; //already claimed
			CI->data[x+width*y]=CI->watershedFlowRecursor(x, y, fd);
		}
	}
	delete(pb);
	
	free(distance); distance=NULL;
	free(fd); fd=NULL;
	free(df); df=NULL;
	
	CI->renumerate();
	printf("Watershedding into %i basins complete.\n",CI->nbasins);
	return CI;
}

struct crawler {
	int pos;
	int own;
	short distance;
};

void ClassifyImage::watershedFloodplainFill(int* fd, int* distance)
{
	crawler* ocs = (crawler*)malloc(size*sizeof(crawler));
	crawler* ncs = (crawler*)malloc(size*sizeof(crawler));
	crawler* tcp;
	
	//set initial crawler list
	int ncrawls = 0;
	for(int i=0; i<size; i++)
	{
		if(fd[i] == 4) continue; //level point
		ocs[ncrawls].pos = i;
		ocs[ncrawls].own = i;
		ocs[ncrawls].distance = 0;
		ncrawls++;
	}
	
	int nnewcrawls = 0;
	int x,y,dx,dy,np;
	
	while(ncrawls)
	{
		nnewcrawls = 0;
		for(int i=0; i<ncrawls; i++)
		{
			if(distance[ocs[i].pos] < ocs[i].distance) continue; //undercut in previous step
			
			for(int k=0; k<9; k++)
			{
				if(k==4) continue; //standstill
				x = ocs[i].pos % width;
				y = ocs[i].pos / width;
				dx = connectdx[k];
				dy = connectdy[k];
				if(!inrange(x+dx,y+dy)) continue;
				
				np = ocs[i].pos + dx + width*dy;
				if( fd[np] == 4 && distance[np] > ocs[i].distance + connectr2[k] )
				{
					data[np] = ocs[i].own;
					distance[np] = ocs[i].distance + connectr2[k];
					ncs[nnewcrawls] = ocs[i];
					ncs[nnewcrawls].pos = np;
					ncs[nnewcrawls].distance += connectr2[k];
					nnewcrawls++;
				}
			}
		}
		ncrawls = nnewcrawls;
		tcp = ncs; ncs=ocs; ocs=tcp;
	}
	free(ocs); free(ncs); 
}

void ClassifyImage::watershedMinimaRecursor(int x, int y, int x0, int y0, int* fd) {
	
	data[x+width*y] = x0+width*y0;
	
	//branch out from here
	for(int i=0; i<connectn; i++) {
		int x1=x+connectdx[i];
		if(x1 == -1 || x1 == width) continue; //edge
		int y1=y+connectdy[i];
		if(y1 == -1 || y1 == height) continue; //edge
		if(fd[x1+width*y1]!=4) continue; //not a level point
		if(data[x1+width*y1]!=-1) continue; //already been claimed
		watershedMinimaRecursor(x1,y1,x0,y0,fd);
	}
}

int ClassifyImage::watershedFlowRecursor(int x, int y, int* fd) {
	if(data[x+width*y]!=-1) return data[x+width*y]; //already know way to terminus
	data[x+width*y]=watershedFlowRecursor(x+connectdx[fd[x+width*y]],y+connectdy[fd[x+width*y]],fd);
	return data[x+width*y];
}

/* ClassifyImage* ClassifyImage::subregions(ClassifyImage* w)
{
	ClassifyImage* foo = copy();
	Image* bi = w->boundaryimage();
	int classn=1;
	for(int i=0; i<nbasins; i++)
	{
		//check if all in same region of w
		int p;
		for(p=1; p<npic[i]; p++) if(bi->data[pic[i][p]] == 1) break;
		if(p == npic[i])
		{
			for(p=0; p<npic[i]; p++) foo->data[pic[i][p]] = classn;
			classn++;
		} else {
			for(p=0; p<npic[i]; p++) foo->data[pic[i][p]] = 0;
		}
	}
	delete(bi);
	sprintf(foo->name,"<%s> Subsets by <%s>",w->name,name); 
	foo->renumerate();
	return foo;
} */

/* Image* ClassifyImage::suppressminima(float t) 
{	
	int s=0;
	Image *foo = new Image(width,height);
	if(!hasboundaries) findboundaries();
	for(int i=0; i<nbasins; i++) {
		if(stats[i]->boundsmin-stats[i]->basinmin<=t) {
			for(int j=0; j<npic[i]; j++) {
				if(underlying->data[pic[i][j]]>stats[i]->boundsmin) foo->data[pic[i][j]]=underlying->data[pic[i][j]];
				else foo->data[pic[i][j]]=stats[i]->boundsmin;
			}
			s++;
			continue;
		}
		for(int j=0; j<npic[i]; j++) foo->data[pic[i][j]]=underlying->data[pic[i][j]];
	}
	printf("Suppressed %i minima.\n",s);
	return foo;
}; */

/* void ClassifyImage::plotminima() {
	float zbest;
	int pbest;
	for(int i=0; i<nbasins; i++) {
		zbest=FLT_MAX;
		for(int j=0; j<npic[i]; j++) {
			if(underlying->data[pic[i][j]]<zbest) {
				zbest=underlying->data[pic[i][j]];
				pbest=pic[i][j];
			}
		}
		addmark(0,pbest%width,pbest/width,4);
	}
} */

/* void ClassifyImage::calcflow(Image* u) {
	Image* dxu = u->deriv(true);
	Image* dyu = u->deriv(false);
	Image* dmag = dxu->copy();
	dmag->quadratureadd(dyu);
	//dxu->divide(dmag);
	//dyu->divide(dmag);
	
	boundaryflow = boundaryimage();
	
	for(int x=0; x<width; x++) {
		for(int y=0; y<height; y++) {
			if(!boundaryflow->data[x+width*y]) continue;
			boundaryflow->data[x+width*y]=0;
			for(int i=0; i<4; i++) //only consider 4-connected boundary
			{
				if(i==5) continue;
				int dx = connectdx[i];
				int dy = connectdy[i];
				if(x+dx<0 || x+dx>=width || y+dy<0 || y+dy>=height) continue; //off edge
				if(data[x+dx+width*(y+dy)] == data[x+width*y]) continue; //in same region
				boundaryflow->data[x+width*y] += (dy*dxu->data[x+width*y] - dx*dyu->data[x+width*y]); //perpendicular component
				//boundaryflow->data[x+width*y] += dy*(dxu->data[(x+dx)+width*(y+dy)] + dxu->data[x+width*y]) - dx*(dyu->data[(x+dx)+width*(y+dy)] + dyu->data[x+width*y]); //perpendicular component
			}
		}
	}
	delete(dxu); delete(dyu); delete(dmag);
} */

/* float ClassifyImage::regionflow(int i, int j){
	if(!boundaryflow) return 0;
	Pointset* p = mutualboundary(i,j);
	float x=0;
	float n=0;
	for(int k=0; k<p->nitems; k++) {
		if(data[p->getitem(k)] == i) x += boundaryflow->data[p->getitem(k)];
		else x -= boundaryflow->data[p->getitem(k)];
		if(boundaryflow->data[p->getitem(k)] != 0) n++;
	}
	delete(p);
	if(n<6) return 0;
	return x;
} */

/* float ClassifyImage::regionhomog(int i, int j){
	if(!boundaryhomog) return 0;
	Pointset* p = mutualboundary(i,j);
	float x=0;
	float n=p->nitems;
	for(int i=0; i<p->nitems; i++) x+=boundaryhomog->data[p->getitem(i)];
	delete(p);
	return x/n;
} */

/* void ClassifyImage::calchomog(Image* u) {
	Image* dxu = u->deriv(true);
	Image* dyu = u->deriv(false);
	Image* dmag = dxu->copy();
	dmag->quadratureadd(dyu);
	dxu->divide(dmag);
	dyu->divide(dmag);
	float n;
	boundaryhomog = boundaryimage();
	for(int x=0; x<width; x++) {
		for(int y=0; y<height; y++) {
			if(!boundaryhomog->data[x+width*y]) continue;
			boundaryhomog->data[x+width*y]=0;
			n=0;
			for(int i=0; i<4; i++) //only consider 4-connected boundary
			{
				int dx = connectdx[i];
				if(x+dx<0 || x+dx>=width) continue; //off edge
				int dy = connectdy[i];				
				if(y+dy<0 || y+dy>=height) continue; //off edge
				if(data[x+dx+width*(y+dy)] == data[x+width*y]) continue; //in same region
				n+=1.0;
				//boundaryhomog->data[x+width*y] += dxu->data[x+width*y]*dxu->data[x+dx+width*(y+dy)] + dyu->data[x+width*y]*dyu->data[x+dx+width*(y+dy)];
				boundaryhomog->data[x+width*y] += dx*dxu->data[x+dx+width*(y+dy)] + dy*dyu->data[x+dx+width*(y+dy)];
			}
			if(n) boundaryhomog->data[x+width*y] /= n;
		}
	}
	delete(dxu); delete(dyu); delete(dmag);
} */