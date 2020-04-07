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
    
    float waveTableOsc[514];
            
    ofPixels buffPlayZonePixels;
    ofPixels buffWaveTablePixels;
    
    float playPosition;
    
    ofRectangle waveTable;
    ofRectangle playZone;
    
    bool mouseOnWaveTable;
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

    void setPlayZoneBuffer(ofRectangle playZone, ofVec2f playZoneSize);
    void setWaveTableBuffer(ofRectangle waveTable, ofVec2f waveTableSize);
    
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
  
    ofRectangle playZoneRightBottom;
    ofRectangle playZoneRightTop;
    ofRectangle playZoneLeftTop;
    ofRectangle playZoneLeftBottom;
    bool mouseOnPlayZoneRightBottom;
    bool mouseOnPlayZoneRightTop;
    bool mouseOnPlayZoneLeftTop;
    bool mouseOnPlayZoneLeftBottom;

    void drawFourZone();
    void drawCirclePart(ofRectangle rect);
    void drawArrowPart(ofRectangle rect, float rotation);
    
    void resetPlayZoneAll();
    void setPlayZoneFourCorner();
    
    void edgeRectangle(ofColor _c, ofVec2f _v);
    
    void allResetByLeftTop(ofVec2f _v);
    void allResetByLeftBottom(ofVec2f _v);
    void allResetByRightTop(ofVec2f _v);
    void allResetByRightBottom(ofVec2f _v);

    ofVec2f setSmallSizeOffsetLeftTop(float x, float y);
    ofVec2f setSmallSizeOffsetLeftBottom(float x, float y);
    ofVec2f setSmallSizeOffsetRightTop(float x, float y);
    ofVec2f setSmallSizeOffsetRightBottom(float x, float y);

    void playZoneWaveTableMouseOff();
    void setAllFourCorner(ofVec2f _v);
    
    void mouseReleasedEvent(ofMouseEventArgs & mouse);
    
};
