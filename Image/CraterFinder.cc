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

#include "Image.hh"
#include "Classify.hh"
#include "RGBImage.hh"
#include "ComplexImage.hh"
#include "Histogram.hh"
#include "Utils.hh"
#include "RasterRegion.hh"

#include <string.h>
#include <string>
using std::string;
#include <cassert>

int comparestatbytemp(const void* a, const void* b) { //for qsort by average z
    float at = (*(BasinStat**)a)->temp;
    float bt = (*(BasinStat**)b)->temp;
    return (int)(bt < at) - (int)(at < bt);
}

struct crawler {
    unsigned int pos;
    unsigned int own;
    unsigned int id;
    short flags;
    float hival;
    float hislp;
    float dist;
    float strength;
    int p0;
};

int searchx[] = {1,1,1,0};
int searchy[] = {1,0,-1,1};
float srmag[] = {(float)sqrt(2.0), 1, (float)sqrt(2.0), 1};

void sameCrawlers(ClassifyImage* bounds, Image* topo, Image* drx, Image* dry, float minstr) {
    vector<crawler> ocs(bounds->size);
    vector<crawler> ncs(bounds->size);
    
    vector<float> dist(bounds->size, FLT_MAX);
    vector<float> rgnmx(bounds->pic.size());
    
    //set initial crawler list
    int ncrawls = 0;
    for(int i=0; i<bounds->size; i++) {
        if(!(bounds->data[i] & 0x02)) continue; //not a starting point
        ocs[ncrawls].pos = i;
        ocs[ncrawls].own = bounds->data[i] >> bounds->shift;
        ocs[ncrawls].hival = 0;
        ocs[ncrawls].hislp = 0;
        ocs[ncrawls].dist = 0;
        ocs[ncrawls].strength = 1;
        ocs[ncrawls].p0 = i;
        ncrawls++;
    }
    printf("Expanding core regions... "); fflush(stdout);
    
    int nnewcrawls = 0;
    int w = bounds->width;
    int dx, dy, op, np;
    int delx, dely;
    float slx,sly,sdd;
    float dv, dz;
    
    while(ncrawls) {
        nnewcrawls = 0;
        for(int i=0; i<ncrawls; i++) {
            if(ocs[i].id != (bounds->data[ocs[i].pos] & 0xF0)) continue; //must've been overwritten in previous step
            if(ocs[i].strength < minstr) continue; //too much time on low slopes
            
            for(int k=0; k<9; k++) //direction of travel
            {
                if(k==4) continue; //standstill
                
                dx = bounds->connectdx[k];
                dy = bounds->connectdy[k];
                op = ocs[i].pos; //old position
                np = ocs[i].pos+dx+w*dy; //new position under consideration
                if(!topo->inrange(np)) continue; //new position is out of image
                
                if((bounds->data[np] >> bounds->shift) != ocs[i].own) continue; //in wrong region
                
                dv = sqrt(dx*dx+dy*dy); //incremental distance of move
                if(dist[np] <= ocs[i].dist + dv) continue; //someone closer got there first
                
                delx = (np%w) - (ocs[i].p0%w);
                dely = (np/w) - (ocs[i].p0/w);
                slx = drx->data[np];
                sly = dry->data[np];
                sdd = (slx*delx + sly*dely)/sqrt((delx*delx+dely*dely)*(slx*slx + sly*sly)); //slope direction dotproduct
                dz = (topo->data[np] - topo->data[op])/dv; //slope of this step
                
                if(!(bounds->data[op] & 0x08)) //restricted movement outside of core region
                {
                    if(dz<0) continue; //don't go downhill!
                    if(sdd > -0.7) continue; //slope in wrong direction
                }
                
                //OK, we can claim the spot
                bounds->data[np] |= 0x04; //mark new spot as claimed
                ncs[nnewcrawls] = ocs[i]; //make copy of the old crawl
                ncs[nnewcrawls].pos = np; //set position to new spot
                ncs[nnewcrawls].dist += dv; //increment distance
                
                if(!(bounds->data[np] & 0x08)) //extra behaviors outside of core region
                {
                    //weaken strength if necessary
                    if(dz < 0.6*ocs[i].hival) ncs[nnewcrawls].strength *= dz/ocs[i].hival;
                    if(dz < 0.2*rgnmx[ocs[i].own >> bounds->shift])  ncs[nnewcrawls].strength *= 3*dz/rgnmx[ocs[i].own >> bounds->shift];
                    
                    if(bounds->data[op] & 0x08) //special behaviors at core region boundary
                    {
                        ncs[nnewcrawls].dist = 1000; //reset distance scale at core edge
                        ncs[nnewcrawls].p0 = op;
                    }
                }
                
                dist[np] = ncs[nnewcrawls].dist;
                if( dz > ocs[i].hival ) { ncs[nnewcrawls].hival = dz; ncs[nnewcrawls].strength = 1.0; }
                if( dz > rgnmx[ocs[i].own >> bounds->shift] ) rgnmx[ocs[i].own >> bounds->shift] = dz;
                
                bounds->data[np] += 0x10;
                ncs[nnewcrawls].id = (bounds->data[np] & 0xF0);
                
                nnewcrawls++;
            }
        }
        printf(" %i",nnewcrawls); fflush(stdout);
        ncrawls = nnewcrawls;
        ncs.swap(ocs);
    }
    
    printf(" Done.\n");
}


int Image::findcraters(const string& basefolder, Image* msk, vector<CraterSpec>& cspecs, bool makeoutimg, float k1, float k2, float k3, float k4, float k5, float k6, float k7) {
    printf("------------------- Finding craters...\n\n");
    
    char* tc = (char*)malloc(2048*sizeof(char)); // temporary character buffer
    
    vector<CraterSpec> specout;
    
    
    int r;    // analysis radius; smaller on first step without mask.
    if(!msk) {
        msk = new Image((RectRegion*)this);
        for(int i=0; i<size; i++) msk->data[i] = 1.0;
        r = 5;
    } else r = 20;
    
    int cycnum = 0; // size reduction cycle number
    while(r <= 20) {
        printf("Blurring for radius %i search...\n", r);
        Image* bld = gaussianblur(r/5.0);
        
        ClassifyImage* topows = ClassifyImage::watershed(bld); // (blurred) topography watershed
        topows->underlying = NULL;
        topows->underlyingstats(bld);
        
        printf("Calculating gradients...\n");
        Image* drx = bld->deriv(true);
        Image* dry = bld->deriv(false);
        Image* grd = dry->copy()->quadratureadd(drx); //slope image
        
        printf("Calculating C-Transform "); fflush(stdout);
        Image* ct = craterFindingTransform(r, msk);
        
        // generate crater cores (upward concave regions in C-Transform, cleaned up)
        printf("Finding C-Transform basins: concavity map;"); fflush(stdout);
        ClassifyImage* combined = ClassifyImage::fromCurvature(ct); // all upward-concave basins from C-Transform
        printf(" finding cores;"); fflush(stdout);
        combined->connectn = 4; // set to 4-connected object finder
        combined->findObjectsByLowBits(1); // separately enumerate each crater core
        combined->renumerateWithKey(0x1); //
        printf(" Done.\n");
        // cleanup holes
        combined->fillHoles(0x01);
        combined->renumerateWithKey(0x01);
        
        // trim candidate to basins in which it contains the minima
        for(size_t i=1; i<combined->pic.size(); i++) {
            for(size_t j=0; j<combined->pic[i].size(); j++) {
                int p = combined->pic[i][j];
                if( combined->data[topows->stats[topows->data[p]].minloc] != combined->data[p]) combined->data[p]=0;
            }
        }
        
        combined->renumerateWithKey(0x01);
        
        // unexpanded regions output files
        sprintf(tc, "%s/unexpand_%i_%03i.bmp", basefolder.c_str(), size, r);
        combined->writeLowBitBMP(tc);
        
        printf("Narrowing down crater candidates from %zu...\n",combined->pic.size());
        
        // First cut: NOT TOO MUCH PREVIOUS STEP
        for(size_t i=1; i<combined->pic.size(); i++) {
            float usum = 0;
            for(size_t j=0; j<combined->pic[i].size(); j++) usum += msk->data[combined->pic[i][j]];
            if(usum/combined->pic[i].size() < 0.7) {
                combined->andRegion(i,0x0);
                combined->pic.clear();
                continue;
            }
        }
        
        combined->renumerateWithKey(0x01);
        
        combined->upShift(8); // reserve lower 0xFF for crawler data
        for(int i=0; i<combined->size; i++) if(combined->data[i] & 0x01) combined->data[i] |= 0x08; //mark Original Region
        combined->renumerateWithKey(0x01);
        printf("Remaining crater candidates: %zu.\n",combined->pic.size());
        
        //Grab whole basin
        vector<int> npicshadow(topows->pic.size());
        for(size_t i = 0; i < topows->pic.size(); i++) npicshadow[i] = topows->pic[i].size();
        for(size_t i=1; i<combined->pic.size(); i++) {
            unsigned int origowner = combined->data[combined->pic[i][0]] >> combined->shift;
            for(size_t j=0; j<combined->pic[i].size(); j++) {
                int p = topows->data[combined->pic[i][j]];
                for(int q=0; q<npicshadow[p]; q++) {
                    if((combined->data[topows->pic[p][q]] >> combined->shift) != origowner) combined->data[topows->pic[p][q]] = (origowner << combined->shift) | 0x01;
                }
                npicshadow[p] = 0;
            }
        }
        
        combined->renumerateWithKey(0x01);
        
        // set crawler seeds
        combined->underlyingstats(bld);
        for(size_t i=1; i<topows->pic.size(); i++) {
            if(combined->data[topows->stats[i].minloc]) combined->data[topows->stats[i].minloc] |= 0x02; //mark basin minima
        }
        sameCrawlers(combined, bld, drx, dry, pow(.10,.2*r));
        combined->renumerateWithKey(0x04); //shrink basins to crawled size
        
        sprintf(tc,"%s/Preculling_%i_%03i.bmp",basefolder.c_str(),size,r);
        RGBImage* preculling_overlay = combined->prettyoverlayimage(grd);
        preculling_overlay->writeBMP(tc);
        delete(preculling_overlay);
        
        // clear extra data from background regions
        printf("Clearing background data...\n");
        //for(int i=0; i<size; i++) if(!(combined->data[i] & 0xFFFF0000)) combined->data[i] = 0;
        for(int i=0; i<size; i++) if(!(combined->data[i] >> combined->shift)) combined->data[i] = 0;
        
        // Clean basin regions
        combined->fillHoles(0x04 | 0x08);
        combined->renumerateWithKey(0x04);
        combined->constrainSize((int)(M_PI*r*r/4),(int)(8*M_PI*(r+5)*(r+5)));
        combined->renumerateWithKey(0x04);
        
        //Characterize each region
        printf("Checking craters...");
        combined->underlying = NULL;
        combined->underlyingstats(this);
        for(size_t i=1; i<combined->pic.size(); i++) {
            CraterSpec c(0);
            float invnpts = 1.0/combined->pic[i].size();
            c.x = invnpts * combined->stats[i].xsum;
            c.y = invnpts * combined->stats[i].ysum;
            c.area = 1.0/invnpts;
            c.hipt = combined->stats[i].basinmax;
            c.lowpt = combined->stats[i].basinmin;
            c.depth = c.hipt - c.lowpt;
            c.r = sqrt(c.area/M_PI);
            radialFourier(c.x, c.y, combined->pic[i], NULL, c.xsft, c.ysft, 10);
            radialFourier(c.x, c.y, combined->pic[i], drx, c.grxxsft, c.grxysft, 10);
            radialFourier(c.x, c.y, combined->pic[i], dry, c.gryxsft, c.gryysft, 10);
            
            fourierDeviations(c.x, c.y, combined->pic[i], c.xsft, c.ysft, c.deviation, 7);
            bool passed = false;
            
            do {
                //basic outer shape test
                if(sqrt(c.xsft[2]*c.xsft[2]+c.ysft[2]*c.ysft[2]) > k1*c.xsft[0]) break; //too skinny? k1 default 0.25
                if(sqrt(c.xsft[3]*c.xsft[3]+c.ysft[3]*c.ysft[3]) > k2*c.xsft[0]) break; //too lumpy? k2 default 0.10
                
                //shape deviation test
                if(c.deviation[1] > k3) break; // k3 0.06
                if(c.deviation[2] > k4) break; // k4 0.02
                
                //gradient fourier test
                //if(c.grxxsft[1] > 0 || c.gryysft[1] > 0) { passed = false; break; }
                float xymx = std::max(fabs(c.grxxsft[1]),fabs(c.gryysft[1]));
                float xymn = std::min(fabs(c.grxxsft[1]),fabs(c.gryysft[1]));
                if(xymn/xymx < k5) break; //x-y balance k5 0.5
                
                float primarycomponent = sqrt(c.grxxsft[1]*c.grxxsft[1]+c.gryysft[1]*c.gryysft[1]);
                
                float gr1 = sqrt(c.grxysft[1]*c.grxysft[1]+c.gryxsft[1]*c.gryxsft[1]);
                if(gr1 > k6*primarycomponent) break; //too large 1st component k6 0.33
                float gr2 = sqrt(c.grxysft[2]*c.grxysft[2] + c.gryxsft[2]*c.gryxsft[2] + c.grxxsft[2]*c.grxxsft[2] + c.gryysft[2]*c.gryysft[2]);
                if(gr2 > k7*primarycomponent) break; //too large 2nd component k7 0.33
                
                passed = true;
                
            } while (0);
            
            if(!passed) {
                combined->andRegion(i,0x0);
                printf("x"); fflush(stdout);
                continue;
            } else {
                //passed all tests
                printf("*"); fflush(stdout);
                c.idnum = specout.size();
                specout.push_back(c);
                combined->cutoutChunkMask(msk,i);
            }
        }
        printf("\nCraterfinding cycle done; cleaning up.\n\n");
        
        
        r *= 2;
        ++cycnum;
        delete(ct);
        delete(combined);
        delete(drx); delete(dry);
        delete(topows);
    }
    
    
    printf("Craterfinding done; saving output.\n");
    
    
    if(width > 40 && height > 40) {
        Image* rd = reduce();
        Image* md = msk->reduce();
        rd->findcraters(basefolder, md, specout, false, k1,k2,k3,k4,k5,k6,k7);
    }
    
    // return cspecs or write to catalog / final image
    if(!makeoutimg) {
        //fix crater coordinates for this size level
        for(auto it = specout.begin(); it != specout.end(); it++) {
            it->x = realx(it->x);
            it->y = realy(it->y);
            it->area *= realdx(1.0)*realdy(1.0);
            it->xsft[0] *= realdx(1.0)*realdy(1.0);
            for(int j=1; j<10; j++) {
                it->xsft[j] = realdx(it->xsft[j]);
                it->ysft[j] = realdy(it->ysft[j]);
            }
            cspecs.push_back(*it);
        }
    } else {
        printf("Final output image...\n");
        
        //final output image and layer images
        RGBImage* finalout = RGBImage::renderTopo(this);
        
        int nlayers = 0;
        
        ClassifyImage* layerImage = NULL;
        sprintf(tc,"%s/Catalog.txt",basefolder.c_str());
        FILE* catfile = fopen(tc,"w");
        assert(catfile);
        sprintf(tc,"%s/Shape.txt",basefolder.c_str());
        FILE* shapefile = fopen(tc,"w");
        assert(shapefile);
        sprintf(tc,"%s/Slope.txt",basefolder.c_str());
        FILE* gradfile = fopen(tc,"w");
        assert(gradfile);
        CraterSpec::writeHeaders(catfile);
        
        printf("\tDrawing %zu craters...\n", specout.size());
        
        vector<unsigned int> craterpts;
        for(size_t i = 0; i < specout.size(); i++) {
            if(specout[i].idnum == 0) {
                if(layerImage) {
                    printf("\t\tSaving layer %i file...\n", nlayers);
                    sprintf(tc,"%s/Layer_%i.txt",basefolder.c_str(),nlayers);
                    layerImage->writeArcGIS(tc);
                    delete(layerImage);
                }
                //layerImage = new ClassifyImage((RectRegion*)this);
                nlayers++;
            }
            specout[i].idnum = i + 1;
            specout[i].writeToFile(catfile);
            specout[i].writeShapeFourierToFile(shapefile);
            specout[i].writeGradFourierToFile(gradfile);
            finalout->fouriermark(specout[i].x, specout[i].y, specout[i].xsft, specout[i].ysft, 4);
            
            if(!layerImage) continue;
            craterpts.clear();
            fourierPoints(specout[i].x, specout[i].y, specout[i].xsft, specout[i].ysft, craterpts);
            for(auto it = craterpts.begin(); it != craterpts.end(); it++) {
                assert(*it < layerImage->data.size());
                layerImage->data[*it] = i+1;
            }
        }
        
        if(layerImage) {
            printf("\t\tSaving layer file...\n");
            sprintf(tc,"%s/Layer_%i.txt",basefolder.c_str(),nlayers);
            layerImage->writeArcGIS(tc);
            delete(layerImage);
        }
        
        fclose(catfile);
        fclose(shapefile);
        fclose(gradfile);
        
        sprintf(tc,"%s/final_%i.bmp",basefolder.c_str(),size);
        finalout->writeBMP(tc);
        delete(finalout);
    }
    
    free(tc);
    return specout.size();
}

//------------------------------------------------------------------

Image* Image::craterFindingTransform(float r0, Image* gx, Image* gy) {
    //printf("Circle Finding Transform: x kernel"); fflush(stdout);
    
    ProgressBar* pb = new ProgressBar(40);
    
    float x,y,r;
    int size = gx->size;
    int width = gx->width;
    int height = gx->height;
    
    Image* xkernel = new Image(width, height);
    for(int i=0; i<size; i++) {
        x=i%(width);
        if(x>width/2) x = x-width;
        y=i/(width);
        if(y>height/2) y = y-height;
        r = sqrt(x*x+y*y);
        if(r==0) r=1;
        xkernel->data[i]=-(x/r)*exp(-r*r/(2*r0*r0));
    }
    
    ComplexImage* kx = ComplexImage::fftreal(xkernel);
    delete(xkernel);
    pb->update(0.2);
    
    ComplexImage* datx = ComplexImage::fftreal(gx);
    datx->mult(kx);
    delete(kx);
    pb->update(0.35);
    
    Image* rdatx = datx->inversefftreal();
    delete(datx);
    pb->update(0.5);
    
    Image* iout = new Image(width,height);
    for(int i=0; i<size; i++) iout->data[i] = rdatx->data[i];
    delete(rdatx);
    
    ComplexImage* daty = ComplexImage::fftreal(gy);
    pb->update(0.65);
    
    Image* ykernel = new Image(width, height);
    for(int i=0; i<size; i++) {
        x=i%(width);
        if(x>width/2) x = x-width;
        y=i/(width);
        if(y>height/2) y = y-height;
        r = sqrt(x*x+y*y);
        if(r==0) r=1;
        ykernel->data[i]=-(y/r)*exp(-r*r/(2*r0*r0));
    }
    
    ComplexImage* ky = ComplexImage::fftreal(ykernel);
    delete(ykernel);
    daty->mult(ky);
    delete(ky);
    pb->update(0.85);
    
    Image* rdaty = daty->inversefftreal();
    delete(daty);
    for(int i=0; i<size; i++) iout->data[i] += rdaty->data[i];
    delete(rdaty);
    pb->update(1.0);
    
    iout->copyfromrr((RectRegion*)gx);
    iout->name = "Crater Transform";
    delete(pb);
    return iout;
}


Image* Image::craterFindingTransform(float r0, Image* mask) {
    Image* gx = deriv(true);  if(mask) gx->mult(mask); 
    Image* bgx = gx->gaussianblur(r0/4); delete(gx);
    Image* gy = deriv(false); if(mask) gy->mult(mask); 
    Image* bgy = gy->gaussianblur(r0/4); delete(gy);
    Image* ct = Image::craterFindingTransform(r0,bgx,bgy);
    delete(bgx);
    delete(bgy);
    return ct;    
}
