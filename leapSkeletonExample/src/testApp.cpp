#include "testApp.h"

/*
// OPENFRAMEWORKS + LEAP MOTION SDK 2.0 HAND SKELETON DEMO 
// By Golan Levin (@golan), http://github.com/golanlevin
// Uses ofxLeapMotion addon by Theo Watson, with assistance from Dan Wilcox
// Supported in part by the Frank-Ratchye STUDIO for Creative Inquiry at CMU
*/


 
/* Note on OS X, you must have this in the Run Script Build Phase of your project. 
where the first path ../../../addons/ofxLeapMotion/ is the path to the ofxLeapMotion addon. 

cp -f ../../../addons/ofxLeapMotion/libs/lib/osx/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libLeap.dylib"; install_name_tool -change ./libLeap.dylib @executable_path/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";

   If you don't have this you'll see an error in the console: dyld: Library not loaded: @loader_path/libLeap.dylib
*/

//--------------------------------------------------------------
void testApp::setup(){

    // ofSetFrameRate(60);
    ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_WARNING);
	ofSetCylinderResolution (16, 1, 16);

	leap.open();
	cam.setOrientation(ofPoint(-55, 0, 0));

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH);
	
	bDrawSimple = false;
	bDrawGrid = true; 
}



//--------------------------------------------------------------
void testApp::update(){
	// Nothing to see here.
}



//--------------------------------------------------------------
void testApp::draw(){

	ofBackground(0,0,0);
	ofSetColor(255);
	ofDrawBitmapString("Leap Connected? " + ofToString(leap.isConnected()), 20, 20);
	ofDrawBitmapString("Press any key to flip display mode", 20, 35);
	ofDrawBitmapString("Press 'c' to restore camera", 20, 50);
	ofDrawBitmapString("Press 'g' to toggle grid", 20, 65);
	
	cam.begin();
	
	// Draw a grid plane.
	if (bDrawGrid){
		ofPushMatrix();
			ofEnableSmoothing();
			ofRotate(90, 0, 0, 1);
			ofSetColor(160,160,160, 100);
			ofDrawGridPlane(200, 10, false);
		ofPopMatrix();
	}
	
	// Get the vector of Hands from ofxLeap.
	vector <Hand> hands = leap.getLeapHands();
	if (hands.size() > 0) {
		
		// For each hand,
		for (int h=0; h<hands.size(); h++){
			
			// Get the current hand
			Hand & hand = hands[h];
			if (hand.isValid()){
	
				drawTheFingersOfHand (hand);
				drawThePalmOfHand (hand);
				drawTheArmOfHand (hand);
			}
		}
	}
	
	cam.end();
	leap.markFrameAsOld();
}



//--------------------------------------------------------------
void testApp::drawTheFingersOfHand (Hand & hand){
	
	// For each finger in the Hand,
	FingerList fingers = hand.fingers();
	for (int f=0; f<fingers.count(); f++){
		
		// Get the current finger, and it's type (index, thumb, etc.);
		const Finger & finger = fingers[f];
		Finger::Type fingerType = finger.type();
		if (finger.isValid()){
		
			// For every bone (i.e. phalange) in the finger,
			for (int b=0; b<4; b++) {
				
				// Get each bone;
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);
				if (bone.isValid()){
					
					// Don't consider zero-length bones, such as the Thumb's metacarpal.
					float boneLength = bone.length();
					if (boneLength > 0){
						
						// The Leap returns data in millimeters.
						ofPoint bonePtC = leap.getofPoint ( bone.center());
						ofPoint bonePt0 = leap.getofPoint ( bone.prevJoint());
						ofPoint bonePt1 = leap.getofPoint ( bone.nextJoint());
						float boneThickness = bone.width();
						
						if (bDrawSimple){
							// Draw a simple white skeleton.
							ofSetColor(255);
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
						
					} // end if boneLength
				} // end if bone isValid()
			} // end for each bone
		} //end if finger isValid()
		
		if (bDrawSimple){
			// Draw the fingertip, which is a point within the last phalange.
			ofSetColor(255);
			ofPoint fingerTipPt = leap.getofPoint ( finger.tipPosition() );
			ofDrawSphere(fingerTipPt, finger.width() * 0.05);
		}
		
	} // end for each finger
}



//--------------------------------------------------------------
void testApp::drawThePalmOfHand (Hand & hand){
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
		ofSetColor(128);
		
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
void testApp::drawTheArmOfHand (Hand & hand){
	
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
			ofDrawSphere(handPt,  8);
			ofDrawSphere(wristPt, 8);
			ofDrawSphere(elbowPt, 8);
			
			ofLine(handPt, wristPt);
			ofLine(wristPt, elbowPt);
			ofLine(handPt, handPt+ basisLen*handNorm);

			
			
			// draw the rotation vectors of the hand.
			{
				Leap::Matrix handMatrix = hand.basis();
				ofPoint handBasisX = leap.getofPoint( handMatrix.xBasis);
				ofPoint handBasisY = leap.getofPoint( handMatrix.yBasis);
				ofPoint handBasisZ = leap.getofPoint( handMatrix.zBasis);
				
				ofSetColor(ofColor::red  );	ofLine(handPt, handPt + basisLen*handBasisX);
				ofSetColor(ofColor::green);	ofLine(handPt, handPt + basisLen*handBasisY);
				ofSetColor(ofColor::blue );	ofLine(handPt, handPt + basisLen*handBasisZ);
				
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
				
				ofSetColor(ofColor::red  );	ofLine(wristPt, wristPt + basisLen*armBasisX);
				ofSetColor(ofColor::green);	ofLine(wristPt, wristPt + basisLen*armBasisY);
				ofSetColor(ofColor::blue );	ofLine(wristPt, wristPt + basisLen*armBasisZ);
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
					ofSetColor(255,0,255);
				
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



//--------------------------------------------------------------
void testApp::setColorByFinger (Finger::Type fingerType, Bone::Type boneType){

	// Set the current color, according to the type of this finger.
	// Thumb is red, Index is green, etc.
	
	switch (fingerType){
		case Finger::TYPE_THUMB:
			ofSetColor(ofColor::red);
			break;
		case Finger::TYPE_INDEX:
			ofSetColor(ofColor::green);
			break;
		case Finger::TYPE_MIDDLE:
			ofSetColor(ofColor::blue);
			break;
		case Finger::TYPE_RING:
			ofSetColor(ofColor::yellow);
			break;
		case Finger::TYPE_PINKY:
			ofSetColor(ofColor::cyan);
			break;
		default:
			ofSetColor(ofColor::gray);
			break;
	}
	
	// For the bones inside the palm, set the color to gray.
	bool bSetInternalBonesToGray = true;
	if (bSetInternalBonesToGray){
		if ( (boneType == Bone::TYPE_METACARPAL) ||
			((boneType == Bone::TYPE_PROXIMAL)   && (fingerType == Finger::TYPE_THUMB))) {
			ofSetColor(128);
		}
	}
}



//--------------------------------------------------------------
void testApp::drawOrientedCylinder (ofPoint pt0, ofPoint pt1, float radius){
	
	// Draw a cylinder between two points, properly oriented in space.
	float dx = pt1.x - pt0.x;
	float dy = pt1.y - pt0.y;
	float dz = pt1.z - pt0.z;
	float dh = sqrt(dx*dx + dy*dy + dz*dz);
	
	ofPushMatrix();
		ofTranslate( (pt0.x+pt1.x)/2, (pt0.y+pt1.y)/2, (pt0.z+pt1.z)/2 );
		
		ofQuaternion q;
		q.makeRotate (ofPoint(0, -1, 0), ofPoint(dx,dy,dz) );
		ofMatrix4x4 m;
		q.get(m);
		glMultMatrixf(m.getPtr());
		
		ofDrawCylinder (radius, dh);
	ofPopMatrix();
}



//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if (key == 'c'){
		// Reset the camera if the user presses 'c'.
		cam.reset();
	} else if (key == 'g'){
		// flip whether or not we're drawing a grid.
		bDrawGrid = !bDrawGrid;
	} else {
		// When the user presses a key, flip the rendering mode.
		bDrawSimple = !bDrawSimple;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
}

//--------------------------------------------------------------
void testApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
