// charcter encoding is UTF-8

#include "ofApp.h"
#ifdef TARGET_WIN32
#include <sapi.h>
#include <atlcomcli.h>
#endif /* TARGET_WIN32 */

/* ---------- variables ---------- */

// system
int camCheckCount;
int tvpScene;
ofxXmlSettings xmlSettings, xmlCamProfFpv, xmlPilots;
bool sysStatEnabled;
// view
ofVideoGrabber grabber[CAMERA_MAXNUM];
ofColor myColorYellow, myColorWhite, myColorLGray, myColorDGray, myColorAlert;
ofColor myColorBGDark, myColorBGMiddle, myColorBGLight;
ofxTrueTypeFontUC myFontNumber, myFontLabel, myFontLap, myFontLapHist;
ofxTrueTypeFontUC myFontNumberSub, myFontLabelSub, myFontLapSub;
ofxTrueTypeFontUC myFontInfo1m, myFontInfo1p, myFontInfo3m;
ofImage logoLargeImage, logoSmallImage;
ofImage bttnFscrImage, bttnQuitImage, bttnSettImage, bttnWndwImage;
ofImage wallImage;
string wallPath;
float wallRatio;
int wallDrawWidth;
int wallDrawHeight;
tvpCamProf camProfFpvExtra;
tvpCamView camView[CAMERA_MAXNUM];
int cameraNum;
int cameraNumVisible;
int cameraIdxSolo;
bool cameraTrimEnabled;
bool fullscreenEnabled;
bool cameraLapHistEnabled;
bool cameraFrameEnabled;
int hideCursorTimer;
// osc
ofxOscReceiver oscReceiver;
// speech
bool speechLangJpn;
// AR lap timer
ofSoundPlayer beepSound, beep3Sound, notifySound, cancelSound;
ofSoundPlayer countSound, finishSound;
ofFile resultsFile;
int arLapMode;
bool raceStarted;
int raceDuraSecs;
int nextNotifyRemainSecs;
int raceDuraLaps;
int minLapTime;
float elapsedTime;
bool useStartGate;
int raceResultTimer;
bool frameTick;
bool lapAfterTmoEnabled;
// overlay
ofxTrueTypeFontUC myFontOvlayP, myFontOvlayP2x, myFontOvlayM;
int overlayMode;
int raceResultPage;
int ovlayMsgTimer;
string ovlayMsgString;
// QR code reader
bool qrEnabled;
int qrUpdCount;
int qrCamIndex;
// gamepad
ofxJoystick gamePad[GPAD_MAX_DEVS];

//--------------------------------------------------------------
void setupInit() {
    // system
    ofSetEscapeQuitsApp(false);
    ofDirectory dir;
    if (dir.doesDirectoryExist("../data") == false) {
        // macOS binary release
        ofSetDataPathRoot("../Resources/data");
    }
    sysStatEnabled = DFLT_SYS_STAT;
    // scene
    tvpScene = SCENE_INIT;
    ofResetElapsedTimeCounter();
    // screen
    ofSetWindowTitle("Tiny View Plus");
    ofBackground(0, 0, 0);
    ofSetVerticalSync(VERTICAL_SYNC);
    ofSetFrameRate(FRAME_RATE);
    myFontNumber.load(FONT_P_FILE, NUMBER_HEIGHT);
    myFontLabel.load(FONT_P_FILE, LABEL_HEIGHT);
    myFontLap.load(FONT_P_FILE, LAP_HEIGHT);
    myFontLapHist.load(FONT_P_FILE, LAPHIST_HEIGHT);
    myFontNumberSub.load(FONT_P_FILE, NUMBER_HEIGHT / 2);
    myFontLabelSub.load(FONT_P_FILE, LABEL_HEIGHT / 2);
    myFontLapSub.load(FONT_P_FILE, LAP_HEIGHT / 2);
    myFontInfo1m.load(FONT_M_FILE, INFO_HEIGHT);
    myFontInfo1p.load(FONT_P_FILE, INFO_HEIGHT);
    myFontInfo3m.load(FONT_M_FILE, INFO_HEIGHT * 3);
    loadOverlayFont();
    cameraTrimEnabled = DFLT_CAM_TRIM;
    fullscreenEnabled = DFLT_FSCR_ENBLD;
    cameraLapHistEnabled = DFLT_CAM_LAPHST;
    cameraFrameEnabled = DFLT_CAM_FRAMED;
    // splash
    logoLargeImage.load(LOGO_LARGE_FILE);
    // wallpaper
    wallPath = DFLT_WALL_FILE;
    // logo
    logoSmallImage.load(LOGO_SMALL_FILE);
    // button
    bttnFscrImage.load(BTTN_FSCR_FILE);
    bttnQuitImage.load(BTTN_QUIT_FILE);
    bttnSettImage.load(BTTN_SETT_FILE);
    bttnWndwImage.load(BTTN_WNDW_FILE);
    // view common
    setupColors();
    hideCursorTimer = HIDECUR_TIME;
    // overlay
    setOverlayMode(OVLMODE_NONE);
    initOverlayMessage();
    raceResultPage = 0;
    // osc
    oscReceiver.setup(OSC_LISTEN_PORT);
    // gamepad (GPAD_MAX_DEVS: 4)
    gamePad[0].setup(GLFW_JOYSTICK_1);
    gamePad[1].setup(GLFW_JOYSTICK_2);
    gamePad[2].setup(GLFW_JOYSTICK_3);
    gamePad[3].setup(GLFW_JOYSTICK_4);
    // AR lap timer
    arLapMode = DFLT_ARAP_MODE;
    minLapTime = DFLT_ARAP_MNLAP;
    raceDuraSecs = DFLT_ARAP_RSECS;
    nextNotifyRemainSecs = -1;
    lapAfterTmoEnabled = DFLT_ARAP_LAPTO;
    raceDuraLaps = DFLT_ARAP_RLAPS;
    useStartGate = DFLT_ARAP_SGATE;
    beepSound.load(SND_BEEP_FILE);
    beep3Sound.load(SND_BEEP3_FILE);
    countSound.load(SND_COUNT_FILE);
    finishSound.load(SND_FINISH_FILE);
    notifySound.load(SND_NOTIFY_FILE);
    cancelSound.load(SND_CANCEL_FILE);
    raceStarted = false;
    elapsedTime = 0;
    raceResultTimer = -1;
    // QR reader
    qrEnabled = false;
    // speech
    autoSelectSpeechLang();
    // extra camera
    camProfFpvExtra.enabled = false;
}

//--------------------------------------------------------------
void loadWallImage(string path) {
    wallImage.clear();
    if (wallImage.load(path) == false) {
        if (tvpScene == SCENE_INIT) {
            ofSystemAlertDialog("Background image not found: " + path);
        }
        wallPath = DFLT_WALL_FILE;
        wallImage.load(wallPath);
    }
    wallRatio = wallImage.getWidth() / wallImage.getHeight();
    setWallParams();
}

//--------------------------------------------------------------
void loadSettingsFile() {
    xmlSettings.loadFile(SETTINGS_FILE);

    // SYSTEM
    // speech language
    speechLangJpn = xmlSettings.getValue(SNM_SYS_SPCLANG, speechLangJpn);
    // system statistics
    sysStatEnabled = xmlSettings.getValue(SNM_SYS_STAT, sysStatEnabled);

    // VIEW
    // fullscreen
    fullscreenEnabled = xmlSettings.getValue(SNM_VIEW_FLLSCR, fullscreenEnabled);
    // wallpaper
    wallPath = xmlSettings.getValue(SNM_VIEW_BGIMG, wallPath);
    // camera view trimming
    cameraTrimEnabled = xmlSettings.getValue(SNM_VIEW_CAMTRM, cameraTrimEnabled);
    // camera frame visibility
    cameraFrameEnabled = xmlSettings.getValue(SNM_VIEW_CAMFRM, cameraFrameEnabled);

    // RACE
    // AR lap timer mode
    arLapMode = xmlSettings.getValue(SNM_RACE_ARMODE, arLapMode);
    // race duration (time, laps)
    raceDuraSecs = xmlSettings.getValue(SNM_RACE_DRSECS, raceDuraSecs);
    raceDuraLaps = xmlSettings.getValue(SNM_RACE_DRLAPS, raceDuraLaps);
    lapAfterTmoEnabled = xmlSettings.getValue(SNM_RACE_LAPTMO, lapAfterTmoEnabled);
    // minimum lap time
    minLapTime = xmlSettings.getValue(SNM_RACE_MINLAP, minLapTime);
    // staggered start
    useStartGate = xmlSettings.getValue(SNM_RACE_STAGGR, useStartGate);
    // lap history view
    cameraLapHistEnabled = xmlSettings.getValue(SNM_RACE_DISPLH, cameraLapHistEnabled);
}

void saveSettingsFile() {
    // SYSTEM
    // speech language
    xmlSettings.setValue(SNM_SYS_SPCLANG, speechLangJpn);
    // system statistics
    xmlSettings.setValue(SNM_SYS_STAT, sysStatEnabled);

    // VIEW
    // fullscreen
    xmlSettings.setValue(SNM_VIEW_FLLSCR, fullscreenEnabled);
    // wallpaper
    xmlSettings.setValue(SNM_VIEW_BGIMG, wallPath);
    // camera view trimming
    xmlSettings.setValue(SNM_VIEW_CAMTRM, cameraTrimEnabled);
    // camera frame visibility
    xmlSettings.setValue(SNM_VIEW_CAMFRM, cameraFrameEnabled);

    // RACE
    // AR lap timer mode
    xmlSettings.setValue(SNM_RACE_ARMODE, arLapMode);
    // race duration (time, laps)
    xmlSettings.setValue(SNM_RACE_DRSECS, raceDuraSecs);
    xmlSettings.setValue(SNM_RACE_DRLAPS, raceDuraLaps);
    xmlSettings.setValue(SNM_RACE_LAPTMO, lapAfterTmoEnabled);
    // minimum lap time
    xmlSettings.setValue(SNM_RACE_MINLAP, minLapTime);
    // staggered start
    xmlSettings.setValue(SNM_RACE_STAGGR, useStartGate);
    // lap history view
    xmlSettings.setValue(SNM_RACE_DISPLH, cameraLapHistEnabled);

    xmlSettings.saveFile(SETTINGS_FILE);
}

void loadPilotsFile() {
    xmlPilots.loadFile(PILOTS_FILE);
    for (int i = 0; i < cameraNum; i++) {
      string caption = xmlPilots.getValue(PLT_PILOT_LABEL+to_string(i), "");
      if (caption != "") {
          camView[i].labelString = caption;
          autoSelectCameraIcon(i + 1, caption);
      }
    }
}

void savePilotsFile() {
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].labelString != "Pilot" +to_string(i + 1)) xmlPilots.setValue(PLT_PILOT_LABEL+to_string(i), camView[i].labelString);
    }
    xmlPilots.saveFile(PILOTS_FILE);
}

//--------------------------------------------------------------
void loadCameraProfileFile() {
    if (xmlCamProfFpv.loadFile(CAM_FPV_FILE) == false) {
        return;
    }
    tvpCamProf *p = &camProfFpvExtra;
    ofxXmlSettings *s = &xmlCamProfFpv;
    // load
    p->enabled = true;
    p->name = s->getValue(CFNM_NAME, "tvp-no-named-camera");
    p->grabW = s->getValue(CFNM_GRAB_W, CAMERA_WIDTH);
    p->grabH = s->getValue(CFNM_GRAB_H, CAMERA_HEIGHT);
    p->cropX = s->getValue(CFNM_CROP_X, 0);
    p->cropY = s->getValue(CFNM_CROP_Y, 0);
    p->cropW = s->getValue(CFNM_CROP_W, CAMERA_WIDTH);
    p->cropH = s->getValue(CFNM_CROP_H, CAMERA_HEIGHT);
    p->drawAspr = s->getValue(CFNM_DRAW_ASPR, "4:3");
    // crop?
    if (p->cropX == 0 && p->cropY == 0 && p->cropW == p->grabW && p->cropH == p->grabH) {
        p->needCrop = false;
    } else {
        p->needCrop = true;
    }
    // resize?
    if ((p->needCrop == true && p->cropW == CAMERA_WIDTH && p->cropH == CAMERA_HEIGHT)
        || (p->needCrop == false && p->grabW == CAMERA_WIDTH && p->grabH && CAMERA_HEIGHT)) {
        p->needResize = false;
    } else {
        p->needResize = true;
    }
    // wide?
    p->isWide = (p->drawAspr == "16:9");
}

//--------------------------------------------------------------
void setupCamCheck() {
    tvpScene = SCENE_CAMS;
    cameraNum = 0;
    camCheckCount = 0;
    reloadCameras();
}

//--------------------------------------------------------------
void reloadCameras() {
    // clear
    for (int i = 0; i < cameraNum; i++) {
        grabber[i].close();
    }
    // load
    ofVideoGrabber tmpgrb;
    vector<ofVideoDevice> devices = tmpgrb.listDevices();
    tvpCamProf* prof = &camProfFpvExtra;
    int cidx = 0;
    cameraNum = 0;
    ofLog() << "Scanning camera... " << devices.size() << " devices";
    for (size_t i = 0; i < devices.size(); i++) {
        int w, h, aw, ah;
        bool extra = false;
        if (prof->enabled == true && regex_search(devices[i].deviceName, regex(prof->name)) == true) {
            extra = true;
        }
        if (regex_search(devices[i].deviceName, regex("USB2.0 PC CAMERA")) == false && extra == false) {
            continue;
        }
        if (devices[i].bAvailable == false) {
            continue;
        }
        grabber[cidx].setDeviceID(devices[i].id);
        if (extra == true) {
            w = prof->grabW;
            h = prof->grabH;
        }
        else {
            w = CAMERA_WIDTH;
            h = CAMERA_HEIGHT;
        }
        if (grabber[cidx].initGrabber(w, h) == false) {
            continue;
        }
        if (extra == true) {
            camView[cidx].needCrop = prof->needCrop;
            camView[cidx].needResize = prof->needResize;
            camView[cidx].isWide = prof->isWide;
        }
        else {
            camView[cidx].needCrop = false;
            camView[cidx].needResize = false;
            camView[cidx].isWide = false;
        }
        aw = grabber[cidx].getWidth();
        ah = grabber[cidx].getHeight();
        ofLog() << "[" << devices[i].id << "] " << devices[i].deviceName;
        ofLog() << "  preferred resolution: " << w << " x " << h;
        ofLog() << "  actual resolution: " << aw << " x " << ah;
        if (extra == true) {
            ofLog() << "  crop: "
                << prof->cropX << ", " << prof->cropY << ", "
                << prof->cropW << ", " << prof->cropH;
        }
        cidx++;
        cameraNum++;
        if (cameraNum == CAMERA_MAXNUM) {
            break;
        }
    }
    tmpgrb.close();
}

//--------------------------------------------------------------
void setupMain() {
    // system
    ofSetFullscreen(fullscreenEnabled);
    tvpScene = SCENE_MAIN;
    // camera
    cameraIdxSolo = -1;
    cameraNumVisible = cameraNum;
    for (int i = 0; i < cameraNum; i++) {
        camView[i].visible = true;
        camView[i].iconImage.load(DFLT_ICON_FILE);
        camView[i].labelString = "Pilot" + ofToString(i + 1);
    }
    loadPilotsFile();
    setViewParams();
    for (int i = 0; i < cameraNum; i++) {
        camView[i].moveSteps = 1;
    }
    // AR laptimer
    for (int i = 0; i < cameraNum; i++) {
        camView[i].aruco.setUseHighlyReliableMarker(ARAP_MKR_FILE);
        camView[i].aruco.setThreaded(true);
        camView[i].aruco.setup2d(CAMERA_WIDTH, CAMERA_HEIGHT);
    }
    initRaceVars();
    // speech
    if (speechLangJpn == true) {
#ifdef TARGET_OSX
        speakAny("jp", "タイニービュープラスへ、ようこそ。");
#else /* TARGET_OSX */
        speakAny("jp", "タイニービュープラスへようこそ。");
#endif /* TARGET_WIN32 TARGET_LINUX */
    } else {
        speakAny("en", "Welcome to Tiny View Plus.");
    }
    // debug
    if (DEBUG_ENABLED == true) {
        generateDummyData();
        fwriteRaceResult();
    }
}

//--------------------------------------------------------------
void ofApp::setup() {
    setupInit();
    loadSettingsFile();
    loadCameraProfileFile();
    loadWallImage(wallPath);
    saveSettingsFile();
}

//--------------------------------------------------------------
void updateInit() {
    if (ofGetElapsedTimeMillis() >= 3000) {
        setupCamCheck();
    }
}

//--------------------------------------------------------------
void updateCamCheck() {
    for (int i = 0; i < cameraNum; i++) {
        grabber[i].update();
    }
    if (camCheckCount > 180) {
        reloadCameras();
        camCheckCount = 0;
        return;
    }
    camCheckCount++;
}

//--------------------------------------------------------------
void ofApp::update() {
    // scene
    if (tvpScene == SCENE_INIT) {
        updateInit();
        return;
    } else if (tvpScene == SCENE_CAMS) {
        updateCamCheck();
        return;
    }
    // timer
    if (raceStarted == true) {
        elapsedTime = ofGetElapsedTimef();
        checkGamePad(elapsedTime);
        // time limited race
        if (raceDuraSecs > 0) {
            float relp = elapsedTime - WATCH_COUNT_SEC;
            // speak remaining time
            if (nextNotifyRemainSecs >= 0 && raceDuraSecs - relp <= nextNotifyRemainSecs) {
                if (nextNotifyRemainSecs > 0 || lapAfterTmoEnabled == true) {
                    if (nextNotifyRemainSecs > 5) notifySound.play();
                    speakRemainTime(nextNotifyRemainSecs);
                }
                setNextNotifyRemainSecs(nextNotifyRemainSecs);
            }
            // finish race by time
            // (do not wait for lap after time limit)
            if (lapAfterTmoEnabled == false && relp >= raceDuraSecs) {
                for (int i = 0; i < cameraNum; i++) {
                    grabberUpdateResize(i);
                    camView[i].foundMarkerNum = 0;
                    camView[i].foundValidMarkerNum = 0;
                    camView[i].enoughMarkers = false;
                }
                stopRace(false);
                recvOsc();
                updateViewParams();
                return;
            }
        }
    }
    // camera
    for (int i = 0; i < cameraNum; i++) {
        grabberUpdateResize(i);
    }
    // QR reader
    if (qrEnabled == true) {
        processQrReader();
    }
    // lap
    frameTick = !frameTick;
    for (int i = 0; i < cameraNum; i++) {
        if (raceStarted == false || elapsedTime < WATCH_COUNT_SEC) {
            continue;
        }
        // AR lap timer
        float elp = elapsedTime;
        if (frameTick == true && arLapMode != ARAP_MODE_OFF) {
            if (camView[i].needCrop == true || camView[i].needResize == true) {
                camView[i].aruco.detectMarkers(camView[i].resizedPixels);
            }
            else {
                camView[i].aruco.detectMarkers(grabber[i].getPixels());
            }
            // all markers
            int anum = camView[i].aruco.getNumMarkers();
            if (anum == 0 && camView[i].foundMarkerNum > 0) {
                camView[i].flickerCount++;
                if (camView[i].flickerCount <= 3) {
                    anum = camView[i].foundMarkerNum; // anti flicker
                } else {
                    camView[i].flickerCount = 0;
                }
            } else {
                camView[i].flickerCount = 0;
            }
            // vaild markers
            int vnum = camView[i].aruco.getNumMarkersValidGate();
            if (vnum == 0 && camView[i].foundValidMarkerNum > 0) {
                camView[i].flickerValidCount++;
                if (camView[i].flickerValidCount <= 3) {
                    vnum = camView[i].foundValidMarkerNum; // anti flicker
                } else {
                    camView[i].flickerValidCount = 0;
                }
            } else {
                camView[i].flickerValidCount = 0;
            }
            // passed gate
            if (anum == 0 && camView[i].enoughMarkers == true
                && ((arLapMode == ARAP_MODE_LOOSE) || (arLapMode == ARAP_MODE_NORM && camView[i].foundMarkerNum == camView[i].foundValidMarkerNum))) {
                float lap = elp - camView[i].prevElapsedSec;
                if (camView[i].totalLaps >= (raceDuraLaps + (useStartGate == true ? 1 : 0))
                    || (raceDuraSecs > 0 && (camView[i].prevElapsedSec - WATCH_COUNT_SEC) >= raceDuraSecs)) {
                    // already finished
                    continue;
                }
                if ((useStartGate == true && camView[i].totalLaps > 0 && lap < minLapTime)
                    || (useStartGate == false && lap < minLapTime)
                    || lap < 0) {
                    // ignore short/negative lap
                    camView[i].foundMarkerNum = 0;
                    camView[i].foundValidMarkerNum = 0;
                    camView[i].enoughMarkers = false;
                    continue;
                }
                // record
                int total = camView[i].totalLaps + 1;
                camView[i].prevElapsedSec = elp;
                camView[i].totalLaps = total;
                camView[i].lastLapTime = lap;
                camView[i].lapHistName[total - 1] = camView[i].labelString;
                camView[i].lapHistLapTime[total - 1] = lap;
                camView[i].lapHistElpTime[total - 1] = elp;
                updateRacePositions();
                if (total >= (raceDuraLaps + (useStartGate == true ? 1 : 0))
                    || (raceDuraSecs > 0 && (elp - WATCH_COUNT_SEC) >= raceDuraSecs)) {
                    // finish by laps / time
                    camView[i].foundMarkerNum = 0;
                    camView[i].foundValidMarkerNum = 0;
                    camView[i].enoughMarkers = false;
                    finishSound.play();
                    continue;
                }
                if (total == ((raceDuraLaps + (useStartGate == true ? 1 : 0)) - 1)) {
                    beep3Sound.play();
                } else {
                    beepSound.play();
                }
                speakLap((i + 1), lap, total);
            }
            camView[i].foundMarkerNum = anum;
            camView[i].foundValidMarkerNum = vnum;
            if (anum == 0) {
                camView[i].enoughMarkers = false;
            } else if ((arLapMode == ARAP_MODE_NORM && vnum >= ARAP_MNUM_THR)
                       || (arLapMode == ARAP_MODE_LOOSE && anum >= ARAP_MNUM_THR)) {
                camView[i].enoughMarkers = true;
            }
        }
    }
    // finish race
    if (raceStarted == true) {
        int i, count;
        bool finish = false;
        // by laps
        count = 0;
        for (i = 0; i < cameraNum; i++) {
            if (camView[i].totalLaps >= (raceDuraLaps + (useStartGate == true ? 1 : 0))) {
                count++;
            }
        }
        if (count == cameraNum) {
            finish = true;
        }
        // by time (wait for lap after time limit)
        if (finish == false && raceDuraSecs > 0 && lapAfterTmoEnabled == true) {
            count = 0;
            for (i = 0; i < cameraNum; i++) {
                if ((camView[i].prevElapsedSec - WATCH_COUNT_SEC) >= raceDuraSecs) {
                    count++;
                }
            }
            if (count == cameraNum) {
                finish = true;
            }
        }
        // finish!
        if (finish == true) {
            stopRace(false);
        }
    }
    // osc
    recvOsc();
    // view
    updateViewParams();
    if (overlayMode == OVLMODE_MSG) {
        if (ovlayMsgTimer <= 0) {
            setOverlayMode(OVLMODE_NONE);
        } else {
            ovlayMsgTimer--;
        }
    }
    // hide cursor timer
    if (hideCursorTimer >= 0) {
        hideCursorTimer--; // stop at -1
        if (hideCursorTimer == 0) {
            ofHideCursor();
        }
    }
    // race result timer
    if (raceResultTimer >= 0) {
        raceResultTimer--; // stop at -1
        if (raceResultTimer == 0) {
            processRaceResultDisplay();
        }
    }
}

//--------------------------------------------------------------
void drawInit() {
    int x = (ofGetWidth() - logoLargeImage.getWidth()) / 2;
    int y = (ofGetHeight() - logoLargeImage.getHeight()) / 2;
    int elpm = ofGetElapsedTimeMillis();
    if (elpm >= 2700) {
        ofSetColor((3000 - elpm) * 255 / 300);
    } else {
        ofSetColor(255);
    }
    logoLargeImage.draw(x, y);
}

//--------------------------------------------------------------
void drawCamCheck() {
    ofxTrueTypeFontUC *font;
    int w, h , x, xoff, y, margin;
    string str;
    bool isalt;
    // common
    w = (ofGetWidth() / 4) - 4;
    h = w / CAMERA_RATIO;
    y = (ofGetHeight() / 2) - (h / 2);
    ofSetColor(255);
    // header
    font = &myFontOvlayP2x;
    margin = font->getLineHeight();
    str = "Camera Setup";
    ofSetColor(myColorYellow);
    font->drawString(str, (ofGetWidth() - font->stringWidth(str)) / 2, y - margin);
    // camera
    ofSetColor(myColorDGray);
    ofFill();
    ofDrawRectangle(-2, y - 2, ofGetWidth() + 4, h + 4);
    if (cameraNum == 0) {
        isalt = true;
        str = "No device";
    } else {
        ofSetColor(myColorWhite);
        xoff = (ofGetWidth() - ((w + 4) * cameraNum)) / 2;
        ofNoFill();
        for (int i = 0; i < cameraNum; i++) {
            x = ((w + 4) * i) + xoff;
            if (grabber[i].isInitialized() == true) {
                grabber[i].draw(x, y, w, h);
            }
            ofDrawRectangle(x, y, w, h);
        }
        isalt = false;
    }
    if (camCheckCount >= 150 || camCheckCount < 30) {
        isalt = true;
        str = "Scanning...";
    }
    ofFill();
    // alert
    if (isalt == true) {
        drawOverlayMessageCore(&myFontLap, str);
    }
    // footer
    font = &myFontOvlayP;
    str = "If all devices found, press any key to continue or Esc key to exit";
    ofSetColor(myColorYellow);
    font->drawString(str, (ofGetWidth() - font->stringWidth(str)) / 2,
                     y + h + margin + font->getLineHeight());
    drawInfo();
}

//--------------------------------------------------------------
void drawCameraImage(int camidx) {
    int i = camidx;
    int x, y, w, h;
    x = camView[i].posX;
    w = camView[i].width;
    if (camView[i].isWide == true) {
        y = camView[i].posYWide;
        h = camView[i].heightWide;
    } else {
        y = camView[i].posY;
        h = camView[i].height;
    }
    if (DEBUG_ENABLED == true && grabber[i].isInitialized() == false) {
        // dummy camera
        ofSetColor(0,19,127);
        ofFill();
        ofDrawRectangle(x, y, w, h);
        return;
    }
    if (camView[i].isWide == true) {
        // background for wide camera
        ofSetColor(0);
        ofFill();
        ofDrawRectangle(x, camView[i].posY, w, camView[i].height);
    }
    ofSetColor(myColorWhite);
    if ((camView[i].needCrop == true || camView[i].needResize == true)
        && camView[i].resizedImage.isAllocated() == true) {
        // wide
        camView[i].resizedImage.draw(x, y, w, h);
    }
    else {
        // normal
        grabber[i].draw(x, y, w, h);
    }
}

//--------------------------------------------------------------
void drawCameraFrame(int camidx) {
    int i = camidx;
    int x0 = max(camView[i].posX, 0);
    int y0 = max(camView[i].posY, 0);
    int x1 = min(camView[i].posX + camView[i].width - 1, ofGetWidth() - 1);
    int y1 = min(camView[i].posY + camView[i].height - 1, ofGetHeight() - 1);
    int vw = x1 - x0 + 1; // camera view width
    int vh = y1 - y0 + 1; // camera view height
    int lw = FRAME_LINEWIDTH; // line width

    ofSetColor(camView[i].baseColor);
    ofFill();
    ofDrawRectangle(x0, y0, vw, lw);          // top
    ofDrawRectangle(x0, y1 - lw + 1, vw, lw); // bottom
    ofDrawRectangle(x0, y0, lw, vh);          // left
    ofDrawRectangle(x1 - lw + 1, y0, lw, vh); // right
}

//--------------------------------------------------------------
void drawCameraARMarker(int idx, bool isSub) {
    int i = idx;
    if (raceStarted == false || arLapMode == ARAP_MODE_OFF) {
        return;
    }
    if (camView[i].totalLaps >= (raceDuraLaps + (useStartGate == true ? 1 : 0))) {
        return; // finished
    }
    // rect
    int tx, ty;
    float sc;
    tx = camView[i].posX;
    sc = camView[i].imageScale;
    if (camView[i].isWide == true) {
        ty = camView[i].posYWide;
    } else {
        ty = camView[i].posY;
    }
    ofPushMatrix();
    ofTranslate(tx, ty);
    ofScale(sc, sc, 1);
    ofSetLineWidth(ARAP_RECT_LINEW);
    camView[i].aruco.draw2dGate(myColorYellow, myColorAlert, false);
    ofPopMatrix();
    if (cameraLapHistEnabled == true) {
        return;
    }
    // meter
    string lv_valid = "";
    string lv_invalid = "";
    int x, y;
    int vnum = camView[i].foundValidMarkerNum;
    int ivnum = camView[i].foundMarkerNum - camView[i].foundValidMarkerNum;
    int offset = (cameraFrameEnabled == true) ? FRAME_LINEWIDTH : 0;
    for (int j = 0; j < vnum; j++) {
        lv_valid += "|";
    }
    for (int j = 0; j < ivnum; j++) {
        lv_invalid += "|";
    }
    x = camView[i].lapPosX + offset;
    y = isSub ? (camView[i].lapPosY + (LAP_HEIGHT / 2) + 5) : (camView[i].lapPosY + LAP_HEIGHT + 10);
    y = y + offset + 10;
    if (vnum > 0) {
        ofSetColor(myColorYellow);
        if (isSub) {
            myFontLapSub.drawString(lv_valid, x, y);
        } else {
            myFontLap.drawString(lv_valid, x, y);
        }
    }
    if (ivnum > 0) {
        ofSetColor(myColorAlert);
        if (isSub) {
            if (vnum > 0) {
                x += 2;
            }
            x = x + myFontLapSub.stringWidth(lv_valid);
            myFontLapSub.drawString(lv_invalid, x, y);
        } else {
            if (vnum > 0) {
                x += 5;
            }
            x = x + myFontLap.stringWidth(lv_valid);
            myFontLap.drawString(lv_invalid, x, y);
        }
    }
}

//--------------------------------------------------------------
void drawCameraPilot(int cidx, bool issub) {
    ofxTrueTypeFontUC *fontnum, *fontlp;
    int icnw, icnh, marg, bgw, offset;

    if (issub == true) {
        fontnum = &myFontNumberSub;
        fontlp = &myFontLabelSub;
        icnw = ICON_WIDTH / 2;
        icnh = ICON_HEIGHT / 2;
        marg = 7;
    } else {
        fontnum = &myFontNumber;
        fontlp = &myFontLabel;
        icnw = ICON_WIDTH;
        icnh = ICON_HEIGHT;
        marg = 15;
    }
    offset = (cameraFrameEnabled == true) ? FRAME_LINEWIDTH : 0;

    // badge
    ofSetColor(camView[cidx].baseColor);
    ofDrawRectangle(camView[cidx].basePosX + offset, camView[cidx].basePosY + offset,
                    camView[cidx].baseWidth, camView[cidx].baseHeight);
    // number
    ofSetColor(myColorWhite);
    fontnum->drawString(ofToString(cidx + 1),
                        camView[cidx].numberPosX + offset, camView[cidx].numberPosY + offset);

    // background
    bgw = icnw;
    if (camView[cidx].labelString != "") {
        bgw = bgw + fontlp->stringWidth(camView[cidx].labelString) + marg;
    }
    ofFill();
    ofSetColor(myColorBGMiddle);
    ofDrawRectangle(camView[cidx].iconPosX + offset, camView[cidx].iconPosY + offset, bgw, icnh);
    // icon
    ofSetColor(myColorWhite);
    camView[cidx].iconImage.draw(camView[cidx].iconPosX + offset, camView[cidx].iconPosY + offset, icnw, icnh);
    // label
    if (camView[cidx].labelString != "") {
        ofSetColor(myColorYellow);
        fontlp->drawString(camView[cidx].labelString,
                           camView[cidx].labelPosX + offset, camView[cidx].labelPosY + offset);
    }

    // position
    if (camView[cidx].moveSteps > 0 || camView[cidx].racePosition == 0) {
        return;
    }
    string str = "";
    int pos = camView[cidx].racePosition;
    int x;
    switch (pos) {
        case 1:
            str = "1st";
            break;
        case 2:
            str = "2nd";
            break;
        case 3:
            str = "3rd";
            break;
        case 4:
            str = "4th";
            break;
    }
    x = min(ofGetWidth(), camView[cidx].posX + camView[cidx].width) - (1 + fontlp->stringWidth(str));
    x = x - (issub ? 5 : 10) - offset;
    drawStringWithShadow(fontlp, myColorWhite, str, x, camView[cidx].labelPosY + offset);
}

//--------------------------------------------------------------
void drawCameraLapTime(int idx, bool issub) {
    string sout;
    int laps, i, offset;

    laps = camView[idx].totalLaps;
    if (laps == 0) {
        return;
    }
    i = idx;
    offset = (cameraFrameEnabled == true) ? FRAME_LINEWIDTH : 0;

    if (raceStarted == false
        || camView[i].totalLaps >= (raceDuraLaps + (useStartGate == true ? 1 : 0))
        || (raceDuraSecs > 0 && (camView[i].prevElapsedSec - WATCH_COUNT_SEC) >= raceDuraSecs)) {
        // race/laps finished
        sout = "Laps: " + ofToString((useStartGate == true && laps > 0) ? laps - 1 : laps);
        if (issub) {
            drawStringWithShadow(&myFontLapSub, myColorWhite, sout,
                                 camView[i].lapPosX + offset, camView[i].lapPosY + offset);
        } else {
            drawStringWithShadow(&myFontLap, myColorWhite, sout,
                                 camView[i].lapPosX, camView[i].lapPosY + offset);
        }
        float blap = getBestLap(i);
        if (blap != 0) {
            sout = "BestLap: " + getLapStr(blap) + "s";
            if (issub) {
                drawStringWithShadow(&myFontLapSub, myColorWhite, sout,
                                     camView[i].lapPosX + offset,
                                     camView[i].lapPosY + offset + (LAP_HEIGHT / 2) + 5);
            } else {
                drawStringWithShadow(&myFontLap, myColorWhite, sout,
                                     camView[i].lapPosX,
                                     camView[i].lapPosY + offset + LAP_HEIGHT + 10);
            }
            sout = "TotalTime: ";
            if (useStartGate == true) {
                sout += getWatchString(camView[i].prevElapsedSec - camView[i].lapHistElpTime[0]);
            } else {
                sout += getWatchString(camView[i].prevElapsedSec - WATCH_COUNT_SEC);
            }
            if (issub) {
                drawStringWithShadow(&myFontLapSub, myColorWhite, sout,
                                     camView[i].lapPosX + offset,
                                     camView[i].lapPosY + offset + LAP_HEIGHT + 10);
            } else {
                drawStringWithShadow(&myFontLap, myColorWhite, sout,
                                     camView[i].lapPosX,
                                     camView[i].lapPosY + offset + (LAP_HEIGHT * 2) + 20);
            }
        }
    } else {
        // not finished
        if (laps == 1 && useStartGate == true) {
            sout = "Started";
        } else {
            sout = "Lap" + ofToString(useStartGate == true ? laps - 1 : laps);
            sout += ": " + getLapStr(camView[i].lastLapTime) + "s";
        }
        if (issub) {
            drawStringWithShadow(&myFontLapSub, myColorWhite, sout,
                                 camView[i].lapPosX + offset, camView[i].lapPosY + offset);
        } else {
            drawStringWithShadow(&myFontLap, myColorWhite, sout,
                                 camView[i].lapPosX, camView[i].lapPosY + offset);
        }
        // history
        if (cameraLapHistEnabled == false || laps < 2 || camView[i].moveSteps > 0) {
            return;
        }
        drawCameraLapHistory(i, issub);
    }
}

//--------------------------------------------------------------
void drawCameraLapHistory(int camidx, bool issub) {
    string text;
    float lap;
    int lapidx = camView[camidx].totalLaps - 2;
    int offset = (cameraFrameEnabled == true) ? FRAME_LINEWIDTH : 0;
    int height = LAPHIST_HEIGHT;
    int marg = LAPHIST_MARGIN;
    int posy = camView[camidx].lapPosY + marg + offset;
    int scry = ofGetHeight() - 1;

    if (issub) {
        height = height / 2;
    }
    for (; lapidx >= 0; lapidx--) {
        posy += height + marg;
        if (posy + marg >= min(camView[camidx].posY + camView[camidx].height, scry) - offset) {
            break;
        }
        lap = camView[camidx].lapHistLapTime[lapidx];
        if (useStartGate == true) {
            if (lapidx == 0) {
                return;
            } else {
                text = ofToString(lapidx);
            }
        } else {
            text = ofToString(lapidx + 1);
        }
        text += ": " + getLapStr(lap) + "s";
        if (issub) {
            drawStringWithShadow(&myFontLapSub, myColorWhite, text, camView[camidx].lapPosX + offset, posy);
        } else {
            drawStringWithShadow(&myFontLapHist, myColorWhite, text, camView[camidx].lapPosX, posy);
        }
    }
}

//--------------------------------------------------------------
void drawCamera(int idx) {
    bool isSub = false;
    if (cameraIdxSolo != -1 && idx != cameraIdxSolo) {
        isSub = true;
    }
    // image
    drawCameraImage(idx);
    // AR marker
    drawCameraARMarker(idx, isSub);
    // frame
    if (cameraFrameEnabled == true) {
        drawCameraFrame(idx);
    }
    // pilot
    drawCameraPilot(idx, isSub);
    // lap time
    drawCameraLapTime(idx, isSub);
}

//--------------------------------------------------------------
string getWatchString(float sec) {
    char buf[16];
    int h, m, s, ss;
    s = (int)(sec) % 60;
    ss = (int)ceil(sec * 100) % 100;
    if (sec >= 3600) {
        h = (int)(sec) / 3600;
        m = ((int)(sec) % 3600) / 60;
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%02d", h, m, s, ss);
    } else {
        m = (int)(sec) / 60;
        snprintf(buf, sizeof(buf), "%02d:%02d.%02d", m, s, ss);
    }
    return ofToString(buf);
}

//--------------------------------------------------------------
void drawWatch() {
    string str;
    if (raceStarted == false) {
        str = "Finished";
    } else if (elapsedTime < 5) {
        str = ofToString(5 - (int)elapsedTime);
    } else if (elapsedTime < 7) {
        str = "Go!";
    } else {
        float sec;
        sec = elapsedTime - WATCH_COUNT_SEC;
        str = getWatchString(sec);
        if (raceDuraSecs > 0) {
            if (raceDuraSecs >= 3600 && sec < 3600) {
                str = "00:" + str;
            }
            str = str + " / " + getWatchString(raceDuraSecs);
        }
    }
    int x = (ofGetWidth() / 2) - (myFontInfo3m.stringWidth(str) / 2);
    x = (int)(x / 10) * 10;
    drawStringWithShadow(&myFontInfo3m, myColorWhite, str, x, ofGetHeight() - 10);
}

//--------------------------------------------------------------
void drawInfo() {
    ofColor *tcolor;
    string str;
    int x, y;
    y = ofGetHeight() - (1 + 4);
    // logo
    if (tvpScene == SCENE_CAMS || overlayMode == OVLMODE_HELP || overlayMode == OVLMODE_RCRSLT) {
        ofSetColor(myColorWhite);
        //logoSmallImage.draw(0, 0);
        tcolor = &myColorWhite;
        // appinfo
        str = "Tiny View Plus " + ofToString(APP_VER);
        drawStringWithShadow(&myFontInfo1p, *tcolor, str, 4, y);
        // date/time
        str = ofGetTimestampString("%F %T");
        x = ofGetWidth() - myFontInfo1m.stringWidth(str);
        x = (int)(x / 5) * 5;
        drawStringWithShadow(&myFontInfo1m, *tcolor, str, x, y);
    }
}

//--------------------------------------------------------------
void drawStringWithShadow(ofxTrueTypeFontUC *font, ofColor color, string str, int x, int y) {
    // shadow
    ofRectangle rect;
    int margin = 4;
    rect = font->getStringBoundingBox(str, x, y);
    rect.width = rect.width + (rect.x - x) + (margin * 2);
    rect.height = rect.height + (margin * 2);
    rect.x = x - margin;
    rect.y = rect.y - margin;
    ofSetColor(myColorBGMiddle);
    ofDrawRectangle(rect);
    // string
    ofSetColor(color);
    font->drawString(str, x, y);
}

//--------------------------------------------------------------
void drawSystemButtons() {
    if (hideCursorTimer < 0) {
        return;
    }
    int x = ofGetWidth() - 1;
    int y = 10;
    ofSetColor(255);
    // quit
    x -= 30;
    bttnQuitImage.draw(x, y);
    // fullscreen/window
    x -= 30;
    if (fullscreenEnabled == true) {
        bttnWndwImage.draw(x, y);
    } else {
        bttnFscrImage.draw(x, y);
    }
    // settings/help
    x -= 30;
    bttnSettImage.draw(x, y);
}

//--------------------------------------------------------------
void ofApp::draw() {
    if (tvpScene == SCENE_INIT) {
        drawInit();
        return;
    } else if (tvpScene == SCENE_CAMS) {
        drawCamCheck();
        return;
    }
    // wallpaper
    ofSetColor(myColorWhite);
    wallImage.draw(0, 0, wallDrawWidth, wallDrawHeight);
    // camera (solo main)
    if (cameraIdxSolo != -1) {
        drawCamera(cameraIdxSolo);
    }
    // camera (solo sub / solo off)
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false || i == cameraIdxSolo) {
            continue;
        }
        drawCamera(i);
    }
    // total time
    if (raceStarted == true || elapsedTime != 0) {
        drawWatch();
    }
    // QR reader
    if (qrEnabled == true) {
        string str = "Scanning QR Code...";
        int x = (ofGetWidth() / 2) - (myFontInfo1p.stringWidth(str) / 2);
        drawStringWithShadow(&myFontInfo1p, myColorYellow, str, x, ofGetHeight() - 10);
    }
    // overlay
    switch (overlayMode) {
        case OVLMODE_HELP:
            drawHelp();
            break;
        case OVLMODE_MSG:
            drawOverlayMessage();
            break;
        case OVLMODE_RCRSLT:
            drawRaceResult(raceResultPage);
            break;
        case OVLMODE_NONE:
            drawSystemButtons();
            break;
        default:
            break;
    }
    // more info
    drawInfo();
    // debug
    if (sysStatEnabled == true) {
        int x = 10;
        int y = 0;
        int h = 15;
        // screen fps
        ofSetColor(myColorYellow);
        ofDrawBitmapString("Screen FPS: " + ofToString(ofGetFrameRate()), x, y += h);
        // AR laptimer
        ofDrawBitmapString("AR Markers/Rects/FPS:", x, y += h);
        for (int i = 0; i < cameraNum; i++) {
            int m, r;
            m = camView[i].aruco.getNumMarkers();
            r = camView[i].aruco.getNumRectangles();
            ofDrawBitmapString("  Cam" + ofToString(i + 1) + ": "
                               + ofToString(m) + "/" + ofToString(r) + "/"
                               + ofToString(camView[i].aruco.getFps()),
                               x, y += h);
        }
    }
}

//--------------------------------------------------------------
void keyPressedOverlayHelp(int key) {
    if (key == 'h' || key == 'H' || ofGetKeyPressed(OF_KEY_ESC)) {
        setOverlayMode(OVLMODE_NONE);
    } else if (key == 'N' || key == 'n'
               || key == 'F' || key == 'f'
               || key == 'T' || key == 't'
               || key == 'E' || key == 'e'
               || key == '1' || key == '2' || key == '3' || key == '4'
               || ofGetKeyPressed(TVP_KEY_ALT)
               || key == 'A' || key == 'a'
               || key == 'D' || key == 'd'
               || key == 'W' || key == 'w'
               || ofGetKeyPressed(OF_KEY_PAGE_UP)
               || ofGetKeyPressed(OF_KEY_PAGE_DOWN)
               || ofGetKeyPressed(OF_KEY_UP)
               || ofGetKeyPressed(OF_KEY_DOWN)
               || ofGetKeyPressed(OF_KEY_LEFT)
               || ofGetKeyPressed(OF_KEY_RIGHT)
               || key == 'G' || key == 'g'
               || key == 'L' || key == 'l'
               || key == 'C' || key == 'c'
               || key == 'S' || key == 's') {
        // stay at help screen
        keyPressedOverlayNone(key);
    } else {
        setOverlayMode(OVLMODE_NONE);
        keyPressedOverlayNone(key);
    }
}

//--------------------------------------------------------------
void keyPressedOverlayMessage(int key) {
    setOverlayMode(OVLMODE_NONE);
    keyPressedOverlayNone(key);
}

//--------------------------------------------------------------
void keyPressedOverlayResult(int key) {
    if (key == 'r' || key == 'R') {
        processRaceResultDisplay();
    } else if (ofGetKeyPressed(OF_KEY_ESC)) {
        raceResultPage = 0;
        setOverlayMode(OVLMODE_NONE);
    }
}

//--------------------------------------------------------------
void keyPressedOverlayNone(int key) {
    if (ofGetKeyPressed(OF_KEY_ESC)) {
        if (fullscreenEnabled == true) {
            toggleFullscreen();
        }
    } else if (ofGetKeyPressed(TVP_KEY_ALT)) {
        if (key == '1') {
            toggleCameraVisibility(1);
        } else if (key == '2') {
            toggleCameraVisibility(2);
        } else if (key == '3') {
            toggleCameraVisibility(3);
        } else if (key == '4') {
            toggleCameraVisibility(4);
        } else if (key == '5' || key == 'z' || key == 'Z') {
            popLapRecord(1);
        } else if (key == '6') {
            popLapRecord(2);
        } else if (key == '7' || key == '/' || key == '?') {
            popLapRecord(3);
        } else if (key == '8') {
            popLapRecord(4);
        }
    } else {
        if (key == '1') {
            toggleCameraSolo(1);
        } else if (key == '2') {
            toggleCameraSolo(2);
        } else if (key == '3') {
            toggleCameraSolo(3);
        } else if (key == '4') {
            toggleCameraSolo(4);
        } else if (key == '5' || key == 'z' || key == 'Z') {
            pushLapRecord(1, ofGetElapsedTimef());
        } else if (key == '6') {
            pushLapRecord(2, ofGetElapsedTimef());
        } else if (key == '7' || key == '/' || key == '?') {
            pushLapRecord(3, ofGetElapsedTimef());
        } else if (key == '8') {
            pushLapRecord(4, ofGetElapsedTimef());
        } else if (key == 'h' || key == 'H') {
            setOverlayMode(OVLMODE_HELP);
        } else if (key == 'i' || key == 'I') {
            initConfig();
        } else if (key == 'n' || key == 'N') {
            toggleSpeechLang();
        } else if (key == 'b' || key == 'B') {
            changeWallImage();
        } else if (key == 'q' || key == 'Q') {
            if (raceStarted == false) {
                toggleQrReader();
            }
        } else if (key == ' ') {
            toggleRace();
        } else if (key == 'r' || key == 'R') {
            if (raceStarted == false) {
                qrEnabled = false;
                processRaceResultDisplay();
            }
        } else if (key == 'c' || key == 'C') {
            if (raceStarted == false) {
                initRaceVars();
                setOverlayMessage("Cleared race result");
            }
        } else if (key == 'a' || key == 'A') {
            toggleARLap();
        } else if (ofGetKeyPressed(OF_KEY_PAGE_UP)) {
            changeMinLap(1);
        } else if (ofGetKeyPressed(OF_KEY_PAGE_DOWN)) {
            changeMinLap(-1);
        } else if (ofGetKeyPressed(OF_KEY_UP)) {
            changeRaceDuration(5);
        } else if (ofGetKeyPressed(OF_KEY_DOWN)) {
            changeRaceDuration(-5);
        } else if (ofGetKeyPressed(OF_KEY_LEFT)) {
            changeRaceLaps(-1);
        } else if (ofGetKeyPressed(OF_KEY_RIGHT)) {
            changeRaceLaps(1);
        } else if (key == 'w' || key == 'W') {
            toggleLapAfterTimeout();
        } else if (key == 'f' || key == 'F') {
            toggleFullscreen();
        } else if (key == 't' || key == 'T') {
            toggleCameraTrim();
        } else if (key == 'e' || key == 'E') {
            toggleCameraFrame();
        } else if (key == 'l' || key == 'L') {
            toggleLapHistory();
        } else if (key == 'g' || key == 'G') {
            toggleUseStartGate();
        } else if (key == 's' || key == 'S') {
            toggleSysStat();
        }
    }
}

//--------------------------------------------------------------
void keyPressedCamCheck() {
    if (ofGetKeyPressed(OF_KEY_ESC)) {
         ofExit();
     } else {
         if (DEBUG_ENABLED == true) {
             cameraNum = CAMERA_MAXNUM;
         }
         if (cameraNum > 0) {
             setupMain();
         }
     }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (tvpScene == SCENE_INIT) {
        setupCamCheck();
        return;
    } else if (tvpScene == SCENE_CAMS) {
        keyPressedCamCheck();
        return;
    }
    raceResultTimer = -1;
    switch (overlayMode) {
        case OVLMODE_HELP:
            keyPressedOverlayHelp(key);
            break;
        case OVLMODE_MSG:
            keyPressedOverlayMessage(key);
            break;
        case OVLMODE_RCRSLT:
            keyPressedOverlayResult(key);
            break;
        case OVLMODE_NONE:
            keyPressedOverlayNone(key);
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    activateCursor();
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    activateCursor();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    activateCursor();
}

//--------------------------------------------------------------
void mouseReleasedOverlayNone(int x, int y, int button) {
    // button
    if (hideCursorTimer > 0 && y >= 10 && y <=29) {
        int xend = ofGetWidth() - 1;
        if (x >= (xend - 30) && x <= (xend -  10)) {
            ofExit();
        } else if (x >= (xend - 60) && x <= (xend -  40)) {
            toggleFullscreen();
        } else if (x >= (xend - 90) && x <= (xend -  70)) {
            setOverlayMode(OVLMODE_HELP);
        }
    }
    // pilot
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false || camView[i].moveSteps > 0) {
            continue;
        }
        // icon
        if (x >= camView[i].iconPosX && x <= (camView[i].iconPosX + ICON_WIDTH)
            && y >= camView[i].iconPosY && y <= (camView[i].iconPosY + ICON_HEIGHT)) {
            changeCameraIcon(i + 1);
        }
        // label
        if (x >= camView[i].labelPosX && x <= (camView[i].posX + camView[i].width)
            && y >= camView[i].posY && y <= (camView[i].iconPosY + ICON_HEIGHT)) {
            changeCameraLabel(i + 1);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    if (tvpScene != SCENE_MAIN) {
        return;
    }
    activateCursor();
    raceResultTimer = -1;
    switch (overlayMode) {
        case OVLMODE_HELP:
            setOverlayMode(OVLMODE_NONE);
            break;
        case OVLMODE_MSG:
            setOverlayMode(OVLMODE_NONE);
            mouseReleasedOverlayNone(x, y, button);
            break;
        case OVLMODE_RCRSLT:
            processRaceResultDisplay();
            break;
        case OVLMODE_NONE:
            mouseReleasedOverlayNone(x, y, button);
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    // wallpaper
    setWallParams();
    // overlay
    loadOverlayFont();
    if (tvpScene != SCENE_MAIN) {
        return;
    }
    // view
    setViewParams();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    if (tvpScene != SCENE_MAIN) {
        return;
    }
    stopRace(true);
    for (int i = 0; i < cameraNum; i++) {
        camView[i].aruco.setThreaded(false);
    }
}

//--------------------------------------------------------------
void grabberUpdateResize(int cidx) {
    tvpCamView *cv = &camView[cidx];
    tvpCamProf *cp = &camProfFpvExtra;
    grabber[cidx].update();
    if (grabber[cidx].isFrameNew() == false
        || (cv->needCrop == false && cv->needResize == false)) {
        return;
    }
    cv->resizedPixels = grabber[cidx].getPixels();
    if (cv->needCrop == true) {
        cv->resizedPixels.crop(cp->cropX, cp->cropY, cp->cropW, cp->cropH);
    }
    if (cv->needResize == true) {
        if (cv->isWide == true) {
            cv->resizedPixels.resize(CAMERA_WIDTH, CAMERA_HEIGHT * 0.75);
        } else {
            cv->resizedPixels.resize(CAMERA_WIDTH, CAMERA_HEIGHT);
        }
    }
    cv->resizedImage.setFromPixels(cv->resizedPixels);
}

//--------------------------------------------------------------
void toggleCameraSolo(int camid) {
    int idx = camid - 1;
    if (cameraNum == 1 || camid < 1 || camid > cameraNum) {
        return;
    }
    if (idx == cameraIdxSolo) {
        cameraIdxSolo = -1;
    } else {
        if (camView[idx].visible == false) {
            toggleCameraVisibility(camid);
        }
        cameraIdxSolo = idx;
    }
    setViewParams();
}

//--------------------------------------------------------------
void enableCameraSolo(int camid) {
    int idx = camid - 1;
    if (cameraNum == 1 || camid < 1 || camid > cameraNum) {
        return;
    }
    if (idx != cameraIdxSolo) {
        if (camView[idx].visible == false) {
            toggleCameraVisibility(camid);
        }
        cameraIdxSolo = idx;
    }
    setViewParams();
}

//--------------------------------------------------------------
void resetCameraSolo() {
    cameraIdxSolo = -1;
    setViewParams();
}

//--------------------------------------------------------------
void toggleCameraVisibility(int camid) {
    int idx = camid - 1;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (camView[idx].visible == true) {
        if (cameraIdxSolo == idx) {
            toggleCameraSolo(camid);
        }
        camView[idx].visible = false;
        cameraNumVisible -= 1;
    } else {
        camView[idx].visible = true;
        cameraNumVisible += 1;
    }
    setViewParams();
}

//--------------------------------------------------------------
int getCameraIdxNthVisibleAll(int nth) {
    int cnt = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == true) {
            cnt++;
            if (cnt == nth) {
                return i;
            }
        }
    }
    return -1;
}

//--------------------------------------------------------------
int getCameraIdxNthVisibleSub(int nth) {
    int cnt = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == true && i != cameraIdxSolo) {
            cnt++;
            if (cnt == nth) {
                return i;
            }
        }
    }
    return -1;
}

//--------------------------------------------------------------
void setupColors() {
    // common
    myColorYellow = ofColor(COLOR_YELLOW);
    myColorWhite = ofColor(COLOR_WHITE);
    myColorLGray = ofColor(COLOR_LGRAY);
    myColorDGray = ofColor(COLOR_DGRAY);
    myColorBGDark = ofColor(COLOR_BG_DARK);
    myColorBGMiddle = ofColor(COLOR_BG_MIDDLE);
    myColorBGLight = ofColor(COLOR_BG_LIGHT);
    myColorAlert = ofColor(COLOR_ALERT);
    // pilot
    for (int i = 0; i < CAMERA_MAXNUM; i++) {
        switch(i) {
            case 0:
                camView[i].baseColor = ofColor(BASE_1_COLOR);
                break;
            case 1:
                camView[i].baseColor = ofColor(BASE_2_COLOR);
                break;
            case 2:
                camView[i].baseColor = ofColor(BASE_3_COLOR);
                break;
            case 3:
                camView[i].baseColor = ofColor(BASE_4_COLOR);
                break;
            default:
                break;
        }
    }
}

//--------------------------------------------------------------
void changeCameraLabel(int camid) {
    string str;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    str = camView[camid - 1].labelString;
#ifdef TARGET_WIN32
    ofSetFullscreen(false);
    str = ansiToUtf8(str);
#endif /* TARGET_WIN32 */
    str = ofTrim(ofSystemTextBoxDialog("Camera" + ofToString(camid) + " label:", str));
    if (str.length() > 0) {
        camView[camid - 1].labelString = str;
        savePilotsFile();
        autoSelectCameraIcon(camid, str);
    }
#ifdef TARGET_WIN32
    ofSetFullscreen(fullscreenEnabled);
#endif/* TARGET_WIN32 */
}

//--------------------------------------------------------------
void changeCameraIcon(int camid) {
    string str;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
#ifdef TARGET_WIN32
    ofSetFullscreen(false);
#endif /* TARGET_WIN32 */
    str = "Camera" + ofToString(camid) + " icon";
    ofFileDialogResult result = ofSystemLoadDialog(str);
    if (result.bSuccess) {
        string path = result.getPath();
        changeCameraIconPath(camid, path, true);
    }
#ifdef TARGET_WIN32
    ofSetFullscreen(fullscreenEnabled);
#endif /* TARGET_WIN32 */
}

//--------------------------------------------------------------
void changeCameraIconPath(int camid, string path, bool synclabel) {
    ofFile file(path);
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    string ext = ofToLower(file.getExtension());
    if (ext == "jpg" || ext == "png" || ext == "bmp") {
        int idx = camid - 1;
        camView[idx].iconImage.clear();
        camView[idx].iconImage.load(path);
        if (synclabel) {
            camView[camid - 1].labelString = ofTrim(file.getBaseName());
            savePilotsFile();
        }
    } else {
        ofSystemAlertDialog("Unsupported file type");
    }
}

//--------------------------------------------------------------
void autoSelectCameraIcon(int camid, string pname) {
    ofFile file;
    string path;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    path = ICON_DIR + pname + ".jpg";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path, false);
        return;
    }
    path = ICON_DIR + pname + ".png";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path, false);
        return;
    }
    path = ICON_DIR + pname + ".bmp";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path, false);
    } else {
        changeCameraIconPath(camid, DFLT_ICON_FILE, false);
    }
}

//--------------------------------------------------------------
void changeWallImage() {
    activateCursor();
#ifdef TARGET_WIN32
    ofSetFullscreen(false);
#endif /* TARGET_WIN32 */
    ofFileDialogResult result = ofSystemLoadDialog("Set background image");
    if (result.bSuccess) {
        string path = result.getPath();
        ofFile file(path);
        string ext = ofToLower(file.getExtension());
        if (ext == "jpg" || ext == "png" || ext == "bmp") {
            wallPath = path;
            loadWallImage(wallPath);
            saveSettingsFile();
        } else {
            ofSystemAlertDialog("Unsupported file type");
        }
    }
#ifdef TARGET_WIN32
    ofSetFullscreen(fullscreenEnabled);
#endif /* TARGET_WIN32 */
}

//--------------------------------------------------------------
void setWallParams() {
    float sratio;
    sratio = float(ofGetWidth()) / float(ofGetHeight());
    if (sratio > wallRatio) {
        wallDrawWidth = ofGetWidth();
        wallDrawHeight = wallDrawWidth / wallRatio;
    } else {
        wallDrawHeight = ofGetHeight();
        wallDrawWidth = wallDrawHeight * wallRatio;
    }
}

//--------------------------------------------------------------
void setViewParams() {
    int idx, i;
    int width = ofGetWidth();
    int height = ofGetHeight();
    float ratio = (float)width / (float)height;
    switch (cameraNumVisible) {
        case 1:
            // 1st visible camera
            idx = getCameraIdxNthVisibleAll(1);
            if (idx == -1) {
                break;
            }
            camView[idx].moveSteps = MOVE_STEPS;
            if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
                // wide-fill, tall-fit
                camView[idx].widthTarget = width;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
            } else {
                // wide-fit, tall-fill
                camView[idx].heightTarget = height;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
            }
            camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
            camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
            break;
        case 2:
            if (cameraIdxSolo == -1) { // solo mode off
                // 1st camera
                idx = getCameraIdxNthVisibleAll(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = (width / 2) - 1;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = (width / 2) - 1;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
            } else { // solo mode on
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                    || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
                    // wide-fill, tall-fit
                    camView[idx].widthTarget = width;
                    camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                } else {
                    // wide-fit, tall-fill
                    camView[idx].heightTarget = height;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                }
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 1st sub camera
                idx = getCameraIdxNthVisibleSub(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            }
            break;
        case 3:
            if (cameraIdxSolo == -1) { // solo mode off
                // 1st camera
                idx = getCameraIdxNthVisibleAll(1);
                camView[idx].moveSteps = MOVE_STEPS;
                if (cameraTrimEnabled == true) {
                    camView[idx].heightTarget = height * 0.55;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                    camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                    camView[idx].posYTarget = 0;
                } else {
                    int cw = (width - 6) / 3;
                    int ch = cw / CAMERA_RATIO;
                    camView[idx].widthTarget = cw;
                    camView[idx].heightTarget = ch;
                    camView[idx].posXTarget = (width / 2) - (cw / 2) - (cw + 2);
                    camView[idx].posYTarget = (height / 2) - (ch / 2);
                }
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                if (cameraTrimEnabled == true) {
                    camView[idx].heightTarget = height * 0.55;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                    camView[idx].posXTarget = 0;
                    camView[idx].posYTarget = height - camView[idx].heightTarget;
                } else {
                    int cw = (width - 6) / 3;
                    int ch = cw / CAMERA_RATIO;
                    camView[idx].widthTarget = cw;
                    camView[idx].heightTarget = ch;
                    camView[idx].posXTarget = (width / 2) - (cw / 2);
                    camView[idx].posYTarget = (height / 2) - (ch / 2);
                }
                // 3rd camera
                idx = getCameraIdxNthVisibleAll(3);
                camView[idx].moveSteps = MOVE_STEPS;
                if (cameraTrimEnabled == true) {
                    camView[idx].heightTarget = height * 0.55;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                    camView[idx].posXTarget = width - camView[idx].widthTarget;
                    camView[idx].posYTarget = height - camView[idx].heightTarget;
                } else {
                    int cw = (width - 6) / 3;
                    int ch = cw / CAMERA_RATIO;
                    camView[idx].widthTarget = cw;
                    camView[idx].heightTarget = ch;
                    camView[idx].posXTarget = (width / 2) - (cw / 2) + (cw + 2);
                    camView[idx].posYTarget = (height / 2) - (ch / 2);
               }
            } else { // solo mode on
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                    || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
                    // wide-fill, tall-fit
                    camView[idx].widthTarget = width;
                    camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                } else {
                    // wide-fit, tall-fill
                    camView[idx].heightTarget = height;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                }
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 1st sub camera
                idx = getCameraIdxNthVisibleSub(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 2nd sub camera
                idx = getCameraIdxNthVisibleSub(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            }
            break;
        case 4:
            if (cameraIdxSolo == -1) { // solo mode off
                // 1st camera
                idx = getCameraIdxNthVisibleAll(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget + 1);
                camView[idx].posYTarget = 0;
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = 0;
                // 3rd camera
                idx = getCameraIdxNthVisibleAll(3);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget + 1);
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 4th camera
                idx = getCameraIdxNthVisibleAll(4);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            } else { // solo mode off
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                    || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
                    // wide-fill, tall-fit
                    camView[idx].widthTarget = width;
                    camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                } else {
                    // wide-fit, tall-fill
                    camView[idx].heightTarget = height;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                }
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 1st sub camera
                idx = getCameraIdxNthVisibleSub(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 2nd sub camera
                idx = getCameraIdxNthVisibleSub(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 3rd sub camera
                idx = getCameraIdxNthVisibleSub(3);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            }
            break;
        default:
            // none
            break;
    }
    for (i = 1; i <= cameraNumVisible; i++) {
        idx = getCameraIdxNthVisibleAll(i);
        if (idx == -1) {
            break;
        }
        camView[idx].basePosXTarget = max(0, camView[idx].posXTarget) + BASE_MARGIN_X;
        camView[idx].basePosYTarget = max(0, camView[idx].posYTarget) + BASE_MARGIN_Y;
        camView[idx].baseWidth = BASE_WIDTH;
        camView[idx].baseHeight = BASE_HEIGHT;
        camView[idx].numberPosXTarget = max(0, camView[idx].posXTarget) + NUMBER_MARGIN_X;
        camView[idx].numberPosYTarget = max(0, camView[idx].posYTarget) + NUMBER_MARGIN_Y;
        camView[idx].iconPosXTarget = max(0, camView[idx].posXTarget) + ICON_MARGIN_X;
        camView[idx].iconPosYTarget = max(0, camView[idx].posYTarget) + ICON_MARGIN_Y;
        camView[idx].labelPosXTarget = max(0, camView[idx].posXTarget) + LABEL_MARGIN_X;
        camView[idx].labelPosYTarget = max(0, camView[idx].posYTarget) + LABEL_MARGIN_Y;
        camView[idx].lapPosXTarget = max(0, camView[idx].posXTarget) + LAP_MARGIN_X;
        camView[idx].lapPosYTarget = max(0, camView[idx].posYTarget) + LAP_MARGIN_Y;
        if (cameraIdxSolo != -1 && idx != cameraIdxSolo) { // sub
            camView[idx].baseWidth = camView[idx].baseWidth / 2;
            camView[idx].baseHeight = camView[idx].baseHeight / 2;
            camView[idx].numberPosXTarget = camView[idx].numberPosXTarget - (NUMBER_MARGIN_X / 2);
            camView[idx].numberPosYTarget = camView[idx].numberPosYTarget - (NUMBER_MARGIN_Y / 2);
            camView[idx].iconPosXTarget = camView[idx].iconPosXTarget - (ICON_MARGIN_X / 2);
            camView[idx].labelPosXTarget = camView[idx].labelPosXTarget - (LABEL_MARGIN_X / 2);
            camView[idx].labelPosYTarget = camView[idx].labelPosYTarget - (LABEL_MARGIN_Y / 2);
            camView[idx].lapPosXTarget = camView[idx].lapPosXTarget - (LAP_MARGIN_X / 2);
            camView[idx].lapPosYTarget = camView[idx].lapPosYTarget - (LAP_MARGIN_Y / 2);
        }
        camView[idx].imageScale = (float)(camView[idx].width) / (float)CAMERA_WIDTH;
        if (camView[idx].isWide == true) {
            camView[idx].posYWideTarget = camView[idx].posYTarget + (camView[idx].heightTarget / 8);
            camView[idx].heightWideTarget = camView[idx].heightTarget * 0.75;
        }
    }
}

//--------------------------------------------------------------
int calcViewParam(int target, int current, int steps) {
    int val, diff;
    if (steps == 0 || target == current) {
        return target;
    }
    if (target > current) {
        diff = (target - current) / steps;
        val = current + diff;
    } else {
        diff = (current - target) / steps;
        val = current - diff;
    }
    return val;
}

//--------------------------------------------------------------
void updateViewParams() {
    int i, idx, steps;
    for (i = 1; i <= cameraNumVisible; i++) {
        // normal view
        idx = getCameraIdxNthVisibleAll(i);
        if (idx == -1) {
            break;
        }
        steps = camView[idx].moveSteps;
        if (steps == 0) {
            continue;
        }
        // camera
        camView[idx].width = calcViewParam(camView[idx].widthTarget, camView[idx].width, steps);
        camView[idx].height = calcViewParam(camView[idx].heightTarget, camView[idx].height, steps);
        camView[idx].posX = calcViewParam(camView[idx].posXTarget, camView[idx].posX, steps);
        camView[idx].posY = calcViewParam(camView[idx].posYTarget, camView[idx].posY, steps);
        camView[idx].imageScale = (float)(camView[idx].width) / (float)CAMERA_WIDTH;
        if (camView[idx].isWide == true) {
            camView[idx].posYWide = camView[idx].posY + (camView[idx].height / 8);
            camView[idx].heightWide = camView[idx].height * 0.75;
        }
        // base
        camView[idx].basePosX = calcViewParam(camView[idx].basePosXTarget, camView[idx].basePosX, steps);
        camView[idx].basePosY = calcViewParam(camView[idx].basePosYTarget, camView[idx].basePosY, steps);
        // number
        camView[idx].numberPosX = calcViewParam(camView[idx].numberPosXTarget, camView[idx].numberPosX, steps);
        camView[idx].numberPosY = calcViewParam(camView[idx].numberPosYTarget, camView[idx].numberPosY, steps);
        // icon
        camView[idx].iconPosX = calcViewParam(camView[idx].iconPosXTarget, camView[idx].iconPosX, steps);
        camView[idx].iconPosY = calcViewParam(camView[idx].iconPosYTarget, camView[idx].iconPosY, steps);
        // label
        camView[idx].labelPosX = calcViewParam(camView[idx].labelPosXTarget, camView[idx].labelPosX, steps);
        camView[idx].labelPosY = calcViewParam(camView[idx].labelPosYTarget, camView[idx].labelPosY, steps);
        // lap
        camView[idx].lapPosX = calcViewParam(camView[idx].lapPosXTarget, camView[idx].lapPosX, steps);
        camView[idx].lapPosY = calcViewParam(camView[idx].lapPosYTarget, camView[idx].lapPosY, steps);
        // finished
        camView[idx].moveSteps--;
    }
}

//--------------------------------------------------------------
void initConfig() {
    int i;
    // system
    autoSelectSpeechLang();
    sysStatEnabled = DFLT_SYS_STAT;
    // wallpaper
    wallPath = DFLT_WALL_FILE;
    loadWallImage(wallPath);
    // camera visibility
    for (i = 0; i < cameraNum; i++) {
        camView[i].visible = true;
    }
    cameraNumVisible = cameraNum;
    cameraIdxSolo = -1;
    setViewParams();
    // camera icon, label, laptime
    for (i = 0; i < cameraNum; i++) {
        camView[i].iconImage.clear();
        camView[i].iconImage.load(DFLT_ICON_FILE);
        camView[i].labelString = "Pilot" + ofToString(i + 1);
        camView[i].lastLapTime = 0;
    }
    // view mode
    cameraTrimEnabled = DFLT_CAM_TRIM;
    fullscreenEnabled = DFLT_FSCR_ENBLD;
    cameraLapHistEnabled = DFLT_CAM_LAPHST;
    cameraFrameEnabled = DFLT_CAM_FRAMED;
    // AR lap timer
    setOverlayMode(OVLMODE_NONE);
    arLapMode = DFLT_ARAP_MODE;
    minLapTime = DFLT_ARAP_MNLAP;
    raceDuraLaps = DFLT_ARAP_RLAPS;
    raceDuraSecs = DFLT_ARAP_RSECS;
    useStartGate = DFLT_ARAP_SGATE;
    nextNotifyRemainSecs = -1;
    raceStarted = false;
    initRaceVars();
    // finish
    xmlPilots.clear();
    savePilotsFile();
    saveSettingsFile();
    setOverlayMessage("Initialized settings");
}

//--------------------------------------------------------------
string getUserLocaleName() {
    string name = "";
#ifdef TARGET_OSX
    FILE *pipe = popen("defaults read -g AppleLocale", "r");
    char buf[8];
    fgets(buf, 8, pipe);
    name = ofToString(buf);
    pclose(pipe);
#endif /* TARGET_OSX */
#ifdef TARGET_WIN32
    name = ofToString(setlocale(LC_ALL, ""));
#endif /* TARGET_WIN32 */
#ifdef TARGET_LINUX
    FILE *pipe = popen("locale|grep LANG=", "r");
    char buf[11];
    fgets(buf, sizeof(buf), pipe);
    name = ofToString(buf);
    pclose(pipe);
#endif /* TARGET_LINUX */
    return name;
}

//--------------------------------------------------------------
void toggleSysStat() {
    sysStatEnabled = !sysStatEnabled;
    saveSettingsFile();
}

//--------------------------------------------------------------
void recvOsc() {
    while (oscReceiver.hasWaitingMessages() == true) {
        ofxOscMessage oscm;
        string addr;
        string method;
        oscReceiver.getNextMessage(oscm);
        addr = oscm.getAddress();
        if (addr.find("/v1/camera/") == 0) {
            string str;
            int camid;
            int argtype;
            if (addr.length() <= 14 || addr[12] != '/') {
                continue;
            }
            // camera id
            str = ofToString(addr[11]);
            camid = ofToInt(str);
            // method
            method = ofToString(&addr[13]);
            // argument
            if (oscm.getNumArgs() != 1) {
                continue;
            }
            argtype = oscm.getArgType(0);
            switch (argtype) {
                case OFXOSC_TYPE_STRING:
                    recvOscCameraString(camid, method, oscm.getArgAsString(0));
                    break;
                case OFXOSC_TYPE_FLOAT:
                    recvOscCameraFloat(camid, method, oscm.getArgAsFloat(0));
                    break;
                case OFXOSC_TYPE_INT32:
                case OFXOSC_TYPE_INT64:
                    recvOscCameraFloat(camid, method, oscm.getArgAsFloat(0));
                    break;
                default:
                    break;
            }
        }
        else if (addr.find("/v1/speech/") == 0) {
            if (oscm.getNumArgs() != 1 || oscm.getArgType(0) != OFXOSC_TYPE_STRING) {
                continue;
            }
            if (addr == "/v1/speech/en/say") {
                recvOscSpeech("en", oscm.getArgAsString(0));
            }
            else if (addr == "/v1/speech/jp/say") {
                recvOscSpeech("jp", oscm.getArgAsString(0));
            }
        }
    }
}

//--------------------------------------------------------------
void recvOscCameraString(int camid, string method, string argstr) {
    ofLogNotice() << "osc cam(s): " << method << "," << camid << "," << argstr;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (method == "solo") {
        if (cameraIdxSolo != (camid - 1) && argstr == "on") {
            toggleCameraSolo(camid);
        }
        if (cameraIdxSolo == (camid - 1) && argstr == "off") {
            toggleCameraSolo(camid);
        }
    } else if (method == "display") {
        if (argstr == "on" && camView[camid - 1].visible == false) {
            toggleCameraVisibility(camid);
        }
        if (argstr == "off" && camView[camid - 1].visible == true) {
            toggleCameraVisibility(camid);
        }
    }
    else if (method == "label") {
        string str = argstr;
#ifdef TARGET_WIN32
        str = utf8ToAnsi(str);
#endif /* TARGET_WIN32 */
        camView[camid - 1].labelString = str;
        autoSelectCameraIcon(camid, str);
    }
}

//--------------------------------------------------------------
void recvOscCameraFloat(int camid, string method, float argfloat) {
    ofLogNotice() << "osc cam(f): " << method << "," << argfloat;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (method == "laptime") {
        if (argfloat <= 0) {
            return;
        }
        if (camView[camid].totalLaps <= 0) {
            pushLapRecord(camid, WATCH_COUNT_SEC + argfloat);
        } else {
            pushLapRecord(camid, camView[camid].prevElapsedSec + argfloat);
        }
        return;
    }
}

//--------------------------------------------------------------
void toggleSpeechLang() {
    speechLangJpn = !speechLangJpn;
    if (speechLangJpn == true) {
        setOverlayMessage("Speech Language: Japanese");
    } else {
        setOverlayMessage("Speech Language: English");
    }
    saveSettingsFile();
}

//--------------------------------------------------------------
void autoSelectSpeechLang() {
    string lname = getUserLocaleName();
    bool isjpn = true;
    if (lname.find("ja_JP") == std::string::npos
        && lname.find("932") == std::string::npos
        && ofToLower(lname).find("japan") == std::string::npos) {
        isjpn = false;
    }
    speechLangJpn = isjpn;
}

//--------------------------------------------------------------
void recvOscSpeech(string lang, string text) {
    ofLogNotice() << "osc spc(s): " << lang << "," << text;
    speakAny(lang, text);
}

#ifdef TARGET_WIN32
HRESULT cpVicehr = E_NOINTERFACE;
CComPtr<ISpVoice> cpVoice;

class sayWin : public ofThread {
public:
    void exec(string lang, string words) {
        if (FAILED(cpVicehr)) {
            cpVicehr = cpVoice.CoCreateInstance(CLSID_SpVoice);
        }
        if (SUCCEEDED(cpVicehr)) {
            if (lang == "en") {
                // 409: English
                this->xmltext = "<xml><lang langid=\"409\"><rate speed=\"2\">" + words + "</rate></lang></xml>";
            }
            else if (lang == "jp") {
                // 411: Japanese
                this->xmltext = "<xml><lang langid=\"411\"><rate speed=\"2\">" + words + "</rate></lang></xml>";
            }
            startThread();
        }
    }
private:
    void threadedFunction() {
        int iBufferSize = ::MultiByteToWideChar(CP_ACP, 0, xmltext.c_str(), -1, (wchar_t *)NULL, 0);
        wchar_t* wpBufWString = (wchar_t*)new wchar_t[iBufferSize];
        ::MultiByteToWideChar(CP_ACP, 0, xmltext.c_str(), -1, wpBufWString, iBufferSize);
        cpVoice->Speak(wpBufWString, SPF_DEFAULT, NULL);
        delete[] wpBufWString;
    }
    string xmltext;
};

sayWin mySayWin[SPCH_SLOT_NUM];
#endif /* TARAGET_WIN32 */

//--------------------------------------------------------------
void speakLap(int camid, float sec, int num) {
    if (camid < 1 || camid > cameraNum || sec == 0.0) {
        return;
    }
    string ssec, sout;
    sout = camView[camid - 1].labelString + ", ";
    if (speechLangJpn == true) {
        sout = regex_replace(sout, regex("(Pilot)(\\d)"), "パイロット $2");
    } else {
        sout = regex_replace(sout, regex("(Pilot)(\\d)"), "$1 $2");
    }
    if (useStartGate == true && num == 1) {
        if (speechLangJpn == true) {
            sout += "スタート";
        } else {
            sout += "started";
        }
        speakAny(speechLangJpn ? "jp" : "en", sout);
        return;
    }
    ssec = getLapStr(sec);
    if (num > 0) {
        sout += (speechLangJpn == true) ? "ラップ" : "lap";
        sout += " " + ofToString(num - (useStartGate == true ? 1 : 0)) + ", ";
    }
    if (speechLangJpn == true) {
        sout += ssec.substr(0, ssec.length() - 3) + "秒";
        sout += ssec.substr(ssec.length() - 2, 1) + " ";
        sout += ssec.substr(ssec.length() - 1, 1);
    }
    else {
        sout += ssec + " seconds";
    }
    speakAny(speechLangJpn ? "jp" : "en", sout);
}

//--------------------------------------------------------------
void setNextNotifyRemainSecs(int curr) {
    int next;
    // ...180,120,60,30,5..1,0
    if (curr > 60) {
        if (curr % 60 == 0) {
            next = curr - 60;
        } else {
            next = (curr / 60) * 60;
        }
    } else if (curr > 30) {
        next = 30;
    } else if (curr > 5) {
        next = 5;
    } else if (curr > 4) {
        next = 4;
    } else if (curr > 3) {
        next = 3;
    } else if (curr > 2) {
        next = 2;
    } else if (curr > 1) {
        next = 1;
    } else if (curr > 0) {
        next = 0;
    } else {
        next = -1;
    }
    nextNotifyRemainSecs = next;
}

//--------------------------------------------------------------
void speakRemainTime(int sec) {
    string str = "";
    bool jp = speechLangJpn;
    if (sec == 0) {
        if (jp == true) {
            str = "規定時間が経過しました";
        } else {
            str = "specified time has passed.";
        }
    } else {
        if (jp == true) {
            str += "残り";
        }
        if (sec >= 60 && sec % 60 == 0) {
            // minute
            int min = sec / 60;
            str += ofToString(min);
            if (jp == true) {
                str += "分";
            } else {
                str += " minute";
                if (min != 1) {
                    str += "s";
                }
            }
        } else {
            // second
            str += ofToString(sec);
            if (jp == true) {
                str += "秒";
            } else {
                if (sec > 5) {
                    str += " seconds";
                    if (sec != 1) {
                        str += "s";
                    }
                }
            }
        }
        if (jp == false) {
            if (sec > 5) {
                str += " to go";
            }
        }
    }
    speakAny(jp == true ? "jp" : "en", str);
}

//--------------------------------------------------------------
void speakAny(string lang, string text) {
#ifdef TARGET_OSX
    int pid = fork();
    if (pid == 0) {
        // child process
        if (lang == "en") {
            execlp("say", "", "-r", "240", "-v", "Victoria", text.c_str(), NULL);
        }
        else if (lang == "jp") {
            execlp("say", "", "-r", "240", "-v", "Kyoko", text.c_str(), NULL);
        }
        OF_EXIT_APP(-1);
    }
#endif /* TARGET_OSX */
#ifdef TARGET_WIN32
    for (int i = 0; i < SPCH_SLOT_NUM; i++) {
        if (mySayWin[i].isThreadRunning() == false) {
            mySayWin[i].exec(lang, text);
            break;
        }
    }
#endif /* TARGET_WIN32 */
#ifdef TARGET_LINUX
    int pid = fork();
    if (pid == 0) {
        // Use speech-dispatcher
        execlp("spd-say", "-w", "-r", "15", "-t", "female1", "-l", lang.c_str(), text.c_str(), NULL);
        OF_EXIT_APP(-1);
    }
#endif /* TARGET_LINUX */
}

//--------------------------------------------------------------
void toggleRace() {
    if (raceStarted == false) {
        startRace();
    }
    else {
        stopRace(false);
    }
}

//--------------------------------------------------------------
void initRaceVars() {
    for (int i = 0; i < cameraNum; i++) {
        camView[i].foundMarkerNum = 0;
        camView[i].foundValidMarkerNum = 0;
        camView[i].enoughMarkers = false;
        camView[i].flickerCount = 0;
        camView[i].flickerValidCount = 0;
        camView[i].prevElapsedSec = WATCH_COUNT_SEC; // countdown
        camView[i].totalLaps = 0;
        camView[i].lastLapTime = 0;
        for (int h = 0; h < ARAP_MAX_RLAPS; h++) {
            camView[i].lapHistName[h] = "";
            camView[i].lapHistLapTime[h] = 0;
            camView[i].lapHistElpTime[h] = 0;
        }
        camView[i].racePosition = 0;
    }
    elapsedTime = 0;
}

//--------------------------------------------------------------
void startRace() {
    if (raceStarted == true) {
        return;
    }
    // stop/init -> start
    finishSound.stop();
    initRaceVars();
    if (raceDuraSecs > 0) {
        setNextNotifyRemainSecs(raceDuraSecs);
    }
    raceStarted = true;
    qrEnabled = false;
    ofResetElapsedTimeCounter();
    countSound.play();
}

//--------------------------------------------------------------
void stopRace(bool appexit) {
    if (raceStarted == false) {
        return;
    }
    // start -> stop
    if (appexit == false) {
        raceStarted = false;
        countSound.stop();
        finishSound.play();
        if (speechLangJpn == true) {
            speakAny("jp", "レース終了");
        } else {
            speakAny("en", "race finished.");
        }
        raceResultTimer = ARAP_RSLT_DELAY;
    }
    fwriteRaceResult();
}

//--------------------------------------------------------------
bool isVariousPilots(int camidx) {
    if (camidx < 0 || camidx >= cameraNum || camView[camidx].totalLaps <= 1) {
        return false;
    }
    string name = camView[camidx].lapHistName[0];
    for (int i = 1; i < camView[camidx].totalLaps; i++) {
        if (name != camView[camidx].lapHistName[i]) {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------
bool isVariousPilotsAll() {
    for (int i = 0; i < cameraNum; i++) {
        if (isVariousPilots(i) == true) {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------
bool isRecordedLaps() {
    bool ret = false;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].totalLaps > 0) {
            ret = true;
            break;
        }
    }
    return ret;
}

//--------------------------------------------------------------
float getBestLap(int camidx) {
    float blap = 0;
    if (camidx < 0 || camidx >= cameraNum) {
        return blap;
    }
    for (int i = 0; i < camView[camidx].totalLaps; i++) {
        if (i == 0 && useStartGate == true) {
            continue;
        }
        float t = camView[camidx].lapHistLapTime[i];
        if (blap == 0) {
            blap = t;
        } else if (t < blap) {
            blap = t;
        }
    }
    return blap;
}

//--------------------------------------------------------------
int getMaxLaps() {
    int laps = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].totalLaps > laps) {
            laps = camView[i].totalLaps;
        }
    }
    if (useStartGate == true && laps > 0) {
        laps--;
    }
    return laps;
}

//--------------------------------------------------------------
string getLapStr(float lap) {
    stringstream stream;
    float val = ceil(lap * 100) / 100;
    stream << fixed << setprecision(2) << val; // 2 digits
    return stream.str();
}

//--------------------------------------------------------------
void pushLapRecord(int cid, float elpsec) {
    if (cid < 1 || cid > cameraNum
        || raceStarted == false || ofGetElapsedTimef() < WATCH_COUNT_SEC) {
        return;
    }
    int i = cid - 1;
    float lap = elpsec - camView[i].prevElapsedSec;
    if (camView[i].totalLaps >= (raceDuraLaps + (useStartGate == true ? 1 : 0))
        || (raceDuraSecs > 0 && (camView[i].prevElapsedSec - WATCH_COUNT_SEC) >= raceDuraSecs)) {
        // already finished
        return;
    }
    if ((useStartGate == true && camView[i].totalLaps > 0 && lap < minLapTime)
        || (useStartGate == false && lap < minLapTime)
        || lap < 0) {
        // ignore short/negative lap
        return;
    }
    // record
    int total = camView[i].totalLaps + 1;
    camView[i].prevElapsedSec = elpsec;
    camView[i].totalLaps = total;
    camView[i].lastLapTime = lap;
    camView[i].lapHistName[total - 1] = camView[i].labelString;
    camView[i].lapHistLapTime[total - 1] = lap;
    camView[i].lapHistElpTime[total - 1] = elpsec;
    updateRacePositions();
    if (total >= (raceDuraLaps + (useStartGate == true ? 1 : 0))
        || (raceDuraSecs > 0 && (elpsec - WATCH_COUNT_SEC) >= raceDuraSecs)) {
        // finish by laps / time
        camView[i].foundMarkerNum = 0;
        camView[i].foundValidMarkerNum = 0;
        camView[i].enoughMarkers = false;
        finishSound.play();
        return;
    }
    // notify
    if (total == ((raceDuraLaps + (useStartGate == true ? 1 : 0)) - 1)) {
        beep3Sound.play();
    } else {
        beepSound.play();
    }
    speakLap(cid, lap, total);
}

//--------------------------------------------------------------
void popLapRecord(int cid) {
    if (raceStarted == false || cid < 1 || cid > cameraNum) {
        return;
    }
    int i = cid - 1;
    int oldlaps = camView[i].totalLaps;
    if (oldlaps == 0 || oldlaps >= (raceDuraLaps + (useStartGate == true ? 1 : 0))
        || (raceDuraSecs > 0 && (camView[i].prevElapsedSec - WATCH_COUNT_SEC) >= raceDuraSecs)) {
        // no record / already finished
        return;
    }
    int newlaps = oldlaps - 1;
    cancelSound.play();
    setOverlayMessage(camView[i].labelString + " Lap"
                      + ofToString(useStartGate == true ? oldlaps - 1 : oldlaps)
                      + " canceled");
    camView[i].lapHistName[oldlaps - 1] = "";
    camView[i].lapHistLapTime[oldlaps - 1] = 0;
    camView[i].lapHistElpTime[oldlaps - 1] = 0;
    camView[i].totalLaps = camView[i].totalLaps - 1;
    if (newlaps == 0) {
        camView[i].prevElapsedSec = WATCH_COUNT_SEC;
        camView[i].lastLapTime = 0;
    } else {
        camView[i].prevElapsedSec = camView[i].lapHistElpTime[newlaps - 1];
        camView[i].lastLapTime = camView[i].lapHistLapTime[newlaps - 1];
    }
    updateRacePositions();
}

//--------------------------------------------------------------
struct posData_t {
    int cidx;
    float elp;
    int laps;
};

//--------------------------------------------------------------
void updateRacePositions() {
    if (cameraNum == 0) {
        return;
    }
    int i;
    vector<posData_t> data(cameraNum);
    // read
    for (i = 0; i < cameraNum; i++) {
        data[i].cidx = i;
        if (useStartGate == true) {
            data[i].elp = camView[i].prevElapsedSec - camView[i].lapHistElpTime[0];
            data[i].laps = (camView[i].totalLaps >= 1) ? (camView[i].totalLaps - 1) : 0;
        } else {
            data[i].elp = camView[i].prevElapsedSec - WATCH_COUNT_SEC;
            data[i].laps = camView[i].totalLaps;
        }
    }
    // sort
    stable_sort(data.begin(), data.end(),
                [](const posData_t& x, const posData_t& y) { return x.elp < y.elp; });
    stable_sort(data.begin(), data.end(),
                [](const posData_t& x, const posData_t& y) { return x.laps > y.laps; });
    // write
    int pos = 0;
    for (i = 0; i < data.size(); i++) {
        if (data[i].laps == 0) {
            camView[data[i].cidx].racePosition = 0;
            continue;
        }
        if (pos == 0) {
            pos = 1;
        } else if (i > 0 && (data[i].laps != data[i - 1].laps || data[i].elp != data[i - 1].elp)) {
            pos++;
        }
        camView[data[i].cidx].racePosition = pos;
    }
}

//--------------------------------------------------------------
void fwriteRaceResult() {
    if (isRecordedLaps() == false) {
        return;
    }
    string timestamp = ofGetTimestampString();
    string newline;
#ifdef TARGET_WIN32
    newline = "\r\n";
#else /* TARGET_WIN32 */
    newline = "\n";
#endif /* TARGET_WIN32 */
    string strsumm = timestamp + newline + newline;
    string strlaph = "";
    string strlapb = "";
    string sep = "  ";
    int maxlap = 0;

    // Summary: Name Position Laps BestLap TotalTime
    updateRacePositions();
    // - head
    strsumm += "- Summary -" + newline;
    strsumm += "Name" + sep  + "Position" + sep + "Laps" + sep + "BestLap" + sep + "TotalTime" + newline;
    // - body
    for (int i = 0; i < cameraNum; i++) {
        string pilot = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        int pos = camView[i].racePosition;
        int lps = camView[i].totalLaps;
        float blap = getBestLap(i);
        float total;
        // ignore first lap
        if (useStartGate == true) {
            lps--;
            total = camView[i].prevElapsedSec - camView[i].lapHistElpTime[0];
        } else {
            total = camView[i].prevElapsedSec - WATCH_COUNT_SEC;
        }
        strsumm += pilot + sep; // Name
        strsumm += ((pos == 0) ? "-" : ofToString(pos)) + sep; // Position
        strsumm += ((lps < 0) ? "-" : ofToString(lps)) + sep; // Laps
        strsumm += ((blap == 0) ? "-.-" : getLapStr(blap)) + sep; // BestLap
        strsumm += ((total <= 0) ? "-:-.-" : getWatchString(total)) + sep; // TotalTime
        strsumm += newline;
    }
    strsumm += newline;

    // All Laps: Lap# P1(lap) P1(total) P2(lap) P2(total) P3(lap) P3(total) P4(lap) P4(total)
    // - head
    strlaph += "- All Laps -" + newline;
    strlaph += "Lap#" + sep;
    for (int i = 0; i < cameraNum; i++) {
        string pilot = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        strlaph += pilot + "(lap)" + sep;
        strlaph += "(total)" + sep;
    }
    strlaph += newline;
    maxlap = getMaxLaps();
    // - body
    for (int lap = 1; lap <= (maxlap + (useStartGate == true ? 1 : 0)); lap++) {
        if (useStartGate == true && lap == 1) {
            continue;
        }
        // lap#
        strlapb += ofToString(useStartGate == true ? (lap - 1) : lap) + sep;
        for (int i = 0; i < cameraNum; i++) {
            if (lap > camView[i].totalLaps) {
                // laptime
                strlapb += "-";
                // totaltime
                strlapb += sep;
                strlapb += "-";
            } else {
                float elp;
                // laptime
                strlapb += getLapStr(camView[i].lapHistLapTime[lap - 1]);
                // totaltime
                if (useStartGate == true) {
                    elp = camView[i].lapHistElpTime[lap - 1] - camView[i].lapHistElpTime[0];
                } else {
                    elp = camView[i].lapHistElpTime[lap - 1] - WATCH_COUNT_SEC;
                }
                strlapb += sep;
                strlapb += (elp < 0) ? "-" : getWatchString(elp);
            }
            if (i < (cameraNum - 1)) {
                strlapb += sep;
            }
        }
        strlapb += newline;
    }

    // write to file
    resultsFile.open(ARAP_RESULT_DIR + timestamp + ".txt" , ofFile::WriteOnly);
    resultsFile << (strsumm + strlaph + strlapb);
    resultsFile.close();
    // copy to clipboard
    ofSetClipboardString(strsumm + strlaph + strlapb);
}

//--------------------------------------------------------------
void toggleARLap() {
    switch (arLapMode) {
        case ARAP_MODE_NORM:
            arLapMode = ARAP_MODE_LOOSE;
            setOverlayMessage("AR Lap Timer Mode: Loose");
            break;
        case ARAP_MODE_LOOSE:
            arLapMode = ARAP_MODE_OFF;
            setOverlayMessage("AR Lap Timer Mode: Off");
            break;
        case ARAP_MODE_OFF:
            arLapMode = ARAP_MODE_NORM;
            setOverlayMessage("AR Lap Timer Mode: Normal");
            break;
    }
    saveSettingsFile();
}

//--------------------------------------------------------------
void changeMinLap(int mlaptime) {
    string value;
    mlaptime = minLapTime + mlaptime;
    if (mlaptime > 0 && mlaptime <= ARAP_MAX_MNLAP) {
        minLapTime = mlaptime;
        saveSettingsFile();
    }
    value = ofToString(minLapTime) + " sec";
    if (minLapTime > 1) {
        value += "s";
    }
    setOverlayMessage("Minimum Lap Time " + value);
}

//--------------------------------------------------------------
void changeRaceDuration(int time) {
    string value;
    time = raceDuraSecs + time;
    if (time >= 0 && time <= ARAP_MAX_RSECS) {
        raceDuraSecs = time;
        saveSettingsFile();
    }
    value = ofToString(raceDuraSecs) + " sec";
    if (raceDuraSecs > 1) {
        value += "s";
    }
    if (raceDuraSecs > 0) {
        int remain;
        if (raceStarted == true) {
            remain = raceDuraSecs - (elapsedTime - WATCH_COUNT_SEC);
        } else {
            remain = raceDuraSecs;
        }
        setNextNotifyRemainSecs(remain);
        setOverlayMessage("Race Time " + value);
    } else {
        nextNotifyRemainSecs = -1;
        setOverlayMessage("No Limit Race Time");
    }
}

//--------------------------------------------------------------
void changeRaceLaps(int lap) {
    lap = raceDuraLaps + lap;
    if (lap > 0 && lap <= ARAP_MAX_RLAPS) {
        raceDuraLaps = lap;
        saveSettingsFile();
    }
    setOverlayMessage("Race Laps " + ofToString(raceDuraLaps));
}

//--------------------------------------------------------------
void toggleUseStartGate() {
    useStartGate = !useStartGate;
    if (useStartGate == true) {
        setOverlayMessage("Staggered Start: On");
    } else {
        setOverlayMessage("Staggered Start: Off");
    }
    saveSettingsFile();
    updateRacePositions();
}

//--------------------------------------------------------------
void toggleLapAfterTimeout() {
    if (raceStarted == true) {
        return;
    }
    lapAfterTmoEnabled = !lapAfterTmoEnabled;
    if (lapAfterTmoEnabled == true) {
        setOverlayMessage("Wait for Lap after Time Limit: On");
    } else {
        setOverlayMessage("Wait for Lap after Time Limit: Off");
    }
    saveSettingsFile();
}

//--------------------------------------------------------------
void toggleFullscreen() {
    fullscreenEnabled = !fullscreenEnabled;
    ofSetFullscreen(fullscreenEnabled);
    saveSettingsFile();
}

//--------------------------------------------------------------
void toggleCameraTrim() {
    cameraTrimEnabled = !cameraTrimEnabled;
    setViewParams();
    saveSettingsFile();
}

//--------------------------------------------------------------
void toggleCameraFrame() {
    cameraFrameEnabled = !cameraFrameEnabled;
    saveSettingsFile();
}

//--------------------------------------------------------------
void toggleLapHistory() {
    cameraLapHistEnabled = !cameraLapHistEnabled;
    if (cameraLapHistEnabled == true) {
        setOverlayMessage("Lap History View: On");
    } else {
        setOverlayMessage("Lap History View: Off");
    }
    saveSettingsFile();
}

//--------------------------------------------------------------
void setOverlayMode(int mode) {
    overlayMode = mode;
    if (mode != OVLMODE_MSG) {
        initOverlayMessage();
    }
}

//--------------------------------------------------------------
void loadOverlayFont() {
    int h = (ofGetHeight() - (OVLTXT_MARG * 2)) / OVLTXT_LINES * 0.7;
    if (myFontOvlayP.isLoaded()) {
        myFontOvlayP.unloadFont();
    }
    if (myFontOvlayP2x.isLoaded()) {
        myFontOvlayP2x.unloadFont();
    }
    if (myFontOvlayM.isLoaded()) {
        myFontOvlayM.unloadFont();
    }
    myFontOvlayP.load(FONT_P_FILE, h);
    myFontOvlayP2x.load(FONT_P_FILE, h * 2);
    myFontOvlayM.load(FONT_M_FILE, h);
}

//--------------------------------------------------------------
void drawStringBlock(ofxTrueTypeFontUC *font, string text,
                     int xblock, int yline, int align, int blocks, int lines) {
    int bw, bh, x, y, xo, yo;
    int margin = OVLTXT_MARG;
    bw = (ofGetWidth() - (margin * 2)) / blocks;
    xo = (ofGetWidth() - (margin * 2)) % blocks / 2;
    bh = (ofGetHeight() - (margin * 2)) / lines;
    yo = (ofGetHeight() - (margin * 2)) % lines / 2;
    // pos-x
    switch (align) {
        case ALIGN_LEFT:
            x = bw * xblock;
            break;
        case ALIGN_CENTER:
            x = (bw * xblock) + (bw / 2) - (font->stringWidth(text) / 2);
            break;
        case ALIGN_RIGHT:
            x = (bw * xblock) + bw - font->stringWidth(text);
            break;
        default:
            return;
    }
    x += (margin + xo);
    // pos-y
    y = margin + ((yline + 1) * bh) + yo;
    // draw
    font->drawString(text, x, y);
}

//--------------------------------------------------------------
void drawLineBlock(int xblock1, int xblock2, int yline, int blocks, int lines) {
    int bw, bh, x, y, w, h, xo, yo;
    int margin = OVLTXT_MARG;

    bw = (ofGetWidth() - (margin * 2)) / blocks;
    xo = (ofGetWidth() - (margin * 2)) % blocks / 2;
    x = (bw * xblock1) + margin + xo;
    w = bw * (xblock2 - xblock1 + 1);

    bh = (ofGetHeight() - (margin * 2)) / lines;
    yo = (ofGetHeight() - (margin * 2)) % lines / 2;
    y = (bh * yline) + (bh * 0.5) + margin - 1 + yo;
    h = 2;

    ofFill();
    ofDrawRectangle(x, y, w, h);
}

//--------------------------------------------------------------
void drawULineBlock(int xblock1, int xblock2, int yline, int blocks, int lines) {
    int bw, bh, x, y, w, h, xo, yo;
    int margin = OVLTXT_MARG;

    bw = (ofGetWidth() - (margin * 2)) / blocks;
    xo = (ofGetWidth() - (margin * 2)) % blocks / 2;
    x = (bw * xblock1) + margin + xo;
    w = bw * (xblock2 - xblock1 + 1);

    bh = (ofGetHeight() - (margin * 2)) / lines;
    yo = (ofGetHeight() - (margin * 2)) % lines / 2;
    y = (bh * yline) + margin - 1 + yo;
    h = 2;

    ofFill();
    ofDrawRectangle(x, y, w, h);
}

//--------------------------------------------------------------
void generateDummyData() {
    // camera
    cameraNum = 4;
    camView[0].labelString = "Pilot1";
    camView[1].labelString = "Pilot2";
    camView[2].labelString = "Pilot3";
    camView[3].labelString = "Pilot4";
    // race result
    camView[0].prevElapsedSec = 180.1;
    camView[1].prevElapsedSec = 180.2;
    camView[2].prevElapsedSec = 180.3;
    camView[3].prevElapsedSec = 180.4;
    camView[0].totalLaps = ARAP_MAX_RLAPS;
    camView[1].totalLaps = ARAP_MAX_RLAPS / 2;
    camView[2].totalLaps = ARAP_MAX_RLAPS / 3;
    camView[3].totalLaps = ARAP_MAX_RLAPS / 4;
    for (int i = 0; i < ARAP_MAX_RLAPS; i++) {
        for (int j = 0; j < 4; j++) {
            camView[j].lapHistName[i] = camView[j].labelString + "_L" + ofToString(i);
            camView[j].lapHistLapTime[i] = 10 + (j * 0.1) + (i * 0.01);
            if (i == 0) {
                camView[j].lapHistElpTime[i] = WATCH_COUNT_SEC + camView[j].lapHistLapTime[i];
            } else {
                camView[j].lapHistElpTime[i] = camView[j].lapHistElpTime[i - 1] + camView[j].lapHistLapTime[i];
            }
        }
    }
}

//--------------------------------------------------------------
int getRaceResultPages() {
    int lnum = getMaxLaps();
    int pages;
    if (lnum <= 0) {
        pages = 0;
    } else {
        pages = (lnum / OVLTXT_LAPS) + ((lnum % OVLTXT_LAPS == 0) ? 0 : 1);
    }
    return pages;
}

//--------------------------------------------------------------
void processRaceResultDisplay() {
    if (isRecordedLaps() == false) { // no result
        return;
    }
    if (overlayMode != OVLMODE_RCRSLT) {
        raceResultPage = 0;
        setOverlayMode(OVLMODE_RCRSLT);
    } else {
        if ((raceResultPage + 1) >= getRaceResultPages()) {
            raceResultPage = 0;
            setOverlayMode(OVLMODE_NONE);
        } else {
            raceResultPage++;
        }
    }
}

//--------------------------------------------------------------
void drawRaceResult(int pageidx) {
    string str;
    int szb = OVLTXT_BLKS - (CAMERA_MAXNUM - cameraNum) - (cameraNum > 1 ? 0 : 1);
    int szl = OVLTXT_LINES;
    int blk, pages, line;

    pages = getRaceResultPages();
    if (pageidx < 0 || (pageidx + 1) > pages) {
        return;
    }

    // background
    ofSetColor(myColorBGDark);
    ofFill();
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

    // title
    line = 1;
    ofSetColor(myColorYellow);
    drawStringBlock(&myFontOvlayP2x, "Race Result", 0, line, ALIGN_CENTER, 1, szl);

    // summary : name position laps bestlap total
    updateRacePositions();
    // _header
    blk = 1;
    line = 3;
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Name", blk++, line, ALIGN_CENTER, szb, szl);
    if (cameraNum > 1) {
        drawStringBlock(&myFontOvlayP, "Position", blk++, line, ALIGN_CENTER, szb, szl);
    }
    drawStringBlock(&myFontOvlayP, "Laps", blk++, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "BestLap", blk++, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "TotalTime", blk, line, ALIGN_CENTER, szb, szl);
    line += 1;
    ofSetColor(myColorYellow);
    drawLineBlock(1, blk, line, szb, szl);
    // _body
    ofSetColor(myColorWhite);
    for (int i = 0; i < cameraNum; i++) {
        string str;
        float fval;
        int lps, pos;
        // _newline
        blk = 1;
        line += 1;
        // pilot
        str = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        drawStringBlock(&myFontOvlayP, str, blk++, line, ALIGN_CENTER, szb, szl);
        // pos
        if (cameraNum > 1) {
            pos = camView[i].racePosition;
            str = (pos == 0) ? "-" : ofToString(pos);
            drawStringBlock(&myFontOvlayP, str, blk++, line, ALIGN_CENTER, szb, szl);
        }
        // laps
        lps = camView[i].totalLaps;
        if (useStartGate == true) {
            lps--;
        }
        str = (lps < 0) ? "-" : ofToString(lps);
        drawStringBlock(&myFontOvlayM, str, blk++, line, ALIGN_CENTER, szb, szl);
        // bestlap
        fval = getBestLap(i);
        str = (fval == 0) ? "-.-" : getLapStr(fval);
        drawStringBlock(&myFontOvlayM, str, blk++, line, ALIGN_CENTER, szb, szl);
        // totaltime
        if (useStartGate == true) {
            fval = camView[i].prevElapsedSec - camView[i].lapHistElpTime[0];
        } else {
            fval = camView[i].prevElapsedSec - WATCH_COUNT_SEC;
        }
        str = (fval <= 0) ? "-:-.-" : getWatchString(fval);
        drawStringBlock(&myFontOvlayM, str, blk, line, ALIGN_CENTER, szb, szl);
    }

    // laptimes : lap p1 p2 p3 p4
    // _header
    int xoff = blk + 2;
    line = 3;
    drawStringBlock(&myFontOvlayP, "Lap", xoff, line, ALIGN_CENTER, szb, szl);
    for (int i = 0; i < cameraNum; i++) {
        string pilot = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        drawStringBlock(&myFontOvlayP, pilot, xoff + i + 1, line, ALIGN_CENTER, szb, szl);
    }
    line += 1;
    ofSetColor(myColorYellow);
    drawLineBlock(xoff, xoff + cameraNum, line, szb, szl);
    // _body
    ofSetColor(myColorWhite);
    int lapidx = (pageidx * OVLTXT_LAPS) + (useStartGate == true ? 1 : 0);
    int lnum = getMaxLaps();
    for (int cnt = 0; cnt < OVLTXT_LAPS; cnt++) {
        if ((lapidx + 1) > (lnum + (useStartGate == true ? 1 : 0))) {
            break;
        }
        // lap#
        line += 1;
        drawStringBlock(&myFontOvlayM,
                        ofToString(useStartGate == true ? lapidx : lapidx + 1),
                        xoff, line, ALIGN_CENTER, szb, szl);
        // laptime
        for (int i = 0; i < cameraNum; i++) {
            string str;
            if ((lapidx + 1) > camView[i].totalLaps) {
                str = "-.-";
            } else {
                str = getLapStr(camView[i].lapHistLapTime[lapidx]);
            }
            drawStringBlock(&myFontOvlayM, str, xoff + i + 1, line, ALIGN_CENTER, szb, szl);
        }
        lapidx += 1;
    }
    // _page
    if (pages > 1) {
        line += 1;
        ofSetColor(myColorLGray);
        drawStringBlock(&myFontOvlayP,
                        "(Page " + ofToString(pageidx + 1) + " of " + ofToString(pages) + ")",
                        xoff + 2, line, ALIGN_CENTER, szb, szl);
    }

    // message
    line = OVLTXT_LINES - 1;
    if (pages > 1 && (pageidx + 1) < pages) {
        str = "Press R key to continue, Esc key to exit";
    } else {
        str = "Press R or Esc key to exit";
    }
    ofSetColor(myColorYellow);
    drawStringBlock(&myFontOvlayP, str, 0, line, ALIGN_CENTER, 1, szl);
}

//--------------------------------------------------------------
void drawHelp() {
    int szl = HELP_LINES;
    int line;
    // background
    ofSetColor(myColorBGDark);
    ofFill();
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    // title(3 lines)
    line = 1;
    ofSetColor(myColorYellow);
    drawStringBlock(&myFontOvlayP2x, "Settings / Commands", 0, line, ALIGN_CENTER, 1, szl);
    line += 2;
    // body
    drawHelpBody(line);
    // message(2 lines)
    line = HELP_LINES - 1;
    ofSetColor(myColorYellow);
    drawStringBlock(&myFontOvlayP, "Press H or Esc key to exit", 0, line, ALIGN_CENTER, 1, szl);
}

//--------------------------------------------------------------
void drawHelpBody(int line) {
    string value;
    int szl = HELP_LINES;
    int szb = 21;
    int blk0 = 6;
    int blk1 = 3;
    int blk2 = 12;
    int blk3 = 16;
    int blk4 = 17;

    // SYSTEM
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "System Command", blk0, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Setting", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Key", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    ofSetColor(myColorYellow);
    drawLineBlock(blk1, blk4, line, szb, szl);
    line++;
    ofSetColor(myColorWhite);
    // Set speech language
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = speechLangJpn ? "Japanese" : "English";
    drawStringBlock(&myFontOvlayP, "Set Speech Language", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "N", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set system statistics
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = sysStatEnabled ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set System Statistics", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "S", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Display help
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Display Help (Settings/Commands)", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "H", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Initialize settings
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Initialize Settings", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "I", blk3, line, ALIGN_CENTER, szb, szl);
    line++;

    // VIEW
    line++;
    drawStringBlock(&myFontOvlayP, "View Command", blk0, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Setting", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Key", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    ofSetColor(myColorYellow);
    drawLineBlock(blk1, blk4, line, szb, szl);
    line++;
    ofSetColor(myColorWhite);
    // Set fullscreen mode
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = fullscreenEnabled ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set Fullscreen Mode", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "F, Esc", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set background image
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Set Background Image", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "B", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set camera view trimming
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = cameraTrimEnabled ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set Camera View Trimming", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "T", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set camera frame visibility
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = cameraFrameEnabled ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set Camera Frame Visibility", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "E", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set camera 1~4 enhanced view
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = (cameraIdxSolo == -1) ? "Disabled" : "Camera " + ofToString(cameraIdxSolo + 1);
    drawStringBlock(&myFontOvlayP, "Set Camera 1~4 Enhanced View", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "1~4", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set camera 1~4 visibility
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = "";
    for (int i = 0; i < CAMERA_MAXNUM; i++) {
        if (i > 0) {
            value += ", ";
        }
        if (i < cameraNum) {
            value += (camView[i].visible == true) ? "On" : "Off";
        } else {
            value += "-";
        }
    }
    drawStringBlock(&myFontOvlayP, "Set Camera 1~4 Visibility", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, ofToString(TVP_STR_ALT) + " + 1~4", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Start/Stop QR Code reader for labal
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Start/Stop QR Code Reader for Label", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Q", blk3, line, ALIGN_CENTER, szb, szl);
    line++;

    // RACE
    line++;
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Race Command", blk0, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Setting", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Key", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    ofSetColor(myColorYellow);
    drawLineBlock(blk1, blk4, line, szb, szl);
    line++;
    ofSetColor(myColorWhite);
    // Set AR lap timer mode
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = (arLapMode == ARAP_MODE_NORM) ? "Normal" : ((arLapMode == ARAP_MODE_LOOSE) ? "Loose" : "Off");
    drawStringBlock(&myFontOvlayP, "Set AR Lap Timer Mode", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "A", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set race duration (time, laps)
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = (raceDuraSecs <= 0) ? "No Limit" : (ofToString(raceDuraSecs) + " sec");
    if (raceDuraSecs > 1) {
        value += "s";
    }
    value += ", " + ofToString(raceDuraLaps) + " lap";
    if (raceDuraLaps > 1) {
        value += "s";
    }
    drawStringBlock(&myFontOvlayP, "Set Race Duration (Time, Laps)", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Up/Down,Left/Right", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set wait for lap after time limit
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = lapAfterTmoEnabled ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set Wait for Lap after Time Limit", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "W", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set minimum lap time
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = ofToString(minLapTime) + " sec";
    if (minLapTime > 1) {
        value += "s";
    }
    drawStringBlock(&myFontOvlayP, "Set Minimum Lap Time", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Pg_Up/Pg_Down", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set staggered start
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = useStartGate ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set Staggered Start", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "G", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Set lap history view
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    value = cameraLapHistEnabled ? "On" : "Off";
    drawStringBlock(&myFontOvlayP, "Set Lap History View", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, value, blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "L", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Start/Stop race
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Start/Stop Race", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Space", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Add lap at camera 1~4
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Add Lap at Camera 1~4,1,3", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "5~8,Z,/", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Delete previous lap at camera 1~4
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Delete Previous Lap at Camera 1~4,1,3", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, ofToString(TVP_STR_ALT) + " + 5~8,Z,/", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Display race result
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Display Race Result", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "R", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
    // Clear race result
    ofSetColor(myColorDGray);
    drawULineBlock(blk1, blk4, line + 1, szb, szl);
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Clear Race Result", blk1, line, ALIGN_LEFT, szb, szl);
    drawStringBlock(&myFontOvlayP, "-", blk2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "C", blk3, line, ALIGN_CENTER, szb, szl);
    line++;
}

//--------------------------------------------------------------
void initOverlayMessage() {
    ovlayMsgTimer = 0;
    ovlayMsgString = "";
}

//--------------------------------------------------------------
void setOverlayMessage(string msg) {
    if (overlayMode != OVLMODE_NONE && overlayMode != OVLMODE_MSG) {
        return;
    }
    ovlayMsgTimer = OLVMSG_TIME;
    ovlayMsgString = msg;
    setOverlayMode(OVLMODE_MSG);
}

//--------------------------------------------------------------
void drawOverlayMessageCore(ofxTrueTypeFontUC *font, string msg) {
    float sw, fh, sx, sy, margin;
    margin = 10;
    sw = font->stringWidth(msg);
    fh = font->getFontSize();
    sx = (ofGetWidth() / 2) - (sw / 2);
    sy = (ofGetHeight() / 2) + (fh / 2);
    // background
    ofSetColor(myColorBGDark);
    ofFill();
    ofDrawRectangle(sx - margin, sy - (fh + margin), sw + (margin * 2), fh + (margin * 2));
    // message
    ofSetColor(myColorWhite);
    font->drawString(msg, sx, sy);
}

//--------------------------------------------------------------
void drawOverlayMessage() {
    ofxTrueTypeFontUC *font = &myFontLap;
    string msg = ovlayMsgString;
    drawOverlayMessageCore(font, msg);
}

#ifdef TARGET_WIN32
//--------------------------------------------------------------
string utf8ToAnsi(string utf8) {
    int ulen, alen;
    wchar_t* ubuf;
    char* abuf;
    string ansi;

    // utf8 -> wchar
    ulen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size() + 1, NULL, 0);
    ubuf = new wchar_t[ulen];
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size() + 1, ubuf, ulen);
    // wchar -> ansi
    alen = WideCharToMultiByte(CP_ACP, 0, ubuf, -1, NULL, 0, NULL, NULL);
    abuf = new char[alen];
    WideCharToMultiByte(CP_ACP, 0, ubuf, ulen, abuf, alen, NULL, NULL);
    ansi = abuf;

    delete[] ubuf;
    delete[] abuf;
    return ansi;
}

string ansiToUtf8(string ansi) {
    int alen, ulen;
    wchar_t* abuf;
    char* ubuf;
    string utf8;

    // ansi -> wchar
    alen = MultiByteToWideChar(CP_THREAD_ACP, 0, ansi.c_str(), ansi.size() + 1, NULL, 0);
    abuf = new wchar_t[alen];
    MultiByteToWideChar(CP_THREAD_ACP, 0, ansi.c_str(), ansi.size() + 1, abuf, alen);
    // wchar -> utf8
    ulen = WideCharToMultiByte(CP_UTF8, 0, abuf, -1, NULL, 0, NULL, NULL);
    ubuf = new char[ulen];
    WideCharToMultiByte(CP_UTF8, 0, abuf, alen + 1, ubuf, ulen, NULL, NULL);
    utf8 = ubuf;

    delete[] abuf;
    delete[] ubuf;

    return utf8;
}
#endif /* TARGET_WIN32 */

//--------------------------------------------------------------
void toggleQrReader() {
    if (qrEnabled == false) {
        // start
        qrUpdCount = 1;
        qrCamIndex = 0;
        for (int i = 0; i < cameraNum; i++) {
            camView[i].qrScanned = false;
        }
        qrEnabled = true;
    }
    else {
        // stop
        qrEnabled = false;
    }
}

//--------------------------------------------------------------
void processQrReader() {
    if (qrUpdCount == QR_CYCLE) {
        bool scanned = false;
        if (camView[qrCamIndex].qrScanned == false) {
            ofxZxing::Result zxres;
            if (camView[qrCamIndex].needCrop == true || camView[qrCamIndex].needResize == true) {
                zxres = ofxZxing::decode(camView[qrCamIndex].resizedPixels, true);
            }
            else {
                zxres = ofxZxing::decode(grabber[qrCamIndex].getPixels(), true);
            }
            if (zxres.getFound()) {
                scanned = true;
                camView[qrCamIndex].qrScanned = true;
                beepSound.play();
                string label = ofTrim(zxres.getText());
#ifdef TARGET_WIN32
                label = utf8ToAnsi(label);
#endif /* TARGET_WIN32 */
                camView[qrCamIndex].labelString = label;
                savePilotsFile();
                autoSelectCameraIcon(qrCamIndex + 1, label);
            }
        }
        qrCamIndex++;
        if (qrCamIndex == cameraNum) {
            qrCamIndex = 0;
        }
        qrUpdCount = 1;
        if (scanned == false) {
            return;
        }
    }
    else {
        qrUpdCount++;
        return;
    }
    // scanned
    int count = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].qrScanned == true) {
            count++;
        }
    }
    if (count == cameraNum) {
        toggleQrReader(); // finished
    }
}

//--------------------------------------------------------------
void checkGamePad(float elpsec) {
    int dev, btn;
    for (dev = 0; dev < GPAD_MAX_DEVS; dev++) {
        if (gamePad[dev].isConnect() == false) {
            continue;
        }
        for (btn = 0; btn < cameraNum; btn++) {
            if (gamePad[dev].isPressed(btn) == true) {
                if (gamePad[dev].isPressed(GPAD_ALT_BTN) == true
                    || gamePad[dev].isPushing(GPAD_ALT_BTN) == true) {
                    popLapRecord((btn + 1));
                } else {
                    pushLapRecord((btn + 1), elpsec);
                }
            }
        }
    }
}

//--------------------------------------------------------------
void activateCursor() {
    if (hideCursorTimer <= 0) {
        ofShowCursor();
    }
    hideCursorTimer = HIDECUR_TIME;
}
