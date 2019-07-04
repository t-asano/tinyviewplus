#pragma once

#include <regex>
#include "ofMain.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOsc.h"
#include "ofxAruco.h"
#include "highlyreliablemarkers.h"
#include "ofxZxing.h"
#include "ofxJoystick.h"
#include "ofxXmlSettings.h"

/* ---------- definitions ---------- */

// system
#define APP_VER         "v0.9.26"
#define DEBUG_ENABLED   false
#define HELP_LINES      33  // must be <= OVLTXT_LINES
#define SCENE_INIT      0
#define SCENE_CAMS      1
#define SCENE_MAIN      2
#ifdef TARGET_OSX
#define TVP_KEY_ALT     OF_KEY_COMMAND
#define TVP_STR_ALT     "command"
#endif /* TARGET_OSX */
#ifdef TARGET_WIN32
#define TVP_KEY_ALT     OF_KEY_ALT
#define TVP_STR_ALT     "Alt"
#endif /* TARGET_WIN32 */
#define DFLT_SYS_STAT   false
#define SETTINGS_FILE   "settings.xml"
#define SNM_SYS_SPCLANG "system:speechJpn"
#define SNM_SYS_STAT    "system:sysStat"
#define SNM_RACE_ARMODE "race:arMode"
#define SNM_RACE_DRSECS "race:duraSecs"
#define SNM_RACE_DRLAPS "race:duraLaps"
#define SNM_RACE_MINLAP "race:minLapTime"
#define SNM_RACE_STAGGR "race:staggStart"
#define SNM_RACE_DISPLH "race:dispLapHist"
#define BTTN_FSCR_FILE  "system/button_fullscreen.png"
#define BTTN_QUIT_FILE  "system/button_quit.png"
#define BTTN_SETT_FILE  "system/button_settings.png"
#define BTTN_WNDW_FILE  "system/button_window.png"

// color
#define COLOR_YELLOW    255,215,0
#define COLOR_WHITE     255,255,255
#define COLOR_LGRAY     127,127,127
#define COLOR_DGRAY     15
#define COLOR_BG_DARK   0,0,0,223
#define COLOR_BG_LIGHT  0,0,0,31
#define COLOR_ALERT     255,0,0
// view
#define FRAME_RATE      60
#define MOVE_STEPS      10
#define VERTICAL_SYNC   true
#define LOGO_LARGE_FILE "system/logo_large.png"
#define LOGO_SMALL_FILE "system/logo_small.png"
#define WALL_FILE       "system/background.png"
#define CAMERA_MAXNUM   4
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_RATIO    1.3333
#define FONT_P_FILE     "system/GenShinGothic-P-Bold.ttf"
#define FONT_M_FILE     "system/GenShinGothic-Monospace-Bold.ttf"
#define ICON_FILE       "system/pilot_icon.png"
#define ICON_DIR        "pilots/"
#define ICON_WIDTH      50
#define ICON_HEIGHT     50
#define ICON_MARGIN_X   20
#define ICON_MARGIN_Y   0
#define NUMBER_HEIGHT   20
#define NUMBER_MARGIN_X 1
#define NUMBER_MARGIN_Y 35
#define LABEL_HEIGHT    30
#define LABEL_MARGIN_X  75
#define LABEL_MARGIN_Y  40
#define BASE_MARGIN_X   0
#define BASE_MARGIN_Y   0
#define BASE_WIDTH      20
#define BASE_HEIGHT     50
#define BASE_1_COLOR    201,58,64
#define BASE_2_COLOR    160,194,56
#define BASE_3_COLOR    0,116,191
#define BASE_4_COLOR    248,128,23
#define LAP_HEIGHT      20
#define LAP_MARGIN_X    20
#define LAP_MARGIN_Y    80
#define LAPHIST_HEIGHT  15
#define DFLT_CAM_TRIM   false
#define DFLT_FSCR_ENBLD false
#define DFLT_CAM_LAPHST false
#define ALIGN_LEFT      0
#define ALIGN_CENTER    1
#define ALIGN_RIGHT     2
#define HIDECUR_TIME    (FRAME_RATE * 3)
// overlay
#define OVLMODE_NONE    0
#define OVLMODE_HELP    1
#define OVLMODE_MSG     2
#define OVLMODE_RCRSLT  3
#define OVLTXT_BLKS     13
#define OVLTXT_LINES    34
#define OVLTXT_LAPS     25
#define OVLTXT_MARG     10
#define OLVMSG_TIME     FRAME_RATE
#define INFO_HEIGHT     15
// sound
#define SND_BEEP_FILE   "system/beep.wav"
#define SND_BEEP3_FILE  "system/beep3.wav"
#define SND_COUNT_FILE  "system/count.wav"
#define SND_FINISH_FILE "system/finish.wav"
#define SND_NOTIFY_FILE "system/notify.wav"
#define SND_CANCEL_FILE "system/cancel.wav"
// AR lap timer
#define ARAP_MODE_NORM  0
#define ARAP_MODE_LOOSE 1
#define ARAP_MODE_OFF   2
#define DFLT_ARAP_MODE  ARAP_MODE_NORM
#define DFLT_ARAP_RLAPS 10
#define DFLT_ARAP_RSECS 0
#define DFLT_ARAP_MNLAP 3
#define DFLT_ARAP_SGATE false
#define ARAP_MKR_FILE   "system/marker.xml"
#define ARAP_RESULT_DIR "results/"
#define ARAP_MNUM_THR   2
#define ARAP_MAX_RLAPS  10000
#define ARAP_MAX_MNLAP  100
#define ARAP_MAX_RSECS  36000
#define ARAP_RSLT_SCRN  0
#define ARAP_RSLT_FILE  1
#define WATCH_COUNT_SEC 5
#define ARAP_RSLT_DELAY (FRAME_RATE * 3)
// osc
#define OSC_LISTEN_PORT 4000
// speech
#define DFLT_SPCH_ENBLD false
#define SPCH_SLOT_NUM	8
// QR code reader
#define QR_CYCLE        6
// gamepad
#define GPAD_MAX_DEVS   4
#define GPAD_ALT_BTN    4

/* ---------- classes ---------- */

class tvpCamView {
public:
    // camera
    bool visible;
    int moveSteps;
    int width;
    int height;
    int widthTarget;
    int heightTarget;
    int posX;
    int posY;
    int posXTarget;
    int posYTarget;
    float imageScale;
    bool needResize;
    // base
    ofColor baseColor;
    int basePosX;
    int basePosY;
    int basePosXTarget;
    int basePosYTarget;
    int baseWidth;
    int baseHeight;
    // number
    int numberPosX;
    int numberPosY;
    int numberPosXTarget;
    int numberPosYTarget;
    // icon
    ofImage iconImage;
    int iconPosX;
    int iconPosY;
    int iconPosXTarget;
    int iconPosYTarget;
    // label
    string labelString;
    int labelPosX;
    int labelPosY;
    int labelPosXTarget;
    int labelPosYTarget;
    // lap
    int lapPosX;
    int lapPosY;
    int lapPosXTarget;
    int lapPosYTarget;
    float lastLapTime;
    // AR lap timer
    ofxAruco aruco;
    int foundMarkerNum;
    int foundValidMarkerNum;
    bool enoughMarkers;
    float prevElapsedSec;
    int totalLaps;
    string lapHistName[ARAP_MAX_RLAPS + 1];
    float lapHistLapTime[ARAP_MAX_RLAPS + 1];
    float lapHistElpTime[ARAP_MAX_RLAPS + 1];
    int flickerCount;
    int flickerValidCount;
    int racePosition;
    // QR reader
    bool qrScanned;
};

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
};

/* ---------- functions ---------- */

// -- splash --
void setupInit();
void loadSettingsFile();
void saveSettingsFile();
void updateInit();
void drawInit();
// -- camera setup --
void setupCamCheck();
void updateCamCheck();
void drawCamCheck();
void keyPressedCamCheck();
void reloadCameras();
// -- main --
// common
void bindCameras();
void initConfig();
string getUserLocaleName();
void toggleSysStat();
// view
void toggleCameraSolo(int);
void enableCameraSolo(int);
void resetCameraSolo();
void toggleCameraVisibility(int);
int getCameraIdxNthVisibleAll(int);
int getCameraIdxNthVisibleSub(int);
void toggleFullscreen();
void toggleSoloTrim();
void setupColors();
void changeCameraLabel(int);
void changeCameraLabelAll();
void changeCameraIcon(int);
void changeCameraIconPath(int, string, bool);
void autoSelectCameraIcon(int, string);
void changeWallImage();
void setWallParams();
void setViewParams();
int calcViewParam(int, int, int);
void updateViewParams();
// osc
void recvOsc();
void recvOscCameraString(int, string, string);
void recvOscCameraFloat(int, string, float);
void recvOscSpeech(string, string);
// speech
void toggleSpeechLang();
void autoSelectSpeechLang();
void speakLap(int, float, int);
void setNextSpeechRemainSecs(int);
void speakRemainTime(int);
void speakAny(string, string);
// draw
void drawCameraImage(int);
void drawCameraARMarker(int, bool);
void drawCameraPilot(int, bool);
void drawCameraLapTime(int, bool);
void drawCameraLapHistory(int);
void drawCamera(int);
string getWatchString(float);
void drawWatch();
void drawInfo();
void drawStringWithShadow(ofxTrueTypeFontUC*, ofColor, string, int, int);
void drawSystemButtons();
// input
void keyPressedOverlayHelp(int);
void keyPressedOverlayHelp(int);
void keyPressedOverlayResult(int);
void keyPressedOverlayNone(int);
void mouseReleasedOverlayNone(int, int, int);
// race
void toggleRace();
void initRaceVars();
void startRace();
void stopRace(bool);
bool isVariousPilots(int);
bool isVariousPilotsAll();
bool isRecordedLaps();
float getBestLap(int);
int getMaxLaps();
string getLapStr(float);
void pushLapRecord(int, float);
void popLapRecord(int);
void updateRacePositions();
void toggleARLap();
void changeMinLap();
void changeRaceDuration();
void toggleUseStartGate();
// overlay - common
void setOverlayMode(int);
void loadOverlayFont();
void drawStringBlock(ofxTrueTypeFontUC*, string, int , int, int, int, int);
void drawLineBlock(int, int, int, int, int);
void drawULineBlock(int, int, int, int, int);
// overlay - race result
void generateDummyData();
void fwriteRaceResult();
int getRaceResultPages();
void processRaceResultDisplay();
void drawRaceResult(int);
// overlay - help
void drawHelp();
void drawHelpBody(int);
// overlay - message
void initOverlayMessage();
void setOverlayMessage(string);
void drawOverlayMessageCore(ofxTrueTypeFontUC*, string);
void drawOverlayMessage();
// QR Code reader
#ifdef TARGET_WIN32
string utf8ToAnsi(string);
string ansiToUtf8(string);
#endif /* TARGET_WIN32 */
void toggleQrReader();
void processQrReader();
// gamepad
void checkGamePad(float);
// others
void toggleLapHistory();
void activateCursor();
