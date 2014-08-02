#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxUI.h"
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
#include "HandWithFingertipsSkeleton.h"
#include "PalmSkeleton.h"
#include "WristSpineSkeleton.h"

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
    
        void setSkeleton(Skeleton* skeleton);
        void setupGui();

        //void setupGui();
		vector<Scene*> scenes;
    
        ofMesh mesh;
        ofImage hand;
        ofxPuppet puppet;
    
        HandSkeleton handSkeleton, immutableHandSkeleton;
        HandWithFingertipsSkeleton handWithFingertipsSkeleton, immutableHandWithFingertipsSkeleton;
        PalmSkeleton palmSkeleton, immutablePalmSkeleton;
        WristSpineSkeleton wristSpineSkeleton, immutableWristSpineSkeleton;
        ThreePointSkeleton threePointSkeleton, immutableThreePointSkeleton;
    
        Skeleton* previousSkeleton, *currentSkeleton;
        vector<string> sceneNames, sceneWithSkeletonNames;
    
        bool showGuis;
        bool showImage, showWireframe, showSkeleton, mouseControl;
        bool frameBasedAnimation;
    
        int sceneRadio;
    

};
