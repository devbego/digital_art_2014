#pragma once

#include "ofMesh.h"
#include "ofTexture.h"
#include "ofxButterfly.h"
#include "ofxUICanvas.h"

class TopologyModifier {
public:
    virtual string getName() const = 0;
    virtual void update(const ofMesh& mesh) = 0;
    virtual ofMesh& getModifiedMesh() = 0;
    
    bool saveFinalMesh = false;
    
    ofxUICanvas *gui;
    
	virtual void initialize() {
        
	}
    
    void initializeGui() {
        gui = new ofxUICanvas();
		gui->setFont("GUI/NewMedia Fett.ttf");
		gui->setPosition(540, 0);
		gui->setVisible(false);
		gui->addLabel("Scene Adjustment Options");
		gui->addSpacer();
        gui->addToggle("Save final mesh", &saveFinalMesh);
    }
    
    virtual void drawBlends() {
    }
    
	virtual void saveMeshes() {
	}
    
    virtual void drawBase() {
        getModifiedMesh().drawFaces();
    }
    virtual void drawSimple (const ofTexture& texture) {
        texture.bind();
        drawBase();
        drawBlends();
        texture.unbind();
    }
	
	virtual void draw (const ofTexture& texture) {
    }
};