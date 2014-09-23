//
//  leapToCameraCalibrator.h
//  leapAndCamRecorder2
//
//  Created by chris on 28/07/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxRay.h"
#include "ofxCvMin.h"
#include "ofxCv.h"

class LeapToCameraCalibrator{
    
    public:
    
    void setup(int camWidth, int camHeight);
    void loadFingerTipPoints(string filePath);
	
    
    void resetProjector();
    void correctCamera();
    void correctCameraPNP(ofxCv::Calibration & myCalibration);
    void setIntrinsics(cv::Mat cameraMatrix);
    void setExtrinsics(cv::Mat rotation, cv::Mat translation);
    
    void drawWorldPoints();
    void drawImagePoints();

    
    
    
    ofVec2f resolution;
    bool calibrated;
    bool hasFingerCalibPoints;
    bool switchYandZ;

    float throwRatioX, throwRatioY;
    float lensOffsetX, lensOffsetY;
    float translationX, translationY, translationZ;
    float rotationX, rotationY, rotationZ;
    float throwRatio;
    ofVec2f lensOffset;
    
    vector<ofVec2f> calibVectorImage;
    vector<ofVec3f> calibVectorWorld;
    cv::Mat camera, distortion;
    cv::Mat rotation, translation;
    ofProjector projector;
    string dirNameLoaded;
    
    

};