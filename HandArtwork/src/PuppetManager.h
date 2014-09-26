//
//  PuppetManager.h
//  HandArtwork
//
//  Created by GL on 9/26/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxXmlSettings.h"

//------------------------------
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



class PuppetManager {
    
public:
	
	
	
	//-------------------------------
	// PUPPETEER

	void setupPuppeteer (HandMeshBuilder &myHandMeshBuilder);
	void updatePuppeteer (bool bComputeAndDisplayPuppet, HandMeshBuilder &myHandMeshBuilder);
	void drawPuppet (bool bComputeAndDisplayPuppet,  ofTexture &handImageTexture );
	
	ofxPuppet puppet;
	void setSkeleton(Skeleton* skeleton);
	vector<Scene*> scenes;
	
	PalmSkeleton		palmSkeleton, immutablePalmSkeleton;
	HandSkeleton		handSkeleton, immutableHandSkeleton;
	ThreePointSkeleton	threePointSkeleton, immutableThreePointSkeleton;
	WristSpineSkeleton	wristSpineSkeleton, immutableWristSpineSkeleton;
	HandWithFingertipsSkeleton handWithFingertipsSkeleton, immutableHandWithFingertipsSkeleton;
	Skeleton*		previousSkeleton, *currentSkeleton;
	vector<string>	sceneNames, sceneWithSkeletonNames;
	
	ofxUICanvas*	puppetGui;
	ofxUICanvas**	puppetGuis;
	ofxUIRadio*		sceneRadio;
	
	void guiEvent(ofxUIEventArgs &e);

	void setupPuppetGui();
	bool showPuppetGuis;
	bool bShowPuppetTexture;
	bool bShowPuppetWireframe;
	bool bShowPuppetControlPoints;
	bool bShowPuppetSkeleton;
	bool bShowPuppetMeshPoints;
	bool bPuppetMouseControl;
	bool frameBasedAnimation;
	
	float elapsedPuppetMicros;
	int   elapsedPuppetMicrosInt;
	
};
