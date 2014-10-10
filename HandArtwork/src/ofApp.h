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
#include "appFaultManager.h"

//------------------------------
#include "PuppetManager.h"
#include "TopologyModifierManager.h"

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
#include "SplayFingers2Scene.h"
#include "SpringFingerScene.h"
#include "TwitchScene.h"
#include "PinkyPuppeteerScene.h"
#include "FingerLengthPuppeteerScene.h"

#include "HandSkeleton.h"
#include "ThreePointSkeleton.h"
#include "HandWithFingertipsSkeleton.h"
#include "PalmSkeleton.h"
#include "WristSpineSkeleton.h"
#include "AppFaultManager.h"


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
    void drawText2();
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
	ofImage backgroundImage; // an image to show behind the puppet, instead of just ofBackground(0).

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
	bool	bUseRGBLeapFbo;
    bool	bShowText;
    bool    bDrawMiniImages;
    bool    bDrawSmallCameraView;
	bool	bShowOffsetByNFrames;
	bool	bDoCompositeThresholdedImageWithLeapFboPixels;
	bool	bComputeAndDisplayPuppet;
	bool	bFullscreen;
	bool	bUseBothTypesOfScenes;
	
    int		framesBackToPlay;
	int		playingFrame;
    float   backgroundGray;
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
    void drawCrosshairMouseCursor();
    void drawDiagnosticMiniImages();
    void drawContourAnalyzer();
    void drawMeshBuilderWireframe();
    void drawGradientOverlay();
    void drawIdleContour();
    void huntForBlendFunc (int period, int defaultSid, int defaultDid);
    bool useCorrectedCam();
	

	
	
	
	//-------------------------------
	
	
	void setupGui();
	ofxUITabBar *guiTabBar;
    ofxUIRadio  *contourAnalyzerUnderlayRadio;
    vector<ofxUICanvas *> guis;
	/* ofxUICanvas* gui; */
	void guiEvent(ofxUIEventArgs &e);
	std::string originalAppDataPath;
	
	
	void updateComputerVision();
	void compositeLeapArmIntoThresholdedFinal(); 
	void extractVideoMatFromLiveVideo();
	void extractVideoMatFromBufferedVideoFrame();
	void extractLuminanceChannelFromSourceVideoMat();
	void computeThresholdFromSkinColor();
	void computeFrameDifferencing();
	void thresholdLuminanceImage();
	void applyMorphologicalOps();
	void applyEdgeAmplification();
	void compositeThresholdedImageWithLeapFboPixels();
	
	
	
	bool bWorkAtHalfScale;
	bool bUseRedChannelForLuminance;
	bool bDoMorphologicalOps;
	bool bUseGradientThreshold;
	bool bDoAdaptiveThresholding;
	bool bComputePixelBasedFrameDifferencing;
	bool bDrawContourAnalyzer;
	bool bShowContourAnalyzerBig;
    bool bDrawMeshBuilderWireframe;
    bool bDrawLeapWorld;
    bool bDrawAppFaultDebugText;
	bool bDrawImageInBackground;
    bool bDrawGradient;
    bool bKioskMode;
    bool bInIdleMode;
    
	ofxCvColorImage colorVideo;
	ofxCvColorImage colorVideoHalfScale;
	
	Mat	videoMat;
	// vector<Mat> rgbVideoChannelMats;
	
	Mat grayMat;
	Mat skinColorPatch;
	Mat prevGrayMat;
	Mat diffGrayMat;
	Mat graySmall;
	Mat blurredSmall;
	Mat adaptiveThreshImg;	// blurred minus Constant; the per-pixel thresholds
	Mat tempGrayscaleMat1;
	Mat tempGrayscaleMat2;
	Mat coloredBinarizedImg;
    Mat maskedCamVidImg;    // The camera or video RGB image, masked to remove the background,
	Mat gradientThreshImg;
	Mat leapArmPixelsOnlyMat;
	
	
	Mat thresholded;		// binarized hand, black-white only
	Mat	thresholdedFinal;	//
	Mat	thresholdedFinal8UC3;
	Mat thresholdedFinalThrice[3];
	Mat rgbVideoChannelMats[3];
	Mat leapFboChannelMats[3];
	Mat blurred;
	Mat thresholdConstMat;
	
	Mat morphStructuringElt;
	
	
	float blurKernelSize;
	int		thresholdValue;
	int		thresholdValueDelta;
	int		prevThresholdValue;
	int		prevThresholdValueDelta;
	float blurredStrengthWeight;
	
	int	  skinColorPatchSize;
	float averageSkinLuminance;
	
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
	
	
	//-------------------------------
	HandContourAnalyzer myHandContourAnalyzer;
	void computeGradientThreshImg();
	float gradientThreshPow;
	float prevGradientThreshPow;
	
	//-------------------------------
	// For app state machine
    AppFaultManager myAppFaultManager;
	float minHandInsertionPercent;
	float maxAllowableMotion;
	float maxAllowableFingerCurl;
	float maxAllowableExtentZ;
    float maxAllowableHeightZ;
	
    AppFaultManager appFaultManager;
    float prevTime;
    
    //-------------------------------
	// MESH BUILDER!
	bool bSuccessfullyBuiltMesh;
	HandMeshBuilder myHandMeshBuilder;
	bool updateHandMesh();
    
	//-------------------------------
	// PUPPETEER!
	PuppetManager myPuppetManager;
    float puppetDisplayScale;
	
	TopologyModifierManager myTopologyModifierManager;
    bool useTopologyModifierManager;
    
    // ------------------------------
    // swipe scenes
    float swipeStart;
	int currentSceneID;
	
};
