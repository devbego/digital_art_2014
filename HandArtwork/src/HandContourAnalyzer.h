//
//  HandContourAnalyzer.h
//  leapAndCamHandSegmenter
//
//  Created by GL on 8/29/14.
//
//

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


using namespace ofxCv;
using namespace cv;

#define INVALID_CONTOUR_INDEX	-1
struct ContourRegion {
	int index_start;
	int index_end;
	int index_len; // length (in terms of number of points)
	int finger_id;
};


#define N_HANDMARKS 16
#define HANDMARK_HISTORY_LENGTH		24
#define DESIRED_N_CONTOUR_POINTS	800 
#define N_FINGER_LENGTH_SAMPLES		11
#define N_FINGER_WIDTH_SAMPLES		5

#define CONTOUR_WINDING_PINKY_TO_THUMB_CCW	 1
#define CONTOUR_WINDING_PINKY_TO_THUMB_CW	-1

enum HandmarkType {
	HANDMARK_INVALID			= -1,
	HANDMARK_PINKY_TIP			= 0,
	HANDMARK_PR_CROTCH			= 1,
	HANDMARK_RING_TIP			= 2,
	HANDMARK_RM_CROTCH			= 3,
	HANDMARK_MIDDLE_TIP			= 4,
	HANDMARK_MI_CROTCH			= 5,
	HANDMARK_POINTER_TIP		= 6,
	HANDMARK_POINTER_SIDE		= 7,
	HANDMARK_IT_CROTCH			= 8,
	HANDMARK_THUMB_TIP			= 9,
	HANDMARK_THUMB_KNUCKLE		= 10,
	HANDMARK_THUMB_BASE			= 11,
	HANDMARK_THUMBSIDE_WRIST	= 12,
	HANDMARK_PINKYSIDE_WRIST	= 13,
	HANDMARK_PALM_BASE			= 14,
	HANDMARK_PINKY_SIDE			= 15
};

struct Handmark { // analogous to landmark
	ofVec3f			point;			// the final actual location
	list<ofVec3f>	pointHistory;
	
	ofVec3f			pointAvg;		// its running average
	ofVec3f			pointStDv;		// its running standard deviation
	
	int				index;			// the final index in the theHandContourFinal
	HandmarkType	type;			// the name (or type) of the point
	float			confidence;		// a score for the Handmark's stability, 0...1
	bool			valid;			// did we find it in this frame?
};

struct SlopeInterceptLine {
	float			slope;
	float			yIntercept;
};

struct PerfectRay {
    float cx, cy;                   // the coordinates of the crotch (initially)
    float kx, ky;                   // the coordinates of a point near the knuckles
	float dx, dy;					// distances between coords
	float lineMag2;					// distance squared.
    int whichCrotch;                // the index of the crotch (0..3) to which this belongs.
	
	// the pixels in the thresholded edge image which belong to me.
	vector<ofVec3f> belongingPixels;
	
	float ax, ay;
	float bx, by;
	float fitSlope;
	float fitIntercept;
	vector<ofVec3f> newContourPoints;
};






class HandContourAnalyzer {
	
public:
	
	int imgW;
	int imgH;
	void setup (int w, int h);
	bool bWorkAtHalfScale;
	
	void update(const Mat &thresholdedImageOfHandMat, const Mat &leapDiagnosticFboMat, LeapVisualizer &lv);
	void draw();

	void computeLabeledContour  (const Mat &thresholdedImageOfHandMat,
								 const Mat &leapDiagnosticFboMat);
	void computeContourRegions  (const Mat &thresholdedImageOfHandMat,
									 const Mat &leapDiagnosticFboMat);
	void acquireProjectedLeapData (LeapVisualizer &lv);
	void acquireFingerOrientations (LeapVisualizer &lv, const Mat &leapDiagnosticFboMat);
	void computeCrotchOrientations ();
	
	
	void computeFingerCentroids();
	void computeNearestContourPointsToFingerCentroids();
	void computeContourDistancesFromKeyHandPoints();
	void computeFingerPointsMaximallyDistantFromKeyHandPoints();
	
	void computePinkySideHandmark();
	void computeIndexSideHandmark();
	void computePalmBaseHandmark();
	void computeThumbBaseHandmark();
	void computeWristHandmarks();
	void computeThumbKnuckleHandmark();
	void computeThumbTopKnuckleHandmark();
	void estimateFingerTipsFromCircleFitting (LeapVisualizer &lv);
	void estimateFingerTipsFromCircleFittingOLD (LeapVisualizer &lv);
	void computeFingerCrotches();
	void sortFingerCrotches();
	void evaluateCrotchQuality();
	void refineFingerTipsBasedOnCrotchQuality();
	void refineOtherHandmarks();
	void assembleHandmarksPreliminary(); 
	
	void drawOrientations();
	void drawCrotchCalculations(int index0, int index1);
	
	bool bContourExists;
	bool bCalculatedDistances;
	
	ofxCv::ContourFinder contourFinder;
	ofPolyline	theHandContourRaw;
	ofPolyline	theHandContourResampled;
	ofPolyline	theHandContourVerySmooth;
	ofPolyline	theHandContourMetaData;
	ofPolyline	crotchQualityData;
	ofPolyline	crotchQualityData2;
	ofPolyline	crotchQualityData3;
	
	ofPolyline  theHandContourRefined;
	
	int			theHandContourWindingDirection;
	int			smoothingOfLowpassContour;
	float		theHandContourArea;
	
	float		crotchContourSearchTukeyMaskPct;
	float		crotchCurvaturePowf;
	float		crotchDerivativePowf;
	int			crotchSearchRadius; 
	int			crotchSearchIndex0;
	int			crotchSearchIndex1;
	int			crotchContourIndices[4];
	int			crotchContourIndicesTmp[4];
	int			crotchContourIndicesExpanded[8];
	float		crotchQuality[4];
	vector<int> crotchContourIndicesDifs;
	
	int			contourIndexOfPalmBase;
	int			contourIndexOfPinkySide;
	int			contourIndexOfPinkysideWrist;
	int			contourIndexOfThumbsideWrist;
	int			contourIndexOfThumbBase; 
	int			contourIndexOfPointerSide;
	int			contourIndexOfThumbKnuckle; 
	
	int			contourIndexOfPRCrotch;
	int			contourIndexOfRMCrotch;
	int			contourIndexOfMICrotch;
	int			contourIndexOfITCrotch;
	
	int			contourIndexOfThumbTip;
	int			contourIndexOfPointerTip;
	int			contourIndexOfMiddleTip;
	int			contourIndexOfRingTip;
	int			contourIndexOfPinkyTip;
	
	vector<ofVec3f> boneCenters;
	vector<ofVec3f> knuckles;
	vector<ofVec3f> fingerCentroids;
	vector<ofVec3f> fingerDistalPts;
	vector<float>   fingerOrientations;
	vector<float>	crotchOrientations;
	float			fingerOrientationAverage; 
	
	vector<ofVec3f> contourPointsClosestToFingerCentroids;
	int				indicesOfClosestContourPoints[8];
	float			distancesOfClosestContourPoints[8];
	
	ofVec3f			wristPosition;
	ofVec3f			wristOrientation; 
	ofVec3f			handCentroidLeap;// the hand centroid reported by the Leap (not the contourFinder)
	
	
	SlopeInterceptLine pinkyKnuckleLine;
	SlopeInterceptLine indexKnuckleLine;
	
	vector<ContourRegion> allContourRegionsTEMP;
	vector<ContourRegion> contourRegionsConsolidated;
	
	
	void labelHandContourWithHandPartIDs();
	void drawLabeledHandContour();
	void recalculateContourRegions(); 
	
	void drawHandmarks();
	void drawFingertipParticleStartPositions(); 
	
	Handmark				Handmarks[N_HANDMARKS];
	vector<Handmark>		provisionalCrotchHandmarks;
	Handmark				HandmarksRefined[N_HANDMARKS];
	
	
	void buildCurvatureAnalysis (ofPolyline& polyline);
	float distanceFromPointToLine (ofVec3f linePt1, ofVec3f linePt2,  ofVec3f aPoint);
	SlopeInterceptLine computeFitLine (vector<ofVec3f> points, int startPointIndex, int endPointIndex);
	int getIndexOfClosestPointOnContour (ofVec3f &aPoint, ofPolyline &aPolyline);
	int getIndexWhereLineSegmentIntersectsPolyline (float x1, float y1, float x2, float y2, ofPolyline &aPolyline);
	int sampleOffsetForCurvatureAnalysis;
	vector<float> handContourCurvatures;
	
	
	ofVec2f intersection1;
	ofVec2f intersection2;
	bool bIntersectionExists1;
	bool bIntersectionExists2;
	float evaluateCircularFitness (float circleRadius, int atWhichContourIndex, int forWhichHandPartId, int ind0, int ind1);
	inline float function_PennerEaseOutCubic (float x);
	inline float function_PennerEaseOutQuartic (float t);
	inline float function_PennerEaseOutQuintic (float t);
	inline float function_PennerEaseInOutCubic (float x);
	inline float function_TukeyWindow (float x, float a);
	inline float function_DoubleExponentialSigmoid (float x, float a);
	
	
	inline void FindLineCircleIntersections (float cx, float cy, float radiusSquared,
											 float p1x, float p1y, float p2x, float p2y);
	
	
	float	getInsertionPercentageOfHand();
	float	getDistanceOfBlobFromEntry();
	bool	doesHandContourExist();
	int		getNumberOfBlobs();
	
    //----------------------------------
    // data and methods for CROTCH REFINEMENT.
    float   minCrotchQuality;
    float   malorientationSuppression;
    bool    refineCrotches(LeapVisualizer &lv,
                           const Mat &grayMat,
                           const Mat &thresholdedImageOfHandMat,
                           const Mat &leapDiagnosticFboMat);
    Mat     tempMat1;
    Mat     tempMat2;
    Mat     edgeMat;
	Mat		grayBlurMat; 
    Mat     thresholdConstMat;
    Mat     unthresholdedInvertedEdgeMat;
    Mat     blurredUnthInvEdgeMat;
    Mat     adaptiveThreshImg;
	Mat		pixelOwnershipMat;
	
	Mat morphStructuringElt;
	void drawHandmarksRefined() ;
    
    float blurKernelSize;
    float thresholdValue;
    float prevThresholdValue;
    float blurredStrengthWeight;
	float lineBelongingTolerance;
	float perpendicularSearch;
    bool  bWasRefinedInPreviousFrame;
    float handmarkBlur; 
    

};