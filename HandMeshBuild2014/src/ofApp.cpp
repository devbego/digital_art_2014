#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	bLoadL = true; // load L or R hand.
	if (bLoadL){
		handImage.loadImage("hand-L.png");
	} else {
		handImage.loadImage("hand-R.png");
	}
	handImageColor.allocate(handImage.width, handImage.height);
	handImageGray.allocate(handImage.width, handImage.height);
	handImageThresholded.allocate(handImage.width, handImage.height);
	
	handImageColor.setFromPixels(handImage.getPixels(), handImage.width, handImage.height);
	handImageGray = handImageColor;
	handImageThresholded = handImageGray;
	handImageThresholded.threshold(30);
	
	int maxBlobArea = (int)(handImage.width * handImage.height * 0.666);
	contourFinder.findContours(handImageThresholded, 100, maxBlobArea, 1, false);
	handContourRaw = contourFinder.blobs[0].pts;
	handContourRaw = handContourRaw.getResampledByCount(DESIRED_N_CONTOUR_POINTS);
	//handContourRaw = handContourRaw.getSmoothed(5.0);
	
	// rotate contour so that bottomost point has index 0.
	// find bottommost point;
	int nPts = handContourRaw.size();
	int indexOfBottomY = 0;
	float valueOfBottomY = 0;
	for (int i=0; i<nPts; i++){
		if (handContourRaw[i].y > valueOfBottomY){
			valueOfBottomY = handContourRaw[i].y;
			indexOfBottomY = i;
		}
	}
	// create new array starting there.
	handContourFinal.clear();
	for (int i=0; i<nPts; i++){
		int j = (i+indexOfBottomY)%nPts;
		float px = handContourRaw[j].x;
		float py = handContourRaw[j].y;
		handContourFinal.addVertex(px, py);
	}
	
	//-------------------
	// Fill in the Handmarks array.
	for (int i=0; i<N_HANDMARKS; i++){
		HandmarksInput[i].type = (HandmarkType)i;
		HandmarksInput[i].index = -1;
		HandmarksInput[i].valid = false;
		HandmarksInput[i].point.set(0,0);
		
	}
	handCentroid.set(282, 369, 0);
	float handmarkCoords[16][2] = {
		{149, 218},
		{228, 296},
		{198, 145},
		{270, 279},
		{290, 102},
		{318, 275},
		{380, 140},
		{361, 295},
		{384, 360},
		{486, 332},
		{409, 410},
		{349, 481},
		{352, 666},
		{236, 667},
		{237, 478},
		{197, 317}
	};
	
	if (!bLoadL){
		handCentroid.x = handImage.width - handCentroid.x;
		for (int i=0; i<16; i++){
			handmarkCoords[i][0] = handImage.width - handmarkCoords[i][0];
		}
	}
	
	
	for (int i=0; i<N_HANDMARKS; i++){
		float px = handmarkCoords[i][0];
		float py = handmarkCoords[i][1];
		HandmarksInput[i].point.set(px,py);
		
		int indexOfClosest = getIndexOfClosestPointOnContour(handContourFinal, px,py);
		HandmarksInput[i].index = indexOfClosest;
		HandmarksInput[i].valid = true;
	}


}

//--------------------------------------------------------------
void ofApp::update(){
	
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	ofFill();
	ofSetColor(100);
	handImageThresholded.draw(0,0);
	
	buildMesh (handContourFinal, handCentroid, HandmarksInput);
	
	
	
	if (myContour.size() > 0){
		
		// myContour.draw();
		// ofEllipse(handCentroid.x,handCentroid.y, 7,7);
		
		bool bDrawHandmarks = true;
		if (bDrawHandmarks){
			for (int i=0; i<N_HANDMARKS; i++){
				ofFill();
				ofSetColor(255,100,0);
				
				float px = myHandmarks[i].point.x;
				float py = myHandmarks[i].point.y;
				// ofSetColor(255,100,0, 190);
				// ofEllipse(px,py, 7,7);
				
				int index = myHandmarks[i].index;
				ofDrawBitmapString( ofToString(index), px-20,py-20);
				float qx = myContour[index].x;
				float qy = myContour[index].y;
				ofEllipse(qx,qy, 7,7);
			}
		}
		
		ofSetColor(255);
		handMesh.drawWireframe();
	}
	
	
	
	
		
}


//--------------------------------------------------------------
int ofApp::getMeshVertexIndexOfControlPoint (int which){
	
	/*
	 pinkyKnuckle	137
	 ringKnuckle	139
	 midKnuckle		141
	 pointerKnuckle	143
	 centroid		129
	 wrist 			115
	 */
	
	return 0;
}




//--------------------------------------------------------------
void ofApp::buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks){

	bCalculatedMesh = false;
	bRenderIntermediate = false;
	
	bool bDraw = false;
	nTrianglesAccum = 0;
	verticesPerFinger = 21;
	
	// Check to make sure the contour contains data
	int nContourPoints = handContour.size();
	if (nContourPoints > 0){
		
		// Check to make sure all indices are legal
		bool bAllHandmarkIndicesValid = true;
		for (int i=0; i<N_HANDMARKS; i++){
			if ((hmarks[i].index < 0) || (hmarks[i].index >= nContourPoints)){
				bAllHandmarkIndicesValid = false;
				printf ("problem with handmark %d: %d\n", i, hmarks[i].index);
                return;
			}
		}
		if (bAllHandmarkIndicesValid){
						
			// Determine if the order of the points in contour needs to be reversed (such as for left hands).
			// Use that when creating our local copy of the contour and handmarks.
			// This fills myHandmarks and myContour, which are now set.
			bool bContourNeedsToBeReversed = doesContourNeedToBeReversed (handContour, hmarks);
			createLocalCopyOfContourAndHandmarks (handContour, hmarks, bContourNeedsToBeReversed );
			bWindingCCW = !bContourNeedsToBeReversed;
			
			// Clear our product, the handMesh
			handMesh.clear();
			handMesh.setupIndicesAuto();
			handMesh.setMode( OF_PRIMITIVE_TRIANGLES );
			
			// Add fingers to handMesh.
			addFingersToHandMesh();
			
			// Add wrist to handMesh.
			addWristToHandMesh();
				
			// Add base of palm to handMesh
			addPalmToHandMesh();
				
			// Add thumb webbing to handMesh
			addThumbWebbingToHandMesh();
			
			bCalculatedMesh = true;
		}
	}
	
}



//--------------------------------------------------------------
void ofApp::checkForDuplicatedVertices(){
	vector<ofVec3f> verts = handMesh.getVertices();
	int nVerts = verts.size();
	// printf("nVerts = %d\n" ,nVerts);
	for (int i=0; i<nVerts; i++){
		ofVec3f ipoint = verts[i];
		for (int j=0; j<i; j++){
			ofVec3f jpoint = verts[j];
			float distance = jpoint.distance(ipoint);
			if (distance < 0.01){
				printf("Problem with %d %d\n", i,j);
				ofSetColor(0,255,255);
				ofEllipse(jpoint.x, jpoint.y, 14,14);
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::addThumbWebbingToHandMesh(){
	// Create a gusset for the thumb.
	
	int contourIndex07 = myHandmarks[HANDMARK_POINTER_SIDE].index;
	int contourIndex08 = myHandmarks[HANDMARK_IT_CROTCH].index;
	int contourIndex11 = myHandmarks[HANDMARK_THUMB_BASE].index;
	int contourIndex10 = myHandmarks[HANDMARK_THUMB_KNUCKLE].index;
	
	// Assumes that thumbsidePalmVertexIndices[6] have already been calculated.

	//-----------
	// Create (and store the 5 vertex indices of) the thumb base vertices,
	// which interpolate between the thumb base and the thumb knuckle.
	ofPoint thumbKnucklePt = myContour[contourIndex10];
	ofPoint thumbBasePt    = myContour[contourIndex11];
	int thumbBaseVertexIndices[5];
	thumbBaseVertexIndices[0] = handMeshWristVertexIndex+1;
	thumbBaseVertexIndices[4] = 2;
	for (int i=1; i<4; i++){
		float fraction = (float)i/4.0;
		ofPoint interpPoint = (1.0-fraction)*thumbBasePt + fraction*thumbKnucklePt;
		ofPoint contourPt = myContour.getClosestPoint(interpPoint);
		thumbBaseVertexIndices[i] = handMesh.getNumVertices();
		handMesh.addVertex( ofPoint(contourPt.x, contourPt.y, 0.0));
	}
	
	//-----------
	// Create and store the indices of the thumbPedestalVertexIndices[5];
	// Most are already known to us, but #3 must be computed.
	int thumbPedestalVertexIndices[5];
	thumbPedestalVertexIndices[4] = thumbsidePalmVertexIndices[5];
	thumbPedestalVertexIndices[2] = 0; // hardcoded base of thumb
	thumbPedestalVertexIndices[1] = 1;
	thumbPedestalVertexIndices[0] = 2;
	thumbPedestalVertexIndices[3] = handMesh.getNumVertices();
	ofPoint pointerSidePt = myContour[contourIndex07];
	ofPoint itCrotchPt    = myContour[contourIndex08];
	ofPoint midpoint78 = (pointerSidePt + itCrotchPt)/2.0;
	ofPoint contour78Pt = myContour.getClosestPoint(midpoint78);
	int indexOfInterpPedestalVert = handMesh.getNumVertices();
	handMesh.addVertex( ofPoint(contour78Pt.x, contour78Pt.y, 0.0));
	
	//--------------
	// Add interior vertices.
	for (int i=1; i<5; i++){
		float topFrac = (float)(i-1)/3.0;
		ofPoint topLPt = (handMesh.getVertices())[thumbsidePalmVertexIndices[2]];
		ofPoint topRPt = (handMesh.getVertices())[thumbPedestalVertexIndices[1]];
		ofPoint topPoint = (1.0-topFrac)*topLPt + (topFrac*topRPt);
		
		if ((i>1) && (i<4)){
			handMesh.addVertex( ofPoint(topPoint.x, topPoint.y, 0.0));
		}
	}
	ofPoint topLPt = (handMesh.getVertices())[thumbsidePalmVertexIndices[3]];
	ofPoint topRPt = (handMesh.getVertices())[thumbPedestalVertexIndices[2]];
	ofPoint topPoint = 0.5*topLPt + 0.5*topRPt;
	handMesh.addVertex( ofPoint(topPoint.x, topPoint.y, 0.0));
	
	//--------------
	// ADD TRIANGLES.
	// The thumb gusset is the most artisanal mesh in the system.
	
	if (bWindingCCW){
		handMesh.addTriangle(thumbsidePalmVertexIndices[1], thumbsidePalmVertexIndices[0],
							 thumbBaseVertexIndices[1]); nTrianglesAccum++;
		handMesh.addTriangle(thumbsidePalmVertexIndices[2], thumbsidePalmVertexIndices[1],
							 thumbBaseVertexIndices[1]); nTrianglesAccum++;
	} else {
		handMesh.addTriangle(thumbsidePalmVertexIndices[0], thumbsidePalmVertexIndices[1],
							 thumbBaseVertexIndices[1]); nTrianglesAccum++;
		handMesh.addTriangle(thumbsidePalmVertexIndices[1], thumbsidePalmVertexIndices[2],
							 thumbBaseVertexIndices[1]); nTrianglesAccum++;
	}
	
	int interiorVIndices = handMesh.getNumVertices() - 3;
	int topRowA[4] = {thumbsidePalmVertexIndices[2],interiorVIndices, interiorVIndices+1, 1};
	for (int i=1; i<4; i++){
		int botIndex0 = thumbBaseVertexIndices[i  ];
		int botIndex1 = thumbBaseVertexIndices[i+1];
		int topIndex0 = topRowA[i-1];
		int topIndex1 = topRowA[i  ];
		if (bWindingCCW){
			handMesh.addTriangle(botIndex0, topIndex1, topIndex0);	nTrianglesAccum++;
			handMesh.addTriangle(botIndex0, botIndex1, topIndex1);	nTrianglesAccum++;
		} else {
			handMesh.addTriangle(botIndex0, topIndex0, topIndex1);	nTrianglesAccum++;
			handMesh.addTriangle(botIndex0, topIndex1, botIndex1);	nTrianglesAccum++;
		}
	}
	
	int topRowB[3] = {thumbsidePalmVertexIndices[3],interiorVIndices+2,0};
	for (int i=0; i<3; i++){
		if (bWindingCCW){
			handMesh.addTriangle(topRowB[i],topRowA[i],topRowA[i+1]); nTrianglesAccum++;
			if (i<2){ handMesh.addTriangle(topRowB[i],topRowA[i+1],topRowB[i+1]); nTrianglesAccum++; }
		} else {
			handMesh.addTriangle(topRowB[i],topRowA[i+1], topRowA[i]); nTrianglesAccum++;
			if (i<2){ handMesh.addTriangle(topRowB[i],topRowB[i+1], topRowA[i+1]); nTrianglesAccum++; }
		}
	}
	
	if (bWindingCCW){
		handMesh.addTriangle (thumbsidePalmVertexIndices[4], thumbsidePalmVertexIndices[3], topRowB[1]); nTrianglesAccum++;
		handMesh.addTriangle (thumbsidePalmVertexIndices[4], topRowB[1], indexOfInterpPedestalVert); nTrianglesAccum++;
		handMesh.addTriangle (indexOfInterpPedestalVert, topRowB[1], topRowB[2]); nTrianglesAccum++;
		handMesh.addTriangle (thumbsidePalmVertexIndices[5], thumbsidePalmVertexIndices[4], indexOfInterpPedestalVert); nTrianglesAccum++;
	} else {
		handMesh.addTriangle (thumbsidePalmVertexIndices[4], topRowB[1], thumbsidePalmVertexIndices[3]); nTrianglesAccum++;
		handMesh.addTriangle (thumbsidePalmVertexIndices[4], indexOfInterpPedestalVert, topRowB[1]); nTrianglesAccum++;
		handMesh.addTriangle (indexOfInterpPedestalVert, topRowB[2], topRowB[1]); nTrianglesAccum++;
		handMesh.addTriangle (thumbsidePalmVertexIndices[4], thumbsidePalmVertexIndices[5], indexOfInterpPedestalVert); nTrianglesAccum++;
	}
	
	//-----------
	if (bRenderIntermediate){
	
		// Draw the thumbBaseVertexIndices[5] (RED)
		for (int i=0; i<5; i++){
			ofPoint contourPt = (handMesh.getVertices())[thumbBaseVertexIndices[i]];
			ofNoFill();
			ofSetColor(255,50,50);
			ofEllipse(contourPt.x, contourPt.y, 16,16);
		}
		
		// Draw the thumbBaseVertexIndices[5] (BLUE)
		for (int i=0; i<5; i++){
			ofPoint contourPt = (handMesh.getVertices())[thumbPedestalVertexIndices[i]];
			ofNoFill();
			ofSetColor(50,50,255);
			ofEllipse(contourPt.x, contourPt.y, 13,13);
		}
		
		// Draw the thumbsidePalmVertexIndices[6] (GREEN)
		for (int i=0; i<6; i++){
			ofPoint contourPt = (handMesh.getVertices())[thumbsidePalmVertexIndices[i]];
			ofNoFill();
			ofSetColor(50,255,50);
			ofEllipse(contourPt.x, contourPt.y, 10,10);
		}
	}
}


//--------------------------------------------------------------
void ofApp::addPalmToHandMesh(){
	
	//---------------
	// Extract contour along palm on pinky side.
	int contourIndex14 = myHandmarks[HANDMARK_PALM_BASE].index;
	int contourIndex15 = myHandmarks[HANDMARK_PINKY_SIDE].index;
	ofPolyline pinkyPalmSide;
	pinkyPalmSide.clear();
	
	for (int i=contourIndex14; i<=contourIndex15; i++){
		ofVec3f aPoint = myContour[i];
		pinkyPalmSide.addVertex(aPoint.x, aPoint.y, 0.0);
	}
	
	//---------------
	// Extract points on thumb side of palm.
	int contourIndex11 = myHandmarks[HANDMARK_THUMB_BASE].index;
	int contourIndex07 = myHandmarks[HANDMARK_POINTER_SIDE].index;
	ofPoint pt11 = myContour[contourIndex11];
	ofPoint pt07 = myContour[contourIndex07];
	
	//---------------
	// Construct the ladder rung vertices

	// Fetch crotch points.
	ofPoint wristPoint = (handMesh.getVertices())[handMeshWristVertexIndex];
	int crotchIndices[5] = {myHandmarks[HANDMARK_PINKY_SIDE].index,
							myHandmarks[HANDMARK_PR_CROTCH].index,
							myHandmarks[HANDMARK_RM_CROTCH].index,
							myHandmarks[HANDMARK_MI_CROTCH].index,
							myHandmarks[HANDMARK_POINTER_SIDE].index};
	
	int NR = 5;
	int NC = 4;
	float x1,x2,x3,x4;
	float y1,y2,y3,y4;
	
	ofSetColor(0,255,0);
	ofNoFill();
	
	int vertexStartIndex = handMesh.getNumVertices();
	thumbsidePalmVertexIndices[0] = vertexStartIndex - 1;
	
	for (int i=1; i<NR; i++){
		// Compute (shaped) percentages for the rung heights.
		float rowFractionP = powf (ofMap(i, 0,NR, 0,1), 0.80);
		float rowFractionT = powf (ofMap(i, 0,NR, 0,1), 1.00);
		
		// Get interpolated coordinates at the extremities of the ladder rungs.
		ofPoint pinkySidePt = pinkyPalmSide.getPointAtPercent(rowFractionP);
		ofPoint thumbSidePt = (1.0-rowFractionT)*pt11 + rowFractionT*pt07;
		
		// Add the pinky side vertex.
		handMesh.addVertex(ofPoint(pinkySidePt.x,pinkySidePt.y, 0.0));
		
	
		// Add vertices in between, which lie at intersections along rays to the crotches
		x1 = pinkySidePt.x;
		y1 = pinkySidePt.y;
		x2 = thumbSidePt.x;
		y2 = thumbSidePt.y;
		x4 = wristPoint.x;
		y4 = wristPoint.y;
		
		// Get the intersections to the three crotch rays.
		for (int j=0; j<3; j++){
			x3 = myContour[crotchIndices[j+1]].x;
			y3 = myContour[crotchIndices[j+1]].y;
			
			float denominator = ((y4-y3)*(x2-x1) - (x4-x3)*(y2-y1));
			if (denominator != 0.0){
				float ua = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3)) / denominator;
				float ub = ((x2-x1)*(y1-y3) - (y2-y1)*(x1-x3)) / denominator;
				if ((ua > 0.0) && (ua < 1.0) &&
					(ub > 0.0) && (ub < 1.0)){
					// this is the intersection point:
					float interx = x1 + ua*(x2-x1);
					float intery = y1 + ua*(y2-y1);
					handMesh.addVertex (ofPoint(interx,intery, 0.0));
				}
			}
		}
		
		// Add the thumbside vertex
		thumbsidePalmVertexIndices[i] = handMesh.getNumVertices();
		handMesh.addVertex(ofPoint(thumbSidePt.x,thumbSidePt.y, 0.0));
	}
	
	
	//----------------------------
	// ADD TRIANGLES to handMesh.
	
	//-----------
	// Add base of palm.
	int palmBaseTopIndexStart = handMesh.getNumVertices() - (NR * NC);
	int palmBaseBotIndexStart = handMeshWristVertexIndex - 1;
	int pt0 = palmBaseTopIndexStart;
	int pb0 = palmBaseBotIndexStart;
	int k = 0;
	if (bWindingCCW){
		handMesh.addTriangle(pb0+0, pb0+1,	pt0+0);		nTrianglesAccum++;
		handMesh.addTriangle(pb0+1, pt0+1,	pt0+0);		nTrianglesAccum++;
		handMesh.addTriangle(pt0+1,	pb0+1,	pt0+2);		nTrianglesAccum++;
		handMesh.addTriangle(pt0+2,	pb0+1,	pt0+3);		nTrianglesAccum++;
		handMesh.addTriangle(pb0+1, pb0+2,	pt0+3);		nTrianglesAccum++;
		handMesh.addTriangle(pt0+3,	pb0+2, 	pt0+4);		nTrianglesAccum++;
	} else {
		handMesh.addTriangle(pb0+0, pt0+0,	pb0+1);		nTrianglesAccum++;
		handMesh.addTriangle(pb0+1, pt0+0,	pt0+1);		nTrianglesAccum++;
		handMesh.addTriangle(pt0+1,	pt0+2,	pb0+1);		nTrianglesAccum++;
		handMesh.addTriangle(pt0+2,	pt0+3,	pb0+1);		nTrianglesAccum++;
		handMesh.addTriangle(pb0+1, pt0+3,	pb0+2);		nTrianglesAccum++;
		handMesh.addTriangle(pt0+3,	pt0+4,	pb0+2);		nTrianglesAccum++;
	}
	
	//-----------
	// Add middle bulk of palm.
	int vM = vertexStartIndex;
	int ND = NC+1;
	for (int j=0; j<3; j++){
		int row = j*ND;
		for (int i=0; i<NC; i++){
			if (bWindingCCW){
				handMesh.addTriangle(vM+row+i+0, vM+row+i+1,	vM+row+i+0+ND);		nTrianglesAccum++;
				handMesh.addTriangle(vM+row+i+1, vM+row+i+1+ND,	vM+row+i+0+ND);		nTrianglesAccum++;
			} else {
				handMesh.addTriangle(vM+row+i+0, vM+row+i+0+ND, vM+row+i+1);		nTrianglesAccum++;
				handMesh.addTriangle(vM+row+i+1, vM+row+i+0+ND, vM+row+i+1+ND);		nTrianglesAccum++;
			}
		}
	}
	
	//-----------
	// Add crown of palm
	int nVerticesAfterPalmBulk = handMesh.getNumVertices();
	
	// Compute "knuckle" vertices
	for (int i=0; i<4; i++){
		ofPoint aPalmPt1  = (handMesh.getVertices())[nVerticesAfterPalmBulk - (NC+1) + i  ];
		ofPoint aPalmPt2  = (handMesh.getVertices())[nVerticesAfterPalmBulk - (NC+1) + i+1];
		ofPoint aCrotchPt3 = myContour[crotchIndices[i  ]];
		ofPoint aCrotchPt4 = myContour[crotchIndices[i+1]];
		
		float x1 = aPalmPt1.x;		float y1 = aPalmPt1.y;
		float x2 = aPalmPt2.x;		float y2 = aPalmPt2.y;
		float x3 = aCrotchPt3.x;	float y3 = aCrotchPt3.y;
		float x4 = aCrotchPt4.x;	float y4 = aCrotchPt4.y;
		
		float knuckleX = (x1+x2+x3+x4)/4.0;
		float knuckleY = (y1+y2+y3+y4)/4.0;
		
		if (i > 0){
			handMesh.addVertex (ofPoint(aCrotchPt3.x, aCrotchPt3.y, 0.0));
		}
		handMesh.addVertex (ofPoint(knuckleX,knuckleY, 0.0));
		// ofEllipse(knuckleX,knuckleY, 5,5);
	}
	
	thumbsidePalmVertexIndices[5] = 4*verticesPerFinger + 2;
	
	// Add triangles.
	for (int i=0; i<4; i++){
		
		int aPalmIndex1  =  nVerticesAfterPalmBulk - (NC+1) + i  ;
		int aPalmIndex2  =  nVerticesAfterPalmBulk - (NC+1) + i+1;
		int aCrotchIndex3 = nVerticesAfterPalmBulk + 2*i - 1;
		int aKnuckleIndex = nVerticesAfterPalmBulk + 2*i + 0;
		int aCrotchIndex4 = nVerticesAfterPalmBulk + 2*i + 1;
		
		// When possible, connect to vertices which were already placed earlier!
		if (i==0){
			aCrotchIndex3 = verticesPerFinger;
		}
		if (i==3){
			aCrotchIndex4 = 4*verticesPerFinger +2;
		}
		
		int fingBaseIndex0 = (i+1)*verticesPerFinger+0;
		int fingBaseIndex1 = (i+1)*verticesPerFinger+1;
		int fingBaseIndex2 = (i+1)*verticesPerFinger+2;
		
		if (bWindingCCW){
			if (i>0){
				handMesh.addTriangle(aCrotchIndex3, aKnuckleIndex, fingBaseIndex0); nTrianglesAccum++;
			}
			handMesh.addTriangle(aPalmIndex1, aKnuckleIndex, aCrotchIndex3);	nTrianglesAccum++;
			handMesh.addTriangle(aPalmIndex1, aPalmIndex2,	 aKnuckleIndex);	nTrianglesAccum++;
			handMesh.addTriangle(aPalmIndex2, aCrotchIndex4, aKnuckleIndex);	nTrianglesAccum++;
			if (i<3){
				handMesh.addTriangle(aCrotchIndex4, fingBaseIndex2, aKnuckleIndex); nTrianglesAccum++;
			}
			handMesh.addTriangle(fingBaseIndex1, aKnuckleIndex, fingBaseIndex2); nTrianglesAccum++;
			handMesh.addTriangle(fingBaseIndex1, fingBaseIndex0, aKnuckleIndex); nTrianglesAccum++;
			
		} else {
			if (i>0){
				handMesh.addTriangle(aCrotchIndex3, fingBaseIndex0, aKnuckleIndex); nTrianglesAccum++;
			}
			handMesh.addTriangle(aPalmIndex1, aCrotchIndex3, aKnuckleIndex);	nTrianglesAccum++;
			handMesh.addTriangle(aPalmIndex1, aKnuckleIndex, aPalmIndex2);	nTrianglesAccum++;
			handMesh.addTriangle(aPalmIndex2, aKnuckleIndex, aCrotchIndex4);	nTrianglesAccum++;
			if (i<3){
				handMesh.addTriangle(aCrotchIndex4, aKnuckleIndex, fingBaseIndex2); nTrianglesAccum++;
			}
			handMesh.addTriangle(fingBaseIndex1, fingBaseIndex2, aKnuckleIndex); nTrianglesAccum++;
			handMesh.addTriangle(fingBaseIndex1, aKnuckleIndex, fingBaseIndex0); nTrianglesAccum++;
		}
	}
	


	if (bRenderIntermediate){
		for (int i=0; i<6; i++){
			int t = thumbsidePalmVertexIndices[i];
			// printf("thumbsidePalmVertexIndices %d = %d\n", i, t);
			vector<ofPoint> pts = handMesh.getVertices();
			ofPoint pt = pts[t];
			ofSetColor(255);
			ofEllipse(pt.x, pt.y, 9,9);
		}
	}

	
	
}


//--------------------------------------------------------------
void ofApp::addWristToHandMesh(){
	
	// Collect structuring vertices.
	float avgx, avgy;
	
	int contourIndex11 = myHandmarks[HANDMARK_THUMB_BASE].index;
	int contourIndex12 = myHandmarks[HANDMARK_THUMBSIDE_WRIST].index;
	int contourIndex13 = myHandmarks[HANDMARK_PINKYSIDE_WRIST].index;
	int contourIndex14 = myHandmarks[HANDMARK_PALM_BASE].index;
	
	ofPoint pt11 = myContour[contourIndex11];
	ofPoint pt12 = myContour[contourIndex12];
	ofPoint pt13 = myContour[contourIndex13];
	ofPoint pt14 = myContour[contourIndex14];

	
	//-------------------
	// Add vertices to handMesh.
	
	// Get the index of the vertex at which to start.
	int vIndex = handMesh.getNumVertices();
	ofPoint midpoint;
	int NR = 3;
	int NW = 3;
	
	//---------------
	// Base line
	avgx = (pt13.x + pt12.x)/2.0;
	avgy = (pt13.y + pt12.y)/2.0;
	midpoint = ofPoint(avgx, avgy);
	ofPoint contourMid1213 = myContour.getClosestPoint(midpoint);
	handMesh.addVertex(ofPoint(pt13.x,pt13.y, 0.0));
	handMesh.addVertex(ofPoint(contourMid1213.x,contourMid1213.y,0.0));
	handMesh.addVertex(ofPoint(pt12.x,pt12.y, 0.0));
	
	//---------------
	// Midline(s)
	for (int i=1; i<NR; i++){
		float nearWrist = ofMap(i, 0,NR, 0,1);
		nearWrist = powf(nearWrist, 0.666);
		
		avgx = (1.0-nearWrist)*pt13.x + nearWrist*pt14.x;
		avgy = (1.0-nearWrist)*pt13.y + nearWrist*pt14.y;
		midpoint = ofPoint(avgx, avgy);
		ofPoint contourMid1314 = myContour.getClosestPoint(midpoint);
		
		avgx = (1.0-nearWrist)*pt12.x + nearWrist*pt11.x;
		avgy = (1.0-nearWrist)*pt12.y + nearWrist*pt11.y;
		midpoint = ofPoint(avgx, avgy);
		ofPoint contourMid1211 = myContour.getClosestPoint(midpoint);
		
		avgx = (contourMid1314.x + contourMid1211.x)/2.0;
		avgy = (contourMid1314.y + contourMid1211.y)/2.0;
		
		handMesh.addVertex(ofPoint(contourMid1314.x,contourMid1314.y, 0.0));
		handMesh.addVertex(ofPoint(avgx,avgy,0.0));
		handMesh.addVertex(ofPoint(contourMid1211.x,contourMid1211.y, 0.0));
	}
	
	//---------------
	// Top line
	avgx = (pt14.x + pt11.x)/2.0;
	avgy = (pt14.y + pt11.y)/2.0;
	handMesh.addVertex(ofPoint(pt14.x,pt14.y, 0.0));
	handMeshWristVertexIndex = handMesh.getNumVertices(); // stash this here!
	handMesh.addVertex(ofPoint(avgx,avgy,0.0));
	handMesh.addVertex(ofPoint(pt11.x,pt11.y, 0.0));
	
	//-------------------
	// Add triangles to handMesh!
	for (int j=0; j<NR; j++){
		int row = j*NW;
		for (int i=0; i<2; i++){
			if (bWindingCCW){
				handMesh.addTriangle(vIndex+row+i+0, vIndex+row+i+1,	vIndex+row+i+0+NW);		nTrianglesAccum++;
				handMesh.addTriangle(vIndex+row+i+1, vIndex+row+i+1+NW,	vIndex+row+i+0+NW);		nTrianglesAccum++;
			} else {
				handMesh.addTriangle(vIndex+row+i+0, vIndex+row+i+0+NW, vIndex+row+i+1);		nTrianglesAccum++;
				handMesh.addTriangle(vIndex+row+i+1, vIndex+row+i+0+NW, vIndex+row+i+1+NW);		nTrianglesAccum++;
			}
		}
	}
}



//--------------------------------------------------------------
void ofApp::addFingersToHandMesh (){
	
	int nContourPoints = myContour.size();
	
	// For each finger, add triangles to handMesh:
	for (int whichFinger=0; whichFinger<5; whichFinger++){
		int contourIndex0 = 0;
		int contourIndex1 = 0;
		int contourIndex2 = 0;
		
		switch (whichFinger){
			case 0: // THUMB
				contourIndex0 = myHandmarks[HANDMARK_IT_CROTCH].index;
				contourIndex1 = myHandmarks[HANDMARK_THUMB_TIP].index;
				contourIndex2 = myHandmarks[HANDMARK_THUMB_KNUCKLE].index;
				break;
			case 1: // PINKY
				contourIndex0 = myHandmarks[HANDMARK_PINKY_SIDE].index;
				contourIndex1 = myHandmarks[HANDMARK_PINKY_TIP].index;
				contourIndex2 = myHandmarks[HANDMARK_PR_CROTCH].index;
				break;
			case 2: // RING
				contourIndex0 = myHandmarks[HANDMARK_PR_CROTCH].index;
				contourIndex1 = myHandmarks[HANDMARK_RING_TIP].index;
				contourIndex2 = myHandmarks[HANDMARK_RM_CROTCH].index;
				break;
			case 3: // MIDDLE
				contourIndex0 = myHandmarks[HANDMARK_RM_CROTCH].index;
				contourIndex1 = myHandmarks[HANDMARK_MIDDLE_TIP].index;
				contourIndex2 = myHandmarks[HANDMARK_MI_CROTCH].index;
				break;
			case 4: // INDEX
				contourIndex0 = myHandmarks[HANDMARK_MI_CROTCH].index;
				contourIndex1 = myHandmarks[HANDMARK_POINTER_TIP].index;
				contourIndex2 = myHandmarks[HANDMARK_POINTER_SIDE].index;
				break;
		}
		
		//-------------
		// Collect the first side of the finger, from contourIndex0 up to contourIndex1
		ofPolyline poly01;
		poly01.clear();
		int contourIndex1a = contourIndex1;
		if (contourIndex1a < contourIndex0) {
			contourIndex1a += nContourPoints;
		}
		for (int i=contourIndex0; i<=contourIndex1a; i++){
			int indexSafe = (i+nContourPoints)%nContourPoints;
			ofVec3f pointi = myContour[indexSafe];
			poly01.addVertex(pointi.x, pointi.y, 0.0);
		}
		
		// Collect the reverse side of the finger, from contourIndex2 down to contourIndex1
		ofPolyline poly21;
		poly21.clear();
		int contourIndex1b = contourIndex1;
		if (contourIndex1b > contourIndex2) {
			contourIndex2 += nContourPoints;
		}
		for (int i=contourIndex2; i>=contourIndex1b; i--){
			int indexSafe = (i+nContourPoints)%nContourPoints;
			ofVec3f pointi = myContour[indexSafe];
			poly21.addVertex(pointi.x, pointi.y, 0.0);
		}
		
		//-------------
		// Compute resampled points on finger sides.
		int N_FING_LEN_RESAMPS = 6;
		
		ofPolyline poly01RS;
		poly01RS.clear();
		float startFrac01 = (0.3/6.5);
		float endFrac01   = (5.5/6.5);
		if ((whichFinger == 0) || (whichFinger == 1)){
			startFrac01 = 0.0; // thumb or pinky
		}
		for (int i=1; i<=N_FING_LEN_RESAMPS; i++){
			float frac = ofMap (i,1,N_FING_LEN_RESAMPS, startFrac01, endFrac01);
			if (whichFinger == 0) frac = powf(frac, 1.25);
			ofVec3f pointInterp = poly01.getPointAtPercent(frac);
			poly01RS.addVertex(pointInterp.x, pointInterp.y, 0.0);
		}
		
		ofPolyline poly21RS;
		poly21RS.clear();
		float startFrac21 = (0.3/6.5);
		float endFrac21   = (5.5/6.5);
		if ((whichFinger == 0) || (whichFinger == 4)){
			startFrac21 = 0.0;
		}
		for (int i=1; i<=N_FING_LEN_RESAMPS; i++){
			float frac = ofMap (i,1,N_FING_LEN_RESAMPS, startFrac21, endFrac21);
			if (whichFinger == 0) frac = powf(frac, 1.25);
			ofVec3f pointInterp = poly21.getPointAtPercent(frac);
			poly21RS.addVertex(pointInterp.x, pointInterp.y, 0.0);
		}
		

	
		//---------------
		// ADD VERTICES TO THE MESH along the finger lengths.
		// Add vertices across the finger, in threes (side, middle, side).
		
		if (poly01RS.size() == poly21RS.size()){
			
			for (int i=0; i<N_FING_LEN_RESAMPS; i++){
				float px01 = poly01RS[i].x;
				float py01 = poly01RS[i].y;
				float px21 = poly21RS[i].x;
				float py21 = poly21RS[i].y;
				
				float avgx = (px01 + px21)/2.0;
				float avgy = (py01 + py21)/2.0;
				
				handMesh.addVertex(ofPoint(px01,py01,0.0));
				handMesh.addVertex(ofPoint(avgx,avgy,0.0));
				handMesh.addVertex(ofPoint(px21,py21,0.0));
			}
		}
		
		//---------------
		// Add vertices for the fingertip
		ofPoint tipPoint = myContour[contourIndex1];
		float tipx = tipPoint.x;
		float tipy = tipPoint.y;

		float qx01 = (poly01RS[N_FING_LEN_RESAMPS-1].x + tipx)/2.0;
		float qy01 = (poly01RS[N_FING_LEN_RESAMPS-1].y + tipy)/2.0;
		ofPoint q01 = ofPoint(qx01, qy01);
		
		float qx21 = (poly21RS[N_FING_LEN_RESAMPS-1].x + tipx)/2.0;
		float qy21 = (poly21RS[N_FING_LEN_RESAMPS-1].y + tipy)/2.0;
		ofPoint q21 = ofPoint(qx21, qy21);
		
		ofPoint QA = myContour.getClosestPoint(q01);
		ofPoint QB = myContour.getClosestPoint(q21);
		 
		handMesh.addVertex(ofPoint(QA.x, QA.y, 0.0));
		handMesh.addVertex(ofPoint(tipx, tipy, 0.0));
		handMesh.addVertex(ofPoint(QB.x, QB.y, 0.0));
		
		//---------------
		// Add triangles
		int vertexIndex = whichFinger * 21; // count 'em up
		int N_FING_WID_RESAMPS = 3;
		int NW = N_FING_WID_RESAMPS;
		
		for (int i=0; i<(N_FING_LEN_RESAMPS-1); i++){
			int row = vertexIndex + i*N_FING_WID_RESAMPS;
			for (int j=0; j<(N_FING_WID_RESAMPS-1); j++){
				if (bWindingCCW){
					handMesh.addTriangle(row+j  , row+j+1,    row+j+NW);	nTrianglesAccum++;
					handMesh.addTriangle(row+j+1, row+j+1+NW, row+j+NW);	nTrianglesAccum++;
				} else {
					handMesh.addTriangle(row+j  , row+j+NW,	  row+j+1);		nTrianglesAccum++;
					handMesh.addTriangle(row+j+1, row+j+NW,	  row+j+1+NW);	nTrianglesAccum++;
				}
			}
		}

		// add triangles at fingertip.
		int row = vertexIndex + (N_FING_LEN_RESAMPS-1)*N_FING_WID_RESAMPS;
		if (bWindingCCW){
			handMesh.addTriangle(row+0, row+1, row+3);		nTrianglesAccum++;
			handMesh.addTriangle(row+3, row+1, row+4);		nTrianglesAccum++;
			handMesh.addTriangle(row+4, row+1, row+5);		nTrianglesAccum++;
			handMesh.addTriangle(row+1, row+2, row+5);		nTrianglesAccum++;
		} else {
			handMesh.addTriangle(row+0, row+3, row+1);		nTrianglesAccum++;
			handMesh.addTriangle(row+3, row+4, row+1);		nTrianglesAccum++;
			handMesh.addTriangle(row+4, row+5, row+1);		nTrianglesAccum++;
			handMesh.addTriangle(row+1, row+5, row+2);		nTrianglesAccum++;
		}
		
		//-------------------------------
		if (bRenderIntermediate){
			ofSetColor(0,255,0);
			poly01.draw();
			ofSetColor(255,0,0);
			poly21.draw();
			
			ofFill();
			ofSetColor(0,255,255);
			vector<ofVec3f> vecs = handMesh.getVertices();
			for (int i=0; i<vecs.size(); i++){
				ofEllipse (vecs[i].x, vecs[i].y, 5,5);
			}
		}
	}
		
}

//--------------------------------------------------------------
bool ofApp::doesContourNeedToBeReversed (ofPolyline &handContour, Handmark *hmarks){
	int a = hmarks[HANDMARK_PR_CROTCH].index;
	int b = hmarks[HANDMARK_RING_TIP ].index;
	int c = hmarks[HANDMARK_RM_CROTCH].index;
	
	bool bContourNeedsToBeReversed = false;
	if ((a > b) && (b > c)){ bContourNeedsToBeReversed = true; }
	return bContourNeedsToBeReversed;
	// ((ofGetElapsedTimeMillis()/1000)%2 == 0);
}

//--------------------------------------------------------------
void ofApp::createLocalCopyOfContourAndHandmarks (ofPolyline &handContour, Handmark *hmarks, bool bReverse){
	int nPointsInContour = handContour.size();
	
	// The case in which no reversal is necessary.
	if (bReverse == false){
		
		// Copy the contour;
		myContour.clear();
		myContour.resize(nPointsInContour);
		for (int i=0; i<nPointsInContour; i++){
			float px = handContour[i].x;
			float py = handContour[i].y;
			myContour[i].set(px, py, 0);
		}
		
		// Copy the handmarks.
		for (int i=0; i<N_HANDMARKS; i++){
			float px = hmarks[i].point.x;
			float py = hmarks[i].point.y;
			myHandmarks[i].point.set(px,py,0);
			
			myHandmarks[i].index = hmarks[i].index;
			myHandmarks[i].type  = hmarks[i].type;
			myHandmarks[i].valid = hmarks[i].valid;
		}
		
	} else { // Reverse the contour when making our local copy.
		
		// Copy the contour, but in reverse;
		myContour.clear();
		myContour.resize(nPointsInContour);
		for (int i=0; i<nPointsInContour; i++){
			int reversei = (nPointsInContour-1) - i;
			float px = handContour[reversei].x;
			float py = handContour[reversei].y;
			myContour[i].set(px, py, 0);
		}
		
		// Copy the handmarks, but reversing the stored contour indices.
		for (int i=0; i<N_HANDMARKS; i++){
			float px = hmarks[i].point.x;
			float py = hmarks[i].point.y;
			myHandmarks[i].point.set(px,py,0);
			
			myHandmarks[i].index = (nPointsInContour-1) - hmarks[i].index;
			myHandmarks[i].type  = hmarks[i].type;
			myHandmarks[i].valid = hmarks[i].valid;
		}
		
		
	}
}


//--------------------------------------------------------------
int ofApp::getIndexOfClosestPointOnContour(ofPolyline& aPolyline, float qx, float qy){
	int nPts = aPolyline.size();
	int indexOfClosestPoint = 0;
	float distanceToClosestPoint = 99999;
	
	for (int i=0; i<nPts; i++){
		float px = aPolyline[i].x;
		float py = aPolyline[i].y;
		float distance = ofDist(px,py, qx,qy);
		if (distance < distanceToClosestPoint){
			distanceToClosestPoint = distance;
			indexOfClosestPoint = i;
		}
	}
	
	return indexOfClosestPoint;
}



//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 's'){
		
		if (bLoadL){
			handMesh.save("hand-2014-151v-L.ply");
		} else {
			handMesh.save("hand-2014-151v-R.ply");
		}
		
	}
	
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
