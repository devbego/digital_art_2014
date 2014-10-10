#pragma once

#include "ofMesh.h"
#include "ofTexture.h"

class TopologyModifier {
public:
    virtual string getName() const = 0;
    virtual void update(const ofMesh& mesh) = 0;
    virtual ofMesh& getModifiedMesh() = 0;
    virtual void drawBlends() {
    }
    virtual void drawBase() {
        getModifiedMesh().drawFaces();
    }
    virtual void draw(const ofTexture& texture) {
        texture.bind();
        drawBase();
        drawBlends();
        texture.unbind();
    }
};