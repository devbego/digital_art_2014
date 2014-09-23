//
//  graveyard.cpp
//  leapAndCamHandSegmenter
//
//  Created by GL on 8/13/14.


//--------------------------------------------------------------
bool bDoBlurOrientationChannels = false;
if (bDoBlurOrientationChannels) {
	// This may be leaky....
	// Could this be accomplished in a shader? YES.
	
	// Acquire diagnostic (orientation) data channels
	split (leapDiagnosticFboMat, leapDiagnosticFboChannelMats);
	cv::Mat leapDiagnosticFboMatR = leapDiagnosticFboChannelMats[0];
	cv::Mat leapDiagnosticFboMatG = leapDiagnosticFboChannelMats[1];
	cv::Mat leapDiagnosticFboMatB = leapDiagnosticFboChannelMats[2];
	
	int kernelSize = 5;
	cv::Size blurSize = cv::Size(kernelSize, kernelSize);
	blur (leapDiagnosticFboMatR, leapDiagnosticFboMatR, kernelSize);
	blur (leapDiagnosticFboMatG, leapDiagnosticFboMatG, kernelSize);
	
	cv::Mat newChans[] = {leapDiagnosticFboMatR, leapDiagnosticFboMatG, leapDiagnosticFboMatB};
	cv::merge(newChans, 3, leapDiagnosticFboMat);
}

//--------------------------------------------------------------
void testApp::spreadDiagnosticInformationByDilation(){
	
	// It turns out that erosion/dilation is not really the right way to spread the data,
	// because these operations favor high or low values.
	
	// Acquire diagnostic (orientation) data channels
	split(leapDiagnosticFboMat, leapDiagnosticFboChannelMats);
	cv::Mat leapDiagnosticFboMatR = leapDiagnosticFboChannelMats[0];
	cv::Mat leapDiagnosticFboMatG = leapDiagnosticFboChannelMats[1];
	cv::Mat leapDiagnosticFboMatB = leapDiagnosticFboChannelMats[2];
	
	// Dilate orientation chanels
	int morphS = 7;
	int morph_type = cv::MORPH_RECT;
	Mat morphElement = getStructuringElement(morph_type, cv::Size(2*morphS+1, 2*morphS+1), cv::Point(morphS,morphS ));
	cv::dilate( leapDiagnosticFboMatR, leapDiagnosticFboMatRD, morphElement );
	cv::dilate( leapDiagnosticFboMatG, leapDiagnosticFboMatGD, morphElement );
	
	// Re-assemble dilated channels
	// cv::Mat newChans[] = {leapDiagnosticFboMatR, leapDiagnosticFboMatG, leapDiagnosticFboMatB};
	// cv::merge(newChans, 3, leapDiagnosticFboMat);
	
	for (int y=0; y<imgH; y++){
		for (int x=0; x<imgW; x++){
			int indexC1 = y*imgW + x;
			int indexC3 = indexC1*3;
			
			unsigned char B = leapDiagnosticFboMatB.data[indexC1];
			unsigned char R = (B > 0) ? leapDiagnosticFboMatR.data[indexC1] : leapDiagnosticFboMatRD.data[indexC1];
			unsigned char G = (B > 0) ? leapDiagnosticFboMatG.data[indexC1] : leapDiagnosticFboMatGD.data[indexC1];
			
			leapDiagnosticFboMat.data[indexC3  ] = R;
			leapDiagnosticFboMat.data[indexC3+1] = G;
			leapDiagnosticFboMat.data[indexC3+2] = B;
		}
	}
}