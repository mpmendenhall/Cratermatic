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

#include "Interactive.hh"

TopInteractor::TopInteractor() : Interactor(NULL)
{
	mystack = new Stack();
	mystack->controller = (Interactor*)this;
	istop = true;
	name = "Craters";
	description = "An environment for image manipulation";
	
	Interactor* stackop = new Interactor(mystack);
	stackop->name = "StackOps";
	stackop->description = "Commands for manipulating the image stack";
	{
		stackop->registerAction(new aDup());
		stackop->registerAction(new aSwap());
		stackop->registerAction(new aRot());
		stackop->registerAction(new aDrop());
		stackop->registerAction(new aDropn());
		stackop->registerAction(new aClear());
	}
	registerCategory(stackop);
	
	Interactor* fileio = new Interactor(mystack);
	fileio->name = "FileIO";
	fileio->description = "Commands for reading and writing image data";
	{
		fileio->registerAction(new aLoad());
		fileio->registerAction(new aLoadppm());
		fileio->registerAction(new aLoadrawdat());
		fileio->registerAction(new aLoadClassification());
		fileio->registerAction(new aSave());
		fileio->registerAction(new aSavepgm());
		fileio->registerAction(new aSaverawbinary());
		fileio->registerAction(new aDraw());
		fileio->registerAction(new aLoadcatalog());
		fileio->registerAction(new aDumpcatalog());
	}
	registerCategory(fileio);
	
	Interactor* imath = new Interactor(mystack);
	imath->name = "ImageMath";
	imath->description = "Commands for math on images";
	{
		imath->registerAction(new aAdd());
		imath->registerAction(new aSubtr());
		imath->registerAction(new aMult());
		imath->registerAction(new aDiv());
		imath->registerAction(new aAbs());
		imath->registerAction(new aQadd());
		imath->registerAction(new aNormalize());
		imath->registerAction(new aComplement());
		imath->registerAction(new aNegate());
		imath->registerAction(new aThreshold());
		imath->registerAction(new aRange());
		imath->registerAction(new aGamma());
		imath->registerAction(new aSGamma());
		imath->registerAction(new aFlatHisto());
		
		Interactor* itrans = new Interactor(mystack);
		itrans->name = "ImageTrans";
		itrans->description = "Commands for image transforms";
		{
			Interactor* analytic = new Interactor(mystack);
			analytic->name = "Analytic";
			analytic->description = "Convolutions and related transform operations";
			{
				analytic->registerAction(new aHtransform());
				analytic->registerAction(new aSHtransform());
				analytic->registerAction(new aDx());
				analytic->registerAction(new aDy());
				analytic->registerAction(new aCurv());
				analytic->registerAction(new aSlope());
				analytic->registerAction(new aDirection());
				analytic->registerAction(new ablur());
				analytic->registerAction(new aCraterTransform());
				analytic->registerAction(new aMaskedCraterFindingTransform());
			}
			itrans->registerCategory(analytic);
			
			Interactor* morpho = new Interactor(mystack);
			morpho->name = "Morphological";
			morpho->description = "Morphological transform operations";
			{
				morpho->registerAction(new aSqdil());
				morpho->registerAction(new aCdil());
				morpho->registerAction(new aCero());
				morpho->registerAction(new aCopen());
				morpho->registerAction(new aCclose());
				morpho->registerAction(new aAnndil());				
				morpho->registerAction(new aUnspike());
				morpho->registerAction(new aEdgefind());
			}
			itrans->registerCategory(morpho);
			
			Interactor* geo = new Interactor(mystack);
			geo->name = "Geometry";
			geo->description = "Geometry transform operations";
			{
				geo->registerAction(new aReduce());
				geo->registerAction(new aBilinear());
				geo->registerAction(new aReraster());
			}
			itrans->registerCategory(geo);
			
			Interactor* fft = new Interactor(mystack);
			fft->name = "FFT";
			fft->description = "Commands for FFTs and complex data";
			{
				fft->registerAction(new aFft());
				fft->registerAction(new aInvfft());
				fft->registerAction(new aReal());
				fft->registerAction(new aImag());
				fft->registerAction(new aMagv());				
			}
			itrans->registerCategory(fft);
		}
		imath->registerCategory(itrans);
	}
	registerCategory(imath);
	
	Interactor* igen = new Interactor(mystack);
	igen->name = "ImageGen";
	igen->description = "Commands for generating images";
	{
		igen->registerAction(new aCircimg());
		igen->registerAction(new aAnnimg());
		igen->registerAction(new aSpeckles());
		igen->registerAction(new aPoints());
		igen->registerAction(new aGrids());
		igen->registerAction(new aColorwheel());
		igen->registerAction(new aFourierSpinner());
		igen->registerAction(new aNeatopic());
	}
	registerCategory(igen);
	
	Interactor* iseg = new Interactor(mystack);
	iseg->name = "ImageSeg";
	iseg->description = "Commands for image segmentation";
	{
		Interactor* iseggen = new Interactor(mystack);
		iseggen->name = "SegGen";
		iseggen->description = "Commands to generate segmentations";
		{
			iseggen->registerAction(new aWatershed());
			iseggen->registerAction(new aGradseg());
			iseggen->registerAction(new aZeros());
			iseggen->registerAction(new aUpcurve());
		}
		iseg->registerCategory(iseggen);
		
		Interactor* isegvis = new Interactor(mystack);
		isegvis->name = "SegVis";
		isegvis->description = "Visualization operations for segmentations";
		{
			isegvis->registerAction(new aBoundaryimage());
			isegvis->registerAction(new aPrettyimage());
			isegvis->registerAction(new aPrettyoverlay());
			iseg->registerAction(new aStatimg());
			//iseg->registerAction(new aMarkminima());
		}
		iseg->registerCategory(isegvis);
		
		iseg->registerAction(new aGetsubregion());
		iseg->registerAction(new aExtractchunk());
		iseg->registerAction(new aSizeconstraint());
		
		iseg->registerAction(new aWSavg());
		iseg->registerAction(new aWSmin());
		iseg->registerAction(new aWSmax());
		iseg->registerAction(new aWSmed());
		iseg->registerAction(new aWScirc());
		iseg->registerAction(new aWSangl());
		iseg->registerAction(new aWSrand());
		
		//iseg->registerAction(new aWSsubsets());
		//iseg->registerAction(new aSuppressminima());
		
		Interactor* merge = new Interactor(mystack);
		merge->name = "Merging";
		merge->description = "Commands for region merging a segmented image";
		{
			merge->registerAction(new aMerge());
			merge->registerAction(new aSetmovie());
			merge->registerAction(new aSetmoviebase());
			merge->registerAction(new aSetmovieframerate());
		}
		iseg->registerCategory(merge);
	}
	registerCategory(iseg);
	
	Interactor* irec = new Interactor(mystack);
	irec->name = "ImageRec";
	irec->description = "High-level image recognition routines";
	{
		irec->registerAction(new aFindcraters());
		irec->registerAction(new aFindcratersParams());
	}
	registerCategory(irec);
	
	Interactor* vis = new Interactor(mystack);
	vis->name = "ImageVis";
	vis->description = "Commands for creating visualizations of image data";
	{
		vis->registerAction(new aColorview());
		vis->registerAction(new aColorvec());
		vis->registerAction(new aGrayview());
		vis->registerAction(new aShadeby());
		vis->registerAction(new aOverlay());
		vis->registerAction(new aEmboss());
		vis->registerAction(new aRenderTopo());
		vis->registerAction(new aRbs());
	}
	registerCategory(vis);
	
	Interactor* program = new Interactor(mystack);
	program->name = "Program";
	program->description = "Macro programming commands (Warning: Here Be Dragons)";
	{
		program->registerAction(new aExec());
		program->registerAction(new aRem());
		program->registerAction(new aIf());
		program->registerAction(new aIfelse());
		program->registerAction(new aLessequalthan());
		program->registerAction(new aGreaterequalthan());
		program->registerAction(new aLessthan());
		program->registerAction(new aGreaterthan());
		program->registerAction(new aEqualto());
		
		Interactor* userdef = new Interactor(mystack);
		userdef->name = "UserDef";
		userdef->description = "User-defined macros";
		{
			userdef->registerAction(new aName);
		}
		program->registerCategory(userdef);
	}
	registerCategory(program);
}
