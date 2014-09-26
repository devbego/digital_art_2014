//
//  HandMeshBuilder.cpp
//  leapAndCamHandSegmenter
//
//  Created by GL on 9/22/14.
//
//

#include "HandMeshBuilder.h"


//============================================================
void HandMeshBuilder::initialize(){
	
	// drawing order, in case you're curious.
	fingerTipIndices[0] = HANDMARK_THUMB_TIP;
	fingerTipIndices[1] = HANDMARK_PINKY_TIP;
	fingerTipIndices[2] = HANDMARK_RING_TIP;
	fingerTipIndices[3] = HANDMARK_MIDDLE_TIP;
	fingerTipIndices[4] = HANDMARK_POINTER_TIP;
	
	informThereIsNoHandPresent();
	bWorkAtHalfScale = true; // most likely
}

//============================================================
void HandMeshBuilder::loadDefaultMesh(){
	handMesh.load("models/handmarksNew.ply");
	for (int i = 0; i < handMesh.getNumVertices(); i++) {
		// handMesh.addTexCoord(handMesh.getVertex(i));
		
		ofVec2f handMeshVertex;
		handMeshVertex.x =  handMesh.getVertex(i).y;
		handMeshVertex.y =  768 - handMesh.getVertex(i).x;
		handMesh.addTexCoord( handMeshVertex );
	}
    
    refinedHandMesh = handMesh; /*
                                 //.load("models/handmarksNew.ply");
    for (int i = 0; i < refinedHandMesh.getNumVertices(); i++) {
        ofVec2f handMeshVertex;
        handMeshVertex.x =  refinedHandMesh.getVertex(i).y;
        handMeshVertex.y =  768 - refinedHandMesh.getVertex(i).x;
        refinedHandMesh.addTexCoord( handMeshVertex );
    }
                                 */
    
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
    //return refinedHandMesh;
}



//============================================================
void HandMeshBuilder::buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks){
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
				printf ("problem with handmark %d: %d\n", i, Handmarks[i].index);
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
                
				// This is NOT the right place to compute the butterfly subdivided mesh,
				// because the subdivision should be done *after* the puppet, not before.
                bool bComputeRefinedMesh = false;
                if (bComputeRefinedMesh){
                    long t0 = ofGetElapsedTimeMicros();
                    refinedHandMesh = butterflyMeshSubdivider.subdivideBoundary(handMesh, 1.7);
                    long t1 = ofGetElapsedTimeMicros();
                    printf("Butterfly = %d\n", (int)(t1-t0));
                }
				
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
		ofSetColor(0,204,255);
		handMesh.drawWireframe();
	}
}

void HandMeshBuilder::drawRefinedMeshWireframe(){
    if (bCalculatedMesh){
        ofSetColor(12,255,100);
        refinedHandMesh.drawWireframe();
    }
}


//ofSetColor(255,100,100, 70);
//handMesh.draw();










