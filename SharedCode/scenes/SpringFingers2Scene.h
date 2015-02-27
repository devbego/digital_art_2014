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
    
    bool lockLinear[3];
    bool lockTorsion[3];
    
    float springy[3];
    float damp[3];
    float toChildDamp[3];
    float angleLimit[3];
};

class LTSpring {
    
public:
    bool initialized;
    
    ofPoint currentPosition;
    ofPoint prevPosition;
    ofPoint restPosition;
    ofPoint currentForce;
    
    float springy;
    float damp;
    float toChildDamp;
    float angleLimit;
    
    bool lockLinear;
    bool lockTorsion;
    
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
    
    void setSpringValues(float _springy, float _damp, float _toChildDamp, float _angleLimit,
                        bool _lockLinear, bool _lockTorsion) {
        springy = _springy;
        damp = _damp;
        toChildDamp = _toChildDamp;
        angleLimit = _angleLimit;
        
        lockLinear = _lockLinear;
        lockTorsion = _lockTorsion;
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
        
        if(lockLinear) {
            newLength = skeletonLength;
        }
        if(lockTorsion) {
            newAngle = skeletonAngle;
        }
        
        // limits
        
        /*
        float angleDiff = skeletonAngle-newAngle;
        if(abs(angleDiff) > angleLimit) {
            newAngle = skeletonAngle + angleLimit;
        }
        
        if(newLength > skeletonLength * 1.2) {
            newLength = skeletonLength * 1.2;
        }
        if(newLength < skeletonLength * 0.8) {
            newLength = skeletonLength * 0.8;
        }*/
        
        currentPosition.x = childPosition.x + cos(newAngle) * newLength;
        currentPosition.y = childPosition.y + sin(newAngle) * newLength;
    }
};