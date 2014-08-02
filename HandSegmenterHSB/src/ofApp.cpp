#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

// This program assumes the hand is on a bluish-green surface.
// The program separates the hand using both chrominance and luminance.
// For a background which is (chromatically) different from skin,
// But absorbs NIR (so it doesn't disturb a LEAP controller), use:
// Martha Stewart MSL140 "Avocado Peel" paint with Glidden base, or
// Crescent 9538 "Deep Woods" artboard.

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetVerticalSync(false);

	inputImage.loadImage("test6.png");
	
	origW = inputImage.width;
	origH = inputImage.height;
	bWorkAtHalfScale = false;
	
	if (bWorkAtHalfScale){
		imgW = origW/2;
		imgH = origH/2;
		inputImage.resize(imgW, imgH);
	} else {
		imgW = inputImage.width;
		imgH = inputImage.height;
	}
	imgC = inputImage.bpp / 8;
	imgN = imgW * imgH * imgC;
	
	
	
	inputImagePixels.allocate(imgW, imgH, imgC);
	inputImageChars = new unsigned char [imgW * imgH * imgC];
	inputImageCvMat.create(imgH, imgW, CV_8UC3);
	// outputImage.allocate(imgW, imgH, OF_IMAGE_COLOR);
	
	//------------------------------
	// For calculation of hue separation
	inputImageCvMatHLS.create(imgH, imgW, CV_8UC3);
	hueMat.create(imgH, imgW, CV_8UC1);
	thresholdedHueMat.create(imgH, imgW, CV_8UC1);
	backgroundHue = 85; // initialize our background hue to this empirical estimate.
	hueThreshold = 0;
	hueCalcTimer = 10000;
	hueThresholdOffset = 0;
	bAlsoLuminanceThreshold = true;
	lumThresholdOffset = -20;
	
	
	redChannelMat.create(imgH, imgW, CV_8UC1);
	blurredLumMat.create(imgH, imgW, CV_8UC1);
	blurredInvLumMat.create(imgH, imgW, CV_8UC1);
	unsharpedMat.create(imgH, imgW, CV_8UC1);
	thresholdedUnsharpedMat.create(imgH, imgW, CV_8UC1);
	
	//------------------------------
	// data to store an NxN patch (for color sampling around the cursor)
	samplePatchSize = 15;
	samplePatchSize = forceOdd (samplePatchSize);
	inputImageRGBMatNxN.create (samplePatchSize,samplePatchSize, CV_8UC3);
	inputImageHLSMatNxN.create (samplePatchSize,samplePatchSize, CV_8UC3);
	

	
	
	hueThresholder = new tmVisThresholderC1 (imgW, imgH);
	lumThresholder = new tmVisThresholderC1 (imgW, imgH);
	
	update();
	getAverageHueFromPatch(100,100);
}

//--------------------------------------------------------------
void ofApp::update(){
	
	// inputImagePixels is a duplicate of the pixels data from inputImage.
	inputImagePixels.setFromPixels(inputImage.getPixels(), imgW, imgH, imgC);
		
	// these are the actual unsigned chars of inputImage; update() inputImage to see effects.
	inputImageChars = inputImage.getPixels();
	
	// this is an openCv matrix connected to the internal data of inputImage; update() inputImage to see effects.
	inputImageCvMat = toCv(inputImage);
	
	// Compute an automatically-threshold hue image
	computeThresholdedHueImage();
	
	// Compute a thresholded luminance image; 'And' it with the chromatic mask.
	computeLuminanceImage();
	
	
	inputImage.update();
	// outputImage.setFromPixels(inputImagePixels);// works.
	// outputImage.setFromPixels(inputImageChars, imgW, imgH, OF_IMAGE_COLOR);

}

//--------------------------------------------------------------
void ofApp::computeLuminanceImage(){
	
	if (bAlsoLuminanceThreshold){
	
		// Separate the image into 3 channels.
		// Use the red channel as a skin-relevant proxy for the luminance.
		vector<Mat> inputImageChannelMats;
		split( inputImageCvMat, inputImageChannelMats );
		redChannelMat = inputImageChannelMats[0];
		
		// If we can afford to do so, median-filter the luminance image.
		medianBlur(redChannelMat, redChannelMat, 3);
		
		// See: http://en.wikipedia.org/wiki/Unsharp_masking
		// See: http://opencv-code.com/quick-tips/sharpen-image-with-unsharp-mask/
		// unsharp mask the luminance image
		int kernelSize = 19;
		double blurSigma = 19;
		GaussianBlur(redChannelMat, blurredLumMat, cv::Size(kernelSize,kernelSize), blurSigma, blurSigma, BORDER_DEFAULT);
		addWeighted(redChannelMat, 1.5, blurredLumMat, -0.5, 0, unsharpedMat);
		
		// Automatically threshold the unsharpedMat.
		int lumThresholdControlFlags = 0;
		ThresholdMethod lumThresholdMethod = THRESHOLD_METHOD_MAXIMUM_ENTROPY;
		unsigned char *unsharpedMatChars = unsharpedMat.ptr();
		unsigned char *thresholdedUnsharpedMatChars = thresholdedUnsharpedMat.ptr();
		lumThresholder->threshold (unsharpedMatChars, thresholdedUnsharpedMatChars, lumThresholdMethod, lumThresholdControlFlags, lumThresholdOffset);
		
		
		unsigned char *threshHueMatChars = thresholdedHueMat.ptr();
		for (int i=0; i<(imgW * imgH); i++){
			thresholdedUnsharpedMatChars[i] = thresholdedUnsharpedMatChars[i] & threshHueMatChars[i];
		}
		
	} else {
		
		unsigned char *thresholdedUnsharpedMatChars = thresholdedUnsharpedMat.ptr();
		unsigned char *threshHueMatChars = thresholdedHueMat.ptr();
		
		for (int i=0; i<(imgW * imgH); i++){
			thresholdedUnsharpedMatChars[i] = threshHueMatChars[i];
		}
		
	}
	
	
}


//--------------------------------------------------------------
void ofApp::computeThresholdedHueImage(){
	
	// Compute HLS version of the inputImage, for chromatic separation
	// Note: Converting RGB to HLS for 1024x768 takes 15.6 milliseconds.
	cv::cvtColor(inputImageCvMat, inputImageCvMatHLS, CV_RGB2HLS);
	unsigned char *hlsChars = inputImageCvMatHLS.ptr();
	unsigned char *hueMatChars = hueMat.ptr();
	//
	// double then = ofGetElapsedTimeMicros();
	// double now = ofGetElapsedTimeMicros();
	// hueCalcTimer = 0.98*hueCalcTimer + 0.02*(now-then);
	// printf("Elapsed converting RGB/HLS = %d\n", (int)hueCalcTimer);
	
	
	// Compute the abs-difference between the hue and backgroundHue
	for (int y=0; y<imgH; y++){
		int row = y*imgW;
		for (int x=0; x<imgW; x++){
			int indexC1 = row+x;
			int indexC3 = indexC1*3;
			int H    = hlsChars[indexC3]; // 0..255
			int L    = hlsChars[indexC3+1];
			int val  = min ( abs(backgroundHue - H),  abs((backgroundHue+255) - H));
			val = (L == 0) ? 0 : val;
			hueMatChars[indexC1] = val;
		}
	}
	
	// If we can afford to do so, median-filter the hue diff image.
	medianBlur(hueMat, hueMat, 3);
	
	// Automatically threshold the hue diff image.
	int hueThresholdControlFlags = 0;
	ThresholdMethod hueThresholdMethod = THRESHOLD_METHOD_OTSU;
	unsigned char *thresholdedHueMatChars = thresholdedHueMat.ptr();
	hueThresholder->threshold (hueMatChars, thresholdedHueMatChars, hueThresholdMethod, hueThresholdControlFlags, hueThresholdOffset);
	hueThreshold = hueThresholder->theThreshold;
	
	
}



//--------------------------------------------------------------
void ofApp::draw(){
	
	
	ofSetColor(255);
	drawMat(inputImageCvMat, 0,0);
	
	ofEnableAlphaBlending();
	ofSetColor(255,255,255, 128);
	drawMat(thresholdedUnsharpedMat, 0,0);//, imgW, imgH); //redChannelMat  thresholdedHueMat
 
	
	
	
	ofSetColor(255,255,0);
	ofDrawBitmapString ( "FPS: " + ofToString(ofGetFrameRate()), 20, 15);
	ofDrawBitmapString ( "Hue: " + ofToString(backgroundHue), 20, 30);
	ofDrawBitmapString ( "Thr: " + ofToString(hueThreshold), 20, 45);
	ofDrawBitmapString ( "dHue: " + ofToString(hueThresholdOffset), 20, 60);
	ofDrawBitmapString ( "dLum: " + ofToString(lumThresholdOffset), 20, 75);
	
	
	
	
	ofSetColor(255);
	drawMat(inputImageRGBMatNxN, 20,80, samplePatchSize*5, samplePatchSize*5);
	
	
	hueThresholder->renderHistogram (20,170+64,256,-64, false);
	lumThresholder->renderHistogram (20,170+64+20+64,256,-64, false);
}





//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == '-'){
		hueThresholdOffset --;
	}
	if (key == '='){
		hueThresholdOffset ++;
	}
	
	if (key == '['){
		lumThresholdOffset --;
	}
	if (key == ']'){
		lumThresholdOffset ++;
	}
	

	if (key == 'l'){
		bAlsoLuminanceThreshold = !bAlsoLuminanceThreshold;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}



//--------------------------------------------------------------
void ofApp::getAverageHueFromPatch (int x, int y){

	// Extract a NxN patch of color from around the mouse,
	// from the inputImageCvMat into inputImageRGBMatNxN.
	// See http://stackoverflow.com/questions/7041181/equivalent-to-cvsetimageroi-in-the-opencv-c-interface
	int halfPatch = (samplePatchSize / 2);
	int roiL = (int) ofClamp( x - halfPatch, 0, imgW-samplePatchSize-1);
	int roiT = (int) ofClamp( y - halfPatch, 0, imgH-samplePatchSize-1);
	cv::Rect patchROI (roiL, roiT, samplePatchSize, samplePatchSize); // Make a cv::Rectangle
	Mat inputImageSubsection = inputImageCvMat (patchROI); //Point a cv::Mat header at it (no allocation is done)
	copy (inputImageSubsection, inputImageRGBMatNxN);
	
	// Heavily blur the RGB patch, in place, to produce an averaged color.
	bool bDoBlurRGBPatch = true;
	if (bDoBlurRGBPatch){
		int medianSize = 5;
		int blurSize = forceOdd(samplePatchSize/2);
		cv::medianBlur(inputImageRGBMatNxN, inputImageRGBMatNxN, medianSize);
		cv::blur(inputImageRGBMatNxN, inputImageRGBMatNxN, cv::Size(blurSize, blurSize));
	}
	
	// Compute HLS version of the NxN RGB patch
	cv::cvtColor(inputImageRGBMatNxN, inputImageHLSMatNxN, CV_RGB2HLS);
	unsigned char *hlsChars = inputImageHLSMatNxN.ptr();
	
	// Extract the hue in the center of the NxN patch
	int hueIndex = ((samplePatchSize/2)*samplePatchSize + (samplePatchSize/2))*3;
	backgroundHue = hlsChars[hueIndex];
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	getAverageHueFromPatch(x,y);
}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	getAverageHueFromPatch(x,y);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
