// Remapped Fingers Scene
// Rispoli 12-4-14

#pragma once

#include "Scene.h"
#include "HandWithFingertipsSkeleton.h"

class RemappedFingersScene : public Scene {
public:
    RemappedFingersScene(ofxPuppet* puppet,
                         HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                         HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton);
	
    void setupGui();
	void setupMouseGui();
	void update();
	void updateMouse(float mx, float my);
	void draw();
    void addFingerRemapping(int s, int c);
    
    float baseAngles[5];
    float currentAngles[15];
    
    vector<int> sourceFingers;
    vector<int> controlledFingers;
    
    bool useRawAngles;
};