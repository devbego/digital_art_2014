#pragma once

// This is a modified version of kyle mcdonald's ofxBlur, which allows for the blue channel to not be blurred.
// The reason for this is that the hand app uses the blue channel to contain identity information,
// while the red and green channels contain the orientation information. 

#include "ofMain.h"

class ofxBlur {
protected:
	ofFbo base;
	vector<ofFbo*> ping, pong;
	
	ofShader blurShader, combineShader;
	float scale, rotation;
	float downsample;
	float brightness;
public:
	ofxBlur();
	
	void setup(int width, int height, int radius = 32, float shape = .2, int passes = 1, float downsample = .5);
	
	void setScale(float scale);
	void setRotation(float rotation);
	void setBrightness(float brightness); // only applies to multipass
	
	void begin();
	void end();
	void draw();
	
	ofTexture& getTextureReference();
	
};

// <3 kyle