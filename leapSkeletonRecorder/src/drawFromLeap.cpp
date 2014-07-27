//
//  drawFromLeap.cpp
//  ofxLeapMotion
//
//  Created by GL on 6/30/14.
//
//

#include "testApp.h"


//--------------------------------------------------------------
void testApp::drawFrame (){
	
	// If hand(s) exist in the frame,
	// Get the vector of Hands from ofxLeap.
	vector <Hand> hands = leap.getLeapHands();
	if (hands.size() > 0) {
		
		// For each hand,
		for (int h=0; h<hands.size(); h++){
			
			// Get the current hand
			Hand & hand = hands[h];
			drawHand (hand);
		}
	}
}

//--------------------------------------------------------------
void testApp::drawHand (Hand & hand){
	if (hand.isValid()){
		drawFingers (hand);
		drawPalm (hand);
		drawArm (hand);
	}
}

//--------------------------------------------------------------
void testApp::drawFingers (Hand & hand){
	
	// For each finger in the Hand,
	FingerList fingers = hand.fingers();
	for (int f=0; f<fingers.count(); f++){
		
		// Get the current finger, and it's type (index, thumb, etc.);
		const Finger & finger = fingers[f];
		if (finger.isValid()){
			
			drawFinger(finger);
			
		} //end if finger isValid()
	} // end for each finger
}

//--------------------------------------------------------------
void testApp::drawFinger (const Finger & finger){
	
	if (finger.isValid()){
		
		// For every bone (i.e. phalange) in the finger,
		for (int b=0; b<4; b++) {
			
			// Get each bone;
			Bone::Type boneType = static_cast<Bone::Type>(b);
			Bone bone = finger.bone(boneType);
			if (bone.isValid()){
				
				// Don't consider zero-length bones, such as the Thumb's metacarpal.
				if (bone.length() > 0){
					drawBone (finger, bone);
					
				} // end if boneLength
			} // end if bone isValid()
		} // end for each bone
		
		if (bDrawSimple){
			// Draw the fingertip, which is an extra point within the last phalange.
			ofSetColor(ofColor::white);
			ofPoint fingerTipPt = leap.getofPoint ( finger.tipPosition() );
			ofDrawSphere(fingerTipPt, finger.width() * 0.05);
		}
	}
}

//--------------------------------------------------------------
void testApp::drawBone (const Finger & finger, Bone & bone){
	
	Finger::Type fingerType = finger.type();
	Bone::Type   boneType   = bone.type();
	
	// The Leap returns data in millimeters.
	ofPoint bonePt0 = leap.getofPoint ( bone.prevJoint());
	ofPoint bonePt1 = leap.getofPoint ( bone.nextJoint());
	float boneThickness = bone.width();
	
	// ofPoint bonePtC = leap.getofPoint ( bone.center()); // works, but:
	ofPoint bonePtC = (bonePt0 + bonePt1)/2.0;
	
	if (bDrawSimple){
		// Draw a simple white skeleton.
		ofSetColor(ofColor::white);
		ofLine(bonePt0, bonePt1);
		ofDrawSphere(bonePt0, boneThickness * 0.15);
		ofDrawSphere(bonePt1, boneThickness * 0.15);
		ofDrawSphere(bonePtC, boneThickness * 0.05);
		
	} else {
		// Draw a colored cylinder with double-sphere caps.
		setColorByFinger (fingerType, boneType);
		drawOrientedCylinder (bonePt0, bonePt1, boneThickness/2.0);
		ofDrawSphere(bonePt0, boneThickness/2.0);
		ofDrawSphere(bonePt1, boneThickness/2.0);
	}
}


//--------------------------------------------------------------
void testApp::drawPalm (Hand & hand){
	// This draws the palm as a gray region.
	
	// Collect the palm vertices into an ofMesh.
	ofMesh palmMesh;
	int nVertices = 0;
	float averageBoneWidth = 0;
	
	// For each finger,
	FingerList fingers = hand.fingers();
	for (int f=0; f<fingers.count(); f++){
		
		// Get the current finger, and it's type (index, thumb, etc.);
		const Finger & finger = fingers[f];
		if (finger.isValid()){
			
			Finger::Type fingerType = finger.type();
			Bone bone;
			if (fingerType == Finger::TYPE_THUMB){
				bone = finger.bone(Bone::TYPE_PROXIMAL);
			} else {
				bone = finger.bone(Bone::TYPE_METACARPAL);
			}
			
			// If we've found the bones we want, add their vertices to the mesh.
			if (bone.isValid()){
				float boneLength = bone.length();
				if (boneLength > 0){
					
					ofPoint pt0 = leap.getofPoint ( bone.prevJoint());
					ofPoint pt1 = leap.getofPoint ( bone.nextJoint());
					
					palmMesh.addVertex(pt0);
					palmMesh.addVertex(pt1);
					
					averageBoneWidth += bone.width();
					nVertices += 2;
				}
			}
		}
	}
	averageBoneWidth /= (nVertices/2);
	
	// Render the palm as a triangle strip surface,
	// (optionally) bordered by cylinders.
	if (nVertices > 3){
		ofSetColor(ofColor::gray);
		
		// Draw the palm as a mesh of triangles.
		int nPalmMeshVertices = palmMesh.getNumVertices();
		for (int i=0; i<(nPalmMeshVertices-2); i++){
			palmMesh.addTriangle(i, i+1, i+2);
		}
		palmMesh.drawFaces();
		
		// Add optional cylinders.
		if (!bDrawSimple){
			float rad = averageBoneWidth / 2.0;
			
			if (nPalmMeshVertices == 10){
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


//--------------------------------------------------------------
void testApp::drawArm (Hand & hand){
	
	// Draw the wrist and elbow points.
	Arm arm = hand.arm();
	if (arm.isValid()){
		
		ofPoint handPt   = leap.getofPoint ( hand.palmPosition());
		ofPoint handNorm = leap.getofPoint ( hand.palmNormal());
		ofPoint wristPt  = leap.getofPoint ( arm.wristPosition());
		ofPoint elbowPt  = leap.getofPoint ( arm.elbowPosition());
		
		float basisLen = 50.0;
		
		
		if (bDrawSimple){
			ofSetColor(ofColor::white);
			ofDrawSphere(handPt,  8.0);
			ofDrawSphere(wristPt, 8.0);
			ofDrawSphere(elbowPt, 8.0);
			
			ofLine(handPt, wristPt);
			ofLine(wristPt, elbowPt);
			ofLine(handPt, handPt+ basisLen*handNorm);
			ofDrawSphere(handPt+ basisLen*handNorm, 2.0);
			
			// draw the rotation vectors of the hand.
			{
				Leap::Matrix handMatrix = hand.basis();
				ofPoint handBasisX = leap.getofPoint( handMatrix.xBasis);
				ofPoint handBasisY = leap.getofPoint( handMatrix.yBasis);
				ofPoint handBasisZ = leap.getofPoint( handMatrix.zBasis);
				
				glLineWidth(2.0);
				ofSetColor(ofColor::red  );	ofLine(handPt, handPt + basisLen*handBasisX);
				ofSetColor(ofColor::green);	ofLine(handPt, handPt + basisLen*handBasisY);
				ofSetColor(ofColor::blue );	ofLine(handPt, handPt + basisLen*handBasisZ);
				glLineWidth(1.0);
				
				// draw the identity of the hand (left or right)
				string whichHandString = "RIGHT";
				if (hand.isLeft()){
					whichHandString = "LEFT";
				}
				// float handConfidence = hand.confidence();
				// whichHandString += " " + ofToString(handConfidence);
				
				ofSetColor(ofColor::white);
				ofDrawBitmapString(whichHandString, (handPt + (basisLen*1.2)*handBasisY));
			}
			
			// draw the rotation vectors of the arm.
			{
				Leap::Matrix armMatrix = arm.basis();
				ofPoint armBasisX = leap.getofPoint( armMatrix.xBasis);
				ofPoint armBasisY = leap.getofPoint( armMatrix.yBasis);
				ofPoint armBasisZ = leap.getofPoint( armMatrix.zBasis);
				
				glLineWidth(2.0);
				ofSetColor(ofColor::red  );	ofLine(wristPt, wristPt + basisLen*armBasisX);
				ofSetColor(ofColor::green);	ofLine(wristPt, wristPt + basisLen*armBasisY);
				ofSetColor(ofColor::blue );	ofLine(wristPt, wristPt + basisLen*armBasisZ);
				glLineWidth(1.0);
			}
			
		} else {
			
			// Draw a cylinder between two points, properly oriented in space.
			float armWidth = arm.width();
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
				Leap::Matrix armMatrix = arm.basis();
				ofPoint armBasisY = leap.getofPoint( armMatrix.yBasis);
				float ax = armBasisY.x;
				float ay = armBasisY.y;
				float az = armBasisY.z;
				
				// Compute the longitudinal rotation of the arm.
				// Sheesh, I really need to learn 3D matrix math.
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
	} // Close if arm isValid
}

