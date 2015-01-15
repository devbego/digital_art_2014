// Delayed Fingers Scene
// Rispoli 12-4-14

#pragma once

#include "Scene.h"
#include "HandWithFingertipsSkeleton.h"
#include <list>

class DelayedFingersScene : public Scene {
public:
    DelayedFingersScene(ofxPuppet* puppet,
                         HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                         HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton);
	
    void setupGui();
	void setupMouseGui();
	void update();
	void updateMouse(float mx, float my);
	void draw();
    void delayFinger(float newestAngle,
                     list<float>* delayList,
                     HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                     int jointIndex,
                     int delayListSize);
    
    // doubly linked lists, stores old angles.
    // the newest angle is pushed onto the front.
    // the oldest angle is popped off the back
    // (if the size of the list is > the delay time).
    list<float> pinkyDelayList;
    list<float> ringDelayList;
    list<float> middleDelayList;
    list<float> indexDelayList;
    list<float> thumbDelayList;
    
    int pinkyDelayListSize;
    int ringDelayListSize;
    int middleDelayListSize;
    int indexDelayListSize;
    int thumbDelayListSize;
};