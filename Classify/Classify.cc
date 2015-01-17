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

#include "Classify.hh"
#include "Utils.hh"

SparseInt::SparseInt(int w, int h)
{
	using namespace std;
	width=w;
	height=h;
	rows = (vector<int>**)malloc(h*sizeof(vector<int>*));
	rowdat = (vector<int>**)malloc(h*sizeof(vector<int>*));
	for(int i=0; i<height; i++)
	{
		rows[i] = new vector<int>;
		rows[i]->reserve(10);
		rows[i]->push_back(-1); rows[i]->push_back(width+1);
		rowdat[i] = new vector<int>;
		rowdat[i]->reserve(10);
		rowdat[i]->push_back(0); rowdat[i]->push_back(0);
	}
}

SparseInt::~SparseInt()
{
	for(int i=0; i<height; i++)
	{
		delete(rows[i]);
		delete(rowdat[i]);
	}
	free(rows);
	free(rowdat);
}

int SparseInt::get(int i,int j)
{
	if(i>=width || j>=height) return 0;
	//slow dumb search for dat
	for(int x=0; x < rows[j]->size(); x++)
	{
		if((*rows[j])[x] == i) return (*rowdat[j])[x];
		if((*rows[j])[x] > i) return 0;
	}
	return 0;
}

void SparseInt::disprow(int row)
{
	printf("{\t");
	for(int i=0; i<rows[row]->size(); i++) printf("%i\t",(*rows[row])[i]);
	printf("}\n");
	printf("{{\t");
	for(int i=0; i<rows[row]->size(); i++) printf("%i\t",(*rowdat[row])[i]);
	printf("}}\n");
}

int SparseInt::columnlist(int row, int*& output)
{
	//disprow(row);
	output = (int*)malloc((rows[row]->size()-2)*sizeof(int));
	for(int i=1; i<rows[row]->size()-1; i++) output[i-1] = (*rows[row])[i];
	return rows[row]->size()-2;
}

int SparseInt::columnvals(int row, int*& output)
{
	output = (int*)malloc((rowdat[row]->size()-2)*sizeof(int));
	for(int i=1; i<rowdat[row]->size()-1; i++) output[i-1] = (*rowdat[row])[i];
	return rowdat[row]->size()-2;
}

void SparseInt::set(int i,int j, int v)
{
	if(i>=width || j>=height) return;
	//slow dumb search for dat
	for(int x=0; x < rows[j]->size(); x++)
	{
		if((*rows[j])[x] == i) {
			if(v) {
				(*rowdat[j])[x] = v;
				return;
			}
			rows[j]->erase(rows[j]->begin()+x);
			rowdat[j]->erase(rowdat[j]->begin()+x);
			return;
		}
		if((*rows[j])[x] > i) {
			if(!v) return;
			rows[j]->insert(rows[j]->begin()+x,i);
			rowdat[j]->insert(rowdat[j]->begin()+x,v);
			return;
		}
	}
}

//------------------------------------------------

CraterPoints::CraterPoints()
{
	nfloorpts=0; floorpts=NULL;
	nrimpts=0; rimpts=NULL;
}

CraterPoints::~CraterPoints()
{
	if(floorpts) free(floorpts);
	floorpts=NULL;
	if(rimpts) free(rimpts);
	rimpts=NULL;
}

void CraterPoints::addfloor(int* d, int n)
{
	if(!floorpts) floorpts = (int*)malloc(n*sizeof(int));
	else floorpts = (int*)realloc(floorpts,(nfloorpts+n)*sizeof(int));
	for(int i=0; i<n; i++) floorpts[nfloorpts+i]=d[i];
	nfloorpts+=n;
}

void CraterPoints::addrim(int* d, int n)
{
	if(!rimpts) rimpts = (int*)malloc(n*sizeof(int));
	else rimpts = (int*)realloc(rimpts,(nrimpts+n)*sizeof(int));
	for(int i=0; i<n; i++) rimpts[nrimpts+i]=d[i];
	nrimpts+=n;
}


//------------------------------------------------

ClassifyImage::ClassifyImage(int w, int h) : RectRegion(w,h)
{
	initialize();
};

ClassifyImage::ClassifyImage(RectRegion* R) : RectRegion(R->width,R->height)
{
	copyfromrr(R);
	initialize();
};

void ClassifyImage::initialize() {
	sprintf(isaName,"ClassifyImage");
	isaNum = COBJ_CLASSIFYIMAGE;
	isclassified = false;
	data = (int*)calloc(size,sizeof(int));
	shift = 0;
	underlying = NULL;
	hasboundaries=false;
	hasconnectivity=false;
	badimg = false;
	npic = NULL; //number of points in each class
	pic = NULL; //pointers in each class
	nbasins = 0;
	nbounds = NULL;
	bounds = NULL;
	cg = NULL;
	markedregion = NULL;
	poi = NULL; npoi=0;
	stats = NULL;
};

ClassifyImage::~ClassifyImage()
{
	free(npic); npic=NULL;
	for(int i=0; i<nbasins; i++) {
		if(pic) {if(pic[i]) {free(pic[i]); pic[i]=NULL;}}
		if(hasboundaries) {free(bounds[i]); bounds[i]=NULL;}
	}
	free(pic); pic=NULL;
	if(cg) {delete(cg); cg=NULL;}
	if(markedregion) free(markedregion);
	if(hasboundaries) {
		free(bounds); bounds=NULL;
		free(nbounds); nbounds=NULL;
	}
};

ClassifyImage* ClassifyImage::copy() //return a copy of this image
{
	ClassifyImage *foo = new ClassifyImage((RectRegion*)this);
	for(int i=0; i<size; i++) foo->data[i] = data[i];
	foo->isclassified = isclassified;
	foo->shift = shift;
	foo->nbasins = nbasins;
	foo->npic = (int*)realloc(foo->npic,nbasins*sizeof(int));
	foo->pic = (int**)realloc(foo->pic,nbasins*sizeof(int*));
	for(int i=0; i<nbasins; i++)
	{
		foo->npic[i]=npic[i];
		foo->pic[i] = (int*)malloc(npic[i]*sizeof(int));
		memcpy(foo->pic[i],pic[i],npic[i]*sizeof(int));
	}
	return foo;
};

//xor cdata values for specified points
void ClassifyImage::xorPoints(int* d, unsigned int n, unsigned int xorkey)
{
	for(int i=0; i<n; i++) data[d[i]] ^= xorkey;
}

//xor cdata values in specified region
void ClassifyImage::xorRegion(unsigned int n, unsigned int xorkey)
{
	if(n>=nbasins) return;
	xorPoints(pic[n],npic[n],xorkey);
}

//and cdata values for specified points
void ClassifyImage::andPoints(int* d, unsigned int n, unsigned int andkey)
{
	for(int i=0; i<n; i++) data[d[i]] &= andkey;
}

//and cdata values in specified region
void ClassifyImage::andRegion(unsigned int n, unsigned int andkey)
{
	if(n>=nbasins) return;
	andPoints(pic[n],npic[n],andkey);
}

ClassifyImage* ClassifyImage::upShift(int nbits) {
	for(int i=0; i<size; i++) data[i] = ((data[i]/(1<<shift)) << nbits) + (data[i]%(1<<shift));
	shift = nbits;
	return this;
}

void ClassifyImage::findObjectsByLowBits(int nbits) {
	if(npic) free(npic);
	for(int i=0; i<nbasins; i++) {if(pic[i]) free(pic[i]);}
	if(pic) free(pic);
	
	shift = nbits;
	nbasins=0;
	npic = (int*)malloc(sizeof(int));
	pic = (int**)malloc(sizeof(int*));
	unsigned int bitmask = 0xFFFFFFFF << shift;
	
	int* nudata = (int*)malloc(size*sizeof(int));
	memset(nudata,0xFF,size*sizeof(int));
	
	for(int i=0; i<size; i++) {
		if(nudata[i] != 0xFFFFFFFF) continue; //already been claimed
		npic = (int*)realloc(npic,(nbasins+1)*sizeof(int));
		pic = (int**)realloc(pic,(nbasins+1)*sizeof(int*));
		npic[nbasins] = seedFillByMaskedBits(i,&(pic[nbasins]),(1<<nbits)-1,bitmask,nbasins<<shift,nudata);
		nbasins++;
	}
	free(data); data=nudata;
	isclassified = true;
}

void ClassifyImage::findObjectsByHigherBits(int nbits) {
	if(npic) free(npic);
	for(int i=0; i<nbasins; i++) {if(pic[i]) free(pic[i]);}
	if(pic) free(pic);
	
	shift = nbits;
	nbasins=0;
	npic = (int*)malloc(sizeof(int));
	pic = (int**)malloc(sizeof(int*));
	unsigned int bitmask = 0xFFFFFFFF << shift;
	
	int* nudata = (int*)malloc(size*sizeof(int));
	memset(nudata,0xFF,size*sizeof(int));
	
	for(int i=0; i<size; i++) {
		if(nudata[i] != 0xFFFFFFFF) continue; //already been claimed
		npic = (int*)realloc(npic,(nbasins+1)*sizeof(int));
		pic = (int**)realloc(pic,(nbasins+1)*sizeof(int*));
		npic[nbasins] = seedFillByMaskedBits(i,&(pic[nbasins]),bitmask,bitmask,nbasins<<shift,nudata);
		nbasins++;
	}
	free(data); data=nudata;
	isclassified = true;
}

int ClassifyImage::seedFillByMaskedBits(int startp, int** pout, int searchmask, int setmask, int setnum, int* nudata) {
	
	using namespace std;
	setnum &= setmask; //make sure we don't set unmasked bits
	unsigned int ptype = data[startp] & searchmask; //point type we are searching for
	nudata[startp] = (data[startp] & ~setmask) | setnum; //mark starting point
	
	vector <int> points;
	vector <int> allpoints;
	points.push_back(startp);
	
	while(points.size()) {
		int p = points.back();
		points.pop_back();
		allpoints.push_back(p);
		int x0=p%width;
		int y0=p/width;
		for(int q=0; q<connectn; q++)
		{
			if(q==4) continue;
			int dx = connectdx[q];
			int dy = connectdy[q];
			if(!inrange(x0+dx,y0+dy)) continue;
			
			int p1 = x0+dx+width*(y0+dy);
			if(nudata[p1] != 0xFFFFFFFF) continue; //someone got there first
			if((data[p1] & searchmask) != ptype) continue; //not of same type
			
			points.push_back(p1);
			nudata[p1] = (data[p1] & ~setmask) | setnum;
		}
	}
	
	*pout = (int*)malloc(allpoints.size()*sizeof(int));
	int i=0;
	while(allpoints.size()) {
		(*pout)[i++]=allpoints.back();
		allpoints.pop_back();
	}
	return i;
}

void ClassifyImage::calcstats() {
	
	if(stats) {
		for(int i=0; i<nbasins; i++) {if(stats[i]) free(stats[i]);}
		free(stats);
	}
	
	stats = (BasinStat**)malloc(nbasins*sizeof(BasinStat*));
	
	for(int i=0; i<nbasins; i++) {
		
		stats[i] = new BasinStat;
		stats[i]->idnum = i;
		stats[i]->npic = npic[i];
		stats[i]->xsum=0;
		stats[i]->xxsum=0;
		stats[i]->ysum=0;
		stats[i]->yysum=0;
		
		int p;
		float x,y;
		for(int j=0; j<npic[i]; j++) {
			p = pic[i][j];
			x=(float)(p%width); y=(float)(p/width);
			stats[i]->xsum += x;
			stats[i]->xxsum += x*x;
			stats[i]->ysum += y;
			stats[i]->yysum += y*y;
		}

	}
	
	printf(" Done.\n");
}

Pointset* ClassifyImage::mutualboundary(int a, int b) {
	Pointset* ps = new Pointset();
	ps->additems(bounds[a],nbounds[a]);
	Pointset bbound = Pointset();
	bbound.additems(bounds[b],nbounds[b]);
	ps->intersect(&bbound);
	return ps;
}

void ClassifyImage::cleardata() {
	
	printf("Clearing outdated data...");
	fflush(stdout);
	
	if(nbounds) {free(nbounds); nbounds=NULL;}
	if(bounds) {
		for(int i=0; i<nbasins; i++) {free(bounds[i]); bounds[i]=NULL;}
		free(bounds); bounds=NULL;
	}
	
	if(stats) {
		for(int i=0; i<nbasins; i++) if(stats[i]) delete(stats[i]);
		free(stats); stats=NULL;
	}
	if(markedregion) {free(markedregion); markedregion=NULL;}
	
	if(cg) {delete(cg); cg=NULL;}
}

void ClassifyImage::connectivitygraph(){
	
	if(hasconnectivity) return;
	
	printf("Determining connectivity graph... ");
	fflush(stdout);
	cg = new SparseInt(nbasins,nbasins);
	
	for(int i=0; i<size; i++){
		Pointset* foo = boundswho(i);
		for(int j=0; j<foo->nitems; j++) cg->set(data[i] >> shift,foo->getitem(j),1);
		delete(foo);
	}
	
	for(int i=0; i<nbasins; i++) cg->set((int)i,i,0);
	
	hasconnectivity=true;
	printf("Done.\n");
}


void ClassifyImage::joinregions(int a, int b, bool dobounds) { //merge region b into a
	
	if(!hasboundaries && dobounds) findboundaries();
	if(a>=nbasins || b>=nbasins || a==b) return; //oops! one of the basins doesn't exist
	
	//assign points to correct class
	pic[a]=(int*)realloc(pic[a],(npic[a]+npic[b])*sizeof(int));
	for(int i=0; i<npic[b]; i++) {
		pic[a][npic[a]+i]=pic[b][i];
		data[pic[b][i]] = (a << shift) + (data[pic[b][i]] & (1<<shift)-1); //preserve b low bits
	}
	npic[a]=npic[a]+npic[b];
	npic[b]=0;
	
	//fix RAG: connect b's neighbors to a
	int* connectedrs;
	int* connectedvs;
	int nconnected = cg->columnlist(b,connectedrs);
	cg->columnvals(b,connectedvs);
	for(int k=0; k<nconnected; k++) {
		int i = connectedrs[k];
		int s = connectedvs[k];
		cg->set(a,i,max(s,cg->get(a,i)));
		cg->set(i,a,max(s,cg->get(a,i)));
		cg->set(i,b,0);
		cg->set(b,i,0);
	}	
	cg->set(a,b,0);
	cg->set(b,a,0);
	free(connectedrs); free(connectedvs);
	
	//fix basin statistics
	if(stats[b]->basinmin < stats[a]->basinmin) stats[a]->basinmin=stats[b]->basinmin;
	stats[a]->xsum += stats[b]->xsum;
	stats[a]->xxsum += stats[b]->xxsum;
	stats[a]->ysum += stats[b]->ysum;
	stats[a]->yysum += stats[b]->yysum;
	stats[a]->zsum += stats[b]->zsum;
	stats[a]->zzsum += stats[b]->zzsum;
	
	if(!dobounds) return;
	
	//fix boundary
	bool* isbound = (bool*)calloc(size,sizeof(bool));
	for(int i=0; i<nbounds[a]; i++) isbound[bounds[a][i]]=true;
	for(int i=0; i<nbounds[b]; i++) {
		Pointset* f = boundswho(bounds[b][i]);
		if(f->nitems >= 2) isbound[bounds[b][i]]=true; //it's a boundary
		else isbound[bounds[b][i]]=false;
		delete(f);
	}
	
	int* newbnds = (int*)malloc(size*sizeof(int));
	int nnewbnds=0;
	for(int i=0; i<nbounds[a]; i++) {
		if(isbound[bounds[a][i]]) {
			newbnds[nnewbnds]=bounds[a][i];
			nnewbnds++;
			isbound[bounds[a][i]]=false;
		}
	}
	for(int i=0; i<nbounds[b]; i++) {
		if(isbound[bounds[b][i]]) {
			newbnds[nnewbnds]=bounds[b][i];
			nnewbnds++;
		}
	}
	free(isbound);
	free(bounds[b]); bounds[b]=NULL;
	nbounds[b]=0;
	free(bounds[a]);
	bounds[a]=(int*)realloc(newbnds,nnewbnds);
	nbounds[a]=nnewbnds;
}

Pointset* ClassifyImage::boundswho(int p) { //check which regions a point is a boundary for
	
	Pointset* bounders = new Pointset();
	int x=p%width;
	int y=p/width;
	
	for(int i=0; i<connectn; i++) {
		int j=x+connectdx[i];
		if(j == -1 || j == width) continue; //edge
		int k=y+connectdy[i];
		if(k == -1 || k == height || connectr2[i]==0) continue; //edge or center 
		bounders->additem(data[j+width*k] >> shift);
	}
	
	return bounders;
}

void ClassifyImage::findboundaries() {
	
	if(hasboundaries) return;
	
	printf("Finding classification boundaries...");
	fflush(stdout);
	
	nbounds=(int*)calloc(nbasins,sizeof(int));
	bounds=(int**)malloc(nbasins*sizeof(int*));
	
	Pointset** adjacents = (Pointset**)malloc(size*sizeof(Pointset*));
	
	//count the boundaries
	for(int i=0; i<size; i++) {
		//adjacents[i]=boundswho(i,true);
		adjacents[i] = boundswho(i);
		if(adjacents[i]->nitems>=2) { //it's a boundary!
			for(int j=0; j<adjacents[i]->nitems; j++) nbounds[adjacents[i]->getitem(j)]++;
		}
	}
	
	for(int m=0; m<nbasins; m++) bounds[m]=(int*)malloc(nbounds[m]*sizeof(int));
	int* tnbounds=(int*)calloc(nbasins,sizeof(int));
	
	for(int i=0; i<size; i++) {
		if(adjacents[i]->nitems >=2) { //it's a boundary!
			for(int j=0; j<adjacents[i]->nitems; j++) {
				int qqq = adjacents[i]->getitem(j);
				int rrr = tnbounds[qqq];
				bounds[qqq][rrr]=i;
				tnbounds[qqq]++;
			}
		}
	}
	
	for(int i=0; i<size; i++) delete(adjacents[i]);
	free(adjacents); adjacents=NULL;
	free(tnbounds); tnbounds=NULL;
	hasboundaries=true;
	calcstats();
	printf("Done.\n");
};

union bstofarr
{
	BasinStat b;
	float a[14];
};

void ClassifyImage::settempstat(unsigned int n)
{
	if(n==0 || n>14) return;
	bstofarr q;
	for(int k=0; k<nbasins; k++)
	{
		q.b = *stats[k];
		stats[k]->temp = q.a[n-1];
	}
}

void ClassifyImage::circularity(){ //calculate circularity of each region
	if(!stats) calcstats();
	for(int i=0; i<nbasins; i++) {
		Circle c = findboundingcirc(pic[i],npic[i]);
		stats[i]->temp = npic[i]/(3.14159*c.r*c.r);
	}
}

void ClassifyImage::random(){ //assign random number in [0,1] to temp stat
	if(!stats) calcstats();
	for(int i=0; i<nbasins; i++) {
		stats[i]->temp = (float)rand()/((float)RAND_MAX);
	}
}

/* void ClassifyImage::angularity(){ //calculate circularity of each region
	if(!stats) calcstats();
	for(int i=0; i<nbasins; i++) {
		if(npic[i]>1000) {stats[i]->temp = 1.5; continue;}
		ClassifyImage* foo = extractbinarychunkmask(i,15);
		Circle c = findboundingcirc(pic[i],npic[i]);
		ClassifyImage* bar = foo->circleopening((int)(1+0.5*c.r));
		float np1 = 0;
		for(int q=0; q<bar->size; q++) np1 += (bar->data[q] & 0x1);
		delete(bar);
		
		ClassifyImage* baz = foo->circleclosing((int)(1+0.5*c.r));
		float np2=0;
		for(int q=0; q<baz->size; q++) np2 += (baz->data[q] & 0x1);
		delete(baz);
		delete(foo);

		stats[i]->temp = (float)(np2-np1)/(float)npic[i];
	}
} */

void ClassifyImage::markedregionstopoi() {
	if(poi) {free(poi); poi=NULL;}
	npoi=0;
	for(int i=0; i<nbasins; i++){
		if(markedregion[i]) {
			addregiontopoi(i);
		}
	}
}

void ClassifyImage::labelboundaries(int c) {
	if(!hasboundaries) findboundaries();
	for(int i=0; i<nbasins; i++){
		for(int j=0; j<nbounds[i]; j++) {
			data[bounds[i][j]] = c << shift;
		}
	}
};

void ClassifyImage::renumerate() 
{
	printf("Re-enumerating basins: Freeing old data;");
	fflush(stdout);
	
	if(npic != NULL) {free(npic); npic=NULL;}
	
	if(pic != NULL) {
		for(int i=0; i<nbasins; i++) {if(pic[i] != NULL) {free(pic[i]); pic[i]=NULL;}}
		free(pic); pic=NULL;
	}
	
	printf(" renumbering;");
	fflush(stdout);
	
	npic = (int*)calloc(size,sizeof(int));
	int* renum = (int*)malloc(size*sizeof(int));
	for(int i=0; i<size; i++) renum[i]=-1;
	nbasins=0;
	for(int i=0; i<size; i++){
		if(renum[data[i] >> shift]!=-1) { 
			data[i] = (renum[data[i] >> shift] << shift) + (data[i] & (1<<shift)-1);
			npic[data[i] >> shift]++;
			continue; 
		}
		renum[data[i] >> shift] = nbasins;
		data[i] = (nbasins << shift) + (data[i] & (1<<shift)-1);
		npic[nbasins++]++;
	}
	free(renum); renum=NULL;
	npic = (int*)realloc(npic,nbasins*sizeof(int));
	
	printf(" %i basins found. Creating point class lists;",nbasins);
	fflush(stdout);
	
	//create point class lists
	int* npict = (int*)calloc(nbasins,sizeof(int));
	pic = (int**)malloc(nbasins*sizeof(int*));
	for(int i=0; i<nbasins; i++) pic[i] = (int*)malloc(npic[i]*sizeof(int));
	for(int i=0; i<size; i++) pic[data[i]>>shift][npict[data[i]>>shift]++]=i;
	free(npict); npict=NULL;
	
	printf(" Done.\n");
	
	//deal with boundaries, connectivity
	hasboundaries=false; //this messes up the boundary lists... could fix, but easy to recompute
	hasconnectivity=false;
	cleardata();
	markedregion = (bool*)calloc(nbasins,sizeof(bool));
};

void ClassifyImage::renumerateWithKey(int andkey) 
{
	printf("Re-enumerating basins: Freeing old data;");
	fflush(stdout);
	
	if(npic != NULL) {free(npic); npic=NULL;}
	
	if(pic != NULL) {
		for(int i=0; i<nbasins; i++) {if(pic[i] != NULL) {free(pic[i]); pic[i]=NULL;}}
		free(pic); pic=NULL;
	}
	
	printf(" renumbering;");
	fflush(stdout);
	
	npic = (int*)calloc(size,sizeof(int));
	int* renum = (int*)malloc(size*sizeof(int));
	for(int i=0; i<size; i++) renum[i]=-1;
	nbasins=1;
	
	
	for(int i=0; i<size; i++){
		if(!(data[i] & andkey))
		{
			data[i] = 0x0 + (data[i] & (1<<shift)-1);
			npic[0]++;
			continue;
		}
		if(renum[data[i] >> shift]!=-1) { 
			data[i] = (renum[data[i] >> shift] << shift) + (data[i] & (1<<shift)-1);
			npic[data[i] >> shift]++;
			continue; 
		}
		renum[data[i] >> shift] = nbasins;
		data[i] = (nbasins << shift) + (data[i] & (1<<shift)-1);
		npic[nbasins++]++;
	}
	free(renum); renum=NULL;
	npic = (int*)realloc(npic,nbasins*sizeof(int));
	
	printf(" %i basins found. Creating point class lists;",nbasins);
	fflush(stdout);
	
	//create point class lists
	int* npict = (int*)calloc(nbasins,sizeof(int));
	pic = (int**)malloc(nbasins*sizeof(int*));
	for(int i=0; i<nbasins; i++) pic[i] = (int*)malloc(npic[i]*sizeof(int));
	for(int i=0; i<size; i++) pic[data[i]>>shift][npict[data[i]>>shift]++]=i;
	free(npict); npict=NULL;
	
	printf(" Done.\n");
	
	//deal with boundaries, connectivity
	hasboundaries=false; //this messes up the boundary lists... could fix, but easy to recompute
	hasconnectivity=false;
	cleardata();
	markedregion = (bool*)calloc(nbasins,sizeof(bool));
};

void ClassifyImage::addregiontopoi(int n){
	if(!poi) {
		npoi = 0;
		poi=(int*)malloc(npic[n]*sizeof(int));
	} else poi=(int*)realloc(poi,(npoi+npic[n])*sizeof(int));
	for(int i=0; i<npic[n]; i++) poi[npoi+i]=pic[n][i];
	npoi+=npic[n];
};