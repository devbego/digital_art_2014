#pragma once

#include "Scene.h"
#include "HandWithFingertipsSkeleton.h"

class SplayFingers2Scene : public Scene {
protected:
	float maxPalmAngleLeft, maxPalmAngleRight;	
	float maxBaseAngleLeft, maxBaseAngleRight;

	float splayHeight;
	float splayAxis;
	float maxAngle;
    
    float averageAngleOffset;
    
public:
	SplayFingers2Scene (ofxPuppet* puppet, HandWithFingertipsSkeleton* handWithFingertipsSkeleton, HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton);
	void setupGui();
	void setupMouseGui();
	void update();
	void updateMouse(float mx, float my);
	void draw();
};
