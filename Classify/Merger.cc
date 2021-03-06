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

#include "Merger.hh"
#include "Classify.hh"
#include "Image.hh"
#include "RGBImage.hh"
#include "Utils.hh"
#include <cassert>

bool domovie;
string moviebase;
int movieframeadvance;
int mergemethod;

int framenum = 1;
char* cfoo = (char*)malloc(128*sizeof(char));

//these functions are in Utils.cpp
extern Matrix* mmult(Matrix* N, Matrix* M);
extern Matrix* madd(Matrix* N, Matrix* M);

void Merger::genfcritvals(float alpha) { //create table of critical-f values
    
    printf("Checking for cached F-distribution critical values...");
    fflush(stdout);
    
    char* fname = (char*)malloc(512*sizeof(char));
    sprintf(fname,"critFcache_%i_%g.txt",nch,alpha);
    
    FILE* f = fopen(fname,"r");
    if(f==NULL) {
        printf(" No; Invoking R;");
        fflush(stdout);
        
        FILE* of = fopen("tempRcommand","w");
        for(int i=1; i<=2000; i++) fprintf(of,"qf(%.12g,%i,%i,lower.tail=FALSE) \n",alpha,nch,i);
        fclose(of);
        
        char* rcomm = (char*)malloc(512*sizeof(char));
        
        //need full path for David's machine
#ifdef WIN32
        sprintf(rcomm,"C:\\\"Program Files\"\\R\\R-2.3.1pat\\bin\\R\\ --vanilla --slave <tempRcommand >%s",fname);
#else
        sprintf(rcomm,"R --vanilla --slave <tempRcommand >%s",fname);
#endif
        assert(!system(rcomm));
        free(rcomm);
        
        f = fopen(fname,"r");
    } else {
        printf(" Yes;");
        fflush(stdout);
    }
    
    printf(" reading critF table;");
    fflush(stdout);
    
    fcritvals = (float*)malloc(std::min(w->size,2000)*sizeof(float));
    char* buf = (char*)malloc(256*sizeof(char));
    for(int i=1; i<=std::min(w->size,2000); i++) {
        assert(fgets(buf,200,f));
        float x=0;
        sscanf(buf,"[1] %f\n",&x);
        fcritvals[i-1]=x;
    }
    fclose(f);
    
    printf(" Done.\n");
}

float Merger::getfcritval(int n){
    if(n<2000) return fcritvals[n-1];
    return fcritvals[std::min(w->size,2000)-1];
}

Merger::Merger(ClassifyImage* ws, Image** c, int n)
{
    printf ("Starting Merger...\n");
    
    w=ws;
    ch=c;
    nch=n;
    
    w->findboundaries();
    w->connectivitygraph();
}

void Merger::domerge(float alpha) {
    genfcritvals(alpha);
    
    basinstats();
    mainloop();
    w->name = w->name+" Merged";
    printf ("%zu basins left after merging, with %.1f Pixels/Basin average\n", w->pic.size(), double(w->size)/w->pic.size());
}

void Merger::basinstats()
{
    fprintf (stdout, "Calculating region statistics... ");
    fflush(stdout);
    
    reg_sum = (Matrix**)malloc((w->pic.size())*sizeof(Matrix*)); //sum of image values in region
    reg_sscp = (Matrix**)malloc((w->pic.size())*sizeof(Matrix*)); //sum of X_i X_j in region
    bestj = (int*)malloc((w->pic.size())*sizeof(int));
    for(size_t i=0; i<w->pic.size(); i++) bestj[i] = -1;
    bestdv= (float*)calloc((w->pic.size()),sizeof(float));
    
    //region sum
    for(size_t i=0; i<w->pic.size(); i++){
        reg_sum[i] = new Matrix(nch,1);
        for(size_t k=0; k<w->pic[i].size(); k++) {
            for(int c=0; c<nch; c++) (*reg_sum[i])(c,0) += ch[c]->data[w->pic[i][k]];
        }
    }
    
    fprintf (stdout, "SSCP Matrix... ");
    fflush(stdout);
    //sscp matrix
    Matrix* v =  new Matrix(1,nch);
    for(size_t i=0; i<w->pic.size(); i++){
        reg_sscp[i] = new Matrix(nch,nch);
        for(size_t k=0; k<w->pic[i].size(); k++) {
            for(int c=0; c<nch; c++) (*v)(0,c) = ch[c]->data[w->pic[i][k]];
            reg_sscp[i]->iadd(mmult(v->transpose(),v->cp()));
        }
    }
    delete(v);
    
    fprintf (stdout, "Link weights... ");
    fflush(stdout);
    int nmergable = 0;
    linkweights = (float**)malloc(w->pic.size()*sizeof(float*));
    for(size_t i=0; i<w->pic.size(); i++) linkweights[i] = (float*)calloc(w->pic.size(),sizeof(float));
    for(size_t i=0; i<w->pic.size(); i++) {
        for(size_t j=i+1; j<w->pic.size(); j++) {
            linkweights[i][j]=calcweight(i,j);
            linkweights[j][i]=linkweights[i][j];
            if(linkweights[i][j]!=-1) nmergable++;
        }
    }
    for(size_t i=0; i<w->pic.size(); i++) linkweights[i][i]=-1;
    
    fprintf (stdout, "%i mergeable links. Done.\n",nmergable);
}

void Merger::mainloop()
{
    fprintf (stdout, "Merging Process Loop\n");
    int mergetotal=0;
    int ibest;
    
    do {
        float zbest = 0;
        ibest=-1;
        
        for(size_t i=0; i<w->pic.size(); i++)
        {
            if(w->pic[i].size() == 0) continue;
            if(bestj[i] == -1) continue;
            if(w->pic[bestj[i]].size() == 0) continue;
            if(bestdv[i]>zbest) {
                zbest=bestdv[i];
                ibest=i;
            }
        }
        
        if(ibest != -1) {
            //printf("Merging %i %i <%f>\n",ibest,bestj[ibest],bestdv[ibest]);
            merge(ibest,bestj[ibest]);
            mergetotal++;
            if(mergetotal%movieframeadvance==0) snapshot();
            if(mergetotal%50==0) {
                printf ("*");
                fflush(stdout);
            }
        }
        
    } while(ibest != -1);
    
    printf ("\n*** Total Merged: %i ***\n",mergetotal);
    w->renumerate();
}


float Merger::calcweight(int i, int j) {
    if(!(w->cg->get(i,j))) return -1; //it's not adjoining
    if(j==i) return -1; //don't try merging a region with itself
    if(w->pic[i].size()*w->pic[j].size() == 0) return -1; //skip empty (already merged) regions
    if((int)(w->pic[i].size()+w->pic[j].size()) <= nch) return -1; // skip too-small-to-merge regions
    
    float nu_ij = w->pic[i].size()+w->pic[j].size()-2;
    
    Matrix* Si = reg_sscp[i]->cp()->iadd(
                                         mmult(reg_sum[i]->cp(),reg_sum[i]->cp()->transpose())->imult(-1.0/w->pic[i].size())
                                         );
    
    Matrix* Sj = reg_sscp[j]->cp()->iadd(
                                         mmult(reg_sum[j]->cp(),reg_sum[j]->cp()->transpose())->imult(-1.0/w->pic[j].size())
                                         );
    
    //Matrix* Wij = madd(Si->cp(),Sj->cp())->imult(1.0/nu_ij);
    
    float deltaV=0;
    for(int p=0; p<nch; p++) deltaV += ((*Si)(p,p)+(*Sj)(p,p))*(1.0-1.0/nu_ij);
    //delete(Wij);
    
    Matrix* t = madd(reg_sum[i]->mult(1.0/w->pic[i].size()),reg_sum[j]->mult(-1.0/w->pic[j].size())); //(M_i-M_j)
    Matrix* S_pooled = madd(Si,Sj);
    if(!S_pooled->det()) { //check for singular matrices
        delete(S_pooled);
        delete(t);
        return -1;
    };
    Matrix* spi = S_pooled->invert()->imult( nu_ij / (1.0/((float)w->pic[i].size()) + 1.0/((float)w->pic[j].size())) );
    delete(S_pooled);

    Matrix* T = mmult(t->transpose(),mmult(spi,t->cp())); //T^2_ij
    delete(t);
    
    float foo = (*T)(0,0)*(w->pic[i].size()+w->pic[j].size()-1-nch)/(nch*nu_ij);
    delete(T);
    float cv = getfcritval(w->pic[i].size()+w->pic[j].size()-1-nch);
    if(foo >= cv) return -1; //fails the merging criterion
    
    //update best link tables
    if(deltaV > bestdv[i]) {
        bestdv[i]=deltaV;
        bestj[i]=j;
    }
    
    return deltaV;
}

void Merger::merge(int i, int j) //merge watershed regions (j into i) and statistics
{    
    for(size_t k=0; k<w->pic.size(); k++) {
        if(w->cg->get(j,k)) {
            linkweights[j][k] = -1;
            linkweights[k][j] = -1;
        }
    }
    
    w->joinregions(i,j,false);
    bestdv[i]=0;
    bestdv[j]=0;
    
    //update region statistics
    reg_sum[i]->iadd(reg_sum[j]);
    reg_sscp[i]->iadd(reg_sscp[j]);
    
    for(size_t k=0; k<w->pic.size(); k++) {
        if(w->cg->get(i,k)) {
            linkweights[i][k] = calcweight(i,k);
            linkweights[k][i]=linkweights[i][k];
        }
    }
    
}

void Merger::snapshot() { //save image of an intermediate frame
    if(!domovie) return;
    char* fname = (char*)malloc(1024*sizeof(char));
    sprintf(fname,moviebase.c_str(),framenum);
    RGBImage* C = w->prettyImage();
    C->writeBMP(fname);
    delete(C);
    framenum++;
    free(fname); fname=NULL;
}
