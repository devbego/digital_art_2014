#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"
#include "ofxCv.h"
#include "ofxCvMin.h"
#include "ofxLibdc.h"
#include "LeapFrame.h"
#include "BufferedVideo.h"
#include "LeapRecorder.h"
#include "LeapVisualizer.h"
#include "FingerTipCalibRecorder.h"
#include "LeapToCameraCalibrator.h"

/* 
 Made some sanity changes in ofxXmlSettings:
	const float floatPrecision = 4; // changed by GL
	fprintf( cfile, "\t" ); // instead of 4 spaces, GOLAN fprintf( cfile, "    " );
 */

#define _USE_CORRECTED_CAMERA
#define _USE_LIBDC_GRABBER

// uncomment this to use a libdc firewire camera standard of an OF video grabber


class testApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();
	
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
	
    //------------------------------
    // Gui
    void drawText();
    
    //------------------------------
	// For recording the CAMERA.
	ofxLibdc::PointGrey cameraLibdc;
	ofVideoGrabber cameraVidGrabber;
    int cameraWidth,cameraHeight;
	
	ofImage currentFrameImg; // where we store the current frame we grabbed from the Camera
    ofImage processFrameImg; // where we store the processed frame for calibration/display

    int currentFrameNumber;
	vector<ofPixels> imageSequence;
	
    
    //------------------------------
    // Leap
	ofxLeapMotion leap;
    LeapVisualizer leapVisualizer;
    LeapRecorder   leapRecorder;
    LeapRecorder   prevLeapFrameRecorder;

    LeapToCameraCalibrator leapCameraCalibrator;
    
	ofEasyCam cam;
    ofFbo fbo;

    bool bPlaying;
    bool bRecording;
    bool bRecordingForCalibration;
    bool bUseVirtualProjector;
    bool bUseFbo;
    bool bInputMousePoints;
    bool bShowCalibPoints;
    bool bRecordThisCalibFrame;
    bool bUseCorrectedCamera;
    bool bShowLargeCamImageOnTop;
    bool bShowText;
    bool bShowOffBy1Frame;
    int framesBackToPlay;
    
	int  playingFrame;
    string folderName;
	
    float drawW, drawH;
    
    //------------------------------
    // Video buffer playback
    BufferedVideo video;
    bool playing;
	bool bEndRecording;
    
	//------------------------------
    // Calibration recording
    FingerTipCalibRecorder indexRecorder;
    ofPoint lastIndexVideoPos;
    ofPoint lastIndexLeapPos;
    
    //------------------------------
    // get camera calibration pre-calculated
    ofxCv::Calibration myCalibration;
    
    //------------------------------
    void finishRecording();
    void calibrateFromXML( string folderName);
    void loadAndPlayRecording(string folderName);
    void loadPlaybackFromDialog();
    void loadCalibrationFromDialog();
    void loadPlaybackFromDialogForCalibration();
    
    void drawLiveForRecording();
    void drawPlayback();
    void drawLeapWorld();
    
    bool useCorrectedCam();
    
};
