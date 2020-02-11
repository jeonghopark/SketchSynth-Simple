#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxDropdown.h"

#define BIT 512

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    void audioOut(ofSoundBuffer & buffer);
    ofSoundStream soundStream;

    void generatAmpFq(float _plaPos);
    
    void setAudio(ofSoundStreamSettings settings);
    
    ofSoundStreamSettings setAudioDevice();
//	ofSoundStreamSettings setWinAudioDevice();
    
    void setGui(vector<ofSoundDevice> _d, int _id);
    int defaultDeviceID;
    ofSoundDevice defaultInputDevice;
    vector<ofSoundDevice> audioInputDeviceList;
    ofSoundStreamSettings settings;
        
    float amplitude[BIT];
    int frequency[BIT];
    float phase[BIT];
    
    bool bPlaying;
    
    float getAmp(float _xPos, float _yPos);
    float getFreq(float _yPos);
            
    void setupImage();
    
    ofImage defaultImg;
    
    float defaultImgWindowSizeRatio;
    
    float waveTable[514];
            
    ofPixels buffPlayZonePixels;
    ofPixels buffWaveTablePixels;
    
    float playPosition;
    
    ofRectangle waveTableZone;
    ofRectangle playZone;
    
    bool mouseOnWaveTableZone;
    bool mouseOnPlayZone;
    
    ofTexture texturePlayZone;
    ofTexture textureWaveTable;

    unsigned char * playZoneChar;
    unsigned char * buffPlayZoneChar;

    unsigned char * waveTableChar;
    unsigned char * buffWaveTableChar;

    ofxPanel gui;
    ofxFloatSlider overlap;
    ofxFloatSlider hifq;
    ofxFloatSlider lofq;
    ofxFloatSlider speed;
    ofxFloatSlider thresholdPlayZone;
    ofxFloatSlider thresholdWaveTable;
    ofxButton openFileButton;
    ofxToggle invertColorToggle;
    ofxFloatSlider volumeSlider;
    ofxIntSlider deviceID;
    unique_ptr<ofxIntDropdown> intDropdown;
    ofParameter<int> guiAudioDevice;

    void openFileButtonPressed();
    void deviceIDChanged(int & _deviceID);
    
    ofParameter<int> deviceNum;
    
    void processOpenFileSelection(ofFileDialogResult openFileResult);
    ofImage loadedImageResize(string _parh);
    void loadProcess(string _parh);
    ofFbo loadedImageFBO(ofImage _img);
    
    float sidebarPosX;
    float sidebarWidth;

    void drawPlayZoneTexture();
    void drawWaveTableTexture();
    
    void openFile();
    
    ofVec2f playZoneSize;
    ofVec2f playZoneScaledSize;
    ofVec2f waveTableSize;
    ofVec2f waveTableScaledSize;

    void setDefaultBuffer();
    
    void displaySidebar();
    void drawSidebarBackground();
    void drawWaveTableScope();
    void drawPlaySpectrum();
    void drawDefaultImage();
    
    ofPixels cropImageToPixels(ofRectangle _r, ofVec2f _s);
    void captureChar(ofPixels _p, unsigned char * _o, unsigned char * _b);
    void captureWaveTable();
    ofPixels captureWaveTablePixels(unsigned char * _wtC, unsigned char * _bpzC);
    void setWaveTableFBO();

    void capturePlayZone();
    ofPixels capturePlayZonePixels(unsigned char * _pzC, unsigned char * _bpzC);
    void setPlayZoneFBO();

    ofVec2f waveTableClickPos;
    ofVec2f playZoneclickPos;
            
    void drawZoneBox(ofRectangle _r, ofColor _c);
    void drawZone(bool _onoff, ofRectangle _r, ofColor _c);
    
    void drawFramerate();
    void drawInfo();
    
    ofVec2f limitMouseDrag(int x, int y, ofRectangle _r, ofVec2f _clickPos);
    
};
