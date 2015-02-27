//
//  LeapRecorder.cpp
//  leapAndCamRecorder2
//
//  Created by chris on 28/07/14.
//
//

#include "LeapRecorder.h"

/*
 KEY TO Leap recording XML TAGS:
 
 <FRAME>						// Frame of data
 <COUNT>					// Count of this Frame
 <TIME>						// Recording time of this Frame (milliseconds)
 <NH>						// Number of Hands in Frame
 <H>						// Hand
 <INDEX>				// Index in the hand array
 <ID>					// Temporally-coherent ID number
 <TYPE>					// LEFT or RIGHT
 <CONF>					// Confidence, 0...1
 <F>					// Finger
 <TYPE>				// Leap Finger::Type (0..4), Thumb = 0
 <WIDTH>			// Thickness of Finger (mm)
 <T>				// Tip point
 <X>			// Tip point X
 <Y>			// Tip point Y
 <Z>			// Tip point Z
 <B>				// Bone
 <TYPE>			// Bone::Type (metacarpal etc.)
 <Q>			// Bone start point (Note: Only reported for first bone)
 <X>		// Bone start point X
 <Y>		// Bone start point Y
 <Z>		// Bone start point Z
 <P>			// Bone end point
 <X>		// Bone end point X
 <Y>		// Bone end point Y
 <Z>		// Bone end point Z
 <AW>					// Arm width (millimeters)
 <PM>					// Palm point
 <X>				// Palm point X
 <Y>				// Palm point Y
 <Z>				// Palm point Z
 <W>					// Wrist point
 <X>				// Wrist point X
 <Y>				// Wrist point Y
 <Z>				// Wrist point Z
 <E>					// Elbow point
 <X>				// Elbow point X
 <Y>				// Elbow point Y
 <Z>				// Elbow point Z
 <PN>					// Palm Normal
 <X>				// Palm Normal X
 <Y>				// Palm Normal Y
 <Z>				// Palm Normal Z
 <HM>					// Hand basis matrix
 <XX>				// Hand basis matrix, X component X
 <XY>				// Hand basis matrix, X component Y
 <XZ>				// Hand basis matrix, X component Z
 <YX>				// Hand basis matrix, Y component X
 <YY>				// Hand basis matrix, Y component Y
 <YZ>				// Hand basis matrix, Y component Z
 <ZX>				// Hand basis matrix, Z component X
 <ZY>				// Hand basis matrix, Z component Y
 <ZZ>				// Hand basis matrix, Z component Z
 <AM>					// Arm basis matrix
 <XX>				// Arm basis matrix, X component X
 <XY>				// Arm basis matrix, X component Y
 <XZ>				// Arm basis matrix, X component Z
 <YX>				// Arm basis matrix, Y component X
 <YY>				// Arm basis matrix, Y component Y
 <YZ>				// Arm basis matrix, Y component Z
 <ZX>				// Arm basis matrix, Z component X
 <ZY>				// Arm basis matrix, Z component Y
 <ZZ>				// Arm basis matrix, Z component Z
 */


void LeapRecorder::setup(){
    
    bRecording  = false;
	bRecordingThisFrame = false;
	
	lastTagNumber				= 0;
	recordingFrameCount			= 0;
	recordingStartTimeMillis	= 0;
	
}
//--------------------------------------------------------------
void LeapRecorder::startRecording(){
    XML.clear();
    lastTagNumber				= 0;
	recordingFrameCount			= 0;
	recordingStartTimeMillis	= 0;
    
    bRecording  = false;
	bRecordingThisFrame = false;
}

//--------------------------------------------------------------
void LeapRecorder::endRecording(string filePath){
    XML.saveFile(filePath);
}

//--------------------------------------------------------------
void LeapRecorder::recordFrameXML(ofxLeapMotion &leap){
	
	//if (leap.isFrameNew()){
    
    // Get the vector of Hands from ofxLeap.
    vector <Hand> hands = leap.getLeapHands();
    
    bRecordingThisFrame = true;
    lastTagNumber = XML.addTag("FRAME");
    if( XML.pushTag("FRAME", lastTagNumber) ){
        
        int frameTime = ofGetElapsedTimeMillis() - recordingStartTimeMillis;
        XML.setValue("COUNT", recordingFrameCount,lastTagNumber);
        XML.setValue("TIME", frameTime, lastTagNumber);
        XML.setValue("NH", (int)hands.size(), lastTagNumber);
        
        // For each hand, get the current hand
        if (hands.size() > 0) {
            for (int h=0; h<hands.size(); h++){
                
                Hand & hand = hands[h];
                recordHandXML (hand, h,leap);
            }
        }
        
        XML.popTag(); // pop FRAME
        bRecordingThisFrame = false;
        recordingFrameCount++;
    }
    
	//} else {
	//	bRecordingThisFrame = false;
	//}
	
	
}

//--------------------------------------------------------------
void LeapRecorder::recordHandXML (Hand & hand, int handIndex,ofxLeapMotion &leap){
	
	if (bRecordingThisFrame){
		if (hand.isValid()){
			
			int handTagNum = XML.addTag("H");
			if( XML.pushTag("H", handTagNum) ){
				XML.setValue("INDEX", handIndex, handTagNum);
				XML.setValue("ID", hand.id(), handTagNum);
				XML.setValue("TYPE", (hand.isLeft() ? "LEFT" : "RIGHT"), handTagNum);
				XML.setValue("CONF", hand.confidence(), handTagNum);
				
				FingerList fingers = hand.fingers();
				for (int f=0; f<fingers.count(); f++){
					const Finger & finger = fingers[f];
					
					if (finger.isValid()){
						recordFingerXML (finger,leap);
					}
				}
				
				recordArmXML (hand,leap);
				XML.popTag(); // pop H(AND)
			}
		}
	}
}

//--------------------------------------------------------------
void LeapRecorder::recordFingerXML (const Finger & finger,ofxLeapMotion &leap){
	
	if (bRecordingThisFrame){
		if (finger.isValid()){
			Finger::Type fingerType = finger.type();
			
			int fingerTagNum = XML.addTag("F");
			if( XML.pushTag("F", fingerTagNum) ){
				XML.setValue("TYPE", (int)fingerType, fingerTagNum);
				XML.setValue("WIDTH", finger.width(), fingerTagNum);
				
				// Add in the fingerTip point as its own point.
				ofPoint fingerTipPt = leap.getofPoint ( finger.tipPosition() );
				int tipTagNum = XML.addTag("T");
				XML.setValue("T:X", fingerTipPt.x, tipTagNum);
				XML.setValue("T:Y", fingerTipPt.y, tipTagNum);
				XML.setValue("T:Z", fingerTipPt.z, tipTagNum);
				
				// For every bone (i.e. phalange) in the finger,
				for (int b=0; b<4; b++) {
					
					// Get each bone;
					Bone::Type boneType = static_cast<Bone::Type>(b);
					Bone bone = finger.bone(boneType);
					if (bone.isValid()){
						
						// Don't consider zero-length bones, such as the Thumb's metacarpal.
						if (bone.length() > 0){
							recordBoneXML (finger, bone,leap);
							
						} // end if boneLength
					} // end if bone isValid()
				} // end for each bone
				
				
				XML.popTag(); // pop F(INGER)
			}
		} //end if finger isValid()
	}
}

//--------------------------------------------------------------
void LeapRecorder::recordBoneXML (const Finger & finger, Bone & bone,ofxLeapMotion &leap){
	
	if (bRecordingThisFrame){
		
		Finger::Type fingerType = finger.type();
		Bone::Type   boneType   = bone.type();
		
		// The Leap returns data in millimeters.
		ofPoint bonePt0 = leap.getofPoint ( bone.prevJoint());
		ofPoint bonePt1 = leap.getofPoint ( bone.nextJoint());
		float boneThickness = bone.width();
		
		int boneTagNum = XML.addTag("B");
		if( XML.pushTag("B", boneTagNum) ){
			XML.setValue("TYPE", (int)boneType, boneTagNum);
			
			if ((boneType == 0) || ((fingerType == 0) && (boneType == 1))) {
				// bone points are redundant, so only use the P point.
				// except for bone 0, which gets both Q and P.
				int p0TagNum = XML.addTag("Q");
				XML.setValue("Q:X", bonePt0.x, p0TagNum);
				XML.setValue("Q:Y", bonePt0.y, p0TagNum);
				XML.setValue("Q:Z", bonePt0.z, p0TagNum);
			}
			
			int p1TagNum = XML.addTag("P");
			XML.setValue("P:X", bonePt1.x, p1TagNum);
			XML.setValue("P:Y", bonePt1.y, p1TagNum);
			XML.setValue("P:Z", bonePt1.z, p1TagNum);
			
			XML.popTag(); // pop B(ONE)
		}
	}
}


//--------------------------------------------------------------
void LeapRecorder::recordArmXML (Hand & hand,ofxLeapMotion &leap){
	
	if (bRecordingThisFrame){
		Arm arm = hand.arm();
		if (arm.isValid()){
			
			float armWidth = arm.width();
			XML.setValue("AW", armWidth, lastTagNumber);
			
			ofPoint palmPt   = leap.getofPoint ( hand.palmPosition());
			ofPoint wristPt  = leap.getofPoint ( arm.wristPosition());
			ofPoint elbowPt  = leap.getofPoint ( arm.elbowPosition());
			ofPoint palmNorm = leap.getofPoint ( hand.palmNormal());
			
			int palmPtTagNum = XML.addTag("PM");
			XML.setValue("PM:X", palmPt.x, palmPtTagNum);
			XML.setValue("PM:Y", palmPt.y, palmPtTagNum);
			XML.setValue("PM:Z", palmPt.z, palmPtTagNum);
			
			int wristPtTagNum = XML.addTag("W");
			XML.setValue("W:X", wristPt.x, wristPtTagNum);
			XML.setValue("W:Y", wristPt.y, wristPtTagNum);
			XML.setValue("W:Z", wristPt.z, wristPtTagNum);
			
			int elbowPtTagNum = XML.addTag("E");
			XML.setValue("E:X", elbowPt.x, elbowPtTagNum);
			XML.setValue("E:Y", elbowPt.y, elbowPtTagNum);
			XML.setValue("E:Z", elbowPt.z, elbowPtTagNum);
			
			int palmNormTagNum = XML.addTag("PN");
			XML.setValue("PN:X", palmNorm.x, palmNormTagNum);
			XML.setValue("PN:Y", palmNorm.y, palmNormTagNum);
			XML.setValue("PN:Z", palmNorm.z, palmNormTagNum);
			
			
			//---------------
			// Export the hand basis matrix
			Leap::Matrix handMatrix = hand.basis();
			ofPoint handBasisX = leap.getofPoint( handMatrix.xBasis);
			ofPoint handBasisY = leap.getofPoint( handMatrix.yBasis);
			ofPoint handBasisZ = leap.getofPoint( handMatrix.zBasis);
			
			int handMatrixTagNum = XML.addTag("HM");
			if( XML.pushTag("HM", handMatrixTagNum) ){
				
				XML.setValue("XX", handBasisX.x, handMatrixTagNum);
				XML.setValue("XY", handBasisX.y, handMatrixTagNum);
				XML.setValue("XZ", handBasisX.z, handMatrixTagNum);
				
				XML.setValue("YX", handBasisY.x, handMatrixTagNum);
				XML.setValue("YY", handBasisY.y, handMatrixTagNum);
				XML.setValue("YZ", handBasisY.z, handMatrixTagNum);
				
				XML.setValue("ZX", handBasisZ.x, handMatrixTagNum);
				XML.setValue("ZY", handBasisZ.y, handMatrixTagNum);
				XML.setValue("ZZ", handBasisZ.z, handMatrixTagNum);
				
				XML.popTag(); // pop HM (Hand Matrix)
			}
			
			//---------------
			// Export the arm basis matrix
			Leap::Matrix armMatrix = arm.basis();
			ofPoint armBasisX = leap.getofPoint( armMatrix.xBasis);
			ofPoint armBasisY = leap.getofPoint( armMatrix.yBasis);
			ofPoint armBasisZ = leap.getofPoint( armMatrix.zBasis);
			
			int armMatrixTagNum = XML.addTag("AM");
			if( XML.pushTag("AM", armMatrixTagNum) ){
				
				XML.setValue("XX", armBasisX.x, armMatrixTagNum);
				XML.setValue("XY", armBasisX.y, armMatrixTagNum);
				XML.setValue("XZ", armBasisX.z, armMatrixTagNum);
				
				XML.setValue("YX", armBasisY.x, armMatrixTagNum);
				XML.setValue("YY", armBasisY.y, armMatrixTagNum);
				XML.setValue("YZ", armBasisY.z, armMatrixTagNum);
				
				XML.setValue("ZX", armBasisZ.x, armMatrixTagNum);
				XML.setValue("ZY", armBasisZ.y, armMatrixTagNum);
				XML.setValue("ZZ", armBasisZ.z, armMatrixTagNum);
				
				XML.popTag(); // pop AM (Arm Matrix)
			}
			
		}
	}
}
