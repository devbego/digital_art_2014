#pragma once

#include "ofMesh.h"
#include "ofTexture.h"
#include "ofxButterfly.h"

class TopologyModifier {
public:
    virtual string getName() const = 0;
    virtual void update(const ofMesh& mesh) = 0;
    virtual ofMesh& getModifiedMesh() = 0;
	virtual void initialize() {
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