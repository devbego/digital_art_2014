//
//  LeapRecorder.h
//  leapAndCamRecorder2
//
//  Created by chris on 28/07/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"
#include "LeapBone.h"
#include "LeapFinger.h"
#include "LeapHand.h"

class LeapRecorder{
    
    public:
    
    void setup();
    void update();
    
    void startRecording();
    void endRecording(string filePath);
    
    void recordFrameXML(ofxLeapMotion &leap);
    void recordHandXML (Hand & hand, int handIndex,ofxLeapMotion &leap);
    void recordFingerXML (const Finger & finger,ofxLeapMotion &leap);
    void recordBoneXML (const Finger & finger, Bone & bone,ofxLeapMotion &leap);
    void recordArmXML (Hand & hand,ofxLeapMotion &leap);
    
    
    bool bRecording;
	bool bRecordingThisFrame;
	int lastTagNumber;
	int recordingFrameCount;
	int recordingStartTimeMillis;
    
    ofxXmlSettings XML;


};