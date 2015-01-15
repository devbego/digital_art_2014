// Spring Fingers Scene 2
// Rispoli 12-4-14

#pragma once

#include "Scene.h"
#include "HandWithFingertipsSkeleton.h"

class LTSpring;

class SpringFingers2Scene : public Scene {
public:
    SpringFingers2Scene(ofxPuppet* puppet,
                        HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                        HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton);
	
    void setupGui();
	void setupMouseGui();
	void update();
	void updateMouse(float mx, float my);
	void draw();
    
    vector<LTSpring> springs;
};

class LTSpring {
    
public:
    bool initialized;
    
    float springy;
    float damp;
    float toChildDamp;
    
    float angleChangeLimit;
    float lengthChangeLimit;
    
    bool lockAngle;
    bool lockLength;
    
    ofPoint currentPosition;
    ofPoint prevPosition;
    ofPoint restPosition;
    ofPoint currentForce;
    
    ofPoint velocity;
    
    LTSpring() {
        initialized = false;
        
        currentPosition = ofPoint(0,0);
        prevPosition = ofPoint(0,0);
        restPosition = ofPoint(0,0);
        
        velocity = ofPoint(0,0);
    }
    
    void initialize(ofPoint initPosition) {
        currentPosition = initPosition;
        prevPosition = initPosition;
        
        initialized = true;
    }
    
    void update(ofPoint skeletonPosition, ofPoint childPosition,
                ofPoint parentForce,
                float skeletonLength, float skeletonAngle) {
        prevPosition = currentPosition;
        restPosition = skeletonPosition;
        
        currentForce = ofPoint((restPosition.x - currentPosition.x) * springy,
                               (restPosition.y - currentPosition.y) * springy);
        
        velocity.x += currentForce.x - parentForce.x*toChildDamp;
        velocity.y += currentForce.y - parentForce.y*toChildDamp;
        velocity.x *= damp;
        velocity.y *= damp;
        
        ofPoint newPosition = ofPoint(currentPosition.x+velocity.x,
                                      currentPosition.y+velocity.y);
        
        // this might seem like a silly way of doing things (converting angle to coords,
        // then back to angle) but it gives us the most control... works pretty well.
        
        float newLength = childPosition.distance(newPosition);
        float newAngle = atan((childPosition.y-newPosition.y) / (childPosition.x-newPosition.x)) + PI;
        
        if(lockLength) {
            newLength = skeletonLength;
        }
        if(lockAngle) {
            newAngle = skeletonAngle;
        }
        
        // TODO: finish limits
        // but this code should work just fine. just need to add limits to gui and find
        // some values that work.
        
        /*
        float angleDiff = skeletonAngle-newAngle;
        if(angleDiff > abs(angleChangeLimit)) {
            newAngle = skeletonAngle + angleChangeLimit;
        }
         
        float lengthDiff = skeletonLength-newLength;
        if(lengthDiff > abs(lengthChangeLimit)) {
            newLength = skeletonLength + lengthChangeLimit;
        }
        */
        
        currentPosition.x = childPosition.x + cos(newAngle) * newLength;
        currentPosition.y = childPosition.y + sin(newAngle) * newLength;
    }
};