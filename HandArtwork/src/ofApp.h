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
#include "HandMeshBuilder.h"

//------------------------------
#include "ofxPuppetInteractive.h"
#include "NoneScene.h"
#include "WaveScene.h"
#include "WiggleScene.h"
#include "WobbleScene.h"
#include "EqualizeScene.h"
#include "NorthScene.h"
#include "LissajousScene.h"
#include "MeanderScene.h"
#include "PropogatingWiggleScene.h"
#include "SinusoidalLengthScene.h"
#include "PulsatingPalmScene.h"
#include "RetractingFingersScene.h"
#include "SinusoidalWiggleScene.h"
#include "MiddleDifferentLengthScene.h"
#include "GrowingMiddleFingerScene.h"
#include "StartrekScene.h"
#include "StraightenFingersScene.h"
#include "SplayFingersScene.h"
#include "TwitchScene.h"
#include "PinkyPuppeteerScene.h"
#include "FingerLengthPuppeteerScene.h"

#include "HandSkeleton.h"
#include "ThreePointSkeleton.h"
#include "HandWithFingertipsSkeleton.h"
#include "PalmSkeleton.h"
#include "WristSpineSkeleton.h"


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



enum ApplicationFault {
	FAULT_NOTHING_WRONG				= 0,	/* Everything appears to be working normally */
	FAULT_NO_USER_PRESENT_BRIEF		= 1,	/* Hand may have been momentarily withdrawn, not present for ~1 second */
	FAULT_NO_USER_PRESENT_LONG		= 2,	/* Nobody is present in the camera nor leap, for more than ~10 seconds */
	FAULT_LEAP_DROPPED_FRAME		= 4,	/* There was a hand in the camera (as recently as a second ago), but there's a leap dropout */
	FAULT_NO_LEAP_HAND_TOO_SMALL	= 8,	/* There's a handlike object in the camera, but it may be too small for the leap to work */
	FAULT_NO_LEAP_OBJECT_PRESENT	= 16,	/* Some bastard put something in the box */
	FAULT_TOO_MANY_HANDS			= 32,	/* There's more than one hand in view */
	FAULT_HAND_TOO_FAST				= 64,	/* The hand is moving too quickly */
	FAULT_HAND_TOO_HIGH				= 128,	/* The hand is physically too close to the cameras */
	FAULT_HAND_TOO_CURLED			= 256,	/* The hand is a fist or curled up, or has a curled finger */
	FAULT_HAND_TOO_VERTICAL			= 512	/* The hand is turned away from the camera */
};



class ofApp : public ofBaseApp{

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
	
	bool bCameraHasNewFrame;
	void updateBufferedVideoPlaybackIfUsingStoredVideo();
	void updateLiveVideoIfUsingCameraSource();
	void updateProcessFrameImg();
	void updateRecordingSystem();
	
    //------------------------------
    // Management
    void drawText();
	void applicationStateMachine();
    
    //------------------------------
	// For recording the CAMERA.
	void initializeCamera();
	ofxLibdc::PointGrey cameraLibdc;
	ofVideoGrabber cameraVidGrabber;
	float cameraLibdcShutterInv;
	float cameraLibdcBrightness;
	float cameraLibdcGain;
	float cameraLibdcGamma;
	
	
	
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
	void	updateLeapHistoryRecorder();
	int		maxNPrevLeapFrames;
	
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
	bool	bShowOffsetByNFrames;
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
	ofxUITabBar *guiTabBar;
    vector<ofxUICanvas *> guis;
	/* ofxUICanvas* gui; */
	void guiEvent(ofxUIEventArgs &e);
	std::string originalAppDataPath;
	
	
	
	
	int	 whichImageToDraw;
	
	void updateComputerVision();
	void extractVideoMatFromLiveVideo();
	void extractVideoMatFromBufferedVideoFrame();
	void extractLuminanceChannelFromSourceVideoMat();
	void computeFrameDifferencing();
	void thresholdLuminanceImage();
	void applyMorphologicalOps();
	void applyEdgeAmplification();
	void compositeThresholdedImageWithLeapFboPixels();
	
	bool bShowAnalysisBig;
	bool bWorkAtHalfScale;
	bool bUseROIForFilters;
	bool bUseRedChannelForLuminance;
	bool bDoMorphologicalOps;
	bool bDoAdaptiveThresholding;
	bool bComputePixelBasedFrameDifferencing;
	bool bDoLaplacianEdgeDetect;
	bool bDrawContourAnalyzer;
	
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
	char amountOfLeapMotion01Str[64];
	
	float motionAlpha;
	float zExtentAlpha;
	float zHandExtent;
	float zHandHeight; 
	float fingerCurlAlpha;
	
	float elapsedMicros;
	int	  elapsedMicrosInt;
	long long  lastFrameTimeMicros;
	
	
	//-------------------------------
	HandContourAnalyzer myHandContourAnalyzer;
	
	
	//-------------------------------
	// For app state machine
	float minHandInsertionPercent;
	float maxAllowableMotion;
	float maxAllowableFingerCurl;
	float maxAllowableExtentZ;
	
    //-------------------------------
	// MESH BUILDER!
	HandMeshBuilder myHandMeshBuilder;
	void updateHandMesh();
	
	//-------------------------------
	// PUPPETEER!
	
	
};
