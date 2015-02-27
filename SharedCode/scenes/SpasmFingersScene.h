// Spasm Fingers Scene
// Rispoli 1-20-14

#pragma once

#include "Scene.h"
#include "HandWithFingertipsSkeleton.h"

class SpasmFinger;

class SpasmFingersScene : public Scene {
public:
    SpasmFingersScene(ofxPuppet* puppet,
                        HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                        HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton);
	
    void setupGui();
	void setupMouseGui();
	void update();
	void updateMouse(float mx, float my);
	void draw();
    
    vector<SpasmFinger> spasmFingers;
    
    bool curlFingers;
};

class SpasmFinger {
    
public:
    bool initialized;
    
    float goalAngle;
    float angle;
    bool flipped;
    
    int spasmResetTimer;
    int spasmResetLimit;
    
    SpasmFinger() {
        initialized = false;
    }
    
    void initialize() {
        initialized = true;
        
        goalAngle = 0.0;
        spasmResetTimer = 0;
        flipped = false;
    }
    
    void resetNextSpasmTime() {
        spasmResetLimit = ofRandom(10, 50);
    }
    
    void update() {
        spasmResetTimer++;
        if(spasmResetTimer >= spasmResetLimit) {
            spasmResetTimer = 0;
            resetNextSpasmTime();
            
            goalAngle = ofRandom(0,15);
            flipped = ofRandom(1) > 0.5;
        }
        
        if(angle > 0) {
            goalAngle -= goalAngle*0.5;
        }
        
        angle += (goalAngle-angle)*0.7;
    }
};