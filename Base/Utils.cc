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

#include "Utils.hh"

vector<string> split(const string& s, const string splitchars) {
    vector<string> v;
    size_t p = 0;
    while(p<s.size()) {
        size_t wstart = s.find_first_not_of(splitchars,p);
        if(wstart == string::npos)
            break;
        p = s.find_first_of(splitchars,wstart);
        if(p == string::npos)
            p = s.size();
        v.push_back(s.substr(wstart,p-wstart));
    }
    return v;
}

extern int floatcompare(const void* a, const void* b);

float* lindilate(int l, float* d, int w) { //overwrite d with its line dilation
	int exwidth = l*(w/l+1);
	if(!w%l) exwidth--;
	int o=l/2;
	
	float* f = (float*)calloc(exwidth,sizeof(float));
	for(int i=0; i<w; i++) f[i]=d[i];
	float* g = (float*)malloc(exwidth*sizeof(float));
	float* h = (float*)malloc(exwidth*sizeof(float));
	for(int n=0; n<exwidth; n+=l){
		g[n]=f[n];
		h[exwidth-1-n]=f[exwidth-1-n];
		for(int i=1; i<l; i++){
			if(f[n+i]>g[n+i-1]) {g[n+i]=f[n+i];}
			else {g[n+i]=g[n+i-1];}
			if(f[exwidth-1-n-i] > h[exwidth-1-n-i+1]) {h[exwidth-1-n-i] = f[exwidth-1-n-i];}
			else {h[exwidth-1-n-i]=h[exwidth-1-n-i+1];}
		}
	}
	for(int i=0; i<w; i++) {
		if(i-o < 0 || (i+l-o-1<exwidth && g[i+l-o-1]>h[i-o])) {d[i] = g[i+l-o-1];}
		else {d[i]=h[i-o];}
	}
	
	free(f); free(g); free(h);
	return d;
}

float* moments(float* d, int size, int n) { //calculate the first n moments of dataset d
	float* m = (float*)calloc(n,sizeof(float));
	
	//calculate the mean
	for(int i=0; i<size; i++) m[0]+=d[i];
	m[0]/=(float)size;
	
	//array of powers of difference from the mean
	float **v = (float**)malloc((n-1)*size*sizeof(float*));
	for(int i=0; i<n-1; i++) v[i]=(float*)malloc(size*sizeof(float));
	float tmp;
	for(int k=0; k<size; k++) {
		tmp=d[k]-m[0];
		v[0][k]=tmp*tmp; //initialize 2nd moment
		for(int i=1; i<n-1; i++) v[i][k]=tmp*v[i-1][k];
	}
	
	//average the powers
	for(int i=0; i<n-1; i++) {
		for(int j=0; j<size; j++) m[i+1]+=v[i][j];
		m[i+1]/=(float)size;
	}
	
	for(int i=0; i<n-1; i++) {free(v[i]); v[i]=NULL;}
	free(v); v=NULL;
	return m;
};

float mean(float* data, int n){
	float t=0;
	for(int i=0; i<n; i++) t+=data[i];
	return t/n;
};

float variance(float* data, int n, float mu) {
	float t=0;
	for(int i=0; i<n; i++) t+=(data[i]-mu)*(data[i]-mu);
	return t/n;
};

/* //divide n datapoints into 2 clumps to as to minimize variance from clump means; return dividing line position
int dividinglines(float* data, int n){
	int prevdirection = 0;
	int s = n/2;
	float xls=0;
	float xxls=0;
	float xrs=0;
	float xxrs=0;
	float delta;
	float xi;
	float a;
	float b;
	
	//starting sums
	for(int i=0; i<s; i++) {
		xls+=data[i];
		xxls+=data[i]*data[i];
	}
	for(int i=s+1; i<n; i++) {
		xrs+=data[i];
		xxrs+=data[i]*data[i];
	}
	
	xi=data[s];
	while(1){
		a=(float)(s);
		b=(float)(n-s-1);
		delta = -xxls/(a*(a+1))+xxrs/(b*(b+1)) 
			+ ( a/((a+1)*(a+1)) - b/((b+1)*(b+1)) )*xi*xi
			+ (2*a+1)/(a*a*(a+1)*(a+1))*xls*xls - (2*b+1)/(b*b*(b+1)*(b+1))*xrs*xrs;
		
		if(delta>0) { //head right
			xls+=xi;
			xxls+=xi*xi;
			s+=1;
			xi=data[s];
			xrs-=xi;
			xxrs-=xi*xi;
			if(prevdirection==-1) break;
			prevdirection=1;
			continue;
		} else if (delta<0) { //head left
			xrs+=xi;
			xxrs+=xi*xi;
			s-=1;
			xi=data[s];
			xls-=xi;
			xxls-=xi*xi;
			if(prevdirection==1) break;
			prevdirection=-1;
			continue;
		} else break;
	}
	
	if(prevdirection==-1) return s+1;
	return s;
}; */

//divide n datapoints into 2 clumps to as to minimize variance from clump means; return dividing line position
/* int dividinglines(float* data, int n){

	float xls=0;
	float xxls=0;
	float xrs=0;
	float xxrs=0;
	
	float vbest=FLT_MAX;
	int sbest=0;
	
	for(int s=1; s<n; s++) {
		xls=0;
		xxls=0;
		for(int i=0; i<s; i++) {
			xls+=data[i];
			xxls+=data[i]*data[i];
		}
		
		xrs=0; xxrs=0;
		for(int i=s; i<n; i++) {
			xrs+=data[i];
			xxrs+=data[i]*data[i];
		}
	
		float vtot = xxls/s-xls*xls/(s*s) + xxrs/(n-s)-xrs*xrs/((n-s)*(n-s));
		if(vtot<vbest){
			vbest=vtot;
			sbest=s;
		}
	}
	
	return sbest;
}; */

Pointset::Pointset() {
	nitems=0;
	maxitems=16;
	items=(int*)malloc(16*sizeof(int));
}

Pointset::~Pointset() {
	free(items); items=NULL;
}

int Pointset::getitem(int n) {return items[n];}

bool Pointset::checkitem(int i){
	for(int j=0; j<nitems; j++){
		if(items[j]==i) return true;
	}
	return false;
}

int Pointset::whereis(int i){
	for(int j=0; j<nitems; j++){
		if(items[j]==i) return j;
	}
	return -1;
}

void Pointset::additem(int i) {
	//check for overlap
	if(checkitem(i)) return;
	additem_unchecked(i);
}

void Pointset::additem_unchecked(int i) {
	//check size
	if(nitems==maxitems){
		maxitems+=16;
		items=(int*)realloc(items,maxitems*sizeof(int));
	}
	items[nitems++]=i;
}

void Pointset::additems(int* l, int n) {
	for(int i=0; i<n; i++) additem(l[i]);
}

void Pointset::intersect(Pointset* p) {
	for(int i=0; i<nitems; i++) {
		if(!p->checkitem(getitem(i))) removeitem(getitem(i));
	}
}

void Pointset::addset(Pointset* p) {
	for(int i=0; i<p->nitems; i++) {
		additem(p->getitem(i));
	}
}

void Pointset::addset_unchecked(Pointset* p) {
	for(int i=0; i<p->nitems; i++) {
		additem_unchecked(p->getitem(i));
	}
}

void Pointset::removeitem(int i) {
	int w=whereis(i);
	if(w==-1) return;
	int k=0;
	for(int j=0;j<nitems;j++){
		items[k]=items[j];
		if(j!=w) k++;
	}
	nitems--;
	if(maxitems-nitems>=16) {
		maxitems-=16;
		items=(int*)realloc(items,maxitems*sizeof(int));
	}
}

Matrix::Matrix(int r, int c)
{
	rows=r;
	cols=c;
	data= (float*)calloc(rows*cols,sizeof(float));
	
}

Matrix* Matrix::cp()
{
	Matrix* M = new Matrix(rows,cols);
	for(int i=0; i<rows*cols; i++) M->data[i]=data[i];
	return M;
}


Matrix::~Matrix()
{
	free(data); data=NULL;
}

float& Matrix::operator() (unsigned i, unsigned j)
{
	return data[j+cols*i];
}


Matrix* Matrix::mult(float c)
{
	Matrix* R = new Matrix(rows,cols);
	for(int i=0; i<rows*cols; i++)
	{
		R->data[i]=data[i]*c;
	}
	return R;
}

Matrix* Matrix::imult(float c) //inplace multiply
{
	for(int i=0; i<rows*cols; i++)
	{
		data[i]=data[i]*c;
	}
	return this;
}

Matrix* Matrix::mult(Matrix* M) //destructive multiply this*M
{
	Matrix* R = new Matrix(rows,M->cols);
	for(int r=0; r<rows; r++)
	{
		for(int c=0; c<M->cols; c++) {
			for(int k=0; k<cols; k++) {
				(*R)(r,c) += operator()(r,k) * (*M)(k,c);
			}
		}
	}
	delete(M);
	return R;
}

Matrix* Matrix::add(Matrix* N)
{
	Matrix* R = new Matrix(rows,cols);
	for(int i=0; i<rows*cols; i++)
	{
		R->data[i] = data[i] + N->data[i];
	}
	return R;
}

Matrix* Matrix::iadd(Matrix* N) //destructive inplace add
{
	for(int i=0; i<rows*cols; i++)
	{
		data[i] += N->data[i];
	}
	delete(N);
	return this;
}

Matrix* Matrix::transpose()
{
	Matrix* M = new Matrix(cols,rows);
	for(int r=0; r<rows; r++) {
		for(int c=0; c<cols; c++) {
			(*M)(c,r)=operator()(r,c);
		}
	}
	return M;
}


Matrix* Matrix::itranspose() //inplace transpose
{
	float* data2 = (float*)malloc(rows*cols*sizeof(float));
	for(int r=0; r<rows; r++) {
		for(int c=0; c<cols; c++) {
			data2[c*rows+r]=operator()(r,c);
		}
	}
	free(data);
	data=data2;
	int t = rows;
	rows=cols;
	cols=t;
	return this;
}

Matrix* Matrix::comat(unsigned i, unsigned j)
{
	Matrix* M = new Matrix(cols-1,rows-1);
	
	int rr=0;
	for(int r=0; r<rows; r++) {
		if(r==i) continue;
		int cc=0;
		for(int c=0; c<cols; c++) {
			if(c==j) continue;
			(*M)(rr,cc)=operator()(r,c);
			cc++;
		}
		rr++;
	}
	return M;
}

float Matrix::det()
{
	if(rows==1 && cols==1) return operator()(0,0);
	if(rows==2 && cols==2) return operator()(0,0)*operator()(1,1)-operator()(1,0)*operator()(0,1);
	float d=0;
	float p=1;
	for(int r=0; r<rows; r++) {
		Matrix* c = comat(r,0);
		d += p*operator()(r,0)*c->det();
		delete(c);
		p*=-1;
	}
	return d;
}

Matrix* Matrix::invert()
{
	float d = det();
	if(d==0) {
		fprintf (stderr, "Can't invert singular matrix!\n"); 
		return new Matrix(0,0);
	}
	if(rows != cols) {fprintf (stderr, "Can't invert non-square matrix!\n"); return new Matrix(0,0);}
	
	Matrix* M = new Matrix(rows,cols);
	if(rows==1) {
		(*M)(0,0)=1.0/operator()(0,0);
		return M;
	}
	
	float p1=1;
	for(int r=0; r<rows; r++) {
		float p2=1;
		for(int c=0; c<cols; c++) {
			Matrix* t = comat(r,c);
			(*M)(r,c)=p1*p2*(t->det());
			delete(t);
			p2*=-1;
		}
		p1*=-1;
	}
	return M->itranspose()->imult(1/d);
}

void Matrix::disp()
{
	fprintf (stdout, "\n");
	for(int r=0; r<rows; r++) {
		fprintf (stdout, "| ");
		for(int c=0; c<cols; c++) {
			fprintf (stdout, "%g\t",operator()(r,c));
		}
		fprintf (stdout, " |\n");
	}
}

Matrix* mmult(Matrix* N, Matrix* M) //destructive multiply of two matrices N*M
{
	Matrix* R = new Matrix(N->rows,M->cols);
	for(int r=0; r<N->rows; r++)
	{
		for(int c=0; c<M->cols; c++) 
		{
			for(int k=0; k<N->cols; k++)
			{
				(*R)(r,c) += (*N)(r,k) * (*M)(k,c);
			}
		}
	}
	delete(N);
	delete(M);
	return R;
}

Matrix* madd(Matrix* N, Matrix* M) //destructive add of two matrices N+M
{
	Matrix* R = new Matrix(N->rows,M->cols);
	for(int r=0; r<N->rows; r++)
	{
		for(int c=0; c<N->cols; c++) {
			(*R)(r,c) = (*N)(r,c)+(*M)(r,c);
		}
	}
	delete(N);
	delete(M);
	return R;
}


//Bresenham line drawing algorithm,
//as in http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
LineIterator::LineIterator(int xa, int ya, int xb, int yb)
{
	x0=xa; y0=ya; x1=xb; y1=yb;
	steep = abs(y1-y0)>abs(x1-x0);
	n=0;
	int t;
	if(steep) {t=x0; x0=y0; y0=t; t=x1; x1=y1; y1=t;}
	reverse = x0>x1;
	if(reverse) {x0=-x0; x1=-x1;}
	y=y0;
	x=x0;
	if(y0<y1) ystep = 1; else ystep = -1;
	deltax = x1-x0;
	deltay = abs(y1-y0);
	error=0;
}

bool LineIterator::step()
{
	++x;
	error += deltay;
	if(error << 1 >= deltax)
	{
		y+=ystep;
		error-=deltax;
	}
	return(x<=x1);
}

int LineIterator::px()
{
	if(steep) return y;
	if(reverse) return -x;
	return x;
}

int LineIterator::py()
{
	if(!steep) return y;
	if(reverse) return -x;
	return x;
}

Minimizer::Minimizer( float* params, float* lbound, float* ubound, int nparams, float (*evalfunc)(float* params, int n, void* p), void* po) {
	n=nparams;
	p=params;
	func=evalfunc;
	parentobject  = po;
	lb = lbound;
	ub = ubound;
	derivs=(float*)malloc(n*sizeof(float));
	dv=(float*)malloc(n*sizeof(float));
	sderivs=(float**)malloc(n*sizeof(float*));
	for(int i=0; i<n; i++) {
		sderivs[i]=(float*)malloc(n*sizeof(float));
		if(lbound && ubound)
		{
			dv[i] = 1e-2*(ubound[i]-lbound[i]);
		} else dv[i] = 1e-1;
	}
	
	printf("Fit "); fflush(stdout);
	bool abort=false;
	while(fitstep() && !abort){
		printf("*"); fflush(stdout);
		if(ub) {
			for(int i=0; i<n; i++) if(p[i]>ub[i]) abort=true;
		}
		if(lb) {
			for(int i=0; i<n; i++) if(p[i]<lb[i]) abort=true;
		}
	}
	if(abort) printf(" :-(\n");
	else printf(" --- :-) ---\n");
	
	printf("\t[ ");
	for(int i=0; i<n; i++) printf("%.4g ",p[i]);
	printf("]\n\t[ ");
	for(int i=0; i<n; i++) printf("%.4g ",dv[i]);
	printf("]\n");
};

Minimizer::~Minimizer() {
	free(derivs);
	for(int i=0; i<n; i++) free(sderivs[i]);
	free(sderivs);
	free(dv);
};

bool Minimizer::fitstep() { //slow, robust method?
	
	float* v0 = (float*)malloc(n*sizeof(float));
	float* v1 = (float*)malloc(n*sizeof(float));
	float* v2 = (float*)malloc(n*sizeof(float));
	float* v3 = (float*)malloc(n*sizeof(float));
	bool goagain = false;
	float f0, f1, f2, f3, t;
	
	for(int i=0; i<n; i++){
		
		//select evaluation points
		for(int k=0; k<n; k++) { v0[k]=p[k]; v1[k]=p[k]; v2[k]=p[k]; v3[k]=p[k]; }
		
		//-------------------initial bracketing
		//set v0[i] (middle), v1[i] (left), v2[i] (right)
		
		v0[i] = p[i];
		v1[i] = p[i]-0.5*dv[i];
		v2[i] = p[i]+0.5*dv[i];
		
		f0 = func(v0,n,parentobject);
		f1 = func(v1,n,parentobject);
		if(f1<=f0) //keep going this way to upturn
		{
			while(1)
			{
				v2[i] = v1[i] - dv[i];
				f2 = func(v2,n,parentobject);
					if(f2 >= f1) //we have brackets!
					{
						t=v0[i];
						v0[i]=v1[i];
						v1[i]=v2[i];
						v2[i]=t;
						t=f0;
						f0=f1;
						f1=f2;
						f2=t;
						break;
					}
					v0[i]=v1[i];
					v1[i]=v2[i];
					f0=f1;
					f1=f2;
					dv[i] *= 1.4;
			}
		}
		f2 = func(v2,n,parentobject);
		if(f2<=f0) //no, keep going this way to upturn
		{
			while(1)
			{
				v1[i] = v2[i] + dv[i];
				f1 = func(v1,n,parentobject);
					if(f1 >= f2) //we have brackets!
					{
						t=v0[i];
						v0[i]=v2[i];
						v2[i]=v1[i];
						v1[i]=t;
						t=f0;
						f0=f2;
						f2=f1;
						f1=t;
						break;
					}
					v0[i]=v2[i];
					v2[i]=v1[i];
					f0=f2;
					f2=f1;
					dv[i] *= 1.4;
			}
		}
		
		//----------------Bracketed!------ Now, binary search it!
		
		while(1)
		{
			//check tolerance
			if(dv[i]<1e-3*(ub[i]-lb[i])) break;
			goagain=true;
			
			//search to the left...
			v3[i] = 0.5*(v0[i]+v1[i]);
			f3 = func(v3,n,parentobject);
			if(f3<=f0)
			{
				v2[i]=v0[i];
				f2 = f0;
				v0[i]=v3[i];
				f0 = f3;
				
				f3 = v2[i]-v0[i];
				t = v0[i]-v1[i];
				if(f3 > t) dv[i]=f3;
				else dv[i] = t;
				continue;
			}
			v1[i]=v3[i];
			f1 = f3;
			
			//search to the right...
			v3[i] = 0.5*(v0[i]+v2[i]);
			f3 = func(v3,n,parentobject);
			if(f3<=f0)
			{
				v1[i]=v0[i];
				f1 = f0;
				v0[i]=v3[i];
				f0 = f3;
				
				f3 = v2[i]-v0[i];
				t = v0[i]-v1[i];
				if(f3 > t) dv[i]=f3;
				else dv[i] = t;
				continue;
			}
			v2[i]=v3[i];
			f2 = f3;
			f3 = v2[i]-v0[i];
			t = v0[i]-v1[i];
			if(f3 > t) dv[i]=f3;
			else dv[i] = t;
		}
		p[i] = v0[i];
	}
	
	free(v0); free(v1); free(v2); free(v3);
	return goagain;
}

/*bool Minimizer::fitstep() {
	calcderivs();
	calcsderivs();
	Matrix* D2 = new Matrix(n,n);
	Matrix* D1 = new Matrix(n,1);
	for(int i=0; i<n; i++){
		(*D1)(i,0)=derivs[i];
		for(int j=0; j<n; j++){
			(*D2)(i,j)=sderivs[i][j];
		}
	}
	if(!D2->det()){ //singular matrix
		delete(D2); delete(D1);
		printf("!");
		return false;
	}
	Matrix* Q = D2->invert();
	delete(D2);
	Matrix* dx = mmult(Q,D1)->imult(-1.0);
	bool goagain=false;
	for(int i=0; i<n; i++) {
		p[i]+=(*dx)(i,0);
		if(fabs((*dx)(i,0)) > dv[i]) goagain=true;
	}
	delete(dx);
	return goagain;
} */

void Minimizer::calcderivs() {
	float* v0 = (float*)malloc(n*sizeof(float));
	float* v1 = (float*)malloc(n*sizeof(float));
	float* v2 = (float*)malloc(n*sizeof(float));
	for(int i=0; i<n; i++){
		
		//select evaluation points
		for(int k=0; k<n; k++) {
			v2[k]=p[k];
			if(k==i) continue;
			v0[k]=p[k];
			v1[k]=p[k];
		}
		
		float f0, f1, f2;
		bool q = false;
		
		//find correct step size
		while(1)
		{
			v0[i]=p[i]-0.5*dv[i];
			v1[i]=p[i]+0.5*dv[i];
			f0 = func(v0,n,parentobject);
			f1 = func(v1,n,parentobject);
			f2 = func(v2,n,parentobject);
			if( fabs(f1-2*f2+f0) < 0.01*fabs(f1-f2) && fabs(f1-2*f2+f0) < 0.01*fabs(f2-f0)) { dv[i]*=2; q=true; continue; }
			if( fabs(f1-2*f2+f0) > 0.2*fabs(f1-f2) && fabs(f1-2*f2+f0) > 0.2*fabs(f2-f0)) { dv[i]*=0.5; if(!q) continue; }
			break;
		}
		derivs[i]=(f1-f0)/dv[i];
		
	}
	free(v0);
	free(v1);
	free(v2);
};

void Minimizer::calcsderivs() {
	float* v0 = (float*)malloc(n*sizeof(float));
	float* v1 = (float*)malloc(n*sizeof(float));
	float* v2 = (float*)malloc(n*sizeof(float));
	float* v3 = (float*)malloc(n*sizeof(float));
	for(int i=0; i<n; i++){
		//printf("\t[ ");
		for(int j=i; j<n; j++){
			for(int k=0; k<n; k++) {
				v0[k]=p[k];
				v1[k]=p[k];
				v2[k]=p[k];
				v3[k]=p[k];
				
				if(k==i){
					v0[k] -= 0.5*dv[k];
					v1[k] += 0.5*dv[k];
					v2[k] -= 0.5*dv[k];
					v3[k] += 0.5*dv[k];
				}
				if(k==j){
					v0[k] -= 0.5*dv[k];
					v1[k] -= 0.5*dv[k];
					v2[k] += 0.5*dv[k];
					v3[k] += 0.5*dv[k];
				}
			}
			float f0 = func(v0,n,parentobject);
			float f1 = func(v1,n,parentobject);
			float f2 = func(v2,n,parentobject);
			float f3 = func(v3,n,parentobject);
			sderivs[i][j]=(f3-f2-f1+f0)/(dv[i]*dv[j]);
			sderivs[j][i]=sderivs[i][j];
			//printf("%g ",sderivs[i][j]);
		}
		//printf("]\n");
	}
	free(v0);
	free(v1);
	free(v2);
	free(v3);
};

ProgressBar::ProgressBar(int l)
{
	length = l;
	npts = 0;
	p = 0;
	if(l) { printf("|"); fflush(stdout); }
}

ProgressBar::~ProgressBar()
{
	update(1.0);
	if(length) printf("|\n");
}

void ProgressBar::update(float u)
{
	p = u;
	if(!length) return;
	while(npts < p*length)
	{
		printf("#");
		npts++;
	}
	fflush(stdout);
}
