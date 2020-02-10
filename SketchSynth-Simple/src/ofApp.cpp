#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {
    #ifdef DEBUG
    #else
        ofSetDataPathRoot("../Resources/data");
    #endif
    
    ofSetFrameRate(60);
    ofBackground(0);

    sidebarPosX = ofGetHeight() * 4 / 3;
    sidebarWidth = ofGetWidth() - sidebarPosX;

    playZoneSize.set(1600, 514);
    waveTableSize.set(514, 514);

    setGui();

    setupImage();

    defaultImgWindowSizeRatio = defaultImg.getWidth() / sidebarPosX;

    playZoneScaledSize = playZoneSize / defaultImgWindowSizeRatio;
    waveTableScaledSize = waveTableSize / defaultImgWindowSizeRatio;

    setDefaultBuffer();

    captureWaveTable();
    capturePlayZone();

    setAudio();

    playPosition = 0;

    mouseOnWaveTableZone = false;
    mouseOnPlayZone = false;

}


//--------------------------------------------------------------
void ofApp::openFileButtonPressed() {

    openFile();

}


//--------------------------------------------------------------
void ofApp::setGui() {

    openFileButton.addListener(this, &ofApp::openFileButtonPressed);

    gui.setup();
//    gui.setDefaultWidth(sidebarWidth * 0.8);
    gui.setPosition(sidebarPosX, 60);
//    gui.add(openFileButton.setup("File Open"));
    gui.add(thresholdPlayZone.setup("PlayZone Contrast", 0.59, 0, 1));
    gui.add(thresholdWaveTable.setup("WaveTable Contrast", 0.59, 0, 1));
    gui.add(invertColorToggle.setup("Invert colors", false));
    gui.add(speed.setup("Speed", 2, 0.1, 10));
    gui.add(overlap.setup("Overlap", 4, 0.1, 20));
    gui.add(hifq.setup("Hi fq", 1800, 800, 4000));
    gui.add(lofq.setup("LO fq", 50, 20, 700));
    gui.add(volumeSlider.setup("Volume", 0.85, 0.0, 1.0));

}


//--------------------------------------------------------------
void ofApp::setupImage() {

    defaultImg.load("c02.jpg"); // Default image size : 4032 x 3024
    defaultImg.setImageType(OF_IMAGE_COLOR_ALPHA);

}


//--------------------------------------------------------------
void ofApp::setDefaultBuffer() {

    buffWaveTablePixels.allocate(waveTableSize.x, waveTableSize.y, OF_PIXELS_GRAY);
    textureWaveTable.allocate(buffWaveTablePixels);

    buffPlayZonePixels.allocate(playZoneSize.x, playZoneSize.y, OF_PIXELS_GRAY);
    texturePlayZone.allocate(buffPlayZonePixels);

    waveTableChar = new unsigned char[514 * 514 * 4];
    buffWaveTableChar = new unsigned char[514 * 514 * 4];
    playZoneChar = new unsigned char[1600 * 514 * 4];
    buffPlayZoneChar = new unsigned char[1600 * 514 * 4];

    waveTableZone.set(650, 190, waveTableScaledSize.x, waveTableScaledSize.x);
    playZone.set(100, 50, playZoneScaledSize.x, playZoneScaledSize.y);

}


//--------------------------------------------------------------
void ofApp::captureWaveTable() {

    ofPixels _p = cropImageToPixels(waveTableZone, waveTableSize);
    captureChar(_p, waveTableChar, buffWaveTableChar);

}


//--------------------------------------------------------------
void ofApp::capturePlayZone() {

    ofPixels _p = cropImageToPixels(playZone, playZoneSize);
    captureChar(_p, playZoneChar, buffPlayZoneChar);

}


//--------------------------------------------------------------
ofPixels ofApp::cropImageToPixels(ofRectangle _r, ofVec2f _s) {

    ofImage _bImg;
    _bImg.cropFrom(defaultImg, _r.getX() * defaultImgWindowSizeRatio, _r.getY() * defaultImgWindowSizeRatio, _s.x, _s.y);
    
    return _bImg.getPixels();

}


//--------------------------------------------------------------
void ofApp::captureChar(ofPixels _p, unsigned char * _o, unsigned char * _b) {
 
    int _size = _p.size();
    memcpy(_o, _p.getData(), _size * sizeof(unsigned char));
    memcpy(_b, _p.getData(), _size * sizeof(unsigned char));

}


//--------------------------------------------------------------
void ofApp::setAudio() {

    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.bufferSize = 512;
    settings.sampleRate = 44100;
    settings.numInputChannels = 0;
    settings.numOutputChannels = 2;
    soundStream.setup(settings);

}


//--------------------------------------------------------------
void ofApp::update() {

    if (bPlaying) {
        playPosition += speed;

        generatAmpFq(playPosition);

        if (playPosition > playZoneSize.x) {
            playPosition = 0;
        }
    }

    buffWaveTablePixels = captureWaveTablePixels(waveTableChar, buffWaveTableChar);
    buffPlayZonePixels = capturePlayZonePixels(playZoneChar, buffPlayZoneChar);

}


//--------------------------------------------------------------
void ofApp::generatAmpFq(float _plaPos) {

    for (int n = 0; n < BIT; n++) {
        amplitude[n] = (amplitude[n] * overlap + getAmp(floor(_plaPos), n)) / (overlap + 1);
        //            frequency[n] = int(ofMap(n, 0, BIT, hifq, lofq));
        frequency[n] = int(getFreq(n));;
    }

}


//--------------------------------------------------------------
float ofApp::getAmp(float _xPos, float _yPos) {

    float _amp = 0;

    if (_yPos > 0 && _yPos < BIT) {
        _amp = buffPlayZonePixels.getColor(_xPos, _yPos).getLightness() / 255.0;
    } else {
        _amp = 0;
    }

    return _amp;

}


//--------------------------------------------------------------
float ofApp::getFreq(float _yPos) {

    float _freq = 0;
    float _yPosToFreq = ofMap(_yPos, 0, BIT, lofq, hifq);
    _freq = 1 - (log(_yPosToFreq) - log(lofq)) / (log(hifq) - log(lofq));
    _freq *= hifq;

    return _freq;

}


//--------------------------------------------------------------
ofPixels ofApp::captureWaveTablePixels(unsigned char * _wtC, unsigned char * _bwtC) {

    float _s;
    float _sumChar;

    for (int i = 0; i < waveTableSize.x; i += 1) {
        float _height = waveTableSize.y * 0.5;

        for (int j = waveTableSize.y - 1; j >= 0; j--) {
            int _index = i + j * waveTableSize.x;
            _sumChar = _wtC[_index * 4 + 0] + _wtC[_index * 4 + 1] + _wtC[_index * 4 + 2];

            if (invertColorToggle) {
                _s = _sumChar;
            } else {
                _s = 255 * 3 - _sumChar;
            }

            if (_s < thresholdWaveTable * 255 * 3) {
                _bwtC[_index] = 0;
            } else {
                _bwtC[_index] = 255;

                _height = j;
            }

            waveTable[i] = ofMap(_height, 0, waveTableSize.y, -1, 1);
        }
    }

    ofPixels _r;
    _r.setFromPixels(_bwtC, waveTableSize.x, waveTableSize.y, 1);
    return _r;

}


//--------------------------------------------------------------
ofPixels ofApp::capturePlayZonePixels(unsigned char * _pzC, unsigned char * _bpzC) {

    float _s;
    int _num = playZoneSize.x * playZoneSize.y;
    float _sumChar;

    for (int i = 0; i < _num; i += 1) {
        _sumChar = _pzC[i * 4 + 0] + _pzC[i * 4 + 1] + _pzC[i * 4 + 2];

        if (invertColorToggle) {
            _s = _sumChar;
        } else {
            _s = 255 * 3 - _sumChar;
        }

        if (_s < thresholdPlayZone * 255 * 3) {
            _bpzC[i] = 0;
        } else {
            _bpzC[i] = 255;
        }
    }

    ofPixels _r;
    _r.setFromPixels(_bpzC, playZoneSize.x, playZoneSize.y, 1);
    return _r;

}


//--------------------------------------------------------------
void ofApp::draw() {

    drawDefaultImage();

    drawZone(mouseOnWaveTableZone, waveTableZone, ofColor(255, 0, 0, 255));
    drawZone(mouseOnPlayZone, playZone, ofColor(0, 255, 0, 255));

    drawPlaySpectrum();

    displaySidebar();

    gui.draw();

}


//--------------------------------------------------------------
void ofApp::drawZone(bool _onOff, ofRectangle _r, ofColor _c) {

    ofPushStyle();

    if (_onOff) {
        ofSetColor(_c, 70);
    } else {
        ofSetColor(_c, 20);
    }

    ofDrawRectangle(_r);
    ofPopStyle();

    ofPushStyle();
    drawZoneBox(_r, ofColor(_c.r, _c.g * 0.5, _c.b, 255));
    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::drawZoneBox(ofRectangle _r, ofColor _c) {

    ofPushStyle();
    ofSetColor(_c);

    ofDrawLine(_r.getX(), _r.getY(), _r.getX() + _r.getWidth(), _r.getY());
    ofDrawLine(_r.getX(), _r.getY() + _r.getHeight(), _r.getX() + _r.getWidth(), _r.getY() + _r.getHeight());
    ofDrawLine(_r.getX(), _r.getY(), _r.getX(), _r.getY() + _r.getHeight());
    ofDrawLine(_r.getX() + _r.getWidth(), _r.getY(), _r.getX() + _r.getWidth(), _r.getY() + _r.getHeight());

    ofSetColor(_c, 120);
    ofDrawLine(_r.getX(), _r.getY() + _r.getHeight() * 0.5, _r.getX() + _r.getWidth(), _r.getY() + _r.getHeight() * 0.5);

    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::drawDefaultImage() {

    ofPushStyle();
    defaultImg.draw(0, 0, defaultImg.getWidth() / defaultImgWindowSizeRatio, defaultImg.getHeight() / defaultImgWindowSizeRatio);
    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::drawPlaySpectrum() {

    if (bPlaying) {
        ofPushStyle();

        int _xStep = 4;
        int _yStep = 10;
        
        int _valueOffset = 50;

        for (int n = 0; n < BIT; n += _yStep) {
            float _yRatio = ofMap(n, 0, BIT - 1, 0, playZoneSize.y) / defaultImgWindowSizeRatio;

            int _floatSize = floor(_valueOffset * amplitude[n]);

            for (int w = 0; w < _floatSize; w += _xStep) {
                ofSetColor(0, 180, 0, ofMap(w, 0, _floatSize, 0, 255));

                float _x = playZone.getX() + playPosition / defaultImgWindowSizeRatio;
                float _y = playZone.getY() + _yRatio;

                ofDrawRectangle(_x + w - _xStep * 0.5, _y, 3, 3);
                ofDrawRectangle(_x - w - _xStep * 0.5, _y, 3, 3);
            }
        }

        ofSetColor(0, 255 * 0.5, 0, 220);
        float _x = playZone.getX() + playPosition / defaultImgWindowSizeRatio;
        float _y = playZone.getY();
        ofDrawLine(_x, _y, _x, _y + playZoneScaledSize.y);

        ofPopStyle();
    }

}


//--------------------------------------------------------------
void ofApp::displaySidebar() {

    ofPushMatrix();

    ofTranslate(sidebarPosX, 0);

    drawSidebarBackground();

    drawFramerate();
    drawInfo();

    int _step = 4;
    float _texturePosY = gui.getHeight() + gui.getPosition().y + _step;
    ofTranslate(0, _texturePosY);
    drawWaveTableTexture();

    ofTranslate(0, waveTableScaledSize.y + _step);
    drawWaveTableScope();

    ofTranslate(0, waveTableScaledSize.y + _step);
    drawPlayZoneTexture();

    ofPopMatrix();

}


//--------------------------------------------------------------
void ofApp::drawSidebarBackground() {

    ofPushStyle();
    ofSetColor(30);
    ofDrawRectangle(0, 0, sidebarWidth, ofGetHeight());
    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::drawWaveTableTexture() {

    textureWaveTable.loadData(buffWaveTablePixels);
    textureWaveTable.draw(0, 0, waveTableScaledSize.x, waveTableScaledSize.y);

}


//--------------------------------------------------------------
void ofApp::drawWaveTableScope() {

    ofPushStyle();
    ofSetColor(20);
    ofDrawRectangle(0, 0, waveTableScaledSize.x, waveTableScaledSize.y);
    ofPopStyle();

    ofPushStyle();
    ofSetColor(0, 255, 0, 255);
    int _rectSize = 2;

    for (int w = 0; w < waveTableSize.y; w++) {
        ofDrawRectangle(w / defaultImgWindowSizeRatio, ofMap(waveTable[w], -1, 1, 0, waveTableSize.y - _rectSize) / defaultImgWindowSizeRatio, _rectSize, _rectSize);
    }

    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::drawPlayZoneTexture() {

    float _playZoneW = sidebarWidth;
    float _playZoneH = (playZoneSize.y / playZoneSize.x) * sidebarWidth;

    texturePlayZone.loadData(buffPlayZonePixels);
    texturePlayZone.draw(0, 0, _playZoneW, _playZoneH);

}


//--------------------------------------------------------------
void ofApp::drawFramerate() {

    ofPushStyle();
    ofSetColor(255);
    ofDrawBitmapString("FPS         : " + ofToString(ofGetFrameRate(), 1), 10, 15);
    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::drawInfo() {

    ofPushStyle();
    ofSetColor(255);
    ofDrawBitmapString("Play / Stop : Space Bar", 10, 30);
    ofDrawBitmapString("Open File   : key 'o'", 10, 45);
    ofPopStyle();

}


//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer) {

    if (bPlaying) {
        for (int i = 0; i < buffer.getNumFrames(); i += 2) {
            float output = 0;
            float remainder = 0;

            for (int n = 0; n < BIT; n++) {
                phase[n] += 512. / 44100 * frequency[n];

                if (phase[n] >= 511) {
                    phase[n] -= 512;
                }
                if (phase[n] < 0) {
                    phase[n] = 0;
                }

                remainder = phase[n] - floor(phase[n]);
                output += ((1 - remainder) * waveTable[1 + (long)phase[n]] + remainder * waveTable[2 + (long)phase[n]]) * amplitude[n];
            }

            output /= 20.0;

            if (output > 1.0) {
                output = 1.0;
            }
            if (output < -1.0) {
                output = -1.0;
            }

            float _volume = volumeSlider;
            buffer[i * buffer.getNumChannels()] = output * _volume;
            buffer[i * buffer.getNumChannels() + 1] = output * _volume;
        }
    } else {
        for (int i = 0; i < buffer.getNumFrames(); i += 2) {
            buffer[i * buffer.getNumChannels()] = 0;
            buffer[i * buffer.getNumChannels() + 1] = 0;
        }
    }

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

}


//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

    if (key == ' ') {
        bPlaying = !bPlaying;
    }

    if (!bPlaying) {
        playPosition = 0;
        ofSoundStreamStop();
    } else {
        ofSoundStreamStart();
    }

    if (key == 'o' || key == 'O') {
        bPlaying = false;
        openFile();
    }

}


//--------------------------------------------------------------
void ofApp::openFile() {

    ofFileDialogResult openFileResult = ofSystemLoadDialog("Select a jpg or png");

    if (openFileResult.bSuccess) {
        ofLogVerbose("User selected a file");
        processOpenFileSelection(openFileResult);
    } else {
        ofLogVerbose("User hit cancel");
    }

}


//--------------------------------------------------------------
void ofApp::processOpenFileSelection(ofFileDialogResult openFileResult) {

    ofLogVerbose("getName(): "  + openFileResult.getName());
    ofLogVerbose("getPath(): "  + openFileResult.getPath());

    ofFile file(openFileResult.getPath());

    if (file.exists()) {
        ofLogVerbose("The file exists - now checking the type via file extension");
        string fileExtension = ofToUpper(file.getExtension());

        if (fileExtension == "JPG" || fileExtension == "PNG" || fileExtension == "JPEG") {
            loadProcess(openFileResult.getPath());
        } else {
            ofSystemAlertDialog("Use a jpg, png or jpeg format.");
        }
    }

}


//--------------------------------------------------------------
void ofApp::loadProcess(string _path) {

    ofImage _image;
    _image = loadedImageResize(_path);

    ofFbo _fbo = loadedImageFBO(_image);
    _fbo.readToPixels(_image);
    defaultImg.setFromPixels(_image);

    captureWaveTable();
    capturePlayZone();

}


//--------------------------------------------------------------
ofImage ofApp::loadedImageResize(string _parh) {

    ofImage _image;
    _image.load(_parh);

    float _whRatio = _image.getWidth() / _image.getHeight();
    float _imgScreenSizeRatioW = 4032 / _image.getWidth();
    float _imgScreenSizeRatioH = 3024 / _image.getHeight();

    if (_whRatio > 1) {
        _image.resize(4032, _image.getHeight() * _imgScreenSizeRatioW);
    } else if (_whRatio < 1) {
        _image.resize(_image.getWidth() * _imgScreenSizeRatioH, 3024);
    } else if (_whRatio == 1) {
        _image.resize(3024, 3024);
    }

    return _image;

}


//--------------------------------------------------------------
ofFbo ofApp::loadedImageFBO(ofImage _img) {

    ofFbo _fboBuffBackground;
    _fboBuffBackground.allocate(4032, 3024);
    _fboBuffBackground.begin();
    ofClear(255, 0);
    _img.draw(0, 0);
    _fboBuffBackground.end();

    return _fboBuffBackground;

}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

    if (waveTableZone.inside(x, y)) {
        mouseOnWaveTableZone = true;
    } else {
        mouseOnWaveTableZone = false;
    }

    if (!mouseOnWaveTableZone && playZone.inside(x, y)) {
        mouseOnPlayZone = true;
    } else {
        mouseOnPlayZone = false;
    }

}


//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

    if (mouseOnWaveTableZone) {
        ofVec2f _vW = limitMouseDrag(x, y, waveTableZone, waveTableClickPos);
        waveTableZone.setPosition(_vW.x, _vW.y);
    }

    if (mouseOnPlayZone) {
        ofVec2f _vP = limitMouseDrag(x, y, playZone, playZoneclickPos);
        playZone.setPosition(_vP.x, _vP.y);
    }

}


//--------------------------------------------------------------
ofVec2f ofApp::limitMouseDrag(int x, int y, ofRectangle _r, ofVec2f _clickPos) {

    ofVec2f _pos = ofVec2f(x - _clickPos.x, y - _clickPos.y);

    if (_pos.x < 0) {
        _pos = ofVec2f(0, _pos.y);
    } else if (_pos.x + _r.getWidth() > sidebarPosX) {
        _pos = ofVec2f(sidebarPosX - _r.getWidth(), _pos.y);
    }

    if (_pos.y < 0) {
        _pos = ofVec2f(_pos.x, 0);
    } else if (_pos.y + _r.getHeight() > ofGetHeight()) {
        _pos = ofVec2f(_pos.x, ofGetHeight() - _r.getHeight());
    }

    return _pos;
    _r.setPosition(_pos.x, _pos.y);

}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

    if (mouseOnWaveTableZone) {
        waveTableClickPos = ofVec2f(x, y) - waveTableZone.getPosition();
    }

    if (mouseOnPlayZone) {
        playZoneclickPos = ofVec2f(x, y) - playZone.getPosition();
    }

}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

    if (waveTableZone.inside(x, y)) {
        mouseOnWaveTableZone = false;
        captureWaveTable();
    }

    if (playZone.inside(x, y)) {
        mouseOnPlayZone = false;
        playPosition = 0;
        capturePlayZone();
    }

}


//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}


//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}


//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
