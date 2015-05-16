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
#include "Utils.hh"
#include <cassert>
#include <map>
using std::map;
#include <string.h>

SparseInt::SparseInt(int w, int h) {
	using namespace std;
	width=w;
	height=h;
	rows = (vector<int>**)malloc(h*sizeof(vector<int>*));
	rowdat = (vector<int>**)malloc(h*sizeof(vector<int>*));
	for(int i=0; i<height; i++) {
		rows[i] = new vector<int>;
		rows[i]->reserve(10);
		rows[i]->push_back(-1); rows[i]->push_back(width+1);
		rowdat[i] = new vector<int>;
		rowdat[i]->reserve(10);
		rowdat[i]->push_back(0); rowdat[i]->push_back(0);
	}
}

SparseInt::~SparseInt() {
	for(int i=0; i<height; i++)
	{
		delete(rows[i]);
		delete(rowdat[i]);
	}
	free(rows);
	free(rowdat);
}

int SparseInt::get(int i,int j) {
	if(i>=width || j>=height) return 0;
	//slow dumb search for dat
	for(size_t x=0; x < rows[j]->size(); x++) {
		if((*rows[j])[x] == i) return (*rowdat[j])[x];
		if((*rows[j])[x] > i) return 0;
	}
	return 0;
}

void SparseInt::disprow(int row)
{
	printf("{\t");
	for(size_t i=0; i<rows[row]->size(); i++) printf("%i\t",(*rows[row])[i]);
	printf("}\n");
	printf("{{\t");
	for(size_t i=0; i<rows[row]->size(); i++) printf("%i\t",(*rowdat[row])[i]);
	printf("}}\n");
}

int SparseInt::columnlist(int row, int*& output) {
	//disprow(row);
	output = (int*)malloc((rows[row]->size()-2)*sizeof(int));
	for(size_t i=1; i<rows[row]->size()-1; i++) output[i-1] = (*rows[row])[i];
	return rows[row]->size()-2;
}

int SparseInt::columnvals(int row, int*& output) {
	output = (int*)malloc((rowdat[row]->size()-2)*sizeof(int));
	for(size_t i=1; i<rowdat[row]->size()-1; i++) output[i-1] = (*rowdat[row])[i];
	return rowdat[row]->size()-2;
}

void SparseInt::set(int i,int j, int v) {
	if(i>=width || j>=height) return;
	//slow dumb search for dat
	for(size_t x=0; x < rows[j]->size(); x++)
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

ClassifyImage::ClassifyImage(int w, int h) : RectRegion(w,h) {
	initialize();
}

ClassifyImage::ClassifyImage(RectRegion* R) : RectRegion(R->width,R->height) {
	copyfromrr(R);
	initialize();
}

void ClassifyImage::initialize() {
	isaName = "ClassifyImage";
	isaNum = COBJ_CLASSIFYIMAGE;
	data.resize(size);
}

ClassifyImage::~ClassifyImage() {
	if(cg) { delete(cg); cg=NULL; }
}

ClassifyImage* ClassifyImage::copy() {
	ClassifyImage *foo = new ClassifyImage((RectRegion*)this);
	foo->data = data;
	foo->isclassified = isclassified;
	foo->shift = shift;
	foo->pic = pic;
    foo->bounds = bounds;
	return foo;
}

void ClassifyImage::xorPoints(vector<int>& d, unsigned int xorkey) {
	for(auto it = d.begin(); it != d.end(); it++) data[*it] ^= xorkey;
}


void ClassifyImage::xorRegion(unsigned int n, unsigned int xorkey) {
	if(n >= pic.size()) return;
	xorPoints(pic[n],xorkey);
}

void ClassifyImage::andPoints(vector<int>& d, unsigned int andkey) {
    for(auto it = d.begin(); it != d.end(); it++) data[*it] &= andkey;
}

void ClassifyImage::andRegion(unsigned int n, unsigned int andkey) {
	if(n >= pic.size()) return;
	andPoints(pic[n], andkey);
}

ClassifyImage* ClassifyImage::upShift(int nbits) {
	for(int i=0; i<size; i++) data[i] = ((data[i]/(1<<shift)) << nbits) + (data[i]%(1<<shift));
	shift = nbits;
	return this;
}

void ClassifyImage::findObjectsByLowBits(int nbits) {
	pic.clear();
	
	shift = nbits;
	unsigned int bitmask =   0xFFFFFFFF << shift;
	vector<int> nudata(size, 0xFFFFFFFF);
	for(int i=0; i<size; i++) {
		if(nudata[i] != 0xFFFFFFFF) continue; //already been claimed
        pic.push_back(vector<int>());
		seedFillByMaskedBits(i, pic.back(), (1<<nbits)-1, bitmask, (pic.size()-1) << shift, nudata);
	}
	data = nudata;
	isclassified = true;
}

void ClassifyImage::findObjectsByHigherBits(int nbits) {
	pic.clear();
	
	shift = nbits;
    unsigned int bitmask =   0xFFFFFFFF << shift;
	vector<int> nudata(size, 0xFFFFFFFF);
	
	for(int i=0; i<size; i++) {
		if(nudata[i] != 0xFFFFFFFF) continue; //already been claimed
        pic.push_back(vector<int>());
		seedFillByMaskedBits(i, pic.back(), bitmask, bitmask, (pic.size()-1) << shift, nudata);
	}
	data = nudata;
	isclassified = true;
}

void ClassifyImage::seedFillByMaskedBits(int startp, vector<int>& pout, int searchmask, int setmask, int setnum, vector<int>& nudata) {
	setnum &= setmask; //make sure we don't set unmasked bits
        int ptype = data[startp] & searchmask; //point type we are searching for
	nudata[startp] = (data[startp] & ~setmask) | setnum; //mark starting point
	assert(nudata.size() == data.size());
    
	vector<int> points;
	points.push_back(startp);
	
	while(points.size()) {
		int p = points.back();
		points.pop_back();
		pout.push_back(p);
		int x0=p%width;
		int y0=p/width;
		for(int q=0; q<connectn; q++)
		{
			if(q==4) continue;
			int dx = connectdx[q];
			int dy = connectdy[q];
			if(!inrange(x0+dx,y0+dy)) continue;
			
			int p1 = x0+dx+width*(y0+dy);
			if(nudata[p1] != (int)0xFFFFFFFF) continue; //someone got there first
			if((data[p1] & searchmask) != ptype) continue; //not of same type
			
			points.push_back(p1);
			nudata[p1] = (data[p1] & ~setmask) | setnum;
		}
	}
}

void ClassifyImage::calcstats() {
	printf("Calculating region statistics... "); fflush(stdout);
    
	stats.resize(pic.size());
	
	for(int i=0; i<pic.size(); i++) {
        stats[i].idnum = i;
		stats[i].npic = pic[i].size();
		stats[i].xsum=0;
		stats[i].xxsum=0;
		stats[i].ysum=0;
		stats[i].yysum=0;
		
		int p;
		float x,y;
		for(int j=0; j<pic[i].size(); j++) {
			p = pic[i][j];
			x=(float)(p%width); y=(float)(p/width);
			stats[i].xsum += x;
			stats[i].xxsum += x*x;
			stats[i].ysum += y;
			stats[i].yysum += y*y;
		}
	}
	
	printf(" Done.\n");
}

void ClassifyImage::connectivitygraph() {
	
	if(hasconnectivity) return;
	
	printf("Determining connectivity graph... ");
	fflush(stdout);
	cg = new SparseInt(pic.size(),pic.size());
	
	for(int i=0; i<size; i++){
		set<int> foo = boundswho(i);
		for(auto it = foo.begin(); it != foo.end(); it++) cg->set(data[i] >> shift, *it, 1);
	}
	
	for(int i=0; i<pic.size(); i++) cg->set((int)i,i,0);
	
	hasconnectivity=true;
	printf("Done.\n");
}


void ClassifyImage::joinregions(int a, int b, bool dobounds) { //merge region b into a
	
	if(!hasboundaries && dobounds) findboundaries();
	if(a>=pic.size() || b>=pic.size() || a==b) return; //oops! one of the basins doesn't exist
	
	//assign points to correct class
	pic[a].insert(pic[a].begin(), pic[b].begin(), pic[b].end());
	for(int i=0; i<pic[b].size(); i++) {
		data[pic[b][i]] = (a << shift) + (data[pic[b][i]] & (1<<shift)-1); //preserve b low bits
	}
	pic[b].clear();
	
	//fix RAG: connect b's neighbors to a
	int* connectedrs;
	int* connectedvs;
	int nconnected = cg->columnlist(b,connectedrs);
	cg->columnvals(b,connectedvs);
	for(int k=0; k<nconnected; k++) {
		int i = connectedrs[k];
		int s = connectedvs[k];
		cg->set(a,i,std::max(s,cg->get(a,i)));
		cg->set(i,a,std::max(s,cg->get(a,i)));
		cg->set(i,b,0);
		cg->set(b,i,0);
	}	
	cg->set(a,b,0);
	cg->set(b,a,0);
	free(connectedrs); free(connectedvs);
	
	//fix basin statistics
	if(stats[b].basinmin < stats[a].basinmin) stats[a].basinmin=stats[b].basinmin;
	stats[a].xsum += stats[b].xsum;
	stats[a].xxsum += stats[b].xxsum;
	stats[a].ysum += stats[b].ysum;
	stats[a].yysum += stats[b].yysum;
	stats[a].zsum += stats[b].zsum;
	stats[a].zzsum += stats[b].zzsum;
	
	if(!dobounds) return;
	
	//fix boundary
	bool* isbound = (bool*)calloc(size,sizeof(bool));
    assert(isbound);
	for(int i=0; i<bounds[a].size(); i++) isbound[bounds[a][i]]=true;
	for(int i=0; i<bounds[b].size(); i++) {
		set<int> f = boundswho(bounds[b][i]);
		if(f.size() >= 2) isbound[bounds[b][i]]=true; //it's a boundary
		else isbound[bounds[b][i]]=false;
	}
	
	vector<int> newbnds(size);
	for(int i=0; i<bounds[a].size(); i++) {
		if(isbound[bounds[a][i]]) {
			newbnds.push_back(bounds[a][i]);
			isbound[bounds[a][i]]=false;
		}
	}
	for(int i=0; i<bounds[b].size(); i++) {
		if(isbound[bounds[b][i]]) {
			newbnds.push_back(bounds[b][i]);
		}
	}
	free(isbound);
	bounds[b].clear();
    bounds[a] = newbnds;
}

set<int> ClassifyImage::boundswho(int p) { //check which regions a point is a boundary for
	
	set<int> bounders;
	int x=p%width;
	int y=p/width;
	
	for(int i=0; i<connectn; i++) {
		int j=x+connectdx[i];
		if(j == -1 || j == width) continue; //edge
		int k=y+connectdy[i];
		if(k == -1 || k == height || connectr2[i]==0) continue; //edge or center 
		bounders.insert(data[j+width*k] >> shift);
	}
	
	return bounders;
}

void ClassifyImage::findboundaries() {
	
	if(hasboundaries) return;
	
	printf("Finding classification boundaries...");
	fflush(stdout);
	
	

    //count the boundaries
	vector< set<int> > adjacents(size);
    vector<int> nbounds(pic.size());
	for(int i=0; i<size; i++) {
		adjacents[i] = boundswho(i);
		if(adjacents[i].size() >= 2) { //it's a boundary!
			for(auto it  = adjacents[i].begin(); it != adjacents[i].end(); it++) nbounds[*it]++;
		}
	}
	
    bounds.resize(pic.size());
	for(int m=0; m<pic.size(); m++) bounds[m].resize(nbounds[m]);
	vector<int> tnbounds(pic.size());
    
	for(int i=0; i<size; i++) {
		if(adjacents[i].size() >=2) { //it's a boundary!
			for(auto it  = adjacents[i].begin(); it != adjacents[i].end(); it++) {
				int qqq = *it;
				int rrr = tnbounds[qqq];
				bounds[qqq][rrr] = i;
				tnbounds[qqq]++;
			}
		}
	}
	
	hasboundaries=true;
	calcstats();
	printf("Done.\n");
}

union bstofarr {
	BasinStat b;
	float a[14];
};

void ClassifyImage::settempstat(unsigned int n)
{
	if(n==0 || n>14) return;
	bstofarr q;
	for(int k=0; k<pic.size(); k++)
	{
		q.b = stats[k];
		stats[k].temp = q.a[n-1];
	}
}

void ClassifyImage::circularity() { //calculate circularity of each region
	if(stats.size() != pic.size()) calcstats();
	for(int i=0; i<pic.size(); i++) {
		Circle c = findboundingcirc(pic[i].data(), pic[i].size());
		stats[i].temp = pic[i].size()/(3.14159*c.r*c.r);
	}
}

void ClassifyImage::random(){ //assign random number in [0,1] to temp stat
	if(stats.size() != pic.size()) calcstats();
	for(int i=0; i<pic.size(); i++) {
		stats[i].temp = (float)rand()/((float)RAND_MAX);
	}
}

/* void ClassifyImage::angularity(){ //calculate circularity of each region
	if(stats.size() != pic.size()) calcstats();
	for(int i=0; i<pic.size(); i++) {
		if(pic[i].size()>1000) {stats[i].temp = 1.5; continue;}
		ClassifyImage* foo = extractbinarychunkmask(i,15);
		Circle c = findboundingcirc(pic[i],pic[i].size());
		ClassifyImage* bar = foo->circleopening((int)(1+0.5*c.r));
		float np1 = 0;
		for(int q=0; q<bar->size; q++) np1 += (bar->data[q] & 0x1);
		delete(bar);
		
		ClassifyImage* baz = foo->circleclosing((int)(1+0.5*c.r));
		float np2=0;
		for(int q=0; q<baz->size; q++) np2 += (baz->data[q] & 0x1);
		delete(baz);
		delete(foo);

		stats[i].temp = (float)(np2-np1)/(float)pic[i].size();
	}
} */

void ClassifyImage::labelboundaries(int c) {
	if(!hasboundaries) findboundaries();
	for(auto it = bounds.begin(); it != bounds.end(); it++){
		for(auto it2 = it->begin(); it2 != it->end(); it++) {
			data[*it2] = c << shift;
		}
	}
}

void ClassifyImage::renumerate() {
	printf("Re-enumerating basins;");
	fflush(stdout);
	
	map<int,int> renum;
    pic.clear();
	for(int i=0; i<size; i++) {
        auto it = renum.find(data[i] >> shift);
        if(it == renum.end()) {
            it = renum.insert(std::pair<int,int>(data[i] >> shift, pic.size())).first;
            pic.push_back(vector<int>());
        }
        data[i] = (it->second << shift) + (data[i] & (1<<shift)-1); // assign to category, preserving lower bits
        pic[it->second].push_back(i);
	}
	
	printf(" %zu basins found; Done.\n", pic.size());
	
    // this messes up the boundary lists... could fix, but easy to recompute
	hasboundaries=false;
    hasconnectivity=false;
    bounds.clear();
    stats.clear();
    if(cg) { delete(cg); cg=NULL; }
};

void ClassifyImage::renumerateWithKey(int andkey) {
	printf("Re-enumerating basins with key %i;", andkey);
	fflush(stdout);
	
	map<int,int> renum;
    pic.clear();
    pic.push_back(vector<int>());
	for(int i=0; i<size; i++){
        if(!(data[i] & andkey)) {	// assign events not matching any bits to category 0
			data[i] = 0x0 + (data[i] & (1<<shift)-1); // assign to category 0, preserving lower bits.
			pic[0].push_back(i);
			continue;
		}
        auto it = renum.find(data[i] >> shift);
        if(it == renum.end()) {
            it = renum.insert(std::pair<int,int>(data[i] >> shift, pic.size())).first;
            pic.push_back(vector<int>());
        }
        data[i] = (it->second << shift) + (data[i] & (1<<shift)-1); // assign to category, preserving lower bits
        pic[it->second].push_back(i);
	}
	
    printf(" %zu basins found; Done.\n", pic.size());
   	
    //this messes up the boundary lists... could fix, but easy to recompute
	hasboundaries=false;
	hasconnectivity=false;
    bounds.clear();
    stats.clear();
    if(cg) { delete(cg); cg=NULL; }
};

void ClassifyImage::addregiontopoi(int n) {
    poi.insert(poi.end(), pic[n].begin(), pic[n].end());
};
