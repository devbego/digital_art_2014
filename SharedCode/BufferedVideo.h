#pragma once

#include "ofMain.h"
#include "ofxTiming.h"

class BufferedVideo : public ofBaseVideo {

protected:
	vector<ofPixels> images;
	int loaded;
	ofTexture texture;
    mutable bool newFrame;
    bool playing;
	bool bRolledOver; 
	int currentFrame;
	DelayTimer timer;
	ofDirectory directory;
	void loadNextAvailable() {
		if(loaded < images.size()) {
			ofLoadImage(images[loaded], directory.getPath(loaded));
			loaded++;
		}
	}
	void updateFrame() {
		if(images[currentFrame].getWidth() > 0) {
			texture.loadData(images[currentFrame]);
		}
		newFrame = true;
	}

public:
	BufferedVideo()
	:currentFrame(0)
	,playing(true)
	,newFrame(false)
	,loaded(0) {
		timer.setFramerate(30);
	}
	bool isLoaded() const {
        if(loaded > 0) return true;
        return false;
    }
    bool isInitialized() const {
        return isLoaded();
    }
    void setFrameRate(float frameRate) {
		timer.setFramerate(frameRate);
	}
	void load(string directoryName) {
		loaded = 0;
        currentFrame = 0;
        newFrame = false;
        directory.listDir(directoryName);
		images.clear();
        images.resize(directory.size());
		loadNextAvailable();
		texture.allocate(images[0]);
		bRolledOver = false; 
	}
	
	int getNFrames(){
		if (isLoaded()){
			return images.size();
		} else {
			return 0;
		}
	}
    
	void close() {
	}
	unsigned char* getPixels() {
		return images[currentFrame].getPixels();
	}
	ofPixels& getPixelsRef() {
		return images[currentFrame];
    }
    const ofPixels& getPixelsRef() const {
        return images[currentFrame];
    }
    bool setPixelFormat(ofPixelFormat pixelFormat) {
    }
    ofPixelFormat getPixelFormat() const {
        return OF_PIXELS_UNKNOWN;
    }
	ofPixels& getFrame(int i) {
		return images[i];
	}
	ofTexture& getTextureReference(){
		return texture;
	}
	void setPlaying(bool playing) {
		this->playing = playing;
	}
	void goToPrevious() {
		currentFrame = (currentFrame - 1 + images.size()) % images.size();
		updateFrame();
	}
	void goToNext() {
		int prevFrame = currentFrame;
		currentFrame = (currentFrame + 1) % images.size();
		if (prevFrame == (images.size()-1)){
			bRolledOver = true; 
		}
		updateFrame();
	}
	void update() {
		loadNextAvailable();
		if(playing) {
			if(timer.tick()) {
				goToNext();
			}
		}
	}
	bool isRolledOver(){
		return bRolledOver;
	}
	bool isFrameNew() const {
		bool cur = newFrame;
		newFrame = false;
		return cur;
	}
	void draw(float x, float y) {
		texture.draw(x, y);
	}
	void draw(float x, float y, float w, float h) {
		texture.draw(x, y, w, h);
	}
	int getCurrentFrameID(){
		return currentFrame;
	}
};