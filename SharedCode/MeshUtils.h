#pragma once

#include "ofMain.h"

#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>
template <typename T>
struct variadic_ : public vector<T> {
    variadic_& operator/(const T& t) {
        this->push_back(t);
        return *this;
    }
    variadic_& operator/(const variadic_<T>& t) {
        for(int i = 0; i < t.size(); i++) {
            this->push_back(t[i]);
        }
        return *this;
    }
    bool contains(const T& t) const {
        int n = this->size();
        for(int i = 0; i < n; i++) {
            if(this->at(i) == t) {
                return true;
            }
        }
        return false;
    }
};

typedef variadic_<int> IndexSet;

class IndexRange : public IndexSet {
private:
    int front, back;
public:
    IndexRange(int front, int back)
    :front(front), back(back) {
        for(int i = front; i <= back; i++) {
            *this/i;
        }
    }
};

ofMesh removeSubmesh(ofMesh& mesh, const IndexSet& indices);
ofMesh copySubmesh(const ofMesh& mesh, const IndexSet& indices);

void addAttributes(const ofMesh& src, int index, ofMesh& dst);
void orientMesh(ofMesh& mesh,
                ofVec2f fromStart, ofVec2f fromEnd,
                ofVec2f toStart, ofVec2f toEnd);
void orientPolyline(ofPolyline& polyline,
                    ofVec2f fromStart, ofVec2f fromEnd,
                    ofVec2f toStart, ofVec2f toEnd);
ofPolyline buildPolyline(ofMesh& mesh, int indices[], int count);
void removeTriangles(ofMesh& mesh, const ofPolyline& region);
ofMesh dropUnusedVertices(ofMesh& mesh);
ofMesh copySubmesh(const ofMesh& mesh, const ofPolyline& region);
void mergeCoincidentVertices(ofMesh& mesh, float epsilon = 10e-5);
ofMesh stitch(ofMesh& mesh, vector<pair<ofIndexType, ofIndexType> >& stitch);
bool isLeft(ofVec2f a, ofVec2f b, ofVec2f c);
ofVec2f closestPointOnLine(const ofVec2f& p1, const ofVec2f& p2, const ofVec2f& p3);
bool sideTest(ofPolyline& polyline, ofVec2f position);
void split(ofMesh& mesh, vector<ofIndexType>& indices);