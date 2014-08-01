#pragma once

#include "ofMain.h"

#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxUI.h"

#include "BufferedVideo.h"
#include "tmVisThresholderC1.h"

using namespace ofxCv;
using namespace cv;

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
	//----------------------------------
	// GUI
	void setupGui();
	ofxUICanvas* gui;
	void exit();
	void guiEvent(ofxUIEventArgs &e);
	std::string originalAppDataPath;
	

	
	BufferedVideo video;
	bool bVideoPlaying;
	
	ofImage maskImage;
	Mat		maskMat;
	
	bool	bSecondToggler;
	bool	bUseRedChannelForLuminance;
	bool	bApplyMedianToLuminance;
	bool	bComputeUnsharpedLuminance;
	bool	bUseROIForFilters;
	bool	bLoadedMaskImage;
	
	
	
	cv::Rect maskROI; // a region actively within the saved mask image.
	cv::Rect maskROIQuarter; // all dimensions /4.
	cv::Rect handROI; // a dynamic ROI computed from the previous frame's hand, with margins.
	
	void computeMaskResources();
	void extractLuminanceChannelFromVideoFrame();
	void applyMaskToLuminanceImage(); 
	void applyMedianFilterToLuminanceImage();
	void applyMorphologicalOpsToLuminanceImage();
	void applyEdgeAmplification();
	void computeUnsharpedLuminanceImage();
	void doAdaptiveThresholding(); 
	
	
	Mat	videoMat;			// cv::Mat version of the current video frame. 
	Mat grayMat;			// grayscale version of hand input
	Mat blurredGrayMat;		// blurred version of grayMat.
	Mat unsharpedMat;		// unsharp-masked version of grayMat.
	
	Mat thresholded;		// binarized hand, black-white only
	Mat	thresholdedFinal;	//
	Mat edgeDetected;		// edges detected by laplacian
	Mat goodEdgesImg;
	Mat graySmall;
	Mat blurredSmall;
	Mat edgesSmall;
	Mat blurred;
	Mat thresholdConstMat;
	Mat adaptiveThreshImg;	// blurred minus Constant; the per-pixel thresholds.
	
	Mat tempGrayscaleMat1;
	Mat tempGrayscaleMat2;
	ofxCvGrayscaleImage tempGrayscaleImg;
	
	//-----------------------------------
	// for edge amplification:
	ContourFinder edgeContourFinder;
	ContourFinder contourFinder;
	
	float thresholdValue;
	float prevThresholdValue;
	float blurKernelSize;
	float blurredStrengthWeight;
	float minLaplaceEdgeStrength;
	int	  luminanceMedianSize;
	int	  edgeContourMinArea;
	
	int	  laplaceKSize;
	float laplaceDelta;
	float laplaceSensitivity;
	
	float minAllowableContourAreaAsAPercentOfImageSize;
	float maxAllowableContourAreaAsAPercentOfImageSize;
	
	bool		bValidHandContourExists;
	ofPolyline	handContourPolyline;
	cv::Point2f	handContourCentroid;
	
	bool bUseMaskImage;
	bool bDoAdaptiveThresholding;
	bool bDoLaplacianEdgeDetect;
	bool bDoCannyEdgeDetect;
	bool bDoMorphologicalOps;
	bool bHandyBool;
	
	int  whichImageToDraw;
	bool bDrawColorVideo;
	bool bDrawFullScale;
	bool bDrawDiagnostics;
	float elapsedMicros;
	std::string elapsedMicrosString;
	
	int imgW; // width of our images for computer vision
	int imgH; // height of our images
	
	float elapsed; 
	
	tmVisThresholderC1*	thresholder;
	
	void huntForBlendFunc(int period, int defaultSid, int defaultDid);
};
