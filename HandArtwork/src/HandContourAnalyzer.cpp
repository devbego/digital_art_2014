//
//  HandContourAnalyzer.cpp
//  leapAndCamHandSegmenter
//
//  Created by GL on 8/29/14.
//
//

#include "HandContourAnalyzer.h"


//--------------------------------------------------------------
void HandContourAnalyzer::setup(int w, int h){
	imgW = w;
	imgH = h;
	bWorkAtHalfScale = true;
	
	contourFinder.setMinArea(imgH*imgW * 0.01);
	contourFinder.setMaxArea(imgH*imgW * 0.33);
	contourFinder.setFindHoles(false);
	contourFinder.setSortBySize(true);
	
	for (int f=0; f<8; f++){ // up to 8 handPartID's, that's why.
		ofVec3f aBlankVec3f;
		aBlankVec3f.set(0,0,0);
		fingerCentroids.push_back(aBlankVec3f);
		fingerDistalPts.push_back(aBlankVec3f);
		contourPointsClosestToFingerCentroids.push_back(aBlankVec3f);
	}
	
	intersection1.set(0, 0);
	intersection2.set(0, 0);
	bIntersectionExists1 = false;
	bIntersectionExists2 = false;
	
	smoothingOfLowpassContour	= 5;
	sampleOffsetForCurvatureAnalysis = 5;
	crotchCurvaturePowf			= 0.22;
	crotchDerivativePowf		= 1.0;
	crotchSearchRadius			= 13;
	crotchContourSearchTukeyMaskPct = 0.1;
	
	crotchContourIndicesDifs.resize(4);
	for (int i=0; i<4; i++){
		crotchQuality[i] = 0;
	}
    
    minCrotchQuality = 0.15;
    malorientationSuppression = 0.75;
   

	
}

//--------------------------------------------------------------
void HandContourAnalyzer::update (const Mat &thresholdedImageOfHandMat, const Mat &leapDiagnosticFboMat, LeapVisualizer &lv){
	
	bCalculatedDistances = false;
	for (int i=0; i<N_HANDMARKS; i++){
		Handmarks[i].index = -1;
		Handmarks[i].valid = false;
	}
	
	computeLabeledContour  (thresholdedImageOfHandMat, leapDiagnosticFboMat);
	computeContourRegions  (thresholdedImageOfHandMat, leapDiagnosticFboMat);
	buildCurvatureAnalysis (theHandContourVerySmooth);
	
	acquireProjectedLeapData (lv);
	
	computePinkySideHandmark();			// Find the outside base-point of the pinky
	computeIndexSideHandmark();			// Find the outside base-point of the pointer
	computePalmBaseHandmark();
	computeThumbBaseHandmark();
	
	acquireFingerOrientations (lv, leapDiagnosticFboMat);
	computeCrotchOrientations ();

	computeContourRegions (thresholdedImageOfHandMat, leapDiagnosticFboMat); // Yes, again
	computeContourDistancesFromKeyHandPoints();
	computeFingerPointsMaximallyDistantFromKeyHandPoints();

	computeFingerCrotches();
	sortFingerCrotches();
	
	estimateFingerTipsFromCircleFitting (lv);
	evaluateCrotchQuality();
	refineFingerTipsBasedOnCrotchQuality();
	refineOtherHandmarks();
	
	computeWristHandmarks();
	computeThumbKnuckleHandmark(); // comes late because it requires thumb tip
	// computeThumbTopKnuckleHandmark(); // WORKING BUT NOT USED
	
	
	assembleHandmarksPreliminary(); // last
}


//--------------------------------------------------------------
void HandContourAnalyzer::refineCrotches (LeapVisualizer &lv,
                                          const Mat &grayMat,
                                          const Mat &leapDiagnosticFboMat)
{
    
}



//--------------------------------------------------------------
void HandContourAnalyzer::acquireFingerOrientations (LeapVisualizer &lv, const Mat &leapDiagnosticFboMat) {
	
	fingerOrientations.clear();
	float scaleFactor = (bWorkAtHalfScale) ? 0.5:1.0;
	int whichBones[5] = {1,4,8,12,16};
	
	// Fetch the orientation angles of the 5 fingers, at the 1'th bone center.
	for (int i=0; i<5; i++){
		ofVec3f aProjectedBoneCenter = lv.getProjectedHandPoint( whichBones[i] );  //[ whichBones[i] ];
		int px = (int) roundf (aProjectedBoneCenter.x * scaleFactor);
		int py = (int) roundf (aProjectedBoneCenter.y * scaleFactor);
		float orientation = 0;
		
		if ((px > 0) && (px < imgW) &&
			(py > 0) && (py < imgH)){
			
			int index1 = py * imgW + px;
			int index3 = index1 * 3;
			
			unsigned char *pixels = leapDiagnosticFboMat.data;
			float pr = (float) pixels[index3+0];
			float pg = (float) pixels[index3+1];
			float pb = (float) pixels[index3+2];
			
			orientation = lv.getDiagnosticOrientationFromColor(pr,pg,pb);
		}
		fingerOrientations.push_back (orientation );
	}
	
	//--------------
	// While we're at it, compute the average orientation of the fingers.
	int nOrientations = fingerOrientations.size();
	float fingerOrientationSum = 0;
	for (int i=0; i<nOrientations; i++){
		fingerOrientationSum += fingerOrientations[i];
		// printf("fing orient %d = %f\n", i, fingerOrientations[i]);
	}
	fingerOrientationAverage = fingerOrientationSum/(float)nOrientations;

}

//--------------------------------------------------------------
void HandContourAnalyzer::computeCrotchOrientations(){
	// Compute the 4 crotch orientations, using the average of neighboring fingerOrientations.
	// Assumes acquireFingerOrientations() has previously been called.
	
	crotchOrientations.clear();
	for (int i=0; i<5; i++){
		float porientation = fingerOrientations[i];
		if (i < 4){
			float qorientation = fingerOrientations[i+1];
			float orientationAvg = (porientation + qorientation)/2.0;
			crotchOrientations.push_back (orientationAvg);
		}
	}
}

//--------------------------------------------------------------
void HandContourAnalyzer::drawOrientations(){
	
	// Fetch the orientation angles of the 5 fingers, at the 1'th bone center.
	// Draw indicators of the local orientation at those knuckles.
	
	int whichBones[5] = {1,4,8,12,16};
	for (int i=0; i<5; i++){
		
		float px = boneCenters[ whichBones[i] ].x;
		float py = boneCenters[ whichBones[i] ].y;
		
		float forientation = fingerOrientations[i];
		float pangX = 30.0 * cos(forientation);
		float pangY = 30.0 * sin(forientation);
		
		ofSetColor(255);
		ofEllipse (px,py, 10,10);
		ofLine (px, py, px+pangX, py+pangY);
		
		if (i < 4){
			float qx = boneCenters[ whichBones[i+1] ].x;
			float qy = boneCenters[ whichBones[i+1] ].y;

			float corientation = crotchOrientations[i];
			float avgAngX = 30.0 * cos(corientation);
			float avgAngY = 30.0 * sin(corientation);
			
			float avgx = (px + qx)/2.0;
			float avgy = (py + qy)/2.0;
			ofSetColor(255, 100,0);
			ofEllipse (avgx, avgy, 10,10);
			ofLine (avgx, avgy, avgx+avgAngX, avgy+avgAngY);
		}
	}
}

//--------------------------------------------------------------
void HandContourAnalyzer::draw(){
	
	drawLabeledHandContour();
	drawHandmarks();
	
	
	// drawOrientations();
	// drawCrotchCalculations (crotchSearchIndex0, crotchSearchIndex1);
	evaluateCrotchQuality();
	
	
	
	/*
	ofSetColor(0,255,0);
	ofNoFill();
	ofEllipse(knuckles[0].x, knuckles[0].y, 20,20);
	ofSetColor(0,25,255);
	ofEllipse(boneCenters[0].x, boneCenters[0].y, 15,15);
	*/
	
	
	
}



//--------------------------------------------------------------
void HandContourAnalyzer::computeFingerCrotches(){
	// Three ways to locate finger crotches; we're using a combination of (A) & (C):
	// In the polyline segments in between each (estimated) fingertip,
	// (A) Find points with the highest concavity.
	//     (May fail if there's a contour irregularity).
	// (B) Find points closest to the hand's centroid
	//     (May fail for the thumb)
	// (C) Find points on the polyline which are extrema along the local orientation
	//     averaged from the orientations of the two adacent fingers
	
	// Assumes the following have been computed and/or acquired:
	// knuckles, wristPosition, crotchOrientations

	int nContourPts = theHandContourResampled.size();
	
	// clear crotchQualityData
	crotchQualityData.clear();
	crotchQualityData2.clear();
	for (int i=0; i<nContourPts; i++){
		ofVec3f zeroVec3f; zeroVec3f.set(0,0,0);
		ofVec3f zeroVec3g; zeroVec3g.set(0,0,0);
		crotchQualityData.addVertex( zeroVec3f );
		crotchQualityData2.addVertex(zeroVec3g );
	}
	
	//---------------------
	// Get the start and end indices for our search.
	// Compute coordinates of a line segment that safely demarcates our contour region of interest:
	float kA1 = 0.85; float kB1 = 1.0-kA1;
	float kA2 = 0.75; float kB2 = 1.0-kA2;
	float x1 = (kA1 * knuckles[0].x) + (kB1 * wristPosition.x);
	float y1 = (kA1 * knuckles[0].y) + (kB1 * wristPosition.y);
	float x2 = (kA2 * knuckles[4].x) + (kB2 * wristPosition.x);
	float y2 = (kA2 * knuckles[4].y) + (kB2 * wristPosition.y);
	float cx12 = (x1+x2)/2.0;
	float cy12 = (y1+y2)/2.0;
	
	// Extend the segment in both directions, to ensure that it intersects the contour
	float rayExtensionAmount = 150.0;
	float dx = x2-x1;
	float dy = y2-y1;
	float dh = sqrt(dx*dx+dy*dy);
	if (dh > 0){ dx /= dh; dy /= dh; }
	x1 -= rayExtensionAmount * dx;
	y1 -= rayExtensionAmount * dy;
	x2 += rayExtensionAmount * dx;
	y2 += rayExtensionAmount * dy;
	
	// Locate the indices at which the line intersects the contour.
	int tipIndexA = getIndexWhereLineSegmentIntersectsPolyline (cx12,cy12, x1,y1, theHandContourResampled);
	int tipIndexB = getIndexWhereLineSegmentIntersectsPolyline (cx12,cy12, x2,y2, theHandContourResampled);
	int index0 = 0;
	int index1 = 0;
	if ((tipIndexA > -1) && (tipIndexB > -1)){
		index0 = (tipIndexA < tipIndexB) ? tipIndexA : tipIndexB;
		index1 = (tipIndexA < tipIndexB) ? tipIndexB : tipIndexA;
		
		// adjust the nearest one to be contourIndexOfPinkySide if appropriate.
		bool bAdjustToContourIndexOfPinkySide = false;
		if (bAdjustToContourIndexOfPinkySide){
			if ((contourIndexOfPinkySide > index0) && (contourIndexOfPinkySide < index1)){
				int di0 = abs(contourIndexOfPinkySide - index0);
				int di1 = abs(contourIndexOfPinkySide - index1);
				if (di0 < di1){
					index0 = contourIndexOfPinkySide; // index0 was closer; clobber it
				} else if (di1 < di0){
					index1 = contourIndexOfPinkySide; // index1 was closer; clobber it
				}
			}
		}
		
		// Something isn't kosher if the concavity is too close to these positions.
		// So trim the search regions by a small percentage (~5%) of the contour on either side.
		// but we can bail on this by using function_TukeyWindow below to weight the results.
		bool bTrimContourPercentage = true;
		if (bTrimContourPercentage) {
			int nContourIndicesInSearchRegion = abs(index1 - index0);
			float percentageToTrim = 0.02;
			int nIndicesToTrim = (int)(percentageToTrim * nContourIndicesInSearchRegion);
			index0 += nIndicesToTrim;
			index1 -= nIndicesToTrim;
		}
		
	} else {
		return;
	}
	
	
	// Sanity check: see if the search takes us around the wrong side of the wrist position.
	bool bConductSanityCheck = false;
	if (bConductSanityCheck){
		float wristPositionX = wristPosition.x;
		bool bWeAreOnTheWrongSideOfWristPosition = false;
		for (int j=index0; j<index1; j++){
			float x = theHandContourVerySmooth[j].x;
			if (x > wristPositionX){
				bWeAreOnTheWrongSideOfWristPosition = true;
			}
		}
		float curt = (ofGetElapsedTimeMillis()/1000.0);
		printf("%f	%d	%d	wrongWristSide = %d\n", curt, index0, index1, bWeAreOnTheWrongSideOfWristPosition);
	}
	
	
	bool bDirectionIsForward = (tipIndexA < tipIndexB);
	crotchSearchIndex0 = index0;
	crotchSearchIndex1 = index1;
	if (abs(index1-index0) > 2){

		//--------------------
		// Produce a normalized metric (0...1) for curvatures in this region.
		// First find the max curvature
		float sharpestCurvatureValue = 0;
		for (int j=index0; j<index1; j++){
			float curvatureAtJ = handContourCurvatures[j];
			if (curvatureAtJ < sharpestCurvatureValue){
				sharpestCurvatureValue = curvatureAtJ;
			}
		}
		// Compute the normalized curvature metric values, produces values in 0...1.
		// Store the curvature metric in the x coordinate of crotchQualityData[j].x.
		for (int j=index0; j<index1; j++){
			float curvatureAtJ = handContourCurvatures[j];
			curvatureAtJ = ofClamp(curvatureAtJ, sharpestCurvatureValue, 0);
			float curvatureMetric01 = ofMap(curvatureAtJ, 0,sharpestCurvatureValue, 0,1);
			curvatureMetric01 = powf(curvatureMetric01, crotchCurvaturePowf);
			crotchQualityData[j].x = curvatureMetric01;
		}
			
		//--------------------
		// Compute a normalized metric (0...1) for derivative of extremality along local orientation
		// Note: Use a very smoothed version of the contour.
		ofPolyline polylineForExtremality = theHandContourVerySmooth;
		ofVec3f centerOfRotation = polylineForExtremality [index0];
		float pix, piy;
		float pjx, pjy;
		float pkx, pky;
		float tmpx, tmpy;
		
		float minDeriv2 =  99999;
		float maxDeriv2 = -99999;
			
		// Initialize variables to store 2 previous coordinates.
		pix = polylineForExtremality[index0].x;
		piy = polylineForExtremality[index0].y;
		pjx = polylineForExtremality[index0].x;
		pjy = polylineForExtremality[index0].y;
		float deriv2 = 0;
		
		// For all of the points between the thumb & pinky fingertip:
		for (int j=index0; j<index1; j++){
			
			// Compute the local orientation as interpolated across the contour.
			float orientationA = crotchOrientations[3];
			float orientationB = crotchOrientations[0];
			if (bDirectionIsForward){
				orientationA = crotchOrientations[0];
				orientationB = crotchOrientations[3];
			}
			float localOrientation = ofMap(j,index0,index1, orientationA,orientationB);
			
			// Draw the local orientation. Disabled.
			if (false){
				float cx = theHandContourResampled[ j ].x;
				float cy = theHandContourResampled[ j ].y;
				float qx = cx + 30*cos( ofMap(j,index0,index1, orientationA,orientationB) );
				float qy = cy + 30*sin( ofMap(j,index0,index1, orientationA,orientationB) );
				ofSetColor(0,100,200);
				ofLine (cx,cy, qx,qy);
			}

			// rotate the contour by the local orientation provided from the color diagram
			ofVec3f pk = polylineForExtremality[j];
			pkx = pk.x;
			pky = pk.y;
			pkx -= centerOfRotation.x;
			pky -= centerOfRotation.y;
			tmpx = (pkx * sin(localOrientation)) - (pky * cos(localOrientation));
			tmpy = (pkx * cos(localOrientation)) + (pky * sin(localOrientation));
			pkx = tmpx;
			pky = tmpy;
				
			// compute 2nd derivative
			if (j > (index0+2)){
				float dijy = pjy - piy;
				float djky = pky - pjy;
				deriv2 = djky - dijy;
				crotchQualityData[j].y = deriv2;
				
				if (deriv2 > maxDeriv2){
					maxDeriv2 = deriv2;
				}
				if (deriv2 < minDeriv2){
					minDeriv2 = deriv2;
				}
			}
			// swap
			pix = pjx; piy = pjy;
			pjx = pkx; pjy = pky;
		}
		
		// Compute the normalized deriv metric values, producing values in 0...1.
		// Store in crotchQualityData[j].y.
		bool bDoDerivativePowf = (abs(crotchDerivativePowf - 1.0) > 0.05);
		for (int j=index0; j<index1; j++){
			float deriv2atJ = crotchQualityData[j+1].y;
			deriv2atJ = ofClamp(deriv2atJ, minDeriv2,0);
			float deriv2metric01 = ofMap(deriv2atJ, minDeriv2,0, 1,0);
			if (bDoDerivativePowf){ deriv2metric01 = powf(deriv2metric01, crotchDerivativePowf); }
			crotchQualityData[j].y = deriv2metric01;
		}
			
		//-----------------------------------------------
		// We'll use crotchQualityData[j].z to store whether or not a segment of contour is
		// already known to contain a peak. This will help us in our peak search.

		//---------------
		// Adaptive threshold discovery: difference of signal and its average.
		// Compute a windowed average of the product of the two metrics.
		// First, compute the product, into crotchQualityData2[j].x
		for (int j=index0; j<index1; j++){
			float curvatureMetric01 = crotchQualityData[j].x;
			float deriv2metric01    = crotchQualityData[j].y;
			float product = (curvatureMetric01 * deriv2metric01);
			crotchQualityData2[j].x = abs(product);
		}
		// Second, compute the windowed average, into crotchQualityData2[j].y
		for (int j=index0; j<index1; j++){
			int lo = j-crotchSearchRadius; if (lo < index0){ lo = index0; }
			int hi = j+crotchSearchRadius; if (hi > index1){ hi = index1; }
			float sum = 0;
			for (int k=lo; k<hi; k++){
				sum += crotchQualityData2[k].x;
			}
			float average = sum / (crotchSearchRadius*2.0);
			crotchQualityData2[j].y = average;
		}
		// Third, compute the clamped difference between the raw and averaged.
		for (int j=index0; j<index1; j++){
			float dif = crotchQualityData2[j].x - crotchQualityData2[j].y;
			crotchQualityData2[j].z = MAX(0, dif);
		}
		// Fourth, window the results according to function_TukeyWindow, parameterized along the contour.
		// Values close to the ends of the search region will be suppressed.
		for (int j=index0; j<index1; j++){
			float frac = ofMap(j, index0,index1, 0.0,1.0);
			float tukeyMask = function_TukeyWindow (frac, crotchContourSearchTukeyMaskPct);
			crotchQualityData2[j].z *= tukeyMask;
		}
		
		
		//----------------
		// Now search for the best 4 peaks.
		int  peakIndices[4] = {-1,-1,-1,-1};
		float peakValues[4] = {0,0,0,0};
		bool bUseAdaptivelyThresholdedProduct = true;
		
		for (int j=index0; j<index1; j++){
			crotchQualityData[j].z = 1.0;
		}
		for (int k=0; k<4; k++){
			int maxIndex = 0;
			float maxValue = 0;
				
			for (int j=index0; j<index1; j++){
				float product = crotchQualityData[j].z;
				if (bUseAdaptivelyThresholdedProduct){
					product *= crotchQualityData2[j].z;
				} else {
					product *= crotchQualityData2[j].x;
				}
				
				if (product > maxValue){
					maxValue = product;
					maxIndex = j;
				}
			}
			
			peakIndices[k] = maxIndex;
			peakValues[k]  = maxValue;
			
			int ja = max(index0, maxIndex-crotchSearchRadius);
			int jb = min(index1, maxIndex+crotchSearchRadius);
			for (int j=ja; j<jb; j++){
				crotchQualityData[j].z = 0.0;
			}
		}
		// Copy over the winners
		for (int k=0; k<4; k++){
			crotchContourIndices[k] = peakIndices[k];
		}
		
	} // end if (abs(index1-index0) > 2)

}

//--------------------------------------------------------------
void HandContourAnalyzer::sortFingerCrotches(){
	// Sort the finger crotches by their distance (along the contour,
	// measured in indices) from contourIndexOfPinkySide.
	
	// First: figure out which end is closer to the pinky.
	int dif0 = abs(crotchSearchIndex0 - contourIndexOfPinkySide);
	int dif1 = abs(crotchSearchIndex1 - contourIndexOfPinkySide);
	bool bCrotchSearchIndex0IsCloserToPinkySide = (dif0 < dif1);
	int start = (bCrotchSearchIndex0IsCloserToPinkySide) ? crotchSearchIndex0 : crotchSearchIndex1;
	int end   = (bCrotchSearchIndex0IsCloserToPinkySide) ? crotchSearchIndex1 : crotchSearchIndex0;
	
	for (int k=0; k<4; k++){
		// Create a temporary copy of the crotchContourIndices.
		crotchContourIndicesTmp[k] = crotchContourIndices[k];
		
		// Store the differences from the base.
		int difk = abs(crotchContourIndices[k] - start);
		crotchContourIndicesDifs[k] = difk;
	}
	
	ofSort(crotchContourIndicesDifs);
	for (int k=0; k<4; k++){
		// Now, the differences are sorted.
		// For each difference, find the crotch index with that difference
		int sortedDif = crotchContourIndicesDifs[k];
		
		for (int j=0; j<4; j++){
			int difj = abs(crotchContourIndicesTmp[j] - start);
			if (difj == sortedDif){
				crotchContourIndices[k] = crotchContourIndicesTmp[j];
			}
		}
	}
	
	// Now label them in order.
	contourIndexOfPRCrotch = crotchContourIndices[0];
	contourIndexOfRMCrotch = crotchContourIndices[1];
	contourIndexOfMICrotch = crotchContourIndices[2];
	contourIndexOfITCrotch = crotchContourIndices[3];
	
	crotchContourIndicesExpanded[0] = 0;
	crotchContourIndicesExpanded[1] = 0;
	crotchContourIndicesExpanded[2        ] = end;
	crotchContourIndicesExpanded[ID_THUMB ] = crotchContourIndices[3]; // 3, IT, index-thumb
	crotchContourIndicesExpanded[ID_INDEX ] = crotchContourIndices[2]; // 4, MI, middle-index
	crotchContourIndicesExpanded[ID_MIDDLE] = crotchContourIndices[1]; // 5, RM, ring-middle
	crotchContourIndicesExpanded[ID_RING  ] = crotchContourIndices[0]; // 6, PR, pinky-ring
	crotchContourIndicesExpanded[ID_PINKY ] = start;
}



//--------------------------------------------------------------
void HandContourAnalyzer::drawCrotchCalculations (int index0, int index1){
	
	float amp		= 60.0;
	float step		= 0;
	float offsetx	= 10;
	float offsety	= 10;
	float scalex	= 0.6;
	
	bool bDrawCurvatureData			= false;
	bool bDrawDerivativeData		= false;
	bool bDrawCombinedMetricData	= false;
	bool bDrawAdaptivelyThresholdedProduct = true;
	bool bDrawPeakToPointConnectors	= true;

	// Render the normalized curvature metric values
	if (bDrawCurvatureData){
		ofSetColor(100,100,255, 170);
		for (int j=index0+1; j<index1; j++){
			float valPrev = crotchQualityData[j-1].x;
			float valCurr = crotchQualityData[j  ].x;
			ofLine (offsetx + step+((j-index0  )*scalex), offsety + amp*valPrev,
					offsetx + step+((j-index0+1)*scalex), offsety + amp*valCurr);
		}
	}
	
	// Render the normalized deriv metric values
	if (bDrawDerivativeData){
		ofSetColor(0,255,100, 255);
		for (int j=index0+1; j<index1; j++){
			float valPrev = crotchQualityData[j-1].y;
			float valCurr = crotchQualityData[j  ].y;
			ofLine (offsetx + step+((j-index0  )*scalex), offsety + amp*valPrev,
					offsetx + step+((j-index0+1)*scalex), offsety + amp*valCurr);
		}
	}
	
	// Render the combined metric (product) values
	if (bDrawCombinedMetricData){
		ofSetColor(255,255,60, 170);
		for (int j=index0+1; j<index1; j++){
			float valPrev = crotchQualityData[j-1].x * crotchQualityData[j-1].y;
			float valCurr = crotchQualityData[j  ].x * crotchQualityData[j  ].y;
			ofLine (offsetx + step+((j-index0  )*scalex), offsety + amp*valPrev,
					offsetx + step+((j-index0+1)*scalex), offsety + amp*valCurr);
		}
	}
	
	if (bDrawAdaptivelyThresholdedProduct){
		ofSetColor(255,40,200, 240);
		for (int j=index0+1; j<index1; j++){
			float valPrev = crotchQualityData2[j-1].z;
			float valCurr = crotchQualityData2[j  ].z;
			ofLine (offsetx + step+((j-index0  )*scalex), offsety + amp*valPrev,
					offsetx + step+((j-index0+1)*scalex), offsety + amp*valCurr);
		}
	}
	
	// Draw lines connecting the detected crotches to their corresponding signal peaks.
	if (bDrawPeakToPointConnectors){
		for (int k=0; k<4; k++){
			int aPeakIndex = crotchContourIndices[k];
			float aPeakValue = crotchQualityData2[aPeakIndex].z;
			
			float cx = theHandContourResampled[ aPeakIndex ].x;
			float cy = theHandContourResampled[ aPeakIndex ].y;
			float px = offsetx + step+((aPeakIndex-index0)*scalex);
			float py = offsety + amp*aPeakValue;
			
			ofSetColor(255);
			ofLine (cx,cy, px,py);
		}
	}
}




//--------------------------------------------------------------
void HandContourAnalyzer::computeContourDistancesFromKeyHandPoints(){
	
	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 1){
		
		// For each point on theHandContourResampled,
		// Compute the distance to the hand centroid, store it in theHandContourMetaData.
		for (int i=0; i<nContourPts; i++){
			ofVec3f aContourPt = theHandContourResampled[i];
			int whichHandPartId = (int) aContourPt.z;
			float distanceToCentroid = handCentroidLeap.distance(aContourPt);
			float distanceToWrist    = wristPosition.distance(aContourPt);
			
			// aMetaDataContourPoint.y will store local curvature, later on
			ofVec3f aMetaDataContourPoint;
			aMetaDataContourPoint.x = distanceToCentroid;
			aMetaDataContourPoint.y = distanceToWrist;
			aMetaDataContourPoint.z = whichHandPartId;
			theHandContourMetaData.addVertex(aMetaDataContourPoint);
		}
		bCalculatedDistances = true;
	}
}

//--------------------------------------------------------------
void HandContourAnalyzer::computeFingerPointsMaximallyDistantFromKeyHandPoints(){
	
	// As an initial guess, compute the contour points (for each handPartID)
	// which are maximally distant from the hand's centroid./
	// Requires prior call to computeContourDistancesFromKeyHandPoints().
	theHandContourMetaData.clear();

	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 1){
		
		// Blank all the fingerDistalPts. We'll use the z-coord to store the maxDistanceEncountered
		// require some sort of dot-product test with the local orientation and the direction to the centroid?
		for (int f=0; f<8; f++){
			fingerDistalPts[f].set(0,0,0);
		}
		
		// Determine, for each handPartID, the maximally distant point.
		for (int i=0; i<nContourPts; i++){
			ofVec3f aContourPt = theHandContourResampled[i];
			int whichHandPart = (int) aContourPt.z;
			if ((whichHandPart >= ID_NONE) && (whichHandPart <= ID_PINKY)){
				float distanceToCentroid = theHandContourMetaData[i].x;
				float distanceToWrist    = theHandContourMetaData[i].y;
				
				if (distanceToWrist > fingerDistalPts[whichHandPart].z){
				/* if (distanceToCentroid > fingerDistalPts[whichHandPart].z){ */
				/*	fingerDistalPts[whichHandPart].z = distanceToCentroid; */
					fingerDistalPts[whichHandPart].z = distanceToWrist;
					fingerDistalPts[whichHandPart].x = aContourPt.x;
					fingerDistalPts[whichHandPart].y = aContourPt.y;
				}
			}
		}
		
		// At this point, each of the fingerDistalPts contains the maximally distant point for that handPartID
		// Observations: outliers outside the main finger contours are causing errors.
		// When things are well-behaved, it's a quick and accurate estimate for the fingertip.
		// But when things are poorly behaved, outliers can really mess this up very badly.
		// Conclusion: nice results sometimes, but poorly supported.
		bool bDrawDistalPoints = true;
		if (bDrawDistalPoints){
			ofFill();
			ofSetColor(ofColor::orange);
			for (int f=0; f<8; f++){
				float distance = fingerDistalPts[f].z;
				if (distance > 0){
					float px = fingerDistalPts[f].x;
					float py = fingerDistalPts[f].y;
					ofEllipse(px, py, 3,3);
				}
			}
		}
		
	}
}


//--------------------------------------------------------------
void HandContourAnalyzer::computeFingerCentroids(){
	
	// As an initial guess, compute the centroid of each finger region.
	//
	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 1){
		
		// blank all the fingerCentroids. We'll use the z-coord to store the count for that handPartID.
		for (int f=0; f<8; f++){ fingerCentroids[f].set(0,0,0);}
		
		// Accumulate the data into fingerCentroids.
		for (int i=0; i<nContourPts; i++){
			ofVec3f aContourPt = theHandContourResampled[i];
			float px = aContourPt.x;
			float py = aContourPt.y;
			int whichHandPart = (int) aContourPt.z;
			
			if ((whichHandPart >= ID_NONE) && (whichHandPart <= ID_PINKY)){
				fingerCentroids[whichHandPart].x += px;
				fingerCentroids[whichHandPart].y += py;
				fingerCentroids[whichHandPart].z ++;
			}
		}
		
		// Compute the centroids by dividing by the counts.
		for (int f=0; f<8; f++){
			float count = fingerCentroids[f].z;
			if (count > 0){
				fingerCentroids[f].x /= count;
				fingerCentroids[f].y /= count;
			}
		}
	}
}

//--------------------------------------------------------------
void HandContourAnalyzer::computeNearestContourPointsToFingerCentroids(){
	// Works, but not currently used.
	// As an initial guess, compute the centroid of each finger region.
	//
	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 1){
		
		// Locate the nearest points on the contour to the finger centroids.
		// Make sure that it's the nearest contour point, with the right handPartID!
		// int indicesOfClosestContourPoints[8];
		// float distancesOfClosestContourPoints[8];
		for (int f=0; f<8; f++){
			// Initialize
			indicesOfClosestContourPoints[f] = -1;
			distancesOfClosestContourPoints[f] = 99999;
			
			// For handPartIds for which a centroid exists,
			float contributingPointCount = fingerCentroids[f].z;
			if (contributingPointCount > 0){
				
				// Search for the nearest kin point on the contour
				for (int i=0; i<nContourPts; i++){
					ofVec3f aContourPt = theHandContourResampled[i];
					
					// if it has the right handPartID (stored in aContourPt.z)
					int whichHandPart = (int) aContourPt.z;
					if (whichHandPart == f){
						float distance = fingerCentroids[f].distance(aContourPt);
						if (distance < distancesOfClosestContourPoints[f]){
							distancesOfClosestContourPoints[f] = distance;
							indicesOfClosestContourPoints[f] = i;
						}
					}
				}
				
				// Store the actual positions, too. 
				if (indicesOfClosestContourPoints[f] != -1){
					int contourIndex = indicesOfClosestContourPoints[f];
					ofVec3f aContourPt = theHandContourResampled[contourIndex];
					contourPointsClosestToFingerCentroids[f].set(aContourPt);
				}
			}
		}
		
		
		// draw a line from the finger centroid to its nearest contour point.
		bool bDrawLinesFromFingerCentroidsToNearestContourPoints = true;
		if (bDrawLinesFromFingerCentroidsToNearestContourPoints){
			for (int f=0; f<8; f++){
				if (indicesOfClosestContourPoints[f] != -1){
					ofSetColor(ofColor::brown);
					float px = contourPointsClosestToFingerCentroids[f].x;
					float py = contourPointsClosestToFingerCentroids[f].y;
					ofLine( px,py, fingerCentroids[f].x, fingerCentroids[f].y);
				}
			}
		}
		
		// Observations: occasionally, the centroids of the finger are outside the contour.
		bool bDrawFingerCentroids = false;
		if (bDrawFingerCentroids){
			ofFill();
			ofSetColor(ofColor::yellow);
			for (int f=3; f<8; f++){
				float count = fingerCentroids[f].z;
				if (count > 0){
					float px = fingerCentroids[f].x;
					float py = fingerCentroids[f].y;
					ofEllipse(px, py, 4,4);
					
					float cx = handCentroidLeap.x;
					float cy = handCentroidLeap.y;
					ofLine(px,py, cx,cy);
					
					/*
					float A = 0.9;
					float B = 1.0-A;
					ofVec3f interpVec3f = (A*fingerCentroids[f]) + (B*handCentroidLeap);
					ofLine(interpVec3f, handCentroidLeap);
					*/
				}
			}
		}
	}
}

//--------------------------------------------------------------
void HandContourAnalyzer::acquireProjectedLeapData (LeapVisualizer &lv){
	
	float scaleFactor = (bWorkAtHalfScale) ? 0.5:1.0;
    
	// Find the 19 bone centers.
	// Ask the LeapVisualizer for those getProjectedHandPoint()s.
	boneCenters.clear();
	int nCurrHandPoints = lv.currHandPoints.size();
	for (int i=0; i<nCurrHandPoints; i++){
		ofVec3f aProjectedBoneCenter = lv.getProjectedHandPoint(i);
		float px = aProjectedBoneCenter.x * scaleFactor;
		float py = aProjectedBoneCenter.y * scaleFactor;
		
		ofVec3f boneCenterPoint;
		boneCenterPoint.set (px,py,0);
		boneCenters.push_back (boneCenterPoint);
	}
	
	// Find the 5 main (in-palm) knuckles.
	// Ask the LeapVisualizer for those getProjectedKnuckle()s.
	knuckles.clear();
	for (int i=0; i<5; i++){
		ofVec3f aProjectedKnuckle = lv.getProjectedKnuckle(i);
		float px = aProjectedKnuckle.x * scaleFactor;
		float py = aProjectedKnuckle.y * scaleFactor;
		
		ofVec3f knucklePoint;
		knucklePoint.set (px,py,0);
		knuckles.push_back (knucklePoint);
	}
	
	// Fetch the wrist position and its normal
	wristPosition    = lv.getProjectedWristPosition();
	wristPosition    *= scaleFactor;
	wristOrientation = lv.getProjectedWristOrientation2();
	wristOrientation *= scaleFactor;
	handCentroidLeap = lv.getProjectedHandCentroid();
	handCentroidLeap *= scaleFactor;
	
	//-------------------------------
	bool bComputeKnuckleLineFitting = false;
	if (bComputeKnuckleLineFitting){
		// Compute knuckle line for pinky finger
		SlopeInterceptLine pkl = computeFitLine (knuckles, 2, 4);
		pinkyKnuckleLine.slope      = pkl.slope;
		pinkyKnuckleLine.yIntercept = pkl.yIntercept;
		
		// Compute knuckle line for index finger
		SlopeInterceptLine ikl = computeFitLine (knuckles, 1, 3);
		indexKnuckleLine.slope      = ikl.slope;
		indexKnuckleLine.yIntercept = ikl.yIntercept;
	}
}







//--------------------------------------------------------------
void HandContourAnalyzer::computeIndexSideHandmark(){
	
	// Find HANDMARK_POINTER_SIDE
	// contourIndexOfPointerSide = -1;
	int nContourPts = theHandContourResampled.size();
	
	if (nContourPts > 1){
		
		ofVec3f knuckleIndex  = knuckles   [1];
		ofVec3f knuckleMiddle = knuckles   [2];
		ofVec3f	boneCtrIndex  = boneCenters[3];
		ofVec3f	boneCtrMiddle = boneCenters[7];
		
		float	Am = 0.35;
		float	Bm = 1.0-Am;
		ofVec3f interpoMiddle = Am*boneCtrMiddle + Bm*knuckleMiddle;
		
		float	Ai = 0.25;
		float	Bi = 1.0-Ai;
		ofVec3f interpoIndex = Ai*boneCtrIndex + Bi*knuckleIndex;
		
		float	As = 0.50;
		float	Bs = 1.0-As;
		ofVec3f indexSideSearchLine1 = As*interpoMiddle + Bs*interpoIndex;
		ofVec3f indexSideSearchLine2 = interpoIndex + 2.0*(interpoIndex - interpoMiddle);
		
		// The result could be improved in two ways:
		// (1) an arbitrary offset by some number of points, along the contour towards the index-tip;
		// (2) possibly, recalculation by fitting a line through the IM and MR crotches at a later time.
		
		float x1 = indexSideSearchLine1.x;
		float y1 = indexSideSearchLine1.y;
		float x2 = indexSideSearchLine2.x;
		float y2 = indexSideSearchLine2.y;
		
		//------------------------------
		// Moving along the contour from the contourIndexOfPinkySide, in theHandContourWindingDirection,
		// Find the intersections of the indexSideSearchLine (x1,y1, x2,y2) with the successive contour points.
		// Record the *first* intersection with a segment whose handPartID is 4 (index) or 3 (thumb).
		
		int aContourIndexA = contourIndexOfPinkySide;
		int aContourIndexB = contourIndexOfPinkySide + theHandContourWindingDirection;
		float x3, y3;
		float x4, y4;
		bool bFoundIndexSide = false;
		
		for (int i=0; i<=nContourPts; i++){
			// If we haven't found our quarry,
			if (bFoundIndexSide == false){
				
				// Make the indices rotation-safe.
				aContourIndexA = (aContourIndexA + nContourPts)%nContourPts;
				aContourIndexB = (aContourIndexB + nContourPts)%nContourPts;
				
				// If the examined contour segment outlines a thumb or index,
				int whichHandPart = theHandContourResampled[aContourIndexB].z;
				bool bAlsoConsiderPointsOnTheThumb = false;
				
				if ((whichHandPart == (int)ID_INDEX) ||
					((whichHandPart == (int)ID_THUMB) && bAlsoConsiderPointsOnTheThumb) ){
					
					x3 = theHandContourResampled[aContourIndexA].x;
					y3 = theHandContourResampled[aContourIndexA].y;
					x4 = theHandContourResampled[aContourIndexB].x;
					y4 = theHandContourResampled[aContourIndexB].y;
					
					float denominator = ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
					if (denominator != 0.0){
						float ua = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / denominator;
						float ub = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / denominator;
						
						if ((ua > 0.0) && (ua < 1.0) &&
							(ub > 0.0) && (ub < 1.0)){
							
							// Found one! Here's our initial estimate for HANDMARK_POINTER_SIDE
							int theIntersectionIndex = aContourIndexB;
							
							// This point seems to be too "low" (close to the wrist), and as such,
							// also tends to intersect the thumb. Let's make an adjustment which
							// will pull (retreat) it back towards the true base of the index finger.
							
							// This dumb method just advances by some constant number of points.
							bool bJustAdvanceSomeNumberOfPoints = false;
							if (bJustAdvanceSomeNumberOfPoints) {
								int offsetAlongContour = 5 * theHandContourWindingDirection;
								theIntersectionIndex -= offsetAlongContour;
								theIntersectionIndex = (theIntersectionIndex + nContourPts)%nContourPts;
							}
							
							// This better method advances by a percentage of a dimension related to the finger.
							// We tried finger length (but it's not stable enough). Instead we'll use the
							// distance between knuckleIndex and knuckleMiddle, which is very stable.
							bool bAdvanceByPercentageOfFingerDimension = true;
							if (bAdvanceByPercentageOfFingerDimension){
								
								// Get the yardstick, the proportion according to which we will retreat.
								float fingerDimension = (knuckleIndex - knuckleMiddle).length();
								if (fingerDimension > 0){
									
									// Compute the amount of length to crawl back up the index finger.
									// Assume the finger is locally straight, so just measure distance
									// from the starting point, rather than along the curve.
									// How about we retreat 45% of the distance between the knuckles.
									float fractionOfFingerDimensionToRetreat = 0.45;
									float targetLengthToRetreat = fractionOfFingerDimensionToRetreat * fingerDimension;
									float retreatedLength = 0;
									bool  bSanityCheck = true;
									
									int retreatIters = 0;
									ofVec3f indexSidePointInitial = theHandContourResampled [theIntersectionIndex];
									int retreatedIndex = theIntersectionIndex;
									while ((retreatedLength < targetLengthToRetreat) && bSanityCheck) {
										retreatedIndex -= theHandContourWindingDirection;
										retreatedIndex = (retreatedIndex + nContourPts)%nContourPts;
										
										ofVec3f retreatContourPoint = theHandContourResampled[retreatedIndex];
										retreatedLength = (retreatContourPoint - indexSidePointInitial).length();
										
										retreatIters++;
										bSanityCheck = (retreatIters < 100);
									}
									theIntersectionIndex = retreatedIndex;
								}
							}
							
							contourIndexOfPointerSide = theIntersectionIndex;
							bFoundIndexSide = true; // crucial to terminate the search.
						}
					}
				}
				
				// Swap and advance.
				aContourIndexA = aContourIndexB;
				aContourIndexB += theHandContourWindingDirection;
			}
		}
		
		// If we didn't find an indexSide point, here's a last-ditch attempt.
		// Select the point on the index finger's contour which is closest to
		// a position interpolated between the knuckleThumb and knuckleIndex
		if (bFoundIndexSide == false) {
			
			float distanceToClosest = 99999;
			int   indexOfClosest    = -1;
			ofVec3f anchorPoint	= (0.65*knuckles[0] + 0.35*knuckles[1]);
			
			for (int i=0; i<nContourPts; i++){
				ofVec3f aContourPt = theHandContourResampled[i];
				int whichHandPart = aContourPt.z;
				if (whichHandPart == (int)ID_INDEX){
					
					float distance = anchorPoint.distance(aContourPt);
					if (distance < distanceToClosest){
						distanceToClosest = distance;
						indexOfClosest = i;
					}
				}
			}
			if ((indexOfClosest != -1) && (indexOfClosest < nContourPts)){
				contourIndexOfPointerSide = indexOfClosest;
				bFoundIndexSide = true;
			}
		}
		
	}
}


//--------------------------------------------------------------
void HandContourAnalyzer::computePinkySideHandmark(){

	// -- We can place HANDMARK_POINTER_SIDE and HANDMARK_PINKY_SIDE,
	//    which are located where the line crosses the contour.
	// -- We can revise the handPartID of contour data armside of the HANDMARK_PINKY_SIDE.
	// -- We can revise the handPartID of contour data armside of the HANDMARK_POINTER_SIDE.
	
	int indexOfContourPointClosestToPinkySide = -1;
	// contourIndexOfPinkySide = -1;
	// pinkyKnuckleLine is unstable for near-vertical lines (which happens),
	// so we'll just use the difference between knuckles 3 & 4 instead.

	// Interpolate between the metatarsal bone centers and the knuckle.
	float   Ar = 0.30;
	float	Br = 1.0-Ar;
	ofVec3f	boneCtrRing = boneCenters[11];
	ofVec3f knuckleRing = knuckles[3];
	ofVec3f interpoRing = Ar*boneCtrRing + Br*knuckleRing;
	
	float   Ap = 0.10;
	float	Bp = 1.0-Ap;
	ofVec3f	boneCtrPink = boneCenters[15];
	ofVec3f knucklePink = knuckles[4];
	ofVec3f interpoPink = Ap*boneCtrPink + Bp*knucklePink;
	
	float	As = 0.55;
	float	Bs = 1.0-As;
	ofVec3f pinkySideSearchLine1 = As*interpoRing + Bs*interpoPink;
	ofVec3f pinkySideSearchLine2 = interpoPink + 2.0*(interpoPink - interpoRing);
	
	float x1 = pinkySideSearchLine1.x;
	float y1 = pinkySideSearchLine1.y;
	float x2 = pinkySideSearchLine2.x;
	float y2 = pinkySideSearchLine2.y;
	
	//-------------------------------
	// FIND HANDMARK_PINKY_SIDE
	// First, find the intersecton of the knuckleLine with the pinkyside contour.
	// This will be the intersection closest to knuckles[4], and/or whose handPartID == ID_PINKY.
	//
	// Using http://paulbourke.net/geometry/pointlineplane/
	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 1){
		
		float x3,y3;
		float x4,y4;
		x3 = theHandContourResampled[0].x;
		y3 = theHandContourResampled[0].y;
		
		float minDistanceFromPinkyKnuckle = 99999;
		float pinkyKnuckleX = knuckles[4].x;
		float pinkyKnuckleY = knuckles[4].y;

		for (int i=0; i<=nContourPts; i++){
			int safeIndex = i%nContourPts;
			x4 = theHandContourResampled[safeIndex].x;
			y4 = theHandContourResampled[safeIndex].y;
			int whichHandPart = theHandContourResampled[safeIndex].z;
			
			float denominator = ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
			if (denominator != 0.0){
				float ua = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / denominator;
				float ub = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / denominator;
				
				if ((ua > 0.0) && (ua < 1.0) &&
					(ub > 0.0) && (ub < 1.0)){ // found one;
					
					float dpx = x4 - pinkyKnuckleX;
					float dpy = y4 - pinkyKnuckleY;
					float dph = sqrt(dpx*dpx + dpy*dpy);
					
					// If this point is closer to the pinky knuckle than any other,
					// or if whichHandPart == ID_PINKY
					if ((dph < minDistanceFromPinkyKnuckle) || (whichHandPart == ID_PINKY)) {
						minDistanceFromPinkyKnuckle = dph;
						indexOfContourPointClosestToPinkySide = i;
					}
					// FWIW, this would be the intersection point:
					// float interx = x1 + ua*(x2-x1);
					// float intery = y1 + ua*(y2-y1);
				}
			}
			x3 = x4;
			y3 = y4;
		}
		
		bool bFoundPinkySide = false;
		if (indexOfContourPointClosestToPinkySide != -1){
			// At this point, we know indexOfContourPointClosestToPinkySide.
			// And thus the global variable contourIndexOfPinkySide.
			// THIS COULD BE IMPROVED with a search (within some distance along the contour)
			// for a location with locally maximal concavity.
			bFoundPinkySide = true;
			contourIndexOfPinkySide = indexOfContourPointClosestToPinkySide;
		}
		
		// Now that we know where the HANDMARK_PINKY_SIDE is,
		if (bFoundPinkySide){
			
			// Determine the winding orientation of the contour.
			// A quick hack is to fetch the centerpoint of the pinky's interior (0th) bone.
			// move +/- 10 pts along the contour, & test whether we are closer/farther from this pt.
			
			int pinkyMetacarpalId = 15; // (it's the 15th in the LV's currHandPoints array.)
			ofVec3f pinkyMetaCarpalCenter = boneCenters[pinkyMetacarpalId];
			
			int del = 4; // a small delta
			int testIndexA = (contourIndexOfPinkySide + del + nContourPts) % nContourPts;
			int testIndexB = (contourIndexOfPinkySide - del + nContourPts) % nContourPts;
			ofVec3f testPtA = theHandContourResampled[testIndexA];
			ofVec3f testPtB = theHandContourResampled[testIndexB];
			
			float distanceA = pinkyMetaCarpalCenter.distance( testPtA );
			float distanceB = pinkyMetaCarpalCenter.distance( testPtB );
			
			theHandContourWindingDirection = (distanceA > distanceB) ?
				CONTOUR_WINDING_PINKY_TO_THUMB_CCW :
				CONTOUR_WINDING_PINKY_TO_THUMB_CW;
			
			// Clobber the handPartID stored in the contour on the wrong side of the contourIndexOfPinkySide
			int aContourIndex = contourIndexOfPinkySide - theHandContourWindingDirection;
			aContourIndex = (aContourIndex + nContourPts) % nContourPts;
			int paranoidSanityCheck = 0;
			while ((theHandContourResampled[aContourIndex].z == ID_PINKY) &&
				   (paranoidSanityCheck < nContourPts)){
				// Assign those contour points to value of 2 (the ID_PALM)
				theHandContourResampled[aContourIndex].z = ID_PALM;
				aContourIndex -= theHandContourWindingDirection;
				aContourIndex = (aContourIndex + nContourPts) % nContourPts;
				paranoidSanityCheck++;
			}
		}
	} // close if nContourPoints > 1
	
	/*
	ofSetColor(ofColor::green);
	ofLine(x1,y1, x2,y2);
	*/
}

//--------------------------------------------------------------
void HandContourAnalyzer::computePalmBaseHandmark(){
	// HANDMARK_PALM_BASE	= 14
	bool bComputeWristAxisIntersection	= false;
	bool bComputeLastPalmIndex			= false;
	bool bComputeHandAxisIntersection	= true;
	
	float	x1,y1, x2,y2;
	int		nContourPts = theHandContourResampled.size();
	float	wristSearchRadius = 500;

	//-------------
	// METHOD 1: compute the place where the wrist axis intersects the contour.
	int indexOfPinkySideWristAxisContourIntersection = -1;
	if (bComputeWristAxisIntersection){
		x1 = wristPosition.x; // (x1,y1) and (x2,y2) are the wrist and its perpendicular ray.
		y1 = wristPosition.y;
		x2 = x1 + wristSearchRadius*(wristOrientation.x - x1);
		y2 = y1 + wristSearchRadius*(wristOrientation.y - y1);
		indexOfPinkySideWristAxisContourIntersection = getIndexWhereLineSegmentIntersectsPolyline (x1,y1, x2,y2, theHandContourResampled);
	}

	//-------------
	// METHOD 2: Locate the furthest end of the PALM contour segment next to the pinky.
	// Note: it's possible for countOfPalmIdPoints
	if (bComputeLastPalmIndex){
		int lastPalmIndex = contourIndexOfPinkySide;
		int countOfPalmIdPoints = 0;
		for (int i=0; i<nContourPts; i++){
			int index = (contourIndexOfPinkySide-(i*theHandContourWindingDirection) +nContourPts) % nContourPts;
			if (theHandContourResampled[index].z == ID_PALM){
				lastPalmIndex = index;
				countOfPalmIdPoints ++;
			}
		}
	}
	
	//-------------
	// METHOD 3. Find orientation of the entire hand; compute perpendicular at wrist; intersect with contour on correct side.
	int indexOfPinkySideHandAxisContourIntersection = -1;
	if (bComputeHandAxisIntersection){
		x1 = wristPosition.x;
		y1 = wristPosition.y;
		float side = theHandContourWindingDirection;
		x2 = x1 + ( side) * wristSearchRadius * sin(fingerOrientationAverage);
		y2 = y1 + (-side) * wristSearchRadius * cos(fingerOrientationAverage);
		indexOfPinkySideHandAxisContourIntersection = getIndexWhereLineSegmentIntersectsPolyline (x1,y1, x2,y2, theHandContourResampled);
	}

	//-------------
	if (indexOfPinkySideHandAxisContourIntersection > -1){
		contourIndexOfPalmBase = indexOfPinkySideHandAxisContourIntersection;
	}
}




//--------------------------------------------------------------
void HandContourAnalyzer::computeThumbBaseHandmark(){
	
	//--------------
	// HANDMARK_THUMB_BASE	= 11
	//
	// First, find indexOfThumbSideFingerOrientationContourIntersection.
	// This is the place at which a ray emanating from the wrist position,
	// perpendicular to the average orientation of the fingers,
	// intersects theHandContourResampled on the thumb's side.
	// Note that we are not using wristOrientation, which is less stable.
	//
	int indexOfThumbSideFingerOrientationContourIntersection = -1;

	// (x1,y1) and (x2,y2) are the wrist and its perpendicular ray.
	// (x3,y3) and (x4,y4) are adjacent contour points.
	float wristSearchRadius = -500;
	float x1,y1, x2,y2, x3,y3, x4,y4;

	x1 = wristPosition.x;
	y1 = wristPosition.y;
	float side = theHandContourWindingDirection;
	x2 = x1 + ( side) * wristSearchRadius * sin(fingerOrientationAverage);
	y2 = y1 + (-side) * wristSearchRadius * cos(fingerOrientationAverage);
	indexOfThumbSideFingerOrientationContourIntersection = getIndexWhereLineSegmentIntersectsPolyline (x1,y1, x2,y2, theHandContourResampled);

	//--------------
	// Next, find point of greatest negative curvature, relative to boneCenters[0] and knuckles[0]
	ofVec3f thumbBone = boneCenters[0];
	ofVec3f thumbKnuckle = knuckles[0];
	ofVec3f thumbBase = thumbBone + (thumbBone - thumbKnuckle);
	
	int contourIndexClosestToBoneCenter0 = getIndexOfClosestPointOnContour (thumbBone, theHandContourVerySmooth);
	int contourIndexClosestToThumbBase   = getIndexOfClosestPointOnContour (thumbBase, theHandContourVerySmooth);
		
	int dif = (contourIndexClosestToThumbBase - contourIndexClosestToBoneCenter0);
	int searchBoundA = contourIndexClosestToThumbBase - dif;
	int searchBoundB = contourIndexClosestToThumbBase + dif/2; // searching too far if use dif; getting caught by sleeve edge
	
	// Reverse the order if necessary.
	if (searchBoundB < searchBoundA){
		int tmp = searchBoundA;
		searchBoundA = searchBoundB;
		searchBoundB = tmp;
	}
	
	// To find the contourIndexOfThumbBase,
	// we will look for the point of most significant negative curvature in that range.
	float sharpestCurvatureValue = 0;
	int sharpestCurvatureIndex = searchBoundA;
	for (int j=searchBoundA; j<searchBoundB; j++){
		float curvatureAtJ = handContourCurvatures[j];
		if (curvatureAtJ < sharpestCurvatureValue){
			sharpestCurvatureValue = curvatureAtJ;
			sharpestCurvatureIndex = j;
		}
	}
	
	//--------------
	// sharpestCurvatureIndex is prone to jitter when sharpestCurvatureValue is modest.
	// So let's engage a different process when sharpestCurvatureValue is too small.
	// Note that indexOfThumbSideWristAxisContourIntersection is prone to no intersection found in some circumstances.
	float curvatureCutoffA = -15.0;
	float curvatureCutoffB = -10.0;
	float lessThanThisIsMessedUp = -80.0;

	// If the curvature is sufficient (and kosher), use the point of greatest curvature.
	if ((sharpestCurvatureValue < curvatureCutoffA) && (sharpestCurvatureValue > lessThanThisIsMessedUp)){
		contourIndexOfThumbBase = sharpestCurvatureIndex;
	// If the curvature is too flat, use the location where the wrist perpendicular intersects the contour.
	} else if ((sharpestCurvatureValue > curvatureCutoffB) && (indexOfThumbSideFingerOrientationContourIntersection > -1)){
		contourIndexOfThumbBase = indexOfThumbSideFingerOrientationContourIntersection;
	// Otherwise, blend these using ofMap
	} else if (indexOfThumbSideFingerOrientationContourIntersection > -1){
		float blend  = ofMap (sharpestCurvatureValue, curvatureCutoffA,curvatureCutoffB, 1,0);
		float indexf = (blend * sharpestCurvatureIndex) + ((1.0-blend) * indexOfThumbSideFingerOrientationContourIntersection);
		contourIndexOfThumbBase = (int)roundf(indexf);
	}
	
	// If we have invalid data, don't set an index.
	if ((sharpestCurvatureValue <= lessThanThisIsMessedUp) || (indexOfThumbSideFingerOrientationContourIntersection <= -1)){
		contourIndexOfThumbBase = -1;
	}
}


//--------------------------------------------------------------
void HandContourAnalyzer::computeWristHandmarks(){
	
	// We assume that the user's wrist is cut off on the right-hand edge of the camera frame.
	// This produces two "corners" (upper and lower), which, depending on the hand orientation,
	// Correspond to the contourIndexOfThumbsideWrist and contourIndexOfPinkysideWrist.
	// Both are on the right-hand edge of the contour. Note, the edge may be irregular.
	// contourIndexOfThumbsideWrist will be the one closer (along the curve) to the thumb tip.
	// contourIndexOfPinkysideWrist will be the one closer (along the curve) to the pinky tip.

	// Let's define the "right hand edge" of the contour to mean, all points in the rightmost 3% of the width.
	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 0){
		
		// Determine the overall width of the contour.
		float contourL =  99999;
		float contourR = -99999;
		for (int i=0; i<nContourPts; i++){
			float ptx = theHandContourResampled[i].x;
			if (ptx > contourR){
				contourR = ptx;
			}
			if (ptx < contourL){
				contourL = ptx;
			}
		}
		float contourWidth = abs(contourR - contourL);
		float xPercent97 = ofMap(0.99, 0,1, contourL,contourR);
		
		// Of those points which are to the right of xPercent97,
		// locate the indices of topmost and the bottommost.
		float bottomMostValue = 99999;
		int   bottomMostIndex = 0;
		float topMostValue = -99999;
		int   topMostIndex = 0;
		for (int i=0; i<nContourPts; i++){
			float ptx = theHandContourResampled[i].x;
			float pty = theHandContourResampled[i].y;
			if (ptx > xPercent97){
				if (pty > topMostValue){
					topMostValue = pty;
					topMostIndex = i;
				}
				if (pty < bottomMostValue){
					bottomMostValue = pty;
					bottomMostIndex = i;
				}
			}
		}
		
		float palmBaseY = theHandContourResampled[contourIndexOfPalmBase].y;
		float thumBaseY = theHandContourResampled[contourIndexOfThumbBase].y;
		if (palmBaseY < thumBaseY){
			contourIndexOfPinkysideWrist = bottomMostIndex;
			contourIndexOfThumbsideWrist = topMostIndex;
		} else {
			contourIndexOfThumbsideWrist = bottomMostIndex;
			contourIndexOfPinkysideWrist = topMostIndex;
		}

		
		bool bDrawWristHandmarks = false;
		if (bDrawWristHandmarks){
			ofNoFill();
			ofSetColor(255,255,255);
			float px = theHandContourResampled[contourIndexOfPinkysideWrist].x;
			float py = theHandContourResampled[contourIndexOfPinkysideWrist].y;
			ofEllipse(px,py, 10,10);
			ofDrawBitmapString("P", px-20,py);
			
			float tx = theHandContourResampled[contourIndexOfThumbsideWrist].x;
			float ty = theHandContourResampled[contourIndexOfThumbsideWrist].y;
			ofEllipse(tx,ty, 10,10);
			ofDrawBitmapString("T", tx-20,ty);
		}
	}
}


//--------------------------------------------------------------
void HandContourAnalyzer::computeThumbKnuckleHandmark(){
	// HANDMARK_THUMB_KNUCKLE = 10
	// Compute the base of the thumb (the lower, proximal knuckle). This is between the tip and wrist.
	// This requires the following: contourIndexOfThumbsideWrist, contourIndexOfThumbTip
	
	contourIndexOfThumbKnuckle = -1;
	if ((contourIndexOfThumbsideWrist > -1) && (contourIndexOfThumbTip > -1)){
		
		// search for the point of highest absolute curvature (postive or negative)
		// located close to the center of the thumb.
		
		// The search starts from the point closest to boneCenters[0]
		ofVec3f thumbBone0 = boneCenters[0];
		ofVec3f thumbBone1 = boneCenters[1];
		int contourIndexClosestToBoneCenter0 = getIndexOfClosestPointOnContour (thumbBone0, theHandContourVerySmooth);
		int contourIndexClosestToBoneCenter1 = getIndexOfClosestPointOnContour (thumbBone1, theHandContourVerySmooth);
		int searchIndexA = contourIndexClosestToBoneCenter0;
		
		// Let the search progress to whichever is closer (along the contour) to searchIndexA:
		// 40% of the way to the thumb tip, or the point on the contour closest to boneCenters[1].
		// This is important because boneCenters[1] can be badly misplaced inside the hand,
		// and its proximal contour point can show up on the other side of the thumb tip.
		float percentB = 0.4;
		int searchIndexB = (int)(percentB*contourIndexOfThumbTip + (1.0-percentB)*searchIndexA);
		int difBones = abs(searchIndexA - contourIndexClosestToBoneCenter1);
		int difIntrp = abs(searchIndexA - searchIndexB);
		if (difBones < difIntrp){
			searchIndexB = contourIndexClosestToBoneCenter1;
		}
		
		// Now slide the searchIndexA some fraction closer to contourIndexOfThumbsideWrist
		float percentA = 0.0;
		searchIndexA = searchIndexA + (int)(percentA*(contourIndexOfThumbsideWrist - searchIndexA));
		
		// Reverse the order if necessary.
		if (searchIndexB < searchIndexA){
			int tmp = searchIndexA;
			searchIndexA = searchIndexB;
			searchIndexB = tmp;
		}
		
		// To find the contourIndexOfThumbsideWrist (ciotsw),
		// we will look for the point of most significant absolute curvature in that range.
		float sharpestCurvatureValue = 0;
		int sharpestCurvatureIndex = searchIndexA;
		for (int j=searchIndexA; j<searchIndexB; j++){
			float curvatureAtJ = abs(handContourCurvatures[j]);
			if (curvatureAtJ > sharpestCurvatureValue){
				sharpestCurvatureValue = curvatureAtJ;
				sharpestCurvatureIndex = j;
			}
		}
		
		// If the curvature signal is strong, use that
		if ((sharpestCurvatureValue > 15.0) && (sharpestCurvatureValue < 80.0)){
			contourIndexOfThumbKnuckle = sharpestCurvatureIndex;
		// If it's very weak, use an average of the bounds.
		} else if (sharpestCurvatureValue < 10.0){
			contourIndexOfThumbKnuckle = (searchIndexA + searchIndexB)/2;
		// If it's in the middle range, make a blend.
		} else if ((sharpestCurvatureValue >= 10.0) && (sharpestCurvatureValue <= 15.0)){
			float above15 = sharpestCurvatureIndex;
			float below10 = (searchIndexA + searchIndexB)/2;
			contourIndexOfThumbKnuckle = (int) ofMap(sharpestCurvatureValue, 10,15, below10,above15);
		} else {
			;
		}

	}
}


//--------------------------------------------------------------
void HandContourAnalyzer::computeThumbTopKnuckleHandmark(){
	// It turns out this knuckle is not used by the 2013 mesher.
	// The 2013 Mesher understands the "thumb knuckle" to be the outside thumb base.
	// This is working code, but not currently used!
	
	
	// If the indices exist
	if ((contourIndexOfThumbBase > -1) && (contourIndexOfThumbTip > -1)){
		
		// get contour-search index bounds, formatted for iteration.
		int index0 = (contourIndexOfThumbBase < contourIndexOfThumbTip) ? contourIndexOfThumbBase : contourIndexOfThumbTip;
		int index1 = (contourIndexOfThumbBase < contourIndexOfThumbTip) ? contourIndexOfThumbTip : contourIndexOfThumbBase;
		int del = (int)(0.20 * abs(index0 - index1));
		index0 += del;
		index1 -= del;
		
		
		// get the point of maximum curvature between handmarks #9 and #11.
		// scale this by a Tukey mask that biases it toward the center of the range. 
		float sharpestCurvatureValue = 0;
		int sharpestCurvatureIndex = index0;
		
		//ofNoFill();
		//ofSetColor(255,0,255);
		//ofBeginShape();
		for (int j=(index0+1); j<index1; j++){
			float curvatureAtJ = (handContourCurvatures[j]);
			
			float jFrac = ofMap(j, index0,index1, 0,1);
			float bias = function_TukeyWindow(jFrac, 0.95);
			curvatureAtJ *= bias;
			
			if (curvatureAtJ > sharpestCurvatureValue){
				sharpestCurvatureValue = curvatureAtJ;
				sharpestCurvatureIndex = j;
			}
			
			//ofVertex(10+ jFrac*256, 10+ curvatureAtJ*5);
		}
		//ofEndShape();
		
		
		// use a shaping function_DoubleExponentialSigmoid in order to blend the following:
		// for low values of sharpestCurvatureValue, use a certain point.
		// for high values of sharpestCurvatureValue, use the sharpestCurvatureIndex.
		float loCurvature  = 0.0;
		float hiCurvature  = 8.0; // crossover point is half of this.
		if (sharpestCurvatureValue > hiCurvature){
			contourIndexOfThumbKnuckle = sharpestCurvatureIndex;
		} else {
			ofVec3f thumbJoint = (boneCenters[1] + boneCenters[2])/2.0;
	
			// Fetch the index of the contour closest to thumbJoint, in the range index0 to index1.
			float minDistValue = 99999;
			int   thumbJointIndex = 0;
			for (int i=index0; i<index1; i++){
				float dist = thumbJoint.distance(theHandContourResampled[i]);
				if (dist < minDistValue){
					minDistValue = dist;
					thumbJointIndex = i;
				}
			}

			float shapedInput = ofMap(sharpestCurvatureValue,loCurvature,hiCurvature, 0,1, true);
			float shapedOutput = function_DoubleExponentialSigmoid (shapedInput, 0.75);
			contourIndexOfThumbKnuckle = ofMap(shapedOutput,0,1, thumbJointIndex,sharpestCurvatureIndex);
		}

	}
}




//--------------------------------------------------------------
void HandContourAnalyzer::drawHandmarks (){
	
	float markR = 7;
	
	int nContourPoints = theHandContourResampled.size();
	if (nContourPoints > 0){
		for (int i=0; i<N_HANDMARKS; i++){
			if (Handmarks[i].valid){
				int aHandmarkIndex = (Handmarks[i].index)%nContourPoints;
				if ((aHandmarkIndex > -1) && (aHandmarkIndex < nContourPoints)){
					ofVec3f aHandmarkPoint = theHandContourResampled [aHandmarkIndex];
					float hx = aHandmarkPoint.x;
					float hy = aHandmarkPoint.y;
					
					float del = 5;
					float rad = markR;

					switch (i){
						case 0:
						case 2:
						case 4:
						case 6:
						case 9:
							ofFill();
							ofSetColor(255,102,51);
							rad = 5;
							break;
						case 1:
						case 3:
						case 5:
						case 8:
							ofFill();
							ofSetColor(51,255,102);
							rad = 5;
							break;
						default:
							ofNoFill();
							ofSetColor(255,0,0);
							break;
							
					}
					
					ofEllipse (hx,hy, rad,rad);
					
					ofSetColor(0,0,0, 190);
					ofDrawBitmapString( ofToString(i), hx-(del+10-1), hy-(del-1));
					
					ofSetColor(255,255,255, 200);
					ofDrawBitmapString( ofToString(i), hx-(del+10), hy-del);
					
				}
			} else {
				; // what to do about invalid handmarks?
			}
			
			
			bool bDrawHandmarkTrails = false;
			if (bDrawHandmarkTrails){
				ofNoFill();
				ofSetColor(255,50,50, 60);
				ofBeginShape();
				for (std::list<ofVec3f>::iterator	it = Handmarks[i].pointHistory.begin();
													it != Handmarks[i].pointHistory.end(); it++){
					ofVertex (it->x, it->y);
				}
				ofEndShape();
			}
		}
	}
	

	ofFill();
	ofSetColor(255,255,255, 144);
	ofEllipse (wristPosition.x, wristPosition.y, markR,markR);
	ofEllipse (handCentroidLeap.x, handCentroidLeap.y, markR,markR);
}



//--------------------------------------------------------------
bool HandContourAnalyzer::doesHandContourExist(){
	return bContourExists;
}

//--------------------------------------------------------------
int	HandContourAnalyzer::getNumberOfBlobs(){
	return contourFinder.size();
}


//--------------------------------------------------------------
void HandContourAnalyzer::drawLabeledHandContour(){
	if (bContourExists){
		
		int nContourPts = theHandContourResampled.size();
		if (nContourPts > 0){
			
			float px,py;
			float qx,qy;
			qx = theHandContourResampled[0].x;
			qy = theHandContourResampled[0].y;
			
			// draw colored perimeters
			for (int i=0; i<nContourPts; i++){
				px = theHandContourResampled[i].x;
				py = theHandContourResampled[i].y;
				int whichHandPart = theHandContourResampled[i].z;
				
				float g = whichHandPart*32;
				ofSetColor(127,g,g);
				ofLine (px,py, qx,qy);
				
				qx = px;
				qy = py;
			}
			
			// draw numeric labels
			bool bDrawContourRegionLabels = false;
			if (bDrawContourRegionLabels){
				for (int c=0; c<contourRegionsConsolidated.size(); c++){
					ContourRegion aContourRegion = contourRegionsConsolidated[c];
					int whichHandPart = aContourRegion.finger_id;
					int index0 = aContourRegion.index_start;
					int index1 = aContourRegion.index_end  ;
					int labelIndex = (index0 + index1)/2;
					
					float g = whichHandPart*32;
					ofSetColor(127,g,g);
					px = theHandContourResampled[index0%nContourPts].x;
					py = theHandContourResampled[index0%nContourPts].y;
					ofEllipse (px,py, 3,3);
				}
			}
		}
	}
	
	bool bDrawKnuckles = true;
	if (bDrawKnuckles){
		
		ofFill();
		ofSetColor(ofColor::green);
		int nKnuckles = knuckles.size();
		
		for (int i=0; i<nKnuckles; i++){
			ofVec3f knucklePoint = knuckles[i];
			float px = knucklePoint.x;
			float py = knucklePoint.y;
			
			ofSetColor(ofColor::black);
			ofEllipse (px, py, 6,6);
			
			ofSetColor(ofColor::blue);
			ofEllipse (px, py, 4,4);
			
			ofSetColor(ofColor::blue);
			ofDrawBitmapString( ofToString(i), px+10,py);
		}
		
	}

}


//--------------------------------------------------------------
float HandContourAnalyzer::getInsertionPercentageOfHand (){
	
	// find the leftmost coordinate of the hand; express this as a fraction (0..1) of imgW.
	int nContourPts = theHandContourResampled.size();
	float leftmostValue = 99999;
	for (int i=0; i<nContourPts; i++){
		float aValue = theHandContourResampled[i].x;
		if (aValue < leftmostValue){
			leftmostValue = aValue;
		}
	}
	float percentage = 1.0 - (leftmostValue / (float) imgW);
	return percentage;
}

//--------------------------------------------------------------
float HandContourAnalyzer::getDistanceOfBlobFromEntry(){
	
	// find the rightmost coordinate of the hand; express this as a fraction (0..1) of imgW.
	int nContourPts = theHandContourResampled.size();
	float rightmostValue = 0;
	for (int i=0; i<nContourPts; i++){
		float aValue = theHandContourResampled[i].x;
		if (aValue > rightmostValue){
			rightmostValue = aValue;
		}
	}
	float percentage = 1.0 - (rightmostValue / (float) imgW);
	return percentage;
}


//--------------------------------------------------------------
void HandContourAnalyzer::computeLabeledContour  (const Mat &thresholdedImageOfHandMat,
												  const Mat &leapDiagnosticFboMat){
	
	// -- Fills theHandContourResampled[i].z = whichHandPart;
	// numbered 0-7 with handPartID;
	
	// Given the thresholdedFinal image (thresholdedImageOfHandMat), detect blobs.
	// The thresholdedImageOfHandMat has already been thresholded,
	// so it's OK to use a fixed threshold here of 128.
	bContourExists = false;
	contourFinder.setThreshold(128);
	contourFinder.findContours(thresholdedImageOfHandMat);
	
	
	theHandContourResampled.clear();
	allContourRegionsTEMP.clear();
	contourRegionsConsolidated.clear();
	
	// If there are blobs, select the largest one.
	if (contourFinder.size() > 0){
		bContourExists = true;
		theHandContourRaw = contourFinder.getPolyline(0); // 0'th is the largest.
		
		// Find the rightmost point;
		// Then create a new polyline in which the indices are rotated so that this becomes the 0'th point.
		int rightmostIndex = 0;
		float rightmostValue = 0;
		int nRawPts = theHandContourRaw.size();
		for (int i=0; i<nRawPts; i++){
			float px = theHandContourRaw[i].x;
			if (px > rightmostValue){
				rightmostValue = px;
				rightmostIndex = i;
			}
		}
		// Make handContourRawRotated
		ofPolyline handContourRawRotated;
		handContourRawRotated.resize(nRawPts);
		for (int i=0; i<nRawPts; i++){
			int rotatedIndex = (rightmostIndex + i)%nRawPts;
			ofPoint p = theHandContourRaw[rotatedIndex];
			handContourRawRotated[i].set(p);
		}
		
		
		
		// Resample the ofPolyline of the largest blob
		float handContourSampling  = 2.0;
		float handContourSmoothing = 5.0;
		theHandContourResampled.clear();
		theHandContourResampled = handContourRawRotated.getResampledByCount(DESIRED_N_CONTOUR_POINTS);
		theHandContourResampled = theHandContourResampled.getSmoothed(handContourSmoothing);
		theHandContourResampled.setClosed (true);
		theHandContourArea = abs(theHandContourResampled.getArea());
		
		theHandContourVerySmooth.clear();
		theHandContourVerySmooth = theHandContourResampled.getSmoothed( smoothingOfLowpassContour );
		
		// For each point on the ofPolyline contour,
		unsigned char *colorPixels = leapDiagnosticFboMat.data;
		int nPointsOnContour = theHandContourResampled.size();
		for (int p=0; p<nPointsOnContour; p++){
			ofPoint aPoint = theHandContourResampled[p];
			
			// Fetch the location of the underlying pixel
			int pxi = (int) ofClamp(roundf(aPoint.x), 0, imgW-1);
			int pyi = (int) ofClamp(roundf(aPoint.y), 0, imgH-1);
			int pindex1 = (pyi * imgW) + pxi;
			int pindex3 = pindex1 * 3;
			
			// Fetch the color of the underlying pixel.
			// Note: since we are able to obtain the colors from leapDiagnosticFboMat, this suggests that we
			// no longer need to compositeThresholdedImageWithLeapFboPixels() to compute coloredBinarizedImg
			int pr = colorPixels[pindex3  ];
			int pg = colorPixels[pindex3+1];
			int pb = colorPixels[pindex3+2];
			
			// Store the color's BLUE value (which encodes the finger identity)
			// into the Z value of the corresponding ofPoint in the ofPolyline.
			int whichFinger = pb / 32;
			theHandContourResampled[p].z = whichFinger;
		}
	}
}

	
	


//--------------------------------------------------------------
void HandContourAnalyzer::computeContourRegions (const Mat &thresholdedImageOfHandMat,
												 const Mat &leapDiagnosticFboMat){

	// -- Fills contourRegionsConsolidated with ContourRegions describing theHandContourResampled.
	//
	// Go through the contour and extract all the contourRegions (into a temp vector).
	// Then search through that vector for the longest ones belonging to each finger.
	// (Stitch segments across the 0th point if they have the same id).
	int nPointsOnContour = theHandContourResampled.size();
	if (nPointsOnContour > 0){
		
		allContourRegionsTEMP.clear();
		contourRegionsConsolidated.clear();
		
		//-----------------
		// Identify all the contour regions.
		int fingerToWhichPrevPointBelongs = -1;
		int fingerToWhichCurrPointBelongs = theHandContourResampled[0].z;
		int startIndexOfCurrentContourRegion = 0;
		int endIndexOfCurrentContourRegion   = 0;
		
		int pt=0;
		for (pt=0; pt<nPointsOnContour; pt++){
			fingerToWhichPrevPointBelongs = fingerToWhichCurrPointBelongs;
			fingerToWhichCurrPointBelongs = theHandContourResampled[pt].z;
			if (fingerToWhichPrevPointBelongs != fingerToWhichCurrPointBelongs){
				endIndexOfCurrentContourRegion = pt;
				
				// Change detected. Save the previous ContourRegion.
				ContourRegion aContourRegion;
				aContourRegion.finger_id   = fingerToWhichPrevPointBelongs;
				aContourRegion.index_start = startIndexOfCurrentContourRegion;
				aContourRegion.index_end   = endIndexOfCurrentContourRegion;
				aContourRegion.index_len   = abs(endIndexOfCurrentContourRegion - startIndexOfCurrentContourRegion);
				allContourRegionsTEMP.push_back (aContourRegion);
				
				startIndexOfCurrentContourRegion = pt;
			}
		}
		// Save the last ContourRegion.
		endIndexOfCurrentContourRegion = pt;
		ContourRegion lastContourRegion;
		lastContourRegion.finger_id   = fingerToWhichPrevPointBelongs;
		lastContourRegion.index_start = startIndexOfCurrentContourRegion;
		lastContourRegion.index_end   = endIndexOfCurrentContourRegion;
		lastContourRegion.index_len   = abs(endIndexOfCurrentContourRegion - startIndexOfCurrentContourRegion);
		allContourRegionsTEMP.push_back (lastContourRegion);
		
		//-----------------
		// Repair the contour regions.
		contourRegionsConsolidated.clear();
		for (int repeat = 1; repeat <= 2; repeat++){
			// Repeat for good measure.
			int minFingerContourLength = 30 / repeat;
			
			int contourRegionsPerHandPartID[8] = {0,0,0,0,0,0,0,0};
			
			// Count ContourRegions for/per each finger.
			// See LeapVisualizer.h for HAND_PART_ID enumeration (thumb starts at 3)
			// ID_NONE  = 0, ID_WRIST = 1, ID_PALM  = 2,
			// ID_THUMB = 3, ID_INDEX = 4, ID_MIDDLE = 5, ID_RING  = 6, ID_PINKY = 7
			bool bAllFingersHaveAContourRegion = false;
			int nContourRegionsTEMP = allContourRegionsTEMP.size();
			for (int c=0; c<nContourRegionsTEMP; c++){
				ContourRegion aContourRegion = allContourRegionsTEMP[c];
				int fingerHandPartID = aContourRegion.finger_id;
				if ((fingerHandPartID >= 0) && (fingerHandPartID <= 7)){
					contourRegionsPerHandPartID[fingerHandPartID]++;
				}
			}
			
			// Pre-calculation for consolidated contourRegions:
			// Set teeny segments to have the same ID as the segments flanking them.
			// This appears to be buggy, so forget it. Not too important anyway,
			bool bClobberTeenySegments = false;
			if (bClobberTeenySegments){
				for (int c=0; c<nContourRegionsTEMP; c++){
					// For each contour region in allContourRegionsTEMP
					ContourRegion aContourRegion = allContourRegionsTEMP[c];
					int fingerHandPartID = aContourRegion.finger_id;
					
					int neighborIndexL = (c-1+nContourRegionsTEMP) % nContourRegionsTEMP;
					int neighborIndexR = (c+1+nContourRegionsTEMP) % nContourRegionsTEMP;
					int handPartIdOfNeighborL = allContourRegionsTEMP[neighborIndexL].finger_id;
					int handPartIdOfNeighborR = allContourRegionsTEMP[neighborIndexR].finger_id;
					
					// General case for small fragments:
					// If that contourRegion's handPartID is over-represented
					if (contourRegionsPerHandPartID[fingerHandPartID] > 1){
						
						// identify the longest contourRegion with the same ID
						int indexOfLongestContourRegionWithSameID = -1;
						int lengthOfLongestContourRegionWithSameID = 0;
						for (int d=0; d<nContourRegionsTEMP; d++){
							ContourRegion anotherContourRegion = allContourRegionsTEMP[d];
							if (anotherContourRegion.finger_id == fingerHandPartID){ // if has same id
								if (anotherContourRegion.index_len > lengthOfLongestContourRegionWithSameID){
									lengthOfLongestContourRegionWithSameID = anotherContourRegion.index_len;
									indexOfLongestContourRegionWithSameID = d;
								}
							}
						}
						
						// if our contourRegion is NOT the longest one with that same ID
						if (c != indexOfLongestContourRegionWithSameID){
							
							// And if that contourRegion is shorter than minFingerContourLength
							int index0 = aContourRegion.index_start;
							int index1 = aContourRegion.index_end;
							int length = index1 - index0;
							if (length < minFingerContourLength){
								
								// If that contourRegion is flanked on both sides
								// by contourRegions whose handPartID's are the same, and not 0,
								// set it to the handPartIDs of its neighbors.
								if ((handPartIdOfNeighborL == handPartIdOfNeighborR) && (handPartIdOfNeighborL > 0)){
									allContourRegionsTEMP[c].finger_id = handPartIdOfNeighborL;
								}
								
								// Else if the contourRegion is flanked on both sides
								// by contourRegions with different handPartIDs,
								// set its handPartID to the greater of its neighbors'.
								else if (handPartIdOfNeighborL != handPartIdOfNeighborR){
									int maxhandPartId = MAX (handPartIdOfNeighborL, handPartIdOfNeighborR);
									allContourRegionsTEMP[c].finger_id = maxhandPartId;
								}
								
							}
						}
					}
				}
			}
			
			// Deal with contourRegions whose handPartIDs are 0 (unknown) or 1 (wrist),
			// regardless of their length.
			for (int c=0; c<nContourRegionsTEMP; c++){
				ContourRegion aContourRegion = allContourRegionsTEMP[c];
				int aContourRegionHandPartID = aContourRegion.finger_id;
				
				if ((aContourRegionHandPartID == 0) ||
					(aContourRegionHandPartID == 1)){
					
					int neighborIndexL2 = (c-2+nContourRegionsTEMP) % nContourRegionsTEMP;
					int neighborIndexL1 = (c-1+nContourRegionsTEMP) % nContourRegionsTEMP;
					int neighborIndexR1 = (c+1+nContourRegionsTEMP) % nContourRegionsTEMP;
					int neighborIndexR2 = (c+2+nContourRegionsTEMP) % nContourRegionsTEMP;
					
					int handPartIdOfNeighborL2 = allContourRegionsTEMP[neighborIndexL2].finger_id;
					int handPartIdOfNeighborL1 = allContourRegionsTEMP[neighborIndexL1].finger_id;
					int handPartIdOfNeighborR1 = allContourRegionsTEMP[neighborIndexR1].finger_id;
					int handPartIdOfNeighborR2 = allContourRegionsTEMP[neighborIndexR2].finger_id;
					
					// If its ID is 0 or 1, and both of its neighbors are the same (and > 1), set it to that.
					aContourRegionHandPartID = allContourRegionsTEMP[c].finger_id;
					if (aContourRegionHandPartID == 0){
						if ((handPartIdOfNeighborL1 > 1) && (handPartIdOfNeighborR1 > 1)){
							if (handPartIdOfNeighborL1 == handPartIdOfNeighborR1){
								allContourRegionsTEMP[c].finger_id = handPartIdOfNeighborL1;
							}
						}
					}
					
					// If the segment's ID (0 or 1) is duplicated across the arrayIndex=0 boundary
					aContourRegionHandPartID = allContourRegionsTEMP[c].finger_id;
					if (aContourRegionHandPartID <= 1){
						if (c == 0){
							if ((handPartIdOfNeighborR1 == handPartIdOfNeighborL2) &&
								(aContourRegionHandPartID != handPartIdOfNeighborR1)) {
								allContourRegionsTEMP[c].finger_id = handPartIdOfNeighborR1;
							}
						} else if (c == (nContourRegionsTEMP-1)){
							if ((handPartIdOfNeighborL1 == handPartIdOfNeighborR2) &&
								(aContourRegionHandPartID != handPartIdOfNeighborL1)) {
								allContourRegionsTEMP[c].finger_id = handPartIdOfNeighborL1;
							}
						}
					}
					
					// If its ID is 0, and *either* of its neigbors has handPartID 1 (wrist), set it to 1.
					aContourRegionHandPartID = allContourRegionsTEMP[c].finger_id;
					if (aContourRegionHandPartID == 0){
						if ((handPartIdOfNeighborL1 == 1) || (handPartIdOfNeighborR1 == 1)) {
							allContourRegionsTEMP[c].finger_id = 1;
						}
					}
					
					// If its ID is 0, and it's between two segments whose ID's are different,
					// then assign it to have the higher-numbered ID of the  of the two segments'.
					// unless one of its neighbors' handPartID is 1, in which case, use that.
					aContourRegionHandPartID = allContourRegionsTEMP[c].finger_id;
					if (aContourRegionHandPartID == 0){
						if (handPartIdOfNeighborL1 != handPartIdOfNeighborR1){
							
							if ((handPartIdOfNeighborL1 > 1) && (handPartIdOfNeighborR1 > 1)){
								int maxhandPartID = MAX(handPartIdOfNeighborL1, handPartIdOfNeighborR1);
								allContourRegionsTEMP[c].finger_id = maxhandPartID;
							}
							else {
								allContourRegionsTEMP[c].finger_id = 1;
							}
						}
					}
					
				}
			}
			
			// Consolidate contourRegions.
			contourRegionsConsolidated.clear();
			
			int lastHandPartID = -1;
			for (int c=0; c<nContourRegionsTEMP; c++){
				
				ContourRegion currContourRegion = allContourRegionsTEMP[c];
				int currHandPartID = currContourRegion.finger_id;
				
				int nAddedSoFar = contourRegionsConsolidated.size();
				if (nAddedSoFar > 0){
					ContourRegion lastContourRegion = contourRegionsConsolidated[nAddedSoFar-1];
					lastHandPartID = lastContourRegion.finger_id;
				}
				
				// If the current contourRegion has the same handPartID as the previous,
				// then extend the end of the previous one.
				if (currHandPartID == lastHandPartID){
					contourRegionsConsolidated[nAddedSoFar-1].index_end = currContourRegion.index_end;
				} else {
					contourRegionsConsolidated.push_back (currContourRegion);
				}
			}
			
			allContourRegionsTEMP.clear();
			allContourRegionsTEMP = contourRegionsConsolidated;
		} // close repeat
		
		// Handle the likely scenario in which the final contourRegion labels the same finger as the 0th.
		// If so, take the points from the 0th region, and add them to the end of the last region
		// (Thus exceeding the length of the polyline: we will use % for safety).
		// Then delete the 0'th contourRegion.
		int nCrConsolidated = contourRegionsConsolidated.size();
		int handPartIdFirst = contourRegionsConsolidated[0].finger_id;
		int handPartIdLast  = contourRegionsConsolidated[nCrConsolidated-1].finger_id;
		if (handPartIdFirst == handPartIdLast){
			int end0 = contourRegionsConsolidated[0].index_end;
			contourRegionsConsolidated[nCrConsolidated-1].index_end += end0;
			contourRegionsConsolidated[nCrConsolidated-1].index_len += end0;
			contourRegionsConsolidated.erase (contourRegionsConsolidated.begin());
		}
		
		// print handPartId's of contourRegionsConsolidated
		bool bPrintContourRegions = false;
		if (bPrintContourRegions){
			printf("-------------\n");
			for (int c=0; c<contourRegionsConsolidated.size(); c++){
				int whichhandPartId = (int)(contourRegionsConsolidated[c].finger_id);
				int contourRegionLen = contourRegionsConsolidated[c].index_len;
				printf("%d: (%d)\n", whichhandPartId, contourRegionLen);
			}
		}
		
		// Relabel theHandContourResampled with the hard-won identity information.
		int nContourPts = theHandContourResampled.size();
		for (int c=0; c<contourRegionsConsolidated.size(); c++){
			ContourRegion aContourRegion = contourRegionsConsolidated[c];
			int whichHandPart = aContourRegion.finger_id;
			int index0 = aContourRegion.index_start;
			int index1 = aContourRegion.index_end  ;
			
			for (int p=index0; p<index1; p++){
				int pSafe = p % nContourPts;
				theHandContourResampled[pSafe].z = whichHandPart;
			}
		}
	} // close if (nPointsOnContour > 0), if contour exists
}










//============================================================
void HandContourAnalyzer::assembleHandmarksPreliminary(){
	
	
	
	
	//--------------------------------------
	// ASSEMBLE HANDMARKS
	
	Handmarks[HANDMARK_PINKY_TIP].index			= contourIndexOfPinkyTip;
	Handmarks[HANDMARK_PR_CROTCH].index			= contourIndexOfPRCrotch;
	Handmarks[HANDMARK_RING_TIP].index			= contourIndexOfRingTip;
	Handmarks[HANDMARK_RM_CROTCH].index			= contourIndexOfRMCrotch;
	Handmarks[HANDMARK_MIDDLE_TIP].index		= contourIndexOfMiddleTip;
	Handmarks[HANDMARK_MI_CROTCH].index			= contourIndexOfMICrotch;
	Handmarks[HANDMARK_POINTER_TIP].index		= contourIndexOfPointerTip;
	Handmarks[HANDMARK_POINTER_SIDE].index		= contourIndexOfPointerSide;
	Handmarks[HANDMARK_IT_CROTCH].index			= contourIndexOfITCrotch;
	Handmarks[HANDMARK_THUMB_TIP].index			= contourIndexOfThumbTip;
	Handmarks[HANDMARK_THUMB_KNUCKLE].index		= contourIndexOfThumbKnuckle;
	Handmarks[HANDMARK_THUMB_BASE].index		= contourIndexOfThumbBase;
	Handmarks[HANDMARK_THUMBSIDE_WRIST].index	= contourIndexOfThumbsideWrist;
	Handmarks[HANDMARK_PINKYSIDE_WRIST].index	= contourIndexOfPinkysideWrist;
	Handmarks[HANDMARK_PALM_BASE].index			= contourIndexOfPalmBase;
	Handmarks[HANDMARK_PINKY_SIDE].index		= contourIndexOfPinkySide;
	
	for (int i=0; i<N_HANDMARKS; i++){
		
		Handmarks[i].type = (HandmarkType) i;
		int aHandMarkIndex = Handmarks[i].index;
		if (aHandMarkIndex > -1){
			Handmarks[i].point	= theHandContourResampled [ aHandMarkIndex ];
			Handmarks[i].valid	= true;
			
			// Compute the running average (not yet implemented)
			// float A = 0.8; float B = 1.0-A;
			// Handmarks[i].pointAvg.x = Handmarks[i].pointAvg.x +
			
			// Store the history
			Handmarks[i].pointHistory.push_back( Handmarks[i].point );
			if (Handmarks[i].pointHistory.size() > 5){
				Handmarks[i].pointHistory.pop_front();
			}
			
			
			
		} else if (aHandMarkIndex <= -1){
			Handmarks[i].valid	= false;
		}
	}
}




//============================================================
// From Kyle McDonald, Digital Interaction
// Compute a curvature analysis of the contour,
// in which the angle reported for each point in the contour
// is computed from 3 points, the left & right of which are some number of indices offset away.
//
void HandContourAnalyzer::buildCurvatureAnalysis (ofPolyline& polyline) {
	int offset = sampleOffsetForCurvatureAnalysis;
	handContourCurvatures.clear();
	
	int n = polyline.size();
	if (offset > n) {
		offset = n;
	}
	vector<float> curvature(n);
	for(int i = 0; i < n; i++) {
		int left = i - offset;
		if (left < 0) {
			left += n;
		}
		int right = i + offset;
		if (right >= n) {
			right -= n;
		}
		ofVec2f a = polyline[left], b = polyline[i], c = polyline[right];
		a -= b;
		c -= b;
		float angle = a.angle(c);
		curvature[i] = -(angle > 0 ? angle - 180 : angle + 180);
	}
	
	handContourCurvatures = curvature;
}



//============================================================
void HandContourAnalyzer::estimateFingerTipsFromCircleFitting (LeapVisualizer &lv){
	// Requires that crotchContourIndicesExpanded has been computed with sortFingerCrotches().

	int nContourPts = theHandContourResampled.size();
	if (nContourPts > 0){
		
		bool bWeightFitnessByDistanceFromHandPoint = true;
		float fingerRadiusFudge = 0.618;
	
		for (int whichFingerToFind = ID_THUMB; whichFingerToFind <= ID_PINKY; whichFingerToFind++){
			
			// Get that finger's thickness, and from it, compute the circle radius.
			int whichFingerThickness = whichFingerToFind - ID_THUMB;
			float fingerDiameter = lv.fingerThicknesses[ whichFingerThickness ];
			float circleRadius = fingerDiameter/2.0; // 8.0mm
			circleRadius = circleRadius * fingerRadiusFudge;
			
			// Fetch the start and end indices for our contour search.
			// See assignment of crotchContourIndicesExpanded[] in sortFingerCrotches().
			int crotchIndexA = crotchContourIndicesExpanded[whichFingerToFind   ];
			int crotchIndexB = crotchContourIndicesExpanded[whichFingerToFind -1];
			if ((crotchIndexA > -1) && (crotchIndexB > -1)){
				
				// Reverse their order if necessary.
				int index0 = (crotchIndexA < crotchIndexB) ? crotchIndexA : crotchIndexB;
				int index1 = (crotchIndexA < crotchIndexB) ? crotchIndexB : crotchIndexA;
				
				// If computeContourDistancesFromKeyHandPoints() has been called,
				// And if computeFingerPointsMaximallyDistantFromKeyHandPoints() has been called
				// then we can optionally weight the results by the distance from centroid.
				ofVec3f aFingerDistalPt = fingerDistalPts[whichFingerToFind];
				float distanceFromFingerDistalToWrist = wristPosition.distance(aFingerDistalPt);
				float distanceFromPointiToWrist;
				
				int		indexOfFittestContourPoint  = -1;
				float	fitnessOfFittestContourPoint = 0;
				int		isafe;
				float	frac;
				
				for (int i=index0; i<index1; i++){
					isafe = i%nContourPts;
					
					//int whichHandPartId = (int)(theHandContourResampled[isafe].z);
					//if (whichHandPartId == whichFingerToFind){
					if (true){
						float fitnessOfCircleAti = evaluateCircularFitness (circleRadius, i, whichFingerToFind, index0, index1);
						
						// Weight the fitness to suppress results from the base of the finger.
						if (bWeightFitnessByDistanceFromHandPoint && bCalculatedDistances){
							distanceFromPointiToWrist = theHandContourMetaData[isafe].y;
							
							frac = distanceFromPointiToWrist / distanceFromFingerDistalToWrist;
							frac = function_PennerEaseOutCubic (frac);
							fitnessOfCircleAti *= frac;
						}
						
						// Update our estimate of best position
						if (fitnessOfCircleAti > fitnessOfFittestContourPoint){
							fitnessOfFittestContourPoint = fitnessOfCircleAti;
							indexOfFittestContourPoint = isafe;
						}
					}
				}
				
				// save the data.
				if ((indexOfFittestContourPoint > -1) && (indexOfFittestContourPoint < nContourPts)){
					switch (whichFingerToFind){
						case ID_THUMB:	 contourIndexOfThumbTip   = indexOfFittestContourPoint; break;
						case ID_INDEX:	 contourIndexOfPointerTip = indexOfFittestContourPoint; break;
						case ID_MIDDLE:	 contourIndexOfMiddleTip  = indexOfFittestContourPoint; break;
						case ID_RING:	 contourIndexOfRingTip    = indexOfFittestContourPoint; break;
						case ID_PINKY:	 contourIndexOfPinkyTip   = indexOfFittestContourPoint; break;
					}
				}
				//printf("finger: %d	index: %d\n", whichFingerToFind, indexOfFittestContourPoint);
			}
		}
	}
}


//============================================================
void HandContourAnalyzer::evaluateCrotchQuality(){

	for (int i=0; i<4; i++){
		int indexA, indexB, indexC;
		// indexA is the first fingertip
		// indexC is the second fingertip
		// indexB is the crotch in between them
		
		int whichCrotch = 3-i; // yeah, I know
		indexB = crotchContourIndices[whichCrotch];
		
		switch(i){
			case 0:
				indexA = contourIndexOfThumbTip;
				indexC = contourIndexOfPointerTip;
				break;
			case 1:
				indexA = contourIndexOfPointerTip;
				indexC = contourIndexOfMiddleTip;
				break;
			case 2:
				indexA = contourIndexOfMiddleTip;
				indexC = contourIndexOfRingTip;
				break;
			case 3:
				indexA = contourIndexOfRingTip;
				indexC = contourIndexOfPinkyTip;
				break;
		}
		
		if ((indexA > -1) && (indexB > -1) && (indexC > -1)){
			ofVec3f pA = theHandContourResampled[indexA];
			ofVec3f pB = theHandContourResampled[indexB];
			ofVec3f pC = theHandContourResampled[indexC];
			
			// Create a triangle between the two fingertips and the crotch between them.
			// As a measure of crotch quality, compute the "height" of the triangle:
			// The perpendicular distance from the AC line to point B
			// See http://www.mathopenref.com/coordtrianglearea.html
			float baseLength = pA.distance(pC);
			float triangleArea = abs(pA.x*(pB.y-pC.y) + pB.x*(pC.y-pA.y) + pC.x*(pA.y-pB.y))/2.0;
			float triangleHeight = 2.0 * triangleArea / baseLength;
			
			// Create a normalized value, based on the relationship between the triangle height and
			// the distance from the centerpoint-between-the-two-fingertips to the wristPosition.
			ofVec3f baseCenter = (pA+pC)/2.0;
			float distanceFromBaseCenterToWrist = wristPosition.distance(baseCenter);
			float triangleHeightToWristFraction = triangleHeight / distanceFromBaseCenterToWrist;
			crotchQuality[whichCrotch] = triangleHeightToWristFraction;
			
			bool bDoDrawCrotchQualityEvaluations = true;
			if (bDoDrawCrotchQualityEvaluations){
				ofSetColor( 50+i*50, 50+(3-i)*50, 50);
				ofLine(pA, pB);
				ofLine(pB, pC);
				ofLine(pA, pC);
				ofDrawBitmapString(ofToString(whichCrotch) + " " + ofToString(triangleHeightToWristFraction), (pA.x+pC.x)/2.0 - 40, (pA.y+pC.y)/2.0);
			}
													 
		} else {
			crotchQuality[whichCrotch] = 0;
		}
	}
}


//============================================================
void HandContourAnalyzer::refineFingerTipsBasedOnCrotchQuality(){
	
	// assumes computeContourDistancesFromKeyHandPoints() and evaluateCrotchQuality() have been called.
	// requires: crotchQuality, theHandContourMetaData
	
	for (int i=0; i<5; i++){
		
		// indexC;
		// indexA is the first crotch
		// indexC is the second crotch
		// indexB is the fingertip in between them
		int indexA, indexB, indexC;
		float crotchQualityA, crotchQualityC;
		
		switch (i){
			case 0:
				indexB = contourIndexOfThumbTip;
				indexA = crotchContourIndicesExpanded[2        ];
				indexC = crotchContourIndicesExpanded[ID_THUMB ];
				crotchQualityA = 1.0;
				crotchQualityC = crotchQuality[3];
				break;
			
			case 1:
				indexB = contourIndexOfPointerTip;
				indexA = crotchContourIndicesExpanded[ID_THUMB ];
				indexC = crotchContourIndicesExpanded[ID_INDEX ];
				crotchQualityA = crotchQuality[3];
				crotchQualityC = crotchQuality[2];
				break;
			
			case 2:
				indexB = contourIndexOfMiddleTip;
				indexA = crotchContourIndicesExpanded[ID_INDEX ];
				indexC = crotchContourIndicesExpanded[ID_MIDDLE];
				crotchQualityA = crotchQuality[2];
				crotchQualityC = crotchQuality[1];
				break;
			
			case 3:
				indexB = contourIndexOfRingTip;
				indexA = crotchContourIndicesExpanded[ID_MIDDLE];
				indexC = crotchContourIndicesExpanded[ID_RING  ];
				crotchQualityA = crotchQuality[1];
				crotchQualityC = crotchQuality[0];
				break;
			
			case 4:
				indexB = contourIndexOfPinkyTip;
				indexA = crotchContourIndicesExpanded[ID_RING  ];
				indexC = crotchContourIndicesExpanded[ID_PINKY ];
				crotchQualityA = crotchQuality[0];
				crotchQualityC = 1.0;
				break;
		}
		
		float crotchQualityGoodEnoughToWarrantFingertipRefinement = 0.04;
		if ((crotchQualityA > crotchQualityGoodEnoughToWarrantFingertipRefinement) &&
			(crotchQualityC > crotchQualityGoodEnoughToWarrantFingertipRefinement)){
			
			if ((indexA > -1) && (indexB > -1) && (indexC > -1)){
				
				// Reverse their order if necessary.
				int indexLo = (indexA < indexC) ? indexA : indexC;
				int indexHi = (indexA < indexC) ? indexC : indexA;
				if ((indexB > indexLo) && (indexB < indexHi)){
					
					
					// find max distance in that region. used for normalization later.
					float maxDistanceInRegion = 0;
					for (int j=indexLo; j<indexHi; j++){
						float distanceAtJ = theHandContourMetaData[j].y;
						if (distanceAtJ > maxDistanceInRegion){
							maxDistanceInRegion = distanceAtJ;
						}
					}
					// find max curvature in that region. used for normalization later.
					float maxCurvatureInRegion = 0;
					for (int j=indexLo; j<indexHi; j++){
						float curvatureAtJ = handContourCurvatures[j];
						if (curvatureAtJ > maxCurvatureInRegion){
							maxCurvatureInRegion = curvatureAtJ;
						}
					}
					
					
					float bestValue = 0;
					int bestIndex = indexB;
					
					for (int j=indexLo; j<indexHi; j++){
						float distanceAtJ01 = theHandContourMetaData[j].y / maxDistanceInRegion;
						float curvatureAtJ01 = handContourCurvatures[j] / maxCurvatureInRegion;
						
						float jFrac = ofMap(j, indexLo,indexHi, 0,1);
						float tukeyMult01 = function_TukeyWindow(jFrac, 0.25);
						float product = tukeyMult01 * distanceAtJ01 * curvatureAtJ01;
						
						if (product > bestValue){
							bestValue = product;
							bestIndex = j;
						}
					}
					
					// crotchQualityData
					// printf("F%d	bestValue = %f	maxCurvRat: %f	maxDistRat: %f\n", i, bestValue, maxCurvatureRatio, maxDistanceRatio);
					
					if (bestIndex != indexB){
						switch (i){
							case 0: contourIndexOfThumbTip   = bestIndex; break;
							case 1: contourIndexOfPointerTip = bestIndex; break;
							case 2: contourIndexOfMiddleTip  = bestIndex; break;
							case 3: contourIndexOfRingTip    = bestIndex; break;
							case 4: contourIndexOfPinkyTip   = bestIndex; break;
						}
						// printf("moved %d to %d\n", indexB, bestIndex);
					}
				}
				
				
			}
		}

	}
}

//============================================================
void HandContourAnalyzer::refineOtherHandmarks(){
	
	// -----------------------
	// Refine Handmark 7 (contourIndexOfPointerSide): Clobber it if it is out of range.
	// Handmark 7 contourIndexOfPointerSide / (HANDMARK_POINTER_SIDE) should be between
	// Handmark 6 contourIndexOfPointerTip / (HANDMARK_POINTER_TIP) and
	// Handmark 8 contourIndexOfITCrotch / (HANDMARK_IT_CROTCH)
	int index6 = contourIndexOfPointerTip;
	int index8 = contourIndexOfITCrotch;
	float frac7 = (contourIndexOfPointerSide - index6)/(float)(index8 - index6);
	float targetFrac7 = 0.75;
	if ((crotchQuality[3] > 0) && (crotchQuality[3] < 0.20)){
		targetFrac7 += (0.20 - crotchQuality[3]);
		targetFrac7 = ofClamp(targetFrac7, 0.50, 0.95);
	}
	contourIndexOfPointerSide = (int)ofMap(targetFrac7,0.0,1.0, index6,index8);
	
}



//============================================================
int HandContourAnalyzer::getIndexWhereLineSegmentIntersectsPolyline (float x1, float y1, float x2, float y2, ofPolyline &aPolyline){
	float x3,y3, x4,y4;
	int outputIndex = -1;
	
	int nContourPts = aPolyline.size();
	for (int i=0; i<nContourPts; i++){
		int j=i+1;
		if (j >= nContourPts){ j-= nContourPts; }
		x3 = aPolyline[i].x;
		y3 = aPolyline[i].y;
		x4 = aPolyline[j].x;
		y4 = aPolyline[j].y;
		
		float denominator = ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
		if (denominator != 0.0){
			float ua = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / denominator;
			float ub = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / denominator;
			
			if ((ua > 0.0) && (ua < 1.0) &&
				(ub > 0.0) && (ub < 1.0)){ // found it;
				outputIndex = i;
			}
		}
	}
	return outputIndex;
}





//============================================================
int HandContourAnalyzer::getIndexOfClosestPointOnContour (ofVec3f &aPoint, ofPolyline &aPolyline){
	
	// Fetch the index of the contour closest to boneCenters[0].
	float minDistValue = 99999;
	int   minDistIndex = 0;
	int	  nContourPts  = aPolyline.size();
	
	for (int i=0; i<nContourPts; i++){
		float dist = aPoint.distance(aPolyline[i]);
		if (dist < minDistValue){
			minDistValue = dist;
			minDistIndex = i;
		}
	}
	
	return minDistIndex;
}



//============================================================
float HandContourAnalyzer::evaluateCircularFitness (float circleRadius, int atWhichContourIndex, int forWhichHandPartId, int ind0, int ind1) {
	// Sum of perpendicular distance from circle.
	
	ofPolyline &polylineForCircleFitTest = theHandContourVerySmooth; //theHandContourResampled
	
	float out = 0;
	int delta = 1;
	int nContourPts = polylineForCircleFitTest.size();
	float testLen = circleRadius;
	float circleRadiusSquared = circleRadius*circleRadius;
	
	int J = atWhichContourIndex; if (J >= nContourPts){ J -= nContourPts; }
	int K = J + delta;           if (K >= nContourPts){ K -= nContourPts; }
	
	float cp1x = polylineForCircleFitTest[J].x;
	float cp1y = polylineForCircleFitTest[J].y;
	float cq1x = polylineForCircleFitTest[K].x;
	float cq1y = polylineForCircleFitTest[K].y;
	
	float cdx = cq1x - cp1x;
	float cdy = cq1y - cp1y;
	float cdh = sqrt(cdx*cdx + cdy*cdy);
	cdx /= cdh;
	cdy /= cdh;
	
	// Given a circle located tangent to the contour point, theHandContourResampled[J],
	float circlex = cp1x + cdy*circleRadius;
	float circley = cp1y - cdx*circleRadius;
	
	//ofFill();
	//ofSetColor(255,255,100, 10);
	//ofEllipse(circlex, circley, circleRadius*2, circleRadius*2);
	
	float sumWeWant = 0;
	float externalSearchFudge =  0.15; // how much the searchline is outside the hand contour
	
	float p1x, p1y;
	float p2x, p2y;
	float q1x, q1y;
	float dx,dy,dh;
	float cpx, cpy;
	float idx, idy;
	float idh1, idh2;
	float distanceFromContourToCircle;
	
	for (int i=ind0; i<ind1; i++) {
		int I0 = i; if (I0 >= nContourPts){ I0 -= nContourPts;}
		ofVec3f contourPointI0 = polylineForCircleFitTest[I0];
		// if (contourPointI0.z == forWhichHandPartId){ // already known to be true
			int I1 = I0 + delta; if (I1 >= nContourPts){ I1 -= nContourPts;}
			ofVec3f contourPointI1 = polylineForCircleFitTest[I1];
			
			cpx = p1x = contourPointI0.x;
			cpy = p1y = contourPointI0.y;
			q1x       = contourPointI1.x;
			q1y       = contourPointI1.y;
			
			dx = q1x - p1x;
			dy = q1y - p1y;
			dh = sqrt(dx*dx + dy*dy);
			if (dh > 0) {
		
				dx *= testLen/dh;
				dy *= testLen/dh;
				
				p1x = p1x - dy*externalSearchFudge;
				p1y = p1y + dx*externalSearchFudge;
				p2x = p1x + dy;
				p2y = p1y - dx;
				
				// ofSetColor (100,255,255, 25);
				// ofLine(p1x, p1y, p2x, p2y);
				
				FindLineCircleIntersections (circlex, circley, circleRadiusSquared, p1x, p1y, p2x, p2y);
				
				if (bIntersectionExists1 || bIntersectionExists2) {
					idh1 = 9999999;
					idh2 = 9999999;
					
					if (bIntersectionExists1) {
						idx = intersection1.x - cpx;
						idy = intersection1.y - cpy;
						idh1 = /*sqrt*/ (idx*idx + idy*idy);
					}
					if (bIntersectionExists2) {
						idx = intersection2.x - cpx;
						idy = intersection2.y - cpy;
						idh2 = /*sqrt*/ (idx*idx + idy*idy);
					}
					
					distanceFromContourToCircle = sqrtf (min (idh1, idh2));
					sumWeWant += (testLen - distanceFromContourToCircle);
				}
			}
	 // }
	}

	out = sumWeWant;
	return out;
}




//============================================================
// Find the points of intersection.
inline void HandContourAnalyzer::FindLineCircleIntersections (
		float cx, float cy, float radiusSquared,
		float p1x, float p1y, float p2x, float p2y) {
	
	float dx, dy, A, B, C, det, t;
	dx = p2x - p1x;
	dy = p2y - p1y;
	
	A = dx * dx + dy * dy;
	B = 2 * (dx * (p1x - cx) + dy * (p1y - cy));
	C = (p1x - cx) * (p1x - cx) + (p1y - cy) * (p1y - cy) - radiusSquared;
	det = B * B - 4 * A * C;
	
	intersection1.set(0, 0);
	intersection2.set(0, 0);
	bIntersectionExists1 = false;
	bIntersectionExists2 = false;
	
	if ((A < 0.000001) || (det < 0)) {
		// No real solutions.
		;
	} else if (det == 0) {
		// One solution.
		t = -B / (2.0 * A);
		if ((t >= 0.0) && (t <= 1.0)) {
			intersection1.set(p1x + t * dx, p1y + t * dy);
			bIntersectionExists1 = true;
		}
	} else {
		// Two solutions.
		float sqrtDet = sqrtf(det);
		t = ((-B + sqrtDet) / (2.0 * A));
		if ((t >= 0.0) && (t <= 1.0)) {
			intersection1.set(p1x + t * dx, p1y + t * dy);
			bIntersectionExists1 = true;
		}
		t = ((-B - sqrtDet) / (2.0 * A));
		if ((t >= 0.0) && (t <= 1.0)) {
			intersection2.set(p1x + t * dx, p1y + t * dy);
			bIntersectionExists2 = true;
		}
	}
}


//--------------------------------------------------------------
SlopeInterceptLine HandContourAnalyzer::computeFitLine (vector<ofVec3f> points, int startPointIndex, int endPointIndex){
	
	// http://faculty.cs.niu.edu/~hutchins/csci230/best-fit.htm
	// find the line that fits the requested points (inclusive).
	// Formula Y = Slope * X + YInt
	SlopeInterceptLine outputLineProperties;
	outputLineProperties.slope = 0;
	outputLineProperties.yIntercept	= 0;
	
	int nPoints = points.size();
	if ((nPoints > 0) && (startPointIndex < nPoints) && (endPointIndex < nPoints)){
		
		float SumX  = 0; // sum of all the X values
		float SumY  = 0; // sum of all the Y values
		float SumX2 = 0; // sum of the squares of the X values
		float SumXY = 0; // sum of the products X*Y for all the points
		
		int nContributingPoints = 0;
		for (int i = startPointIndex; i <= endPointIndex; i++){
			ofVec3f aPoint = points[i];
			SumX  +=  aPoint.x;
			SumY  +=  aPoint.y;
			SumX2 += (aPoint.x * aPoint.x);
			SumXY += (aPoint.x * aPoint.y);
			nContributingPoints++;
		}
		
		float XMean = SumX / (float) nContributingPoints;
		float YMean = SumY / (float) nContributingPoints;
		float Slope = (SumXY - SumX * YMean) / (SumX2 - SumX * XMean);
		float YInt  = YMean - Slope * XMean;
		
		outputLineProperties.slope      = Slope;
		outputLineProperties.yIntercept	= YInt;
	}
	
	return outputLineProperties;
}

//============================================================
float HandContourAnalyzer::distanceFromPointToLine (ofVec3f linePt1, ofVec3f linePt2,  ofVec3f aPoint){
	// see http://paulbourke.net/geometry/pointlineplane/
	
	float p1x = linePt1.x;
	float p1y = linePt1.y;
	float p2x = linePt2.x;
	float p2y = linePt2.y;
	
	float dx = p2x - p1x;
	float dy = p2y - p1y;
	float lineMag2 = dx*dx + dy*dy;
	
	float p3x  = aPoint.x;
	float p3y  = aPoint.y;
	float u   = ((p3x-p1x)*(p2x-p1x) + (p3y-p1y)*(p2y-p1y)) / lineMag2;
	
	// intersection point
	float inx = p1x + u * (p2x - p1x);
	float iny = p1y + u * (p2y - p1y);
	
	float dist = ofDist (p3x,p3y, inx,iny);
	return dist;
}



//------------------------------------------------------------------
inline float HandContourAnalyzer::function_PennerEaseOutQuartic (float t) {
	// functionName = "Penner's EaseOut Quartic";
	t = t-1;
	return -1.0 * (t*t*t*t - 1.0);
}

//------------------------------------------------------------------
inline float HandContourAnalyzer::function_PennerEaseOutQuintic (float t) {
	// functionName = "Penner's EaseOut Quintic";
	t = t-1;
	return (t*t*t*t*t + 1.0);
}

//------------------------------------------------------------------
inline float HandContourAnalyzer::function_PennerEaseOutCubic (float x) {
	// functionName = "Penner's EaseOut Cubic";
	x = x-1.0;
	return (x*x*x + 1);
}

//------------------------------------------------------------------
inline float HandContourAnalyzer::function_PennerEaseInOutCubic (float x) {
	// functionName = "Penner's EaseInOut Cubic";
	
	x *= 2.0;
	float y = 0;
	
	if (x < 1) {
		y = 0.5 * x*x*x;
	}
	else {
		x -= 2.0;
		y = 0.5 * (x*x*x + 2.0);
	}
	return y;
}

//------------------------------------------------------------------
inline float HandContourAnalyzer::function_DoubleExponentialSigmoid (float x, float a){
	// functionName = "Double-Exponential Sigmoid";
	
	float min_param_a = 0.0 + EPSILON;
	float max_param_a = 1.0 - EPSILON;
	a = ofClamp(a, min_param_a, max_param_a);
	a = 1-a;
	
	float y = 0;
	if (x<=0.5){
		y = (powf(2.0*x, 1.0/a))/2.0;
	}
	else {
		y = 1.0 - (powf(2.0*(1.0-x), 1.0/a))/2.0;
	}
	return y;
}


//------------------------------------------------------------------
inline float HandContourAnalyzer::function_TukeyWindow (float x, float a) {
	// functionName = "Tukey Window";
	// http://en.wikipedia.org/wiki/Window_function
	// The Tukey window, also known as the tapered cosine window,
	// can be regarded as a cosine lobe of width \tfrac{\alpha N}{2}
	// that is convolved with a rectangle window of width \left(1 -\tfrac{\alpha}{2}\right)N.
	// At alpha=0 it becomes rectangular, and at alpha=1 it becomes a Hann window.
	
	float ah = a/2.0;
	float omah = 1.0 - ah;
	
	float y = 1.0;
	if (x <= ah) {
		y = 0.5 * (1.0 + cos(PI* ((2*x/a) - 1.0)));
	}
	else if (x > omah) {
		y = 0.5 * (1.0 + cos(PI* ((2*x/a) - (2/a) + 1.0)));
	}
	return y;
}


