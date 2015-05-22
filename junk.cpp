#if 0

// unused code chunks of dubious value


//characterize an isolated watershed region from a crater-split image with estimated radius
//assuming a small (without flat bottom) crater is present;
//update the gradient mask

float planeval(float* d, int i, int width, int height)
{
    float x = (float)(i%width) - (float)(width/2);
    float y = (float)(i/width) - (float)(height/2);
    return d[0]+d[1]*x+d[2]*y;
}

float coneval(float* d, int i, int width, int height)
{
    float x = (float)(i%width) - d[2];
    float y = (float)(i/width) - d[3];
    float r = sqrt(x*x+y*y);
    if(r >= d[0]) return 1;
    return 1 + d[1]*(d[0]-r)/d[0];
}

float slantconeval(float* d, int i, int width, int height)
{
    float x = (float)(i%width) - d[2];
    float y = (float)(i/width) - d[3];
    float r = sqrt(x*x+y*y);
    if(r >= d[0]) return d[4]+d[5]*x+d[6]*y;
    if(r <= fabs(d[7])) return d[1]*(d[0]-fabs(d[7]))/d[0]+d[4]+d[5]*x+d[6]*y;
    return d[1]*(d[0]-r)/d[0]+d[4]+d[5]*x+d[6]*y;
    //return d[1]*(d[0]-r)*(d[0]+r)/(d[0]*d[0])+d[4]+d[5]*x+d[6]*y;
}

float Image::fit_wrapper(float* d, int n, void* parentobj) {
    Image* i = (Image*)parentobj;
    return i->fitdev(d);
}

float Image::fitdev(float* d)
{
    float z = 0;
    float w = 0;
    float t;
    for(int i=0; i<size; i++)
    {
        t = data[i] - fitterfunc(d,i,width,height);
        z -= exp(-t*t/(0.1*0.1));
        w += 1;
    }
    return z/w;
}

void ClassifyImage::chainmergeslope(Image* u, float r)
{
    Image* dx = u->deriv(true);
    Image* dy = u->deriv(false);
    
    if(!cg) connectivitygraph();
    if(!hasboundaries) findboundaries();
    if(stats.size() != pic.size()) calcstats();
    
    //calculate region average dx, dy
    float* mdx = (float*)malloc(pic.size()*sizeof(float));
    float* mdy = (float*)malloc(pic.size()*sizeof(float));
    float* slp = (float*)malloc(pic.size()*sizeof(float));
    float* bx = (float*)malloc(pic.size()*sizeof(float));
    float* by = (float*)malloc(pic.size()*sizeof(float));
    
    for(int i=0; i<pic.size(); i++) bx[(int)(stats[i].idnum)]=(stats[i].xsum)/((float)npic[(int)(stats[i].idnum)]);
    for(int i=0; i<pic.size(); i++) by[(int)(stats[i].idnum)]=(stats[i].ysum)/((float)npic[(int)(stats[i].idnum)]);
    
    underlyingavg(dx);
    for(int i=0; i<pic.size(); i++) mdx[(int)(stats[i].idnum)]=stats[i].temp;
    underlyingavg(dy);
    for(int i=0; i<pic.size(); i++) mdy[(int)(stats[i].idnum)]=stats[i].temp;
    for(int i=0; i<pic.size(); i++) slp[i]=sqrt(mdx[i]*mdx[i]+mdy[i]*mdy[i]);
    
    float* slopedat = (float*)malloc(size*sizeof(float));
    for(int i=0; i<size; i++) slopedat[i] = sqrt(dx->data[i]*dx->data[i]+dy->data[i]*dy->data[i]);
    
    
    RGBImage* C;
    
    BasinStat*** rankings = (BasinStat***)calloc(pic.size(),sizeof(BasinStat**));
    int* nranks = (int*)calloc(pic.size(),sizeof(int));
    
    int* connectedrs=NULL;
    int* connectedvs=NULL;
    int nnbrs;
    
    
    for(int p=0; p<pic.size(); p++)
    {
        nnbrs = cg->columnlist(p,connectedrs);
        cg->columnvals(p,connectedvs);
        rankings[p] = (BasinStat**)calloc(2*nnbrs,sizeof(BasinStat*));
        nranks[p] = 2*nnbrs;
        for(int i=0; i<nnbrs; i++)
        {
            rankings[p][i] = (BasinStat*)malloc(sizeof(BasinStat));
            rankings[p][i]->idnum = connectedrs[i];
            rankings[p][i]->temp=0;
            rankings[p][i+nnbrs] = (BasinStat*)malloc(sizeof(BasinStat));
            rankings[p][i+nnbrs]->idnum = connectedrs[i];
            rankings[p][i+nnbrs]->temp=0;
        }
        
        for(int i=0; i<nbounds[p]; i++)
        {
            int z = bounds[p][i];
            int x = z%width;
            int y = z/width;
            
            for(int d=0; d<4; d++) //only consider 4-connected boundary
            {
                int ddx = connectdx[d];
                int ddy = connectdy[d];
                if(x+ddx<0 || x+ddx>=width || y+ddy<0 || y+ddy>=height) continue; //off edge
                if(data[x+ddx+width*(y+ddy)] == data[x+width*y]) continue; //in same region
                float w = ddy*dx->data[x+width*y] - ddx*dy->data[x+width*y];
                int q=0; if(w>0) q = nnbrs;
                while((q+1)%nnbrs != 0 && rankings[p][q]->idnum != data[x+ddx+width*(y+ddy)]) q++;
                if(rankings[p][q]->idnum != data[x+ddx+width*(y+ddy)]) continue;
                rankings[p][q]->temp += w;
            }
            
        }
        
        free(connectedrs); connectedrs = NULL;
        free(connectedvs); connectedvs = NULL;
        
        //sort by rank
        qsort(rankings[p],2*nnbrs,sizeof(BasinStat*),comparestatbytemp);
        
        if(p%(pic.size()/100)==0)
        {
            printf("%i: < ",p);
            for(int q=0; q<2*nnbrs; q++) printf("%.3g ",rankings[p][q]->temp);
            printf(">\n");
        }
        stats[p]->temp = rankings[p][nnbrs-1]->temp;
    }
    
    plotminima();
    C = prettyoverlayimage(orig);
    C->writeBMP("foo.bmp");
    delete(C);
    clearmarks();
    
    //mark agreeing neighbors for merging
    int nmarked=0;
    for(int p=0; p<pic.size(); p++)
    {
        if(nranks[p]<1) continue;
        do {
            if(rankings[p][0]->temp >= 0) break;
            int q = (int)rankings[p][0]->idnum;
            if(nranks[q]<0) break;
            if(rankings[q][nranks[q]-1]->idnum != p) break;
            cg->set(p,q,2);
            cg->set(q,p,2);
            nmarked++;
        }while(0);
        do {
            if(rankings[p][nranks[p]-1]->temp <= 0) break;
            int q = (int)(rankings[p][nranks[p]-1]->idnum);
            if(nranks[q]<0) break;
            if(rankings[q][0]->idnum != p) break;
            cg->set(p,q,2);
            cg->set(q,p,2);
            nmarked++;
        }while(0);
    }
    
    nmarked/=2;
    printf("Marked %i for merging.\n",nmarked);
    
    //merge marked
    for(int i=0; i<pic.size(); i++) {
        nnbrs = cg->columnlist(i,connectedrs);
        cg->columnvals(i,connectedvs);
        for(int j=0; j<nnbrs; j++) {
            if(connectedvs[j] > 1) joinregions(i,connectedrs[j],false);
        }
    }
    renumerate();
    findboundaries();
    
    return;
}

/* float ClassifyImage::averageradius(float x0, float y0) {
 if(!npoi) return 0;
 float r=0;
 int x,y;
 for(int i=0; i<npoi; i++) {
    x=poi[i]%width;
    y=poi[i]/width;
    r += sqrt((float)((x-x0)*(x-x0)+(y-y0)*(y-y0)));
 }
 return r/npoi;
 };
 
 float ClassifyImage::singleregionaverageradius(float x0, float y0, int n) {
    if(!pic[n].size()) return 0.0;
    float r=0;
    int x,y;
    for(int i=0; i<pic[n].size(); i++) {
 x=pic[n][i]%width;
 y=pic[n][i]/width;
 r += sqrt((float)((x-x0)*(x-x0)+(y-y0)*(y-y0)));
    }
    return r/pic[n].size();
 };
 
 float ClassifyImage::singleregionradialvariance(float* d, float ravg, int n) {
    float x0=d[0];
    float y0=d[1];
    
    float rv = 0;
    int x,y;
    float z;
    for(int i=0; i<pic[n].size(); i++) {
 x=pic[n][i]%width;
 y=pic[n][i]/width;
 z = sqrt((float)((x-x0)*(x-x0)+(y-y0)*(y-y0)))-ravg;
 rv += z*z;
    }
    return rv/pic[n].size();
 };
 
 float ClassifyImage::radialvariance(float* d, int n) {
    float x0=d[0];
    float y0=d[1];
    if(!npoi) return 0;
    float ravg = averageradius(x0, y0);
    float rv = 0;
    int x,y;
    float z;
    for(int i=0; i<npoi; i++) {
 x=poi[i]%width;
 y=poi[i]/width;
 z = sqrt((float)((x-x0)*(x-x0)+(y-y0)*(y-y0)))-ravg;
 rv += z*z;
    }
    return rv/npoi;
 };
 
 float ClassifyImage::radialvariance_wrapper(float* d, int n, void* parentobj) {
    ClassifyImage* w = (ClassifyImage*)parentobj;
    return w->radialvariance(d,n);
 };
 
 float* ClassifyImage::averagelocation() {
    float* a = (float*)calloc(2,sizeof(float));
    if(!npoi) return a;
    for(int i=0; i<npoi; i++) {
 a[0]+=poi[i]%width; // x
 a[1]+=poi[i]/width; // y
    }
    a[0]/=npoi;
    a[1]/=npoi;
    return a;
 };
 
 float* ClassifyImage::find_equidistant() {
    float* guess = averagelocation();
    ImageCoords ib;
    ib.lx=0; ib.ly=0;
    ib.ux=width; ib.uy=height;
    Minimizer* M = new Minimizer(guess,&(ib.lx),&(ib.ux),2,ClassifyImage::radialvariance_wrapper,(void*)this);
    delete(M);
    return guess;
 }; */

//-----------------------------

/* class aWSsubsets: public Action {
 public:
 aWSsubsets() : Action(){
    description="Find <ClassifyImage> region subsets of <ClassifyImage>";
    addname("wsss");
    ninputs = 2;
    inputtypes[0] = COF_CLASSIFYIMAGE;
    inputtypes[1] = COF_CLASSIFYIMAGE;
 };
 
 void DoIt() {
    mystack->push(((ClassifyImage*)(mystack->get(0)))->subregions(((ClassifyImage*)(mystack->get(1)))));
 }
 }; */

//-----------------------------

/* class aMarkminima: public Action {
 public:
 aMarkminima() : Action(){
    description="mark locations of minima on <ClassifyImage>";
    addname("markmin");
    ninputs = 1;
    inputtypes[0] = COF_CLASSIFYIMAGE;
 };
 
 void DoIt() {
    ((ClassifyImage*)(mystack->get()))->plotminima();
 }
 }; */

//-----------------------------

/* class aSuppressminima: public Action {
 public:
 aSuppressminima() : Action(){
    description="Suppress local minima below specified <threshold> of the Image used to generate a <ClassifyImage>";
    addname("suppressminima");
    ninputs = 2;
    inputtypes[0] = COF_CFLOAT;
    inputtypes[1] = COF_CLASSIFYIMAGE;
 };
 
 void DoIt() {
    Image* J = ((ClassifyImage*)mystack->get(1))->suppressminima(mystack->getfloat(0));
    mystack->drop();
    mystack->push(J);
 }
 }; */

//-----------------------------


#endif