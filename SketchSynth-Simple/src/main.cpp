#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

    ofSetupOpenGL(1080 + 280, 810, OF_WINDOW); // 4 : 3

    ofRunApp( new ofApp() );

}
