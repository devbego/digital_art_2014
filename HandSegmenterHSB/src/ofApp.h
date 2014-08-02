#pragma once

#include "ofMain.h"
#include "ofxCv.h"


#include "ipp.h"
#include "tmVisThresholderC1.h"

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
	
	
	int				origW;
	int				origH;
	bool			bWorkAtHalfScale;
	
	int				imgW;
	int				imgH;
	int				imgC; // number of channels
	int				imgN; // number of bytes total
	
	ofImage			inputImage;
	ofPixels		inputImagePixels;
	ofImage			inputImageHalfScale;
	cv::Mat			inputImageCvMat;
	unsigned char	*inputImageChars;
	
	bool bAlsoLuminanceThreshold;
	void computeLuminanceImage();
	int  lumThresholdOffset;
	
	void computeThresholdedHueImage();
	int				backgroundHue; // 0..255
	int				hueThreshold;
	int				hueThresholdOffset;
	cv::Mat			inputImageCvMatHLS;
	cv::Mat			hueMat;
	cv::Mat			thresholdedHueMat;
	float			hueCalcTimer;
	
	cv::Mat			redChannelMat;
	cv::Mat			blurredLumMat;
	cv::Mat			blurredInvLumMat;
	cv::Mat			unsharpedMat;
	cv::Mat			thresholdedUnsharpedMat;

	
	void			getAverageHueFromPatch (int x, int y);
	int				samplePatchSize;
	cv::Mat			inputImageRGBMatNxN;
	cv::Mat			inputImageHLSMatNxN;
	

	tmVisThresholderC1	*hueThresholder;
	tmVisThresholderC1	*lumThresholder;
	
		
};
