//
//  FingerTipVideoRecorder.h
//  leapAndCamRecorder2
//
//  Created by chris on 28/07/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class FingerTipCalibRecorder{
    
    public:
    
    int xOffset, yOffset;
    int lastTagNumber;
    int lastFrameCount;
    string filePath;
    ofxXmlSettings XML;
    
    void setup(string dirPath, string fileName){
        lastTagNumber = 0;
        lastFrameCount = -1;
        xOffset = 0;
        yOffset = 0;
        
        ofDirectory dir;
        dir.open(dirPath);
        if(!dir.exists())
            dir.createDirectory(dirPath);
        
        this->filePath = dirPath+"/"+fileName;
        
        // load any previous points
        XML.clear();
        XML.loadFile(this->filePath);
    }
    
    void setDrawOffsets(int x, int y){
        xOffset = x;
        yOffset = y;
    }
    
    void recordPosition(float mouseX, float mouseY, ofPoint fingerCoord, int frameCount=0){
        
        if (fingerCoord == ofVec3f(-1.0f,-1.0f,-1.0f)){
			cout << "NOTHING RECORDED" << endl;
			return;
		}
        
        ofPoint mouseCoord = ofPoint((mouseX-xOffset),mouseY-yOffset);
        
        // check if we have this frame
        int numTags = XML.getNumTags("CALIB_READ");
        bool bFoundTag = false;
        for(int i = 0; i < numTags; i++){
            int frameNum = XML.getAttribute("CALIB_READ","frame",0,i);
            if(frameNum == frameCount){
                lastTagNumber = i;
                bFoundTag = true;
            }
        }
    
        if(!bFoundTag) lastTagNumber = XML.addTag("CALIB_READ");
		
        XML.setAttribute("CALIB_READ", "frame", frameCount,lastTagNumber);
        
        
        if( XML.pushTag("CALIB_READ", lastTagNumber) )
		{
			int tagNum = 0;//XML.addTag("MOUSE");
			XML.setValue("MOUSE:X", mouseCoord.x, tagNum);
			XML.setValue("MOUSE:Y", mouseCoord.y, tagNum);
			
			tagNum = 0;//XML.addTag("FINGER");
			XML.setValue("FINGER:X", fingerCoord.x, tagNum);
			XML.setValue("FINGER:Y", fingerCoord.y, tagNum);
			XML.setValue("FINGER:Z", fingerCoord.z, tagNum);
			XML.popTag();
		}
		XML.popTag();
		XML.saveFile(filePath);
        
        lastFrameCount = frameCount;

    }
    
    
    void drawPointHistory(int thisFrame){
        
        int numTags = XML.getNumTags("CALIB_READ");
        
        for(int i = 0; i < numTags; i++){
            int frameNum = XML.getAttribute("CALIB_READ","frame",0,i);
            XML.pushTag("CALIB_READ", i) ;
            float mx = XML.getValue("MOUSE:X",0);
            float my = XML.getValue("MOUSE:Y",0);
            ofNoFill();
            if(thisFrame == frameNum) ofSetColor(255,0,0);
            else ofSetColor(255,200);
            ofEllipse(mx+xOffset,my,10,10);
            XML.popTag();
            
        }
    }
    
};
