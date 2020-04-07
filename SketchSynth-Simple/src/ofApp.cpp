#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup() {

#ifdef __APPLE_CC__
#ifdef DEBUG
#else
    ofSetDataPathRoot("../Resources/data");
#endif
#endif

    ofSetFrameRate(60);
    ofBackground(0);

    sidebarPosX = ofGetHeight() * 4 / 3;
    sidebarWidth = ofGetWidth() - sidebarPosX;

    playZoneSize.set(1600, 514);
    waveTableSize.set(514, 514);

    defaultDeviceID = 0;
    
    setAudio(setAudioDevice());

    setGui(audioInputDeviceList, defaultDeviceID);

    setupImage();

    defaultImgWindowSizeRatio = defaultImg.getWidth() / sidebarPosX;

    playZoneScaledSize = playZoneSize / defaultImgWindowSizeRatio;
    waveTableScaledSize = waveTableSize / defaultImgWindowSizeRatio;

    playZone.set(97, 62, playZoneScaledSize.x, playZoneScaledSize.y);
    setPlayZoneBuffer(playZone, playZoneSize);

    waveTable.set(677, 96, waveTableScaledSize.x, waveTableScaledSize.x);
    setWaveTableBuffer(waveTable, waveTableSize);

    captureWaveTable();
    capturePlayZone();

    playPosition = 0;

    mouseOnWaveTable = false;
    mouseOnPlayZone = false;
    
    setPlayZoneFourCorner();
    
}


//--------------------------------------------------------------
void ofApp::openFileButtonPressed() {

    openFile();

}


//--------------------------------------------------------------
ofSoundStreamSettings ofApp::setAudioDevice() {

    ofSoundStreamSettings _settingsOuput;
    vector<ofSoundDevice> _buffDev = soundStream.getDeviceList();

    for (int i = 0; i < _buffDev.size(); i++) {
        if (_buffDev.at(i).outputChannels > 0) {
            audioInputDeviceList.push_back(_buffDev.at(i));
        }
    }

#ifdef __APPLE_CC__

    for (int i = 0; i < audioInputDeviceList.size(); i++) {
        if (audioInputDeviceList.at(i).name == "Apple Inc.: Built-in Output") {
            defaultDeviceID = i;
            defaultInputDevice = audioInputDeviceList.at(i);
        }
	}

#else

    defaultDeviceID = 0;
    defaultInputDevice = audioInputDeviceList.at(0);

#endif

    _settingsOuput.setOutDevice(defaultInputDevice);
    return _settingsOuput;
    
}


//--------------------------------------------------------------
void ofApp::setAudio(ofSoundStreamSettings settings) {
    
    int _chNum = defaultInputDevice.outputChannels;
    settings.setOutListener(this);
    settings.bufferSize = 512;
    settings.sampleRate = 44100;
    settings.numInputChannels = 0;
    settings.numOutputChannels = _chNum;
    soundStream.setup(settings);

}


//--------------------------------------------------------------
void ofApp::deviceIDChanged(int & _deviceID){
        
    defaultInputDevice = audioInputDeviceList.at(_deviceID);

    settings.setOutDevice(defaultInputDevice);

    setAudio(settings);

    string _name = defaultInputDevice.name;
    int _i = _name.find(':');
    _name.erase(_name.begin(), _name.begin() + _i + 2);

    deviceNum.setName(_name);

}


//--------------------------------------------------------------
void ofApp::setGui(vector<ofSoundDevice> _d, int _id) {
    
    ofAddListener(ofEvents().mouseReleased, this, &ofApp::mouseReleasedEvent, OF_EVENT_ORDER_BEFORE_APP);
    
    openFileButton.addListener(this, &ofApp::openFileButtonPressed);
    guiAudioDevice.addListener(this, &ofApp::deviceIDChanged);

    gui.setup();
//    gui.setDefaultWidth(sidebarWidth * 0.8);
    gui.setPosition(sidebarPosX, 60);
//    gui.add(openFileButton.setup("File Open"));
    gui.add(thresholdPlayZone.setup("PlayZone Contrast", 0.52, 0, 1));
    gui.add(thresholdWaveTable.setup("WaveTable Contrast", 0.515, 0, 1));
    gui.add(invertColorToggle.setup("Invert colors", false));
    gui.add(speed.setup("Speed", 2, 0.1, 10));
    gui.add(overlap.setup("Overlap", 4, 0.1, 20));
    gui.add(hifq.setup("Hi fq", 5280, 800, 12000));
    gui.add(lofq.setup("LO fq", 50, 20, 700));
    gui.add(volumeSlider.setup("Volume", 0.85, 0.0, 1.0));
    
	intDropdown = make_unique<ofxIntDropdown>(guiAudioDevice);

#ifdef __APPLE_CC__

    for (int i = 0; i < _d.size(); i++) {
        string _name = _d.at(i).name;
        int _i = _name.find(':');
        _name.erase(_name.begin(), _name.begin() + _i + 2);
        intDropdown->add(i, _name);
    }

#else

	for (int i = 0; i < _d.size(); i++) {
		string _name = _d.at(i).name;
		intDropdown->add(i, _name.substr(0, 17).append("..."));
	}

#endif

    intDropdown->disableMultipleSelection();
    intDropdown->setDropDownPosition(intDropdown->DD_BELOW);
    guiAudioDevice.set(0);
    gui.add(intDropdown.get());
    
}


//--------------------------------------------------------------
void ofApp::setupImage() {

    defaultImg.load("c03.jpeg"); // Default image size : 4032 x 3024
    defaultImg.setImageType(OF_IMAGE_COLOR_ALPHA);

}


//--------------------------------------------------------------
void ofApp::setWaveTableBuffer(ofRectangle waveTable, ofVec2f waveTableSize) {

    buffWaveTablePixels.allocate(waveTableSize.x, waveTableSize.y, OF_PIXELS_GRAY);
    textureWaveTable.allocate(buffWaveTablePixels);

    waveTableChar = new unsigned char[int(waveTableSize.x * waveTableSize.y) * 4];
    buffWaveTableChar = new unsigned char[int(waveTableSize.x * waveTableSize.y) * 4];
    
}


//--------------------------------------------------------------
void ofApp::setPlayZoneBuffer(ofRectangle playZone, ofVec2f playZoneSize) {
    
    buffPlayZonePixels.allocate(playZoneSize.x, playZoneSize.y, OF_PIXELS_GRAY);
    texturePlayZone.allocate(buffPlayZonePixels);

    playZoneChar = new unsigned char[int(playZoneSize.x * playZoneSize.y) * 4];
    buffPlayZoneChar = new unsigned char[int(playZoneSize.x * playZoneSize.y) * 4];

}


//--------------------------------------------------------------
void ofApp::captureWaveTable() {

    ofPixels _p = cropImageToPixels(waveTable, waveTableSize);
    captureChar(_p, waveTableChar, buffWaveTableChar);

    buffWaveTablePixels = captureWaveTablePixels(waveTableChar, buffWaveTableChar);

}


//--------------------------------------------------------------
void ofApp::capturePlayZone() {

    ofPixels _p = cropImageToPixels(playZone, playZoneSize);
    captureChar(_p, playZoneChar, buffPlayZoneChar);

    buffPlayZonePixels = capturePlayZonePixels(playZoneChar, buffPlayZoneChar);

}


//--------------------------------------------------------------
void ofApp::setPlayZoneFourCorner() {
    
    mouseOnPlayZoneRightBottom = false;
    playZoneRightBottom.setFromCenter(playZone.getX() + playZone.getWidth(), playZone.getY() + playZone.getHeight(), 20, 20);

    mouseOnPlayZoneRightTop = false;
    playZoneRightTop.setFromCenter(playZone.getX() + playZone.getWidth(), playZone.getY(), 20, 20);

    mouseOnPlayZoneLeftTop = false;
    playZoneLeftTop.setFromCenter(playZone.getX(), playZone.getY(), 20, 20);

    mouseOnPlayZoneLeftBottom = false;
    playZoneLeftBottom.setFromCenter(playZone.getX(), playZone.getY() + playZone.getHeight(), 20, 20);

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
void ofApp::update() {

    if (bPlaying) {
        playPosition += speed;

        generatAmpFq(playPosition);

        if (playPosition > playZoneSize.x) {
            playPosition = 0;
        }
    }
    
    buffWaveTablePixels = captureWaveTablePixels(waveTableChar, buffWaveTableChar);
//    buffPlayZonePixels = capturePlayZonePixels(playZoneChar, buffPlayZoneChar);

}


//--------------------------------------------------------------
void ofApp::generatAmpFq(float _plaPos) {

    for (int n = 0; n < BIT; n++) {
        amplitude[n] = (amplitude[n] * overlap + getAmp(floor(_plaPos), n)) / (overlap + 1);
        //            frequency[n] = int(ofMap(n, 0, BIT, hifq, lofq));
        frequency[n] = int(getFreq(n));
    }

}


//--------------------------------------------------------------
float ofApp::getAmp(float _xPos, float _yPos) {

    float _amp = 0;

    if (_yPos > 0 && _yPos < BIT) {
        float _yRatio = playZoneSize.y / BIT;
        _amp = buffPlayZonePixels.getColor(_xPos, _yPos * _yRatio).getLightness() / 255.0;
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

            waveTableOsc[i] = ofMap(_height, 0, waveTableSize.y, -1, 1);
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

    drawZone(mouseOnWaveTable, waveTable, ofColor(255, 0, 0, 255));
    drawZone(mouseOnPlayZone, playZone, ofColor(0, 255, 0, 255));

    drawPlaySpectrum();

    displaySidebar();

    gui.draw();
    
    drawFourZone();
    
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
        int _yStep = 10 / (playZoneSize.y / BIT);
        
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
        ofDrawLine(_x, _y, _x, _y + playZone.getHeight());

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
void ofApp::drawFourZone() {
    
    if (mouseOnPlayZoneRightBottom) {
        drawCirclePart(playZoneRightBottom);
        drawArrowPart(playZoneRightBottom, 0);
        drawArrowPart(playZoneRightBottom, 90);
    }

    if (mouseOnPlayZoneRightTop) {
        drawCirclePart(playZoneRightTop);
        drawArrowPart(playZoneRightTop, 0);
        drawArrowPart(playZoneRightTop, -90);
    }

    if (mouseOnPlayZoneLeftTop) {
        drawCirclePart(playZoneLeftTop);
        drawArrowPart(playZoneLeftTop, 180);
        drawArrowPart(playZoneLeftTop, -90);
    }

    if (mouseOnPlayZoneLeftBottom) {
        drawCirclePart(playZoneLeftBottom);
        drawArrowPart(playZoneLeftBottom, 180);
        drawArrowPart(playZoneLeftBottom, 90);
    }

}


//--------------------------------------------------------------
void ofApp::drawCirclePart(ofRectangle rect) {
 
    ofPushMatrix();
    ofPushStyle();

    ofSetLineWidth(2);

    ofSetColor(50, 140, 50, 20);
    ofDrawCircle(rect.getCenter(), 10);

    ofNoFill();
    ofSetColor(50, 140, 50, 180);
    ofDrawCircle(rect.getCenter(), 10);

    ofPopStyle();
    ofPopMatrix();

}


//--------------------------------------------------------------
void ofApp::drawArrowPart(ofRectangle rect, float rotation) {

    ofPushMatrix();
    ofPushStyle();
    
    ofTranslate(rect.getCenter().x, rect.getCenter().y);
    ofRotateZDeg(rotation);

    ofSetColor(50, 140, 50, 255);
    
    ofVec3f _sR = ofVec3f(10, 0);
    ofVec3f _eR = ofVec3f(20, 0);
    ofDrawArrow(_sR, _eR, 3);
    
    ofPopStyle();
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

    edgeRectangle(ofColor(255, 120), waveTableScaledSize);
    
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
        ofDrawRectangle(w / defaultImgWindowSizeRatio, ofMap(waveTableOsc[w], -1, 1, 0, waveTableSize.y - _rectSize) / defaultImgWindowSizeRatio, _rectSize, _rectSize);
    }

    ofPopStyle();
    
    edgeRectangle(ofColor(255, 120), waveTableScaledSize);
    
}


//--------------------------------------------------------------
void ofApp::drawPlayZoneTexture() {

    float _playZoneW = sidebarWidth;
    float _playZoneH = (playZoneSize.y / playZoneSize.x) * sidebarWidth;

    texturePlayZone.loadData(buffPlayZonePixels);
    
    float _w = texturePlayZone.getWidth();
    float _h = texturePlayZone.getHeight();
    
    if (_w > sidebarWidth) {
        _w = sidebarWidth;
        _h = (playZoneSize.y / playZoneSize.x) * sidebarWidth;
    }

    float _texturePosY = gui.getHeight() + gui.getPosition().y + 4 + waveTableScaledSize.y + 4 + waveTableScaledSize.y + 4;
    if (_h > ofGetHeight() - _texturePosY) {
        _h = ofGetHeight() - _texturePosY;
        _w = (playZoneSize.x / playZoneSize.y) * (ofGetHeight() - _texturePosY);
    }
    
    texturePlayZone.draw(0, 0, _w, _h);

    edgeRectangle(ofColor(255, 120), ofVec2f(_w, _h));

}


//--------------------------------------------------------------
void ofApp::edgeRectangle(ofColor _c, ofVec2f _v) {
    
    ofPushStyle();
    ofNoFill();
    ofSetColor(_c);
    ofDrawRectangle(0, 0, _v.x, _v.y);
    ofPopStyle();

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
                output += ((1 - remainder) * waveTableOsc[1 + (long)phase[n]] + remainder * waveTableOsc[2 + (long)phase[n]]) * amplitude[n];
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

    if (waveTable.inside(x, y)) {
        mouseOnWaveTable = true;
    } else {
        mouseOnWaveTable = false;
    }
    
    if (!mouseOnWaveTable && playZone.inside(x, y)) {
        mouseOnPlayZone = true;
    } else {
        mouseOnPlayZone = false;
    }
    
    if (playZoneLeftTop.inside(x, y)) {
        mouseOnPlayZoneLeftTop = true;
        playZoneWaveTableMouseOff();
    } else {
        mouseOnPlayZoneLeftTop = false;
    }

    if (playZoneLeftBottom.inside(x, y)) {
        mouseOnPlayZoneLeftBottom = true;
        playZoneWaveTableMouseOff();
    } else {
        mouseOnPlayZoneLeftBottom = false;
    }

    if (playZoneRightTop.inside(x, y)) {
        mouseOnPlayZoneRightTop = true;
        playZoneWaveTableMouseOff();
    } else {
        mouseOnPlayZoneRightTop = false;
    }

    if (playZoneRightBottom.inside(x, y)) {
        mouseOnPlayZoneRightBottom = true;
        playZoneWaveTableMouseOff();
    } else {
        mouseOnPlayZoneRightBottom = false;
    }

}


//--------------------------------------------------------------
void ofApp::playZoneWaveTableMouseOff() {
    
    mouseOnWaveTable = false;
    mouseOnPlayZone = false;

}


//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

    if (mouseOnWaveTable) {
        ofVec2f _vW = limitMouseDrag(x, y, waveTable, waveTableClickPos);
        waveTable.setPosition(_vW.x, _vW.y);
    }

    if (mouseOnPlayZone) {
        ofVec2f _vP = limitMouseDrag(x, y, playZone, playZoneclickPos);
        playZone.setPosition(_vP.x, _vP.y);
        
        setAllFourCorner(_vP);
    }
            
    if (x < 20) {
        x = 20;
    }
    if (x > sidebarPosX - 20) {
        x = sidebarPosX - 20;
    }
    if (y < 20) {
        y = 20;
    }
    if (y > ofGetHeight() - 20) {
        y = ofGetHeight() - 20;
    }

    if (mouseOnPlayZoneLeftTop) {
        allResetByLeftTop(setSmallSizeOffsetLeftTop(x, y));
    }
    
    if (mouseOnPlayZoneLeftBottom) {
        allResetByLeftBottom(setSmallSizeOffsetLeftBottom(x, y));
    }
    
    if (mouseOnPlayZoneRightTop && x < sidebarPosX && y > 0) {
        allResetByRightTop(setSmallSizeOffsetRightTop(x, y));
    }
    
    if (mouseOnPlayZoneRightBottom && x < sidebarPosX && y < ofGetHeight()) {
        allResetByRightBottom(setSmallSizeOffsetRightBottom(x, y));
    }

}


//--------------------------------------------------------------
void ofApp::setAllFourCorner(ofVec2f _vP) {
    
    float _xLT = _vP.x;
    float _yLT = _vP.y;
    playZoneLeftTop.setFromCenter(_xLT, _yLT, 20, 20);

    float _xLB = _vP.x;
    float _yLB = _vP.y + playZone.getHeight();
    playZoneLeftBottom.setFromCenter(_xLB, _yLB, 20, 20);

    float _xRT = _vP.x + playZone.getWidth();
    float _yRT = _vP.y;
    playZoneRightTop.setFromCenter(_xRT, _yRT, 20, 20);

    float _xRB = _vP.x + playZone.getWidth();
    float _yRB = _vP.y + playZone.getHeight();
    playZoneRightBottom.setFromCenter(_xRB, _yRB, 20, 20);
    
}


//--------------------------------------------------------------
ofVec2f ofApp::setSmallSizeOffsetLeftTop(float x, float y) {
    
    float _x;
    float _y;
    
    float _offsetX = playZone.getPosition().x + playZone.getWidth() - 100 / defaultImgWindowSizeRatio;
    if (x >= _offsetX) {
        _x = _offsetX;
    } else {
        _x = x;
    }

    float _offsetY = playZone.getPosition().y + playZone.getHeight() - 100 / defaultImgWindowSizeRatio;
    if (y >= _offsetY) {
        _y = _offsetY;
    } else {
        _y = y;
    }

    return ofVec2f(_x, _y);
    
}


//--------------------------------------------------------------
ofVec2f ofApp::setSmallSizeOffsetLeftBottom(float x, float y) {

    float _x;
    float _y;
    
    float _offsetX = playZone.getPosition().x + playZone.getWidth() - 100 / defaultImgWindowSizeRatio;
    if (x >= _offsetX) {
        _x = _offsetX;
    } else {
        _x = x;
    }

    float _offsetY = playZone.getPosition().y + 100 / defaultImgWindowSizeRatio;
    if (y <= _offsetY) {
        _y = _offsetY;
    } else {
        _y = y;
    }

    return ofVec2f(_x, _y);
    
}


//--------------------------------------------------------------
ofVec2f ofApp::setSmallSizeOffsetRightTop(float x, float y) {

    float _x;
    float _y;
    
    float _offsetX = playZone.getPosition().x + 100 / defaultImgWindowSizeRatio;;
    if (x <= _offsetX) {
        _x = _offsetX;
    } else {
        _x = x;
    }

    float _offsetY = playZone.getPosition().y + playZone.getHeight() - 100 / defaultImgWindowSizeRatio;
    if (y >= _offsetY) {
        _y = _offsetY;
    } else {
        _y = y;
    }

    return ofVec2f(_x, _y);
    
}


//--------------------------------------------------------------
ofVec2f ofApp::setSmallSizeOffsetRightBottom(float x, float y) {

    float _x;
    float _y;
    
    float _offsetX = playZone.getPosition().x + 100 / defaultImgWindowSizeRatio;
    if (x <= _offsetX) {
        _x = _offsetX;
    } else {
        _x = x;
    }

    float _offsetY = playZone.getPosition().y + 100 / defaultImgWindowSizeRatio;
    if (y <= _offsetY) {
        _y = _offsetY;
    } else {
        _y = y;
    }

    return ofVec2f(_x, _y);
    
}



//--------------------------------------------------------------
void ofApp::allResetByLeftTop(ofVec2f _v) {
    float x = _v.x;
    float y = _v.y;
    
    playZoneLeftTop.setFromCenter(x, y, 20, 20);
    playZoneLeftBottom.setFromCenter(x, y + playZone.getHeight(), 20, 20);
    playZoneRightTop.setFromCenter(x + playZone.getWidth(), y, 20, 20);
    playZoneRightBottom.setFromCenter(x + playZone.getWidth(), y + playZone.getHeight(), 20, 20);

    playZone.set(x, y, playZone.getPosition().x + playZone.getWidth() - x, playZone.getHeight() + playZone.getPosition().y - y);
    
}


//--------------------------------------------------------------
void ofApp::allResetByLeftBottom(ofVec2f _v) {

    float x = _v.x;
    float y = _v.y;

    playZoneLeftTop.setFromCenter(x, y - playZone.getHeight(), 20, 20);
    playZoneLeftBottom.setFromCenter(x, y, 20, 20);
    playZoneRightBottom.setFromCenter(x + playZone.getWidth(), y, 20, 20);
    playZoneRightTop.setFromCenter(x + playZone.getWidth(), y - playZone.getHeight(), 20, 20);

    playZone.set(x, playZone.getPosition().y, playZone.getPosition().x + playZone.getWidth() - x, y - playZone.getPosition().y);
    
}


//--------------------------------------------------------------
void ofApp::allResetByRightTop(ofVec2f _v) {
    
    float x = _v.x;
    float y = _v.y;
    
    playZoneLeftTop.setFromCenter(x - playZone.getWidth(), y, 20, 20);
    playZoneLeftBottom.setFromCenter(x - playZone.getWidth(), y + playZone.getHeight(), 20, 20);
    playZoneRightTop.setFromCenter(x, y, 20, 20);
    playZoneRightBottom.setFromCenter(x, y + playZone.getHeight(), 20, 20);

    playZone.set(playZone.getPosition().x, y, x - playZone.getPosition().x, playZone.getHeight() + playZone.getPosition().y - y);
    
}


//--------------------------------------------------------------
void ofApp::allResetByRightBottom(ofVec2f _v) {
    
    float x = _v.x;
    float y = _v.y;
    
    playZoneLeftTop.setFromCenter(x - playZone.getWidth(), y - playZone.getHeight(), 20, 20);
    playZoneLeftBottom.setFromCenter(x - playZone.getWidth(), y, 20, 20);
    playZoneRightTop.setFromCenter(x, y - playZone.getHeight(), 20, 20);
    playZoneRightBottom.setFromCenter(x, y, 20, 20);

    playZone.set(playZone.getPosition(), x - playZone.getPosition().x, y - playZone.getPosition().y);
    
}


//--------------------------------------------------------------
ofVec2f ofApp::limitMouseDrag(int x, int y, ofRectangle _r, ofVec2f _clickPos) {

    ofVec2f _pos = ofVec2f(x - _clickPos.x, y - _clickPos.y);

    if (_pos.x < 20) {
        _pos = ofVec2f(20, _pos.y);
    } else if (_pos.x + _r.getWidth() > sidebarPosX - 20) {
        _pos = ofVec2f(sidebarPosX - 20 - _r.getWidth(), _pos.y);
    }

    if (_pos.y < 20) {
        _pos = ofVec2f(_pos.x, 20);
    } else if (_pos.y + _r.getHeight() > ofGetHeight() - 20) {
        _pos = ofVec2f(_pos.x, ofGetHeight() - 20 - _r.getHeight());
    }

    return _pos;
    _r.setPosition(_pos.x, _pos.y);

}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

    if (mouseOnWaveTable) {
        waveTableClickPos = ofVec2f(x, y) - waveTable.getPosition();
    }

    if (mouseOnPlayZone) {
        playZoneclickPos = ofVec2f(x, y) - playZone.getPosition();
    }

    if (playZoneLeftTop.inside(x, y)) {
        mouseOnPlayZoneLeftTop = true;
    }

    if (playZoneLeftBottom.inside(x, y)) {
        mouseOnPlayZoneLeftBottom = true;
    }

    if (playZoneRightTop.inside(x, y)) {
        mouseOnPlayZoneRightTop = true;
    }

    if (playZoneRightBottom.inside(x, y)) {
        mouseOnPlayZoneRightBottom = true;
    }

}


//--------------------------------------------------------------
void ofApp::mouseReleasedEvent(ofMouseEventArgs & mouse){
    
    if (thresholdPlayZone.mouseReleased(mouse)) {
        capturePlayZone();
    }

//    if (thresholdWaveTable.mouseReleased(mouse)) {
//        captureWaveTable();
//    }

}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
            
    if (x < 20) {
        x = 20;
    }
    if (x > sidebarPosX - 20) {
        x = sidebarPosX - 20;
    }
    if (y < 20) {
        y = 20;
    }
    if (y > ofGetHeight() - 20) {
        y = ofGetHeight() - 20;
    }

    if (waveTable.inside(x, y)) {
        mouseOnWaveTable = false;
        captureWaveTable();
    }

    if ((x <= 20 || y <= 20 || x >= sidebarPosX - 20 || y >= ofGetHeight() - 20) && mouseOnPlayZone) {
        mouseOnPlayZone = false;
        playPosition = 0;
        capturePlayZone();
    }
    if (playZone.inside(x, y) && mouseOnPlayZone) {
        mouseOnPlayZone = false;
        playPosition = 0;
        capturePlayZone();
    }

    bool _checkIn = x <= 20 || y <= 20 || x >= sidebarPosX - 20 || y >= ofGetHeight() - 20;
    
    if (mouseOnPlayZoneLeftTop) {
        mouseOnPlayZoneLeftTop = false;
        playPosition = 0;
        allResetByLeftTop(setSmallSizeOffsetLeftTop(x, y));
        resetPlayZoneAll();
    }
    
    if (mouseOnPlayZoneLeftBottom) {
        mouseOnPlayZoneLeftBottom = false;
        playPosition = 0;
        allResetByLeftBottom(setSmallSizeOffsetLeftBottom(x, y));
        resetPlayZoneAll();
    }
    
    if (mouseOnPlayZoneRightTop) {
        mouseOnPlayZoneRightTop = false;
        playPosition = 0;
        allResetByRightTop(setSmallSizeOffsetRightTop(x, y));
        resetPlayZoneAll();
    }
    
    if (mouseOnPlayZoneRightBottom) {
        mouseOnPlayZoneRightBottom = false;
        playPosition = 0;
        allResetByRightBottom(setSmallSizeOffsetRightBottom(x, y));
        resetPlayZoneAll();
    }

}


//--------------------------------------------------------------
void ofApp::resetPlayZoneAll() {
    
    playZoneSize.set(playZone.getWidth() * defaultImgWindowSizeRatio, playZone.getHeight() * defaultImgWindowSizeRatio);
    setPlayZoneBuffer(playZone, playZoneSize);
    capturePlayZone();

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
