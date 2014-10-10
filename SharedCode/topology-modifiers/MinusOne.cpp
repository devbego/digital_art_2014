#include "MinusOne.h"

const IndexSet ringIndices = IndexSet()
/115
/119/120/124/125/129/130/134/135
/140/141/142
/IndexRange(63, 83);

const IndexSet ringBaseIndices = IndexSet()
/115
/119/120/124/125/129/130/134/135
/140/141/142
/63/64/65;

string MinusOne::getName() const {
    return "MinusOne";
}

void MinusOne::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;

    // make a copy of the removal region, to be used for blending
    blendMesh = copySubmesh(handMesh, ringBaseIndices);

    // remove the triangles for the remaining indices
    handMesh = removeSubmesh(handMesh, ringIndices);

    // stitch sides together
    // post-removal indices, not original indices
    int toStitchLeft[] = {99, 104, 109, 114, 120};
    int toStitchRight[] = {98, 103, 108, 113, 119};
    int toStitchCount = 5;
    stitchIndices.clear();
    for(int i = 0; i < toStitchCount; i++) {
        stitchIndices.push_back(pair<ofIndexType, ofIndexType>(toStitchLeft[i], toStitchRight[i]));
    }
    handMesh = stitch(handMesh, stitchIndices);
    
    float opacity[] = {
        0,0,0,0, 0,0,0,0,
        0,1,1, 1,1,1,
        0};
    int blendIndices[] = {
        3, 4, 6, 8, 10, 12, 0, 1, 2, 14, 11, 9, 7, 5, 13};
    int handIndices[] = {
        94, 97, 102, 107, 112, 117, 42, 43, 44, 119, 113, 108, 103, 98, 118};
    int baseCount = 15;
    vector<ofFloatColor> colors(baseCount);
    for(int i = 0; i < baseCount; i++) {
        ofVec3f& from = handMesh.getVertices()[handIndices[i]];
        ofVec3f& to = blendMesh.getVertices()[blendIndices[i]];
        to = from;
        colors[blendIndices[i]] = ofFloatColor(1, opacity[i]);
    }
    blendMesh.addColors(colors);
    
    final = handMesh;
}

ofMesh& MinusOne::getModifiedMesh() {
    return final;
}

void MinusOne::drawBlends() {
    blendMesh.drawFaces();
}