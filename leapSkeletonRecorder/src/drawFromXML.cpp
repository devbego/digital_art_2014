//
//  drawFromXML.cpp
//  ofxLeapMotion
//
//  Created by GL on 6/30/14.
//
//

#include "testApp.h"


//--------------------------------------------------------------
void testApp::drawFrameFromXML (int whichFrame){
	
	int nFrameTags = XML.getNumTags("FRAME");
	if ((whichFrame >= 0) && (whichFrame < nFrameTags)){
		
		//we push into the which'h FRAME tag; this temporarily treats the tag as the document root.
		XML.pushTag("FRAME", whichFrame);
		
		int nHandTags = XML.getNumTags("H");
		if (nHandTags > 0){
			for (int h=0; h<nHandTags; h++){
				drawHandFromXML (h);
			}
		}
		
		//this pops us out of the FRAME tag, sets the root back to the xml document
		XML.popTag();
	}
}

//--------------------------------------------------------------
void testApp::drawHandFromXML (int whichHand){
	XML.pushTag("H", whichHand);
	
	drawFingersFromXML();
	drawPalmFromXML();
	drawArmFromXML();
	
	XML.popTag();
}

//--------------------------------------------------------------
void testApp::drawFingersFromXML(){
	
	int nFingerTags = XML.getNumTags("F");
	if (nFingerTags > 0){
		for (int f=0; f<nFingerTags; f++){
			XML.pushTag("F", f);
			drawFingerFromXML();
			XML.popTag();
		}
	}
}

//--------------------------------------------------------------
void testApp::drawFingerFromXML(){
	
	Finger::Type	fingerType =  (Finger::Type) XML.getValue("TYPE", 0);
	float			fingerWidth = XML.getValue("WIDTH", 0.0);
	ofPoint			fingerTipPt = ofPoint(XML.getValue("T:X",0.0), XML.getValue("T:Y",0.0), XML.getValue("T:Z",0.0));
	
	int nBoneTags = XML.getNumTags("B");
	if (nBoneTags > 0){
		
		ofPoint bonePt0;
		ofPoint bonePt1;
		ofPoint bonePtC;
		for (int b=0; b<nBoneTags; b++){
			XML.pushTag("B", b);
			Bone::Type	boneType =  (Bone::Type) XML.getValue("TYPE", 0);
			if (XML.getNumTags("Q") > 0){
			bonePt0 = ofPoint(XML.getValue("Q:X",0.0), XML.getValue("Q:Y",0.0), XML.getValue("Q:Z",0.0));
			}
			bonePt1 = ofPoint(XML.getValue("P:X",0.0), XML.getValue("P:Y",0.0), XML.getValue("P:Z",0.0));
			bonePtC = (bonePt0 + bonePt1)/2.0;
			
			if (bDrawSimple){
				// Draw a simple skeleton.
				ofSetColor(ofColor::orange);
				ofLine(bonePt0, bonePt1);
				ofDrawSphere(bonePt0, fingerWidth * 0.15);
				ofDrawSphere(bonePt1, fingerWidth * 0.15);
				ofDrawSphere(bonePtC, fingerWidth * 0.05);
				
			} else {
				// Draw a colored cylinder with double-sphere caps.
				setColorByFinger (fingerType, boneType);
				drawOrientedCylinder (bonePt0, bonePt1, fingerWidth/2.0);
				ofDrawSphere(bonePt0, fingerWidth/2.0);
				ofDrawSphere(bonePt1, fingerWidth/2.0);
			}
			bonePt0 = bonePt1;
			XML.popTag();
		}
	}
}


//--------------------------------------------------------------
void testApp::drawPalmFromXML(){
	
	int nFingerTags = XML.getNumTags("F");
	if (nFingerTags > 0){
		
		ofMesh palmMesh;
		int nVertices = 0;
		float averageFingerWidth = 0;
		
		for (int f=0; f<nFingerTags; f++){
			XML.pushTag("F", f);
			float fingerWidth = XML.getValue("WIDTH", 0.0);
			averageFingerWidth += fingerWidth;
			
			if (XML.getNumTags("B") > 0){
				XML.pushTag("B", 0);
				palmMesh.addVertex( ofPoint(XML.getValue("Q:X",0.0), XML.getValue("Q:Y",0.0), XML.getValue("Q:Z",0.0)) );
				palmMesh.addVertex( ofPoint(XML.getValue("P:X",0.0), XML.getValue("P:Y",0.0), XML.getValue("P:Z",0.0)) );
				nVertices += 2;
				XML.popTag();
			}
			XML.popTag();
		}
		
		if (nVertices > 3){
			if (bDrawSimple){
				ofSetColor(ofColor::brown);
			} else {
				ofSetColor(ofColor::gray);
			}
			
			// Draw the palm as a mesh of triangles.
			int nPalmMeshVertices = palmMesh.getNumVertices();
			for (int i=0; i<(nPalmMeshVertices-2); i++){
				palmMesh.addTriangle(i, i+1, i+2); }
			palmMesh.drawFaces();
			
			if (!bDrawSimple){
				averageFingerWidth /= nFingerTags;
				if (nPalmMeshVertices == 10){
					float rad = averageFingerWidth / 2.0;
					for (int i=0; i<4; i++){
						ofVec3f p0 = palmMesh.getVertex( i   *2);
						ofVec3f p1 = palmMesh.getVertex((i+1)*2);
						drawOrientedCylinder (p0, p1, 10);
						ofDrawSphere(p0, rad);
						ofDrawSphere(p1, rad);
					}
					for (int i=0; i<4; i++){
						ofVec3f p0 = palmMesh.getVertex( i   *2 + 1);
						ofVec3f p1 = palmMesh.getVertex((i+1)*2 + 1);
						drawOrientedCylinder (p0, p1, 10);
						ofDrawSphere(p0, rad);
						ofDrawSphere(p1, rad);
					}
				}
			}
		}
	}
}


//--------------------------------------------------------------
void testApp::drawArmFromXML(){
	
	float armWidth = XML.getValue("AW", 0.0);
	float basisLen = 50.0;
	
	ofPoint	handPt    = ofPoint(XML.getValue("PM:X",0.0), XML.getValue("PM:Y",0.0), XML.getValue("PM:Z",0.0));	// palm
	ofPoint	wristPt   = ofPoint(XML.getValue("W:X",0.0),  XML.getValue("W:Y",0.0),  XML.getValue("W:Z",0.0));	// wrist
	ofPoint	elbowPt   = ofPoint(XML.getValue("E:X",0.0),  XML.getValue("E:Y",0.0),  XML.getValue("E:Z",0.0));	// elbow
	ofPoint	handNorm  = ofPoint(XML.getValue("PN:X",0.0), XML.getValue("PN:Y",0.0), XML.getValue("PN:Z",0.0));	// palm normal
	
	//Hand basis matrix
	XML.pushTag("HM", 0);
		ofPoint handBasisX = ofPoint(XML.getValue("XX",0.0), XML.getValue("XY",0.0), XML.getValue("XZ",0.0));
		ofPoint handBasisY = ofPoint(XML.getValue("YX",0.0), XML.getValue("YY",0.0), XML.getValue("YZ",0.0));
		ofPoint handBasisZ = ofPoint(XML.getValue("ZX",0.0), XML.getValue("ZY",0.0), XML.getValue("ZZ",0.0));
	XML.popTag();
	
	// Arm basis matrix
	XML.pushTag("AM", 0);
		ofPoint armBasisX = ofPoint(XML.getValue("XX",0.0), XML.getValue("XY",0.0), XML.getValue("XZ",0.0));
		ofPoint armBasisY = ofPoint(XML.getValue("YX",0.0), XML.getValue("YY",0.0), XML.getValue("YZ",0.0));
		ofPoint armBasisZ = ofPoint(XML.getValue("ZX",0.0), XML.getValue("ZY",0.0), XML.getValue("ZZ",0.0));
	XML.popTag();
	

	// Draw the wrist and elbow points.
	if (bDrawSimple){
		ofSetColor(ofColor::orange);
		ofDrawSphere(handPt,  8.0);
		ofDrawSphere(wristPt, 8.0);
		ofDrawSphere(elbowPt, 8.0);
		
		ofLine(handPt, wristPt);
		ofLine(wristPt, elbowPt);
		ofLine(handPt, handPt+ basisLen*handNorm);
		ofDrawSphere(handPt+ basisLen*handNorm, 2.0);
		
		glLineWidth(2.0);
			// draw the rotation vectors of the hand.
			ofSetColor(ofColor::red  );	ofLine(handPt, handPt + basisLen*handBasisX);
			ofSetColor(ofColor::green);	ofLine(handPt, handPt + basisLen*handBasisY);
			ofSetColor(ofColor::blue );	ofLine(handPt, handPt + basisLen*handBasisZ);
		
			// draw the rotation vectors of the arm.
			ofSetColor(ofColor::red  );	ofLine(wristPt, wristPt + basisLen*armBasisX);
			ofSetColor(ofColor::green);	ofLine(wristPt, wristPt + basisLen*armBasisY);
			ofSetColor(ofColor::blue );	ofLine(wristPt, wristPt + basisLen*armBasisZ);
		glLineWidth(1.0);
		
		string handType = XML.getValue("TYPE", "RIGHT");
		ofSetColor(ofColor::orange);
		ofDrawBitmapString(handType, (handPt + (basisLen*1.2)*handBasisY));
		

		 
	} else {
		
		// Draw a cylinder between two points, properly oriented in space.
		float dx = wristPt.x - elbowPt.x;
		float dy = wristPt.y - elbowPt.y;
		float dz = wristPt.z - elbowPt.z;
		float dh = sqrt(dx*dx + dy*dy + dz*dz);
		
		ofPushMatrix();
		{
			ofTranslate( (elbowPt.x+wristPt.x)/2, (elbowPt.y+wristPt.y)/2, (elbowPt.z+wristPt.z)/2 );
			
			float theta =   90 - RAD_TO_DEG * asin(dz/dh);
			float phi   =        RAD_TO_DEG * atan2(dy,dx);
			ofRotate(phi,   0,0,1);
			ofRotate(theta, 0,1,0);
			ofRotate(90,	1,0,0);
			
			// Get the arm Matrix, which provides its orthogonal basis vectors.
			float ax = armBasisY.x;
			float ay = armBasisY.y;
			float az = armBasisY.z;
			
			// Compute the longitudinal rotation of the arm
			ofNode armBasisYNode;
			armBasisYNode.setPosition(armBasisY);
			armBasisYNode.rotateAround(0-   phi, ofVec3f(0,0,1), ofVec3f(0,0,0));
			armBasisYNode.rotateAround(0- theta, ofVec3f(0,1,0), ofVec3f(0,0,0));
			armBasisYNode.rotateAround(0-    90, ofVec3f(1,0,0), ofVec3f(0,0,0));
			ofPoint newArmBasisY = armBasisYNode.getPosition();
			float armRotation = RAD_TO_DEG * atan2f(newArmBasisY.z, newArmBasisY.x);
			
			ofPushMatrix();
			{
				ofRotate(armRotation, 0,-1,0);
				float armThicknessRatio = 0.6;
				glScalef(armThicknessRatio, 1.0, 1.0);
				ofSetColor(ofColor::magenta);
				
				// Oblate arm cylinder
				ofDrawCylinder (armWidth/2.0, dh);
				
				// Wrist endcap
				ofPushMatrix();
				ofTranslate(ofPoint(0, dh/2,0));
				glScalef(1.0, armThicknessRatio, 1.0);
				ofDrawSphere(armWidth/2.0);
				ofPopMatrix();
				
				// Elbow endcap
				ofPushMatrix();
				ofTranslate(ofPoint(0, -dh/2,0));
				glScalef(1.0, armThicknessRatio, 1.0);
				ofDrawSphere(armWidth/2.0);
				ofPopMatrix();
				
			} // Close popMatrix
			ofPopMatrix();
		} // Close popMatrix
		ofPopMatrix();
	} // Close if !drawSimple

}




