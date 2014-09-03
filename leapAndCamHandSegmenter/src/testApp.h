#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCV.h"
#include "ofxCv.h"
#include "ofxCvMin.h"
#include "ofxLibdc.h"
#include "LeapFrame.h"
#include "BufferedVideo.h"
#include "LeapRecorder.h"
#include "LeapVisualizer.h"
#include "FingerTipCalibRecorder.h"
#include "LeapToCameraCalibrator.h"
#include "HandContourAnalyzer.h"

/* 
 Made some sanity changes in ofxXmlSettings:
	const float floatPrecision = 4; // changed by GL
	fprintf( cfile, "\t" ); // instead of 4 spaces, GOLAN fprintf( cfile, "    " );
 */

#define _USE_CORRECTED_CAMERA
#define _USE_LIBDC_GRABBER
// uncomment this to use a libdc firewire camera standard of an OF video grabber


using namespace ofxCv;
using namespace cv;


/*
#define INVALID_CONTOUR_INDEX	-1
struct ContourRegion {
	int index_start;
	int index_end;
	int index_len; // length (in terms of number of points)
	int finger_id;
};
*/


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
	
	
	void updateBufferedVideoPlayback();
	void updateProcessFrameImg();
	
    //------------------------------
    // Gui
    void drawText();
    
    //------------------------------
	// For recording the CAMERA.
	void initializeCamera();
	ofxLibdc::PointGrey cameraLibdc;
	ofVideoGrabber cameraVidGrabber;
	
	
	int		cameraWidth;
	int		cameraHeight;
	int		imgW;
	int		imgH;
	int		drawW;
	int		drawH;
    
	
	ofImage currentFrameImg; // where we store the current frame we grabbed from the Camera
    ofImage processFrameImg; // where we store the processed frame for calibration/display

    int currentFrameNumber;
	vector<ofPixels> imageSequence;
	
	

	
	// Computer Vision
	Mat						leapDiagnosticFboMat;
	ofPixels				leapFboPixels;
	void					renderDiagnosticLeapFboAndExtractItsPixelData();
    
    //------------------------------
    // Leap
	ofxLeapMotion			leap;
    LeapVisualizer			leapVisualizer;
    LeapRecorder			leapRecorder;
	LeapRecorder			prevLeapFrameRecorder;
    LeapToCameraCalibrator	leapToCameraCalibrator;
	void updateLeapHistoryRecorder();
	
	ofEasyCam				cam;
    ofFbo					leapDiagnosticFbo;
	ofFbo					leapColorFbo;
	
    bool	bInPlaybackMode;
    bool	bRecording;
    bool	bRecordingForCalibration;
    bool	bUseVirtualProjector;
    bool	bUseFbo;
    bool	bInputMousePoints;
    bool	bShowCalibPoints;
    bool	bRecordThisCalibFrame;
    bool	bUseCorrectedCamera;
    bool	bShowLargeCamImageOnTop;
	bool	bUseRGBLeapFbo;
    bool	bShowText;
	bool	bUseVoronoiExpansion;
	bool	bShowOffBy1Frame;
	bool	bDoCompositeThresholdedImageWithLeapFboPixels;
	
    int		framesBackToPlay;
	int		playingFrame;
    string	folderName;
	

    
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
	void initializeCameraCalibration();
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
	

	
	
	
	//-------------------------------
	
	
	void setupGui();
	ofxUICanvas* gui;
	void guiEvent(ofxUIEventArgs &e);
	std::string originalAppDataPath;
	
	int	 whichImageToDraw;
	
	void updateComputerVision();
	void extractLuminanceChannelFromVideoFrame();
	void computeFrameDifferencing();
	void thresholdLuminanceImage();
	void applyMorphologicalOps();
	void applyEdgeAmplification();
	void compositeThresholdedImageWithLeapFboPixels();
	
	
	bool bWorkAtHalfScale;
	bool bUseROIForFilters;
	bool bUseRedChannelForLuminance;
	bool bDoMorphologicalOps;
	bool bDoAdaptiveThresholding;
	bool bComputePixelBasedFrameDifferencing;
	bool bDoLaplacianEdgeDetect; 
	
	ofxCvColorImage colorVideo;
	ofxCvColorImage colorVideoHalfScale;
	
	Mat	videoMat;
	// vector<Mat> rgbVideoChannelMats;
	
	Mat grayMat;
	Mat prevGrayMat;
	Mat diffGrayMat;
	Mat graySmall;
	Mat blurredSmall;
	Mat adaptiveThreshImg;	// blurred minus Constant; the per-pixel thresholds
	Mat tempGrayscaleMat1;
	Mat tempGrayscaleMat2;
	Mat coloredBinarizedImg;
	Mat handPartIDImg;		// grayscale image in which each pixel represents what part of the hand it represents
	Mat handPartIDTmpImg;	// just used to store the information for a single finger at a time.
	
	Mat thresholded;		// binarized hand, black-white only
	Mat edgesMat1;
	Mat	thresholdedFinal;	//
	Mat	thresholdedFinal8UC3;
	Mat thresholdedFinalThrice[3];
	Mat rgbVideoChannelMats[3];
	Mat leapDiagnosticFboChannelMats[3]; 
	Mat blurred;
	Mat thresholdConstMat;
	
	Mat morphStructuringElt;
	
	
	float blurKernelSize;
	float thresholdValue;
	float prevThresholdValue;
	float blurredStrengthWeight;
	
	int	  laplaceKSize;
	float laplaceDelta;
	float laplaceSensitivity;
	
	void  computeHandStatistics(); 
	float amountOfPixelMotion01;
	float amountOfLeapMotion01;
	float amountOfFingerCurl01;
	
	float motionAlpha;
	float zExtentAlpha;
	float zHandExtent;
	float fingerCurlAlpha;
	
	float elapsedMicros;
	long long  lastFrameTimeMicros;
	
	
	//-------------------------------
	HandContourAnalyzer myHandContourAnalyzer;
	
    
};
