//
//  HandMeshBuilder.cpp
//  leapAndCamHandSegmenter
//
//  Created by GL on 9/22/14.
//
//

#include "HandMeshBuilder.h"


//============================================================
void HandMeshBuilder::initialize(int w, int h){
	
	imgW = w;
	imgH = h;
	
	// drawing order, in case you're curious.
	fingerTipIndices[0] = HANDMARK_THUMB_TIP;
	fingerTipIndices[1] = HANDMARK_PINKY_TIP;
	fingerTipIndices[2] = HANDMARK_RING_TIP;
	fingerTipIndices[3] = HANDMARK_MIDDLE_TIP;
	fingerTipIndices[4] = HANDMARK_POINTER_TIP;
	
	informThereIsNoHandPresent();
	bWorkAtHalfScale = true; // absolutely definitely.
	
	nTolerableTriangleIntersections = 200;
}

//============================================================
void HandMeshBuilder::loadDefaultMesh(){
	handMesh.load("models/hand-2014-151v-R.ply");
	for (int i = 0; i < handMesh.getNumVertices(); i++) {
		// handMesh.addTexCoord(handMesh.getVertex(i));
		
        // This is important
		ofVec2f handMeshVertex;
		handMeshVertex.x =  handMesh.getVertex(i).y;
		handMeshVertex.y =  768 - handMesh.getVertex(i).x;
		handMesh.addTexCoord( handMeshVertex );
	}
}

//============================================================
void HandMeshBuilder::setWorkAtHalfScale (bool bwork){
	bWorkAtHalfScale = bwork;
}

//============================================================
void HandMeshBuilder::informThereIsNoHandPresent(){
	
	currentHandType = HAND_NONE;
	currentHandExistsFrameCount = 0;
	
	for (int i=0; i<N_HANDMARKS; i++){
		Handmarks[i].pointHistory.clear();
		Handmarks[i].pointAvg.set     (0,0);
		Handmarks[i].pointStDv.set    (0,0);
	}
}

//============================================================
ofMesh& HandMeshBuilder::getMesh(){
    return handMesh;
}


//============================================================
bool HandMeshBuilder::buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, ofVec3f &leapWristPoint, Handmark *hmarks){
	
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
				bCalculatedMesh = false;
                return bCalculatedMesh;
			}
		}
		if (bAllHandmarkIndicesValid){
			
			// Determine if the order of the points in contour needs to be reversed (such as for left hands).
			// Use that when creating our local copy of the contour and handmarks.
			// This fills myHandmarks and myContour, which are now set.
			bool bContourNeedsToBeReversed = doesContourNeedToBeReversed (handContour, hmarks);
			createLocalCopyOfContourAndHandmarks (handContour, hmarks, bContourNeedsToBeReversed );
			bWindingCCW = !bContourNeedsToBeReversed;
			
			// Catch problems with the contour and handmarks before they are passed to meshing.
			bool contourAndHandmarksAreFaulty = areContourAndHandmarksFaulty (nContourPoints);
			if (contourAndHandmarksAreFaulty){
				handMesh.clear();
				bCalculatedMesh = false;
				return bCalculatedMesh;
			}
            
            theLeapCentroid.set (handCentroid.x, handCentroid.y);
            theLeapWrist.set (leapWristPoint.x, leapWristPoint.y);
            
			
			//-------------------------------------
			// MESHING HAPPENS HERE.
			
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
		
			//-------------------------------------
			// Catch fatal problems with the mesh before it is passed to the puppet.
			bool meshIsFaulty = isHandMeshFaulty();
			if (!meshIsFaulty){
			
				// Add texture coordinates to meshes; be cognizant of half-scale.
				float vertexScale = (bWorkAtHalfScale) ? 2.0 : 1.0;
				for (int i = 0; i < handMesh.getNumVertices(); i++) {
					handMesh.addTexCoord( vertexScale*(handMesh.getVertex(i)));
				}
				bCalculatedMesh = true;
				
			} else {
				handMesh.clear();
				bCalculatedMesh = false;
				return bCalculatedMesh;
			}
			
			
		}
	}
	
	return bCalculatedMesh;
}

//--------------------------------------------------------------
bool HandMeshBuilder::areContourAndHandmarksFaulty (int nContourPts){
	// CONTOUR/HANDMARK DEGENERACY TESTER.
	// Catch fatal problems with the contour before it is passed to the mesher.
	// Returns TRUE if something is faulty.
	//
	
	bool bVerbose = false;

	// TEST 1. Are the handmark indices valid?
	bool bHasInvalidHandmarkIndices = false;
	for (int i=0; i<N_HANDMARKS; i++){
		int index = myHandmarks[i].index;
		if ((index < 0) || (index >= nContourPts)){
			if (bVerbose) {
				printf("%d: -------- Problem with handmark! %d is invalid.\n", (int)ofGetElapsedTimeMillis(), i);
			}
			bHasInvalidHandmarkIndices = true;
			return true;
			
		}
	}
	
	// TEST 2. Are the indices stored in the Handmarks, monotonically increasing?
	bool bHasHandmarksOutOfSequence = false;
	int prevIndex = 0;
	for (int i=0; i<12; i++){ // only up to 12, for now.
		int currIndex = myHandmarks[i].index;
		if (currIndex < prevIndex){
			if (bVerbose){
				printf("%d: -------- Problem with handmark order! %d is out of sequence.\n", (int)ofGetElapsedTimeMillis(), i);
			}
			bHasHandmarksOutOfSequence = true;
			return true;
		}
		prevIndex = currIndex;
	}
	
	return false;
}

//--------------------------------------------------------------
bool HandMeshBuilder::isHandMeshFaulty(){
	// MESH DEGENERACY TESTER.
	// Catch fatal problems with the mesh before it is passed to the puppet.
	// Returns TRUE if the mesh is faulty.
	//
    
    bool bDoIncorrectNumVerticesTest	= true;
    bool bDoHasIncorrectNumFacesTest	= true;
    bool bDoHasBadValuesTest			= true;
    bool bDoDuplicatedVerticesTest		= true;
    bool bDoIntersectingTrianglesTest	= true; // doesn't seem necessary; puppeteer seems resilient...
	bool bDoTestForTeenyTriangles		= true;
    
    
	// TEST 1. Does the mesh have the right number of vertices?
	bool bHasIncorrectNumVertices = false;
    if (bDoIncorrectNumVerticesTest){
        int numVertices = handMesh.getNumVertices();
        int numVerticesThereShouldBe = 151;
        if ((numVertices != numVerticesThereShouldBe) && (numVertices > 0)){
            bHasIncorrectNumVertices = true;
            
            // printf("%d: --------Problem with numVertices: %d\n", (int)ofGetElapsedTimeMillis(), numVertices);
            return true;
        }
    }
	
	// TEST 2. Does the mesh have the right number of faces?
	bool bHasIncorrectNumFaces = false;
    if (bDoHasIncorrectNumFacesTest){
        int numFaces = handMesh.getUniqueFaces().size();
        int numFacesThereShouldBe = 205;
        if ((numFaces != numFacesThereShouldBe) && (numFaces > 0)){
            bHasIncorrectNumFaces = true;
            
            // printf("%d: --------Problem with numFaces: %d\n", (int)ofGetElapsedTimeMillis(), numFaces);
            return true;
        }
    }
	
	// TEST 3: Check for degenerate vertex coordinate values, such as zeroes.
	bool bFoundBadValues = false;
    if (bDoHasBadValuesTest){
        for (int i = 0; i < handMesh.getNumVertices(); i++) {
            float px = handMesh.getVertex(i).x;
            float py = handMesh.getVertex(i).y;
            if ((px <= 0.001) || (py <= 0.001) || (px > imgW) || (py > imgH)){
                bFoundBadValues = true;
                
                // printf("%d: --------Problem with vertex %d:	%f	%f\n", (int)ofGetElapsedTimeMillis(), i, px,py);
                return true;
            }
        }
    }
	
	// TEST 4. Check for vertices that have become overlapped.
    bool bFoundDuplicatedVertices = false;
    if (bDoDuplicatedVerticesTest){
        bFoundDuplicatedVertices = checkForDuplicatedVertices();
        if (bFoundDuplicatedVertices){
			
            //printf("%d: --------Problem with duplicated vertices: \n", (int)ofGetElapsedTimeMillis());
            return true;
        }
    }
    
    // TEST 5. Check whether any triangles intersect each other.
    // Also checks to see if any vertices reside on the edge of another triangle.
    bool bFoundTooManyIntersectingTriangles = false;
    if (bDoIntersectingTrianglesTest){
		int nIntersectingTriangles = count_triangle_intersections (handMesh);
        bFoundTooManyIntersectingTriangles = (nIntersectingTriangles > nTolerableTriangleIntersections); // WOW ARBITRARY
        if (bFoundTooManyIntersectingTriangles){
            
			//printf("%d: --------Problem with too many intersecting triangles! %d \n", (int)ofGetElapsedTimeMillis(), nIntersectingTriangles);
            return true;
        }
    }
    
	// TEST 6. Check for triangles whose sides or areas are less than some epsilon.
	bool bFoundTrianglesThatWereTooTeeny = false;
	if (bDoTestForTeenyTriangles){
		bFoundTrianglesThatWereTooTeeny = checkForTrianglesThatAreTooTeeny (handMesh);
		if (bFoundTrianglesThatWereTooTeeny){
			
			// printf("%d: --------Problem with triangles too small! \n", (int)ofGetElapsedTimeMillis());
			return true;
		}
	}
    
	// Other possible tests:
	// Are all faces oriented the correct direction?
	
	//------------------------
    // The mesh is fine, it's not faulty.
    return false;
}


//--------------------------------------------------------------
bool HandMeshBuilder::checkForDuplicatedVertices(){
	bool bFoundDuplicatedVertices = false;
	
	vector<ofVec3f> verts = handMesh.getVertices();
	int nVerts = verts.size();
	// printf("nVerts = %d\n" ,nVerts);
	for (int i=0; i<nVerts; i++){
		ofVec3f ipoint = verts[i];
		for (int j=0; j<i; j++){
			ofVec3f jpoint = verts[j];
			float distance = jpoint.distance(ipoint);
			if (distance < 0.001){
				bFoundDuplicatedVertices = true;
				// printf("%d: --------Problem: duplicated verts %d %d\n", (int)ofGetElapsedTimeMillis(), i,j);
				// ofSetColor(0,255,255);
				// ofEllipse(jpoint.x, jpoint.y, 14,14);
			}
		}
	}
	
	return bFoundDuplicatedVertices;
}

//--------------------------------------------------------------
void HandMeshBuilder::addThumbWebbingToHandMesh(){
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
void HandMeshBuilder::addPalmToHandMesh(){
	
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
void HandMeshBuilder::addWristToHandMesh(){
	
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
	avgx = (pt14.x + pt11.x)/2.0; // old
	avgy = (pt14.y + pt11.y)/2.0; // old
    float wx = theLeapWrist.x; // try this on for size
    float wy = theLeapWrist.y;
    float wh = sqrtf((wx-avgx)*(wx-avgx) + (wy-avgy)*(wy-avgy));
    if (wh < 50){
        avgx = wx;
        avgy = wy;
    }
    
    
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
void HandMeshBuilder::addFingersToHandMesh (){
	
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
		} else {
			printf ("Unexpected problem in addFingersToHandMesh()!\n");
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
bool HandMeshBuilder::doesContourNeedToBeReversed (ofPolyline &handContour, Handmark *hmarks){
	int a = hmarks[HANDMARK_PR_CROTCH].index;
	int b = hmarks[HANDMARK_RING_TIP ].index;
	int c = hmarks[HANDMARK_RM_CROTCH].index;
	
	bool bContourNeedsToBeReversed = false;
	if ((a > b) && (b > c)){ bContourNeedsToBeReversed = true; }
	return bContourNeedsToBeReversed;
	// ((ofGetElapsedTimeMillis()/1000)%2 == 0);
}

//--------------------------------------------------------------
void HandMeshBuilder::createLocalCopyOfContourAndHandmarks (ofPolyline &handContour, Handmark *hmarks, bool bReverse){
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
int HandMeshBuilder::getIndexOfClosestPointOnContour(ofPolyline& aPolyline, float qx, float qy){
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



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//============================================================
void HandMeshBuilder::buildMesh2013 (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks){
    //
    // OLD OLD OLD.
	// This function builds a mesh matching the 2013 "handmarksNew.ply",
	// with 381 vertices and 612 (triangle) faces.
	bool bDraw = false;
	bCalculatedMesh = false;
	int nTriangles = 0;

	
	// Check to make sure the contour contains data
	int nContourPoints = handContour.size();
	if (nContourPoints > 0){
		
		// Check to make sure all indices are legal
		bool bAllHandmarkIndicesValid = true;
		for (int i=0; i<N_HANDMARKS; i++){
			Handmarks[i] = hmarks[i];
			if ((Handmarks[i].index < 0) || (Handmarks[i].index >= nContourPoints)){
				bAllHandmarkIndicesValid = false;
				printf ("%d: problem with handmark %d: %d\n", (int) ofGetElapsedTimeMillis(), i, Handmarks[i].index);
				bCalculatedMesh = false;
                return;
			}
		}
		if (bAllHandmarkIndicesValid){
			
			handMesh.clear();
			handMesh.setupIndicesAuto();
			handMesh.setMode( OF_PRIMITIVE_TRIANGLES );
			
			// Add triangles for each finger to handMesh:
			for (int f=0; f<5; f++){
				
				int f0 = (fingerTipIndices[f] - 1 + N_HANDMARKS) % N_HANDMARKS;
				int f1 = (fingerTipIndices[f]     + N_HANDMARKS) % N_HANDMARKS;
				int f2 = (fingerTipIndices[f] + 1 + N_HANDMARKS) % N_HANDMARKS;
				
				int contourIndex0 = Handmarks[f0].index;
				int contourIndex1 = Handmarks[f1].index;
				int contourIndex2 = Handmarks[f2].index;
				
				// Collect the first side of the finger, from contourIndex0 up to contourIndex1
				ofPolyline poly01;
				poly01.clear();
				int contourIndex1a = contourIndex1;
				if (contourIndex1a < contourIndex0) {
					contourIndex1a += nContourPoints;
				}
				for (int i=contourIndex0; i<=contourIndex1a; i++){
					int indexSafe = (i+nContourPoints)%nContourPoints;
					ofVec3f pointi = handContour[indexSafe];
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
					ofVec3f pointi = handContour[indexSafe];
					poly21.addVertex(pointi.x, pointi.y, 0.0);
				}
				
				
				//-----------------------
				// Resample the finger's two sides.
				// Note: ofxPolyline.getResampledByCount() does not return a consistent number of points. Do it my damn self.
				
				ofPolyline poly01RS;
				poly01RS.clear();
				int poly01size = poly01.size();
				for (int i=0; i<=N_FINGER_LENGTH_SAMPLES; i++){
					// int poly01index = (int)ofMap((float)i,0,N_FINGER_LENGTH_SAMPLES, 0,poly01size-1);
					float indexf = ofMap((float)i,0,N_FINGER_LENGTH_SAMPLES, 0,poly01size-1);
					int poly01index = (int)((i==N_FINGER_LENGTH_SAMPLES) ? floorf(indexf) : roundf(indexf));
					ofVec3f pointi = poly01[poly01index];
					poly01RS.addVertex(pointi.x, pointi.y, 0.0);
				}
				ofPolyline poly21RS;
				poly21RS.clear();
				int poly21size = poly21.size();
				for (int i=0; i<=N_FINGER_LENGTH_SAMPLES; i++){
					float indexf = ofMap((float)i,0,N_FINGER_LENGTH_SAMPLES, 0,poly21size-1);
					int poly21index = (int)((i==N_FINGER_LENGTH_SAMPLES) ? floorf(indexf) : roundf(indexf));
					ofVec3f pointi = poly21[poly21index];
					poly21RS.addVertex(pointi.x, pointi.y, 0.0);
				}
				
				//-----------------------
				// Render the two finger sides.
				bool bRenderFingerEdgePolylines = true;
				if (bDraw && bRenderFingerEdgePolylines){
					ofNoFill();
					ofSetColor(0,200,100);
					poly01RS.draw();
					ofSetColor(200,100,0);
					poly21RS.draw();
				}
				
				bool bDrawContourTexPoints = false;
				if (bDraw && bDrawContourTexPoints){
					ofSetColor(0,200,250);
					vector<ofPoint> poly01RSpts = poly01RS.getVertices();
					int nPoly01RSpts = poly01RSpts.size();
					for (int i=0; i<nPoly01RSpts; i++){
						float ex = poly01RSpts[i].x;
						float ey = poly01RSpts[i].y;
						ofEllipse (ex, ey, 6, 6);
					}
					ofSetColor(250,200,0);
					vector<ofPoint> poly21RSpts = poly21RS.getVertices();
					int nPoly21RSpts = poly21RSpts.size();
					for (int i=0; i<nPoly21RSpts; i++){
						float ex = poly21RSpts[i].x;
						float ey = poly21RSpts[i].y;
						ofEllipse (ex, ey, 9, 9);
					}
				}
				
				//---------------
				// ADD VERTICES TO THE MESH.
				vector<ofPoint> poly01RSpts = poly01RS.getVertices();
				vector<ofPoint> poly21RSpts = poly21RS.getVertices();
				int nPoly01RSpts = poly01RSpts.size();
				int nPoly21RSpts = poly21RSpts.size();

				if ((nPoly01RSpts == nPoly21RSpts) &&
					(nPoly01RSpts == (N_FINGER_LENGTH_SAMPLES+1))){
					
					for (int i=0; i<=N_FINGER_LENGTH_SAMPLES; i++){
						float px01 = poly01RSpts[i].x;
						float py01 = poly01RSpts[i].y;
						float px21 = poly21RSpts[i].x;
						float py21 = poly21RSpts[i].y;
						
						if (i == N_FINGER_LENGTH_SAMPLES){
							// Special case for the tip.
							// N.B., px01 and px21 are the same point
							
							float qx01 = (poly01RSpts[N_FINGER_LENGTH_SAMPLES-1].x + px01)/2.0;
							float qy01 = (poly01RSpts[N_FINGER_LENGTH_SAMPLES-1].y + py01)/2.0;
							ofPoint q01 = ofPoint(qx01, qy01);
							
							float qx21 = (poly21RSpts[N_FINGER_LENGTH_SAMPLES-1].x + px21)/2.0;
							float qy21 = (poly21RSpts[N_FINGER_LENGTH_SAMPLES-1].y + py21)/2.0;
							ofPoint q21 = ofPoint(qx21, qy21);
							
							ofPoint QA = handContour.getClosestPoint(q01);
							ofPoint QB = handContour.getClosestPoint(q21);
							
							if (bDraw){
								ofEllipse(QA.x, QA.y,  2, 2);
								ofEllipse(px01, py01,  2, 2);
								ofEllipse(QB.x, QB.y,  2, 2);
							}
							
							handMesh.addVertex(ofPoint(QA.x, QA.y, 0.0));
							handMesh.addVertex(ofPoint(px01, py01, 0.0));
							handMesh.addVertex(ofPoint(QB.x, QB.y, 0.0));
							
							
						} else {
							// The usual case, up the sides of the finger.
							if (f < 2){
								for (int j=0; j<N_FINGER_WIDTH_SAMPLES; j++){
									float ex = ofMap(j, 0,N_FINGER_WIDTH_SAMPLES-1, px01,px21);
									float ey = ofMap(j, 0,N_FINGER_WIDTH_SAMPLES-1, py01,py21);
									if (bDraw){ofEllipse(ex,ey, 2, 2);}
									handMesh.addVertex(ofPoint(ex,ey,0.0));
								}
							} else {
								// skip the 0th point for fingers 2,3,4,
								// because these are shared vertices with points we've already laid.
								for (int j=0; j<N_FINGER_WIDTH_SAMPLES; j++){
									float ex = ofMap(j, 0,N_FINGER_WIDTH_SAMPLES-1, px01,px21);
									float ey = ofMap(j, 0,N_FINGER_WIDTH_SAMPLES-1, py01,py21);
									if (bDraw){ofEllipse(ex,ey, 2, 2);}
									if ((j==0) && (i==0)){
										;
									} else {
										handMesh.addVertex(ofPoint(ex,ey,0.0));
									}
								}
								
							}
						}
					}
					
					//---------------
					// ADD TRIANGLES TO THE MESH.
					// add the triangles on the interior of the finger.
					int vertexIndex = f * (N_FINGER_WIDTH_SAMPLES * N_FINGER_LENGTH_SAMPLES + 3);
					int NW = N_FINGER_WIDTH_SAMPLES;
					
					if (f >= 2){
						// Because we skip the 0th point for fingers 2,3,4
						// (Because the start point for a finger is coincident with the end point
						// for the previous finger)...
						vertexIndex -= (f-1);
						
						for (int i=0; i<(N_FINGER_LENGTH_SAMPLES-1); i++){
							int row = vertexIndex + i*N_FINGER_WIDTH_SAMPLES;
							for (int j=0; j<(N_FINGER_WIDTH_SAMPLES-1); j++){
								
								if ((i==0) && (j==0)){
									int screwCase = row - (11*5 + 3 - 5);
									handMesh.addTriangle(screwCase, row+j+1,  row+j+NW);	nTriangles++;
									handMesh.addTriangle(row+j+1, row+j+1+NW, row+j+NW);	nTriangles++;
								} else {
									handMesh.addTriangle(row+j  , row+j+1,    row+j+NW);	nTriangles++;
									handMesh.addTriangle(row+j+1, row+j+1+NW, row+j+NW);	nTriangles++;
								}
							}
						}
						// add triangles at fingertip.
						int row = vertexIndex + (N_FINGER_LENGTH_SAMPLES-1)*N_FINGER_WIDTH_SAMPLES;
						handMesh.addTriangle(row+0, row+1, row+NW);		nTriangles++;
						handMesh.addTriangle(row+1, row+2, row+NW);		nTriangles++;
						handMesh.addTriangle(row+2, row+6, row+NW);		nTriangles++;
						handMesh.addTriangle(row+2, row+7, row+1+NW);	nTriangles++;
						handMesh.addTriangle(row+2, row+3, row+2+NW);	nTriangles++;
						handMesh.addTriangle(row+3, row+4, row+2+NW);	nTriangles++;
						
						
					} else {
						// The simple cases, thumb and pinky.
						for (int i=0; i<(N_FINGER_LENGTH_SAMPLES-1); i++){
							int row = vertexIndex + i*N_FINGER_WIDTH_SAMPLES;
							for (int j=0; j<(N_FINGER_WIDTH_SAMPLES-1); j++){
								handMesh.addTriangle(row+j  , row+j+1,    row+j+NW);	nTriangles++;
								handMesh.addTriangle(row+j+1, row+j+1+NW, row+j+NW);	nTriangles++;
							}
						}
						// add triangles at fingertip.
						int row = vertexIndex + (N_FINGER_LENGTH_SAMPLES-1)*N_FINGER_WIDTH_SAMPLES;
						handMesh.addTriangle(row+0, row+1, row+NW);		nTriangles++;
						handMesh.addTriangle(row+1, row+2, row+NW);		nTriangles++;
						handMesh.addTriangle(row+2, row+6, row+NW);		nTriangles++;
						handMesh.addTriangle(row+2, row+7, row+1+NW);	nTriangles++;
						handMesh.addTriangle(row+2, row+3, row+2+NW);	nTriangles++;
						handMesh.addTriangle(row+3, row+4, row+2+NW);	nTriangles++;
					}
				}
			}
			
			//----------------------------------------------------
			// Add vertices for the knuckles.
			// TRIAGE: START USING SPECIFIC INDICES (rather than computed indices) to build the rest of the mesh.
			// 60 = N_FINGER_LENGTH_SAMPLES*N_FINGER_WIDTH_SAMPLES + 3, then +2 more into the finger, etc.
			//
			int fingerBaseMiddleIndices[] = {60, 117, 174, 231};
			for (int i=0; i<4; i++){
				int fingerBaseMiddle0 = fingerBaseMiddleIndices[i];
				int fingerBaseMiddle1 = fingerBaseMiddle0 + N_FINGER_WIDTH_SAMPLES;
				ofVec3f P0 = handMesh.getVertex(fingerBaseMiddle0);
				ofVec3f P1 = handMesh.getVertex(fingerBaseMiddle1);
				ofVec3f Pk = P0 - 1.25*(P1 - P0);
				
				// move it slightly toward the hand centroid
				ofVec3f Pc = ofVec3f(handCentroid.x, handCentroid.y, 0);
				float amountToMoveKnucklesTowardCentroid = 0.25;
				Pk = Pk - amountToMoveKnucklesTowardCentroid*(Pk - Pc);
				if (bDraw){ ofEllipse(Pk.x,Pk.y, 2, 2);}
				handMesh.addVertex(ofPoint(Pk.x, Pk.y, 0.0));
				
				int nv = handMesh.getNumVertices();
				for (int j=0; j<4; j++){
					if (i==0){
						int fi0 = fingerBaseMiddle0 + j - 2;
						int fi1 = fingerBaseMiddle0 + j - 1;
						handMesh.addTriangle(nv-1, fi1, fi0);	nTriangles++;
					} else {
						int fi0 = fingerBaseMiddle0 + j - 2;
						if (j == 0) { fi0 = fingerBaseMiddleIndices[i-1] + 2; }
						int fi1 = fingerBaseMiddle0 + j - 1;
						handMesh.addTriangle(nv-1, fi1, fi0);	nTriangles++;
					}
				}
			}
			
			//----------------------------------------------------
			// Add vertices for the wrist.
			
			int wristIndex0 = Handmarks[HANDMARK_THUMB_BASE      ].index;
			int wristIndex1 = Handmarks[HANDMARK_THUMBSIDE_WRIST ].index;
			int wristIndex2 = Handmarks[HANDMARK_PINKYSIDE_WRIST ].index;
			int wristIndex3 = Handmarks[HANDMARK_PALM_BASE       ].index;
			
			ofVec2f wristP0 = handContour[wristIndex0];
			ofVec2f wristP1 = handContour[wristIndex1];
			ofVec2f wristP2 = handContour[wristIndex2];
			ofVec2f wristP3 = handContour[wristIndex3];
			
			vector<ofPoint> wSide01;
			vector<ofPoint> wSide32;
			vector<ofPoint> wSide12;
			
			for (int i=0; i<=4; i++){
				ofVec2f whcn = ((4.0 - (float)i)*wristP0 + ((float)i)*wristP1)/4.0;
				ofPoint wristP_hcn = handContour.getClosestPoint(whcn);
				wSide01.push_back (wristP_hcn);
			}
			for (int i=0; i<=4; i++){
				ofVec2f whcn = ((4.0 - (float)i)*wristP3 + ((float)i)*wristP2)/4.0;
				ofPoint wristP_hcn = handContour.getClosestPoint(whcn);
				wSide32.push_back (wristP_hcn);
			}
			for (int i=0; i<=4; i++){
				ofVec2f whcn = ((4.0 - (float)i)*wristP1 + ((float)i)*wristP2)/4.0;
				ofPoint wristP_hcn = handContour.getClosestPoint(whcn);
				wSide12.push_back (wristP_hcn);
			}
			
			int nvBeforeWrist = handMesh.getNumVertices();
			for (int wy=0; wy<=4; wy++){
				ofVec2f pL = wSide01[wy];
				ofVec2f pR = wSide32[wy];
				if (wy < 4){
					for (int wx=4; wx>=0; wx--){
						ofVec2f wp = ((4.0 - (float)wx)*pL + ((float)wx)*pR)/4.0;
						if (bDraw){ ofEllipse(wp.x,wp.y, 2, 2);}
						handMesh.addVertex(ofPoint(wp.x,wp.y,0.0));
					}
				} else {
					for (int wx=4; wx>=0; wx--){
						ofVec2f wp = wSide12[wx];
						if (bDraw){ ofEllipse(wp.x,wp.y, 2, 2);}
						handMesh.addVertex(ofPoint(wp.x,wp.y,0.0));
					}
				}
			}
			
			int nvb = nvBeforeWrist;
			for (int wy=0; wy<4; wy++){
				for (int wx=0; wx<4; wx++){
					handMesh.addTriangle(nvb+wx,   nvb+wx+5, nvb+wx+1);		nTriangles++;
					handMesh.addTriangle(nvb+wx+1, nvb+wx+5, nvb+wx+6);		nTriangles++;
				}
				nvb += 5;
			}
			
			
			//----------------------------------------------------
			// Add vertices for the thumb base.
			
			// 0,1,2,3,4, 295
			
			// Get interpolated values on the outside contour:
			int thumbBaseIndex0 = Handmarks[HANDMARK_THUMB_KNUCKLE ].index;
			int thumbBaseIndex1 = Handmarks[HANDMARK_THUMB_BASE    ].index;
			
			ofVec2f thumbBaseHcn0 = handContour[thumbBaseIndex0];
			ofVec2f thumbBaseHcn1 = handContour[thumbBaseIndex1];
			
			// create a polyline copy of the handContourNice sub-section between the 2 indices
			ofPolyline thumbCurve;
			if (thumbBaseIndex0 < thumbBaseIndex1){
				for (int i=thumbBaseIndex0; i<=thumbBaseIndex1; i++){
					ofPoint tpoint = handContour[i];
					thumbCurve.addVertex(ofPoint(tpoint.x, tpoint.y, 0.0));
				}
			} else {
				// don't want to deal with an unlikely situation
				thumbCurve = handContour;
			}
			
			vector<ofPoint> thumbBaseSide;
			for (int i=0; i<=4; i++){
				ofVec2f hcn = ((4.0 - (float)i)*thumbBaseHcn0 + ((float)i)*thumbBaseHcn1)/4.0;
				ofPoint Phcn = thumbCurve.getClosestPoint(hcn);
				thumbBaseSide.push_back (Phcn);
				//ofEllipse(Phcn.x,Phcn.y, 10, 10);
				// n.b., we will not use points i=0 & i=4 because they are already stored as handMesh vertices 4 & 295
			}
			
			vector<ofPoint> thumbBaseHypotenuse;
			ofVec2f thumbBaseP0 = ofVec2f(handMesh.getVertex(0).x, handMesh.getVertex(0).y);
			for (int i=0; i<=4; i++){
				ofVec2f hcn = ((4.0 - (float)i)*thumbBaseP0 + ((float)i)*thumbBaseHcn1)/4.0;
				thumbBaseHypotenuse.push_back (hcn);
				//ofEllipse(hcn.x,hcn.y, 10, 10);
				// n.b., we will not use points i=0 & i=4 because they are already stored as handMesh vertices 0 & 295
			}
			int nvBeforeThumbBase = handMesh.getNumVertices();
			for (int y=1; y<4; y++){ // skipping y=0 and y=4 because those vertices already exist in handMesh.
				int topx = 4-y;
				for (int x=0; x<=topx; x++){
					ofPoint T0 = thumbBaseHypotenuse[y];
					ofPoint T1 = thumbBaseSide[y];
					ofPoint Tinterp = ((topx-x)*T0 + (x)*T1) / (float)topx;
					handMesh.addVertex(ofPoint(Tinterp.x, Tinterp.y, 0.0));
					if (bDraw){ ofEllipse(Tinterp.x,Tinterp.y, 2, 2);}
				}
			}
			
			// clockwise triangles
			int ti = nvBeforeThumbBase;
			for (int y=0; y<1; y++){ // y == 0 case
				int topx = 4-y;
				for (int x=0; x<topx; x++){
					if (x == 0){
						handMesh.addTriangle(x, ti+x, x+1);			nTriangles++;
					} else {
						handMesh.addTriangle(x, ti+(x-1), ti+x);	nTriangles++;
						handMesh.addTriangle(x, ti+x, x+1);			nTriangles++;
					}
				}
			}
			
			int thumbBaseVertexIndex = 295;
			for (int y=1; y<4; y++){
				int topx = 4-y;
				for (int x=0; x<topx; x++){
					if (x == 0){
						int tj = ti+x+topx+1;
						if (y == 3){ tj = thumbBaseVertexIndex; }
						handMesh.addTriangle(ti+x,   tj, ti+x+1);							nTriangles++;
					} else {
						handMesh.addTriangle(ti+x,   ti+x+topx+1,     ti+x+1);				nTriangles++;
						handMesh.addTriangle(ti+x,   ti+x+topx  ,     ti+x+topx+1 );		nTriangles++;
					}	
				}
				ti += topx+1;
			}
			
			//----------------------------------------------------
			// Add vertices for the thumb webbing.
			
			// Get interpolated values on the outside contour:
			int thumbWebIndex0 = Handmarks[HANDMARK_POINTER_SIDE ].index;
			int thumbWebIndex1 = Handmarks[HANDMARK_IT_CROTCH    ].index;
			
			ofVec2f thumbWebHcn0 = handContour[thumbWebIndex0];
			ofVec2f thumbWebHcn1 = handContour[thumbWebIndex1];
			
			// create a polyline copy of the handContourNice sub-section between the 2 indices
			ofPolyline thumbWebCurve;
			if (thumbWebIndex0 < thumbWebIndex1){
				for (int i=thumbWebIndex0; i<=thumbWebIndex1; i++){
					ofPoint tpoint = handContour[i];
					thumbWebCurve.addVertex(ofPoint(tpoint.x, tpoint.y, 0.0));
				}
			} else {
				// don't want to deal with an unlikely situation
				thumbWebCurve = handContour;
			}
			
			// vector of interpolated points along top edge
			vector<ofPoint> thumbWebSide1;
			for (int i=0; i<=4; i++){
				ofVec2f hcn = ((4.0 - (float)i)*thumbWebHcn0 + ((float)i)*thumbWebHcn1)/4.0;
				ofPoint Phcn = thumbWebCurve.getClosestPoint(hcn);
				thumbWebSide1.push_back (ofPoint(Phcn.x, Phcn.y, 0.0));
				// ofEllipse(Phcn.x,Phcn.y, 10, 10);
				// n.b., we will not use points i=0 & i=4, becasue they are handMesh vertices 0 and 233
			}
			
			vector<ofPoint> thumbWebHypotenuse;
			ofVec2f thumbWebP0 = ofVec2f(handMesh.getVertex(233).x, handMesh.getVertex(233).y);
			ofVec2f thumbWebP1 = ofVec2f(handMesh.getVertex(thumbBaseVertexIndex).x, handMesh.getVertex(thumbBaseVertexIndex).y);
			int nWebHypSamps = 8;
			for (int i=0; i<=nWebHypSamps; i++){
				ofVec2f hcn = ((nWebHypSamps - (float)i)*thumbWebP0 + ((float)i)*thumbWebP1)/(float)nWebHypSamps;
				thumbWebHypotenuse.push_back (ofPoint(hcn.x, hcn.y, 0.0));
				// ofEllipse(hcn.x,hcn.y, 10, 10);
				// n.b., we will not use points i=0 & i=4 because they are already stored as handMesh vertices 233 & 295
			}
			
			int nvBeforeThumbWeb = handMesh.getNumVertices();
			for (int y=1; y<4; y++){ // skipping y=0 and y=4 because those vertices already exist in handMesh.
				int topx = 4-y;
				for (int x=0; x<=topx; x++){
					ofPoint T0 = thumbWebHypotenuse[8-(y*2)];
					ofPoint T1 = thumbWebSide1[4-y];
					ofPoint Tinterp = ((topx-x)*T0 + (x)*T1) / (float)topx;
					handMesh.addVertex(ofPoint(Tinterp.x,Tinterp.y,0.0));
					if (bDraw){ ofEllipse(Tinterp.x,Tinterp.y, 2, 2);}
				}
			}
			for (int i=7; i>=1; i-=2){
				handMesh.addVertex( thumbWebHypotenuse[i] );
				if (bDraw){ ofEllipse(thumbWebHypotenuse[i].x,thumbWebHypotenuse[i].y, 2, 2);}
			}
			
			
			int thumbWebSideIndices[] = {0, 316, 320, 323, 295};
			
			int wi = nvBeforeThumbWeb;
			// handMesh.addTriangle(thumbWebSideIndices[4-0], thumbWebSideIndices[4-1], wi+0); // replace with 2 triangles
			handMesh.addTriangle(thumbWebSideIndices[4-0], thumbWebSideIndices[4-1], wi+9);		nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-1], wi+0, wi+9);							nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-1], thumbWebSideIndices[4-2], wi+0);		nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-2], wi+1, wi+0);							nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-2], thumbWebSideIndices[4-3], wi+1);		nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-3], wi+2, wi+1);							nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-3], thumbWebSideIndices[4-4], wi+2);		nTriangles++;
			handMesh.addTriangle(thumbWebSideIndices[4-4], wi+3, wi+2);							nTriangles++;
			
			// handMesh.addTriangle(wi+0, wi+1, wi+4); // replace with 2 triangles, as follows:
			handMesh.addTriangle(wi+0, wi+1, wi+10);											nTriangles++;
			handMesh.addTriangle(wi+1, wi+4, wi+10);											nTriangles++;
			handMesh.addTriangle(wi+1, wi+2, wi+4);												nTriangles++;
			handMesh.addTriangle(wi+2, wi+5, wi+4);												nTriangles++;
			handMesh.addTriangle(wi+2, wi+3, wi+5);												nTriangles++;
			handMesh.addTriangle(wi+3, wi+6, wi+5);												nTriangles++;
			
			// handMesh.addTriangle(wi+4, wi+5, wi+7); // replace with 2 triangles, as follows:
			handMesh.addTriangle(wi+4, wi+5, wi+11);											nTriangles++;
			handMesh.addTriangle(wi+5, wi+7, wi+11);											nTriangles++;
			handMesh.addTriangle(wi+5, wi+6, wi+7);												nTriangles++;
			handMesh.addTriangle(wi+6, wi+8, wi+7);												nTriangles++;
			
			// handMesh.addTriangle(wi+7, wi+8, 233 ); // replace with 2 triangles, as follows:
			handMesh.addTriangle(wi+7, wi+8, wi+12 );											nTriangles++;
			handMesh.addTriangle(wi+8, 233,  wi+12 );											nTriangles++;
			
			//----------------------------------------------------
			// Mesh the palm.
			
			// Get interpolated values on the outside contour:
			int palmContourIndex0 = Handmarks[HANDMARK_PALM_BASE     ].index;
			int palmContourIndex1 = Handmarks[HANDMARK_PINKY_SIDE    ].index;
			
			ofPolyline palmSideContour;
			ofPolyline palmSideContourResampled;
			bool bGotPalmSideContour = true;
			if (palmContourIndex0 < palmContourIndex1){
				for (int i=palmContourIndex0; i<=palmContourIndex1; i++){
					ofPoint cpt = handContour[i];
					palmSideContour.addVertex( ofPoint(cpt.x, cpt.y, 0.0) );
				}
				
				int nDesiredResampledPoints = 9;
				palmSideContourResampled.clear();
                // THIS PART WAS FUCKED UP. BAD BAD BAD.
				int palmSideContourSize = palmSideContour.size();
				int nDesiredSamples = nDesiredResampledPoints;
				for (int i=0; i<nDesiredSamples; i++){
					float indexf = ofMap((float)i,0,nDesiredSamples-1, 0,palmSideContourSize-1);
					int outindex = (int)((i==(nDesiredSamples-1)) ? floorf(indexf) : roundf(indexf));
					ofVec3f pointi = palmSideContour[outindex];
					palmSideContourResampled.addVertex(pointi.x, pointi.y, 0.0);
				}
                // EVIL
//				ofPoint cpt = handContour[palmContourIndex1];
//				palmSideContourResampled.addVertex( cpt.x, cpt.y );
				
				
			} else {
				// hopefully this is really unlikely.
				// but, it happens for all left hands facing down, or right hands facing up :)
				bGotPalmSideContour = false;
				printf("Problem meshing palm side.\n");
                return;
			}
			
			if (bGotPalmSideContour){
				
				int nPalmSideResampledContourPoints = palmSideContourResampled.size();
				for (int i=1; i<(nPalmSideResampledContourPoints-1); i++){
					ofPoint cpt = palmSideContourResampled[i];
					handMesh.addVertex ( ofPoint(cpt.x, cpt.y, 0.0));
					if (bDraw){ ofEllipse(cpt.x, cpt.y, 2,2);}
				}
				
				handMesh.addTriangle (344, 287, 58);	nTriangles++;
				handMesh.addTriangle (287, 288, 62);	nTriangles++;
				handMesh.addTriangle (288, 289, 119);	nTriangles++;
				handMesh.addTriangle (289, 290, 176);	nTriangles++;
				handMesh.addTriangle (290, 337, 233);	nTriangles++;
				
				int wristPointMeshIndex = 293;
				int wristPointMeshIndices[]    = {292, 293,293,293,293, 294};
				int knuckleMeshIndices[]       = {344, 287,288,289,290, 337, 337};
				int thumbSidePalmMeshIndices[] = {295, 334, 325, 335, 329, 336, 332, 337, 233};
				
				for (int k=0; k<6; k++){
					float wx = handMesh.getVertex(wristPointMeshIndices[k]).x;
					float wy = handMesh.getVertex(wristPointMeshIndices[k]).y;
					float kx = handMesh.getVertex(knuckleMeshIndices[k]).x;
					float ky = handMesh.getVertex(knuckleMeshIndices[k]).y;
					for (int i=1; i<7; i++){
						float frac = (float)i/7.0;
						float px = (1-frac)*wx + frac*kx;
						float py = (1-frac)*wy + frac*ky;
						handMesh.addVertex( ofVec3f (px,py, 0.0));
						if (bDraw){ ofEllipse(px, py, 2,2);}
					}
				}
				
				int starti = 338;
				for (int j=0; j<=6; j++){
					
					int dn = 6;
					if (j==0){
						dn = 7;
					}
					
					if (j == 6){
						
						for (int i=0; i<=5; i++){
							int a = starti + i;
							int b = thumbSidePalmMeshIndices[i+1];
							int c = starti + i + 1;
							int d = thumbSidePalmMeshIndices[i+2];
							
							if (i==5){
								c = 337;
								handMesh.addTriangle (a, b, c);			nTriangles++;
							} else {
								handMesh.addTriangle (a, b, c);			nTriangles++;
								handMesh.addTriangle (c, b, d);			nTriangles++;
							}
						}
						
					} else {
						if ((j>=1) && (j < 5)){
							int a = starti;
							int b = wristPointMeshIndex;
							int c = starti + dn;
							handMesh.addTriangle (a, b, c);				nTriangles++;
						}
						
						for (int i=0; i<=5; i++){
							int a = starti + i;
							int b = starti + i + dn;
							int c = starti + i + 1;
							int d = starti + i + dn+1;
							
							if (i==5){
								if (j > 0){
									c = knuckleMeshIndices[j-1];
									d = knuckleMeshIndices[j  ];
									handMesh.addTriangle (a, b, c);		nTriangles++;
									handMesh.addTriangle (c, b, d);		nTriangles++;
								}
							} else {
								handMesh.addTriangle (a, b, c);			nTriangles++;
								handMesh.addTriangle (c, b, d);			nTriangles++;
							}
						}
					}
					
					starti += dn;
				}
				
				
				handMesh.addTriangle (344, 343, 350);					nTriangles++;
				handMesh.addTriangle (291, 292, 338);					nTriangles++;
				handMesh.addTriangle (292, 345, 338);					nTriangles++;
				handMesh.addTriangle (292, 293, 345);					nTriangles++;
				handMesh.addTriangle (293, 294, 369);					nTriangles++;
				handMesh.addTriangle (294, 375, 369);					nTriangles++;
				handMesh.addTriangle (294, 295, 375);					nTriangles++;
				handMesh.addTriangle (295, 334, 375);					nTriangles++;
                // printf("nTriangles = %d\n", nTriangles);
				
				
				// Add texture coordinates to meshes;
				// Be cognizant of half-scale stuff.
				float vertexScale = (bWorkAtHalfScale) ? 2.0 : 1.0;
				for (int i = 0; i < handMesh.getNumVertices(); i++) {
					handMesh.addTexCoord( vertexScale*(handMesh.getVertex(i)));
				}
				

				bCalculatedMesh = true;
			}
			
			
		}
	}
}


//=================================================================
void HandMeshBuilder::drawMesh(){
	if (bCalculatedMesh){
		ofSetColor(ofColor::white);
		handMesh.drawFaces();
	}
}

void HandMeshBuilder::drawMeshWireframe(){
	if (bCalculatedMesh){
		ofPushStyle();
		ofSetColor(0,204,255);
		handMesh.drawWireframe();
		ofPopStyle();
	}
}

void HandMeshBuilder::drawRefinedMeshWireframe(){
    if (bCalculatedMesh){
		ofPushStyle();
        ofSetColor(12,255,100);
        refinedHandMesh.drawWireframe();
		ofPopStyle();
    }
}


bool HandMeshBuilder::checkForTrianglesThatAreTooTeeny (ofMesh &mesh){
	
	float myLengthEpsilon = 0.05;
	float myAreaEpsilon   = 0.05;
	
	bool bKeepTrackOfShortest = false;
	float shortestEdge = 99999;
	float smallestArea = 99999;
	
	int len = mesh.getNumIndices();
    for (int i=0; i<len; i+=3){
        ofVec3f v1 = mesh.getVertex(mesh.getIndex(i    ));
        ofVec3f v2 = mesh.getVertex(mesh.getIndex(i + 1));
        ofVec3f v3 = mesh.getVertex(mesh.getIndex(i + 2));
		
		// Determine if any sides are too short.
		//
		float d12 = v1.distance(v2);
		if (d12 < myLengthEpsilon){ return true; }
		float d13 = v1.distance(v3);
		if (d13 < myLengthEpsilon){ return true; }
		float d23 = v2.distance(v3);
		if (d23 < myLengthEpsilon){ return true; }
		
		// Use Heron's formula to get area.
		// See http://mathworld.wolfram.com/TriangleArea.html
		//
		float s = (d12 + d13 + d23)/2.0; // semiPerimeter
		float triangleArea = sqrtf ( s*(s - d12)*(s - d13)*(s - d23));
		if (triangleArea < myAreaEpsilon){ return true; }
		
		
		// keep track of minima, for research purposes.
		if (bKeepTrackOfShortest){
			if (d12 < shortestEdge){ shortestEdge = d12; }
			if (d13 < shortestEdge){ shortestEdge = d13; }
			if (d23 < shortestEdge){ shortestEdge = d23; }
			if (triangleArea < smallestArea){
				smallestArea = triangleArea;
			}
		}
	}

	bool bVerbose = false;
	if (bVerbose && bKeepTrackOfShortest){
		printf ("Smallest triArea = %f		Shortest edge = %f\n", smallestArea, shortestEdge);
	}
	
	return false;
}



// Returns true iff the mesh contains two index defined traingles that intersect.
bool HandMeshBuilder::check_triangle_intersections(ofMesh &mesh)
{
	
    int len = mesh.getNumIndices();
    for(int i = 0; i < len; i += 3)
    {
        ofVec3f v1 = mesh.getVertex(mesh.getIndex(i));
        ofVec3f v2 = mesh.getVertex(mesh.getIndex(i + 1));
        ofVec3f v3 = mesh.getVertex(mesh.getIndex(i + 2));
        
        for(int j = 0; j < len; j += 3)
        {
            if(j == i)
            {
                continue;
            }
            
            ofVec3f p1 = mesh.getVertex(mesh.getIndex(j));
            ofVec3f p2 = mesh.getVertex(mesh.getIndex(j + 1));
            ofVec3f p3 = mesh.getVertex(mesh.getIndex(j + 2));
            
            int b  = Intersecting(p1, p2, v1, v2, v3);
            int b2 = Intersecting(p2, p3, v1, v2, v3);
            int b3 = Intersecting(p3, p1, v1, v2, v3);
            
            if(b == 1 || b2 == 1 || b3 == 1)
            {
                //cout << "DEbug1" << endl;
                //handMesh.save("PotentialFault1.ply");
				return true;
            }
            
            bool bb  = point_triangle_intersection(v1, v2, v3, p1);
            bool bb2 = point_triangle_intersection(v1, v2, v3, p2);
            bool bb3 = point_triangle_intersection(v1, v2, v3, p3);
            
            if(bb || bb2 || bb3)
            {
                //cout << "DEbug2" << endl;
                //return true;
            }
            
            
        }
        
    }
	
    return false;
    
}




// Counts triangles that intersect.
int HandMeshBuilder::count_triangle_intersections (ofMesh &mesh)
{
    
	int nTriangleIntersections = 0;
	
    int len = mesh.getNumIndices();
    for(int i = 0; i < len; i += 3)
    {
        ofVec3f v1 = mesh.getVertex(mesh.getIndex(i));
        ofVec3f v2 = mesh.getVertex(mesh.getIndex(i + 1));
        ofVec3f v3 = mesh.getVertex(mesh.getIndex(i + 2));
        
        for(int j = 0; j < len; j += 3)
        {
            if(j == i)
            {
                continue;
            }
            
            ofVec3f p1 = mesh.getVertex(mesh.getIndex(j));
            ofVec3f p2 = mesh.getVertex(mesh.getIndex(j + 1));
            ofVec3f p3 = mesh.getVertex(mesh.getIndex(j + 2));
            
            int b  = Intersecting(p1, p2, v1, v2, v3);
            int b2 = Intersecting(p2, p3, v1, v2, v3);
            int b3 = Intersecting(p3, p1, v1, v2, v3);
            
            if (b == 1 || b2 == 1 || b3 == 1){
				nTriangleIntersections++;
            }
            
        }
        
    }
	
	if (nTriangleIntersections > 0){
		//printf ("%d ------- nTriangleIntersections = %d\n", (int)ofGetElapsedTimeMillis(), nTriangleIntersections);
	}
    
    return nTriangleIntersections;
}


/* Check whether P and Q lie on the same side of line AB */
float HandMeshBuilder::Side(ofVec3f &p, ofVec3f &q, ofVec3f &a, ofVec3f b)
{
    float z1 = (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y);
    float z2 = (b.x - a.x) * (q.y - a.y) - (q.x - a.x) * (b.y - a.y);
    return z1 * z2;
}

/* Check whether segment P0P1 intersects with triangle t0t1t2 */
int HandMeshBuilder::Intersecting(ofVec3f &p0, ofVec3f &p1, ofVec3f &t0, ofVec3f &t1, ofVec3f &t2)
{
    /* Check whether segment is outside one of the three half-planes
     * delimited by the triangle. */
    float f1 = Side(p0, t2, t0, t1), f2 = Side(p1, t2, t0, t1);
    float f3 = Side(p0, t0, t1, t2), f4 = Side(p1, t0, t1, t2);
    float f5 = Side(p0, t1, t2, t0), f6 = Side(p1, t1, t2, t0);
    /* Check whether triangle is totally inside one of the two half-planes
     * delimited by the segment. */
    float f7 = Side(t0, t1, p0, p1);
    float f8 = Side(t1, t2, p0, p1);
    
    /* If segment is strictly outside triangle, or triangle is strictly
     * apart from the line, we're not intersecting */
    if ((f1 < 0 && f2 < 0) || (f3 < 0 && f4 < 0) || (f5 < 0 && f6 < 0)
        || (f7 > 0 && f8 > 0))
        return 0;
    
    /* If segment is aligned with one of the edges, we're overlapping */
    if ((f1 == 0 && f2 == 0) || (f3 == 0 && f4 == 0) || (f5 == 0 && f6 == 0))
        return 2;
    
    /* If segment is outside but not strictly, or triangle is apart but
     * not strictly, we're touching */
    if ((f1 <= 0 && f2 <= 0) || (f3 <= 0 && f4 <= 0) || (f5 <= 0 && f6 <= 0)
        || (f7 >= 0 && f8 >= 0))
        return 2; // Touching.
    
    /* If both segment points are strictly inside the triangle, we
     * are not intersecting either */
    if (f1 > 0 && f2 > 0 && f3 > 0 && f4 > 0 && f5 > 0 && f6 > 0)
        return 0;
    
    /* Otherwise we're intersecting with at least one edge */
    return 1;
}

// Returns true iff the point is inside or on the exterior of the triangle and not equal to one of the triangle vertices.
bool HandMeshBuilder::point_triangle_intersection(ofVec3f &t1, ofVec3f &t2, ofVec3f &t3, ofVec3f &point)
{
    
    if(point == t1 || point == t2 || point == t3)
    {
        //return false;
    }
    
    float c1, c2, c3;
    
    BarycentricCoords(t1, t2, t3, point, c1, c2, c3);
    
    if(0 < c1 && c1 < 1 &&
       0 < c2 && c2 < 1 &&
       0 < c3 && c3 < 1)
    {
        return true;
    }
}


void HandMeshBuilder::BarycentricCoords(const ofVec3f & vTriVtx1,
                       const ofVec3f & vTriVtx2,
                       const ofVec3f & vTriVtx3,
                       const ofVec3f & vVertex,
                       float & fBary1, float & fBary2, float & fBary3 )
{
    
    ofVec3f kV02 = vTriVtx1 - vTriVtx3;
    ofVec3f kV12 = vTriVtx2 - vTriVtx3;
    ofVec3f kPV2 = vVertex - vTriVtx3;
    
    float fM00 = kV02.dot(kV02);
    float fM01 = kV02.dot(kV12);
    float fM11 = kV12.dot(kV12);
    float fR0  = kV02.dot(kPV2);
    float fR1  = kV12.dot(kPV2);
    float fDet = fM00*fM11 - fM01*fM01;
    
    float fInvDet = ((float)1.0)/fDet;
    
    fBary1 = (fM11*fR0 - fM01*fR1)*fInvDet;
    fBary2 = (fM00*fR1 - fM01*fR0)*fInvDet;
    fBary3 = (float)1.0 - fBary1 - fBary2;
}






