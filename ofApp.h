#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
//#include "ofxNDI.h"

bool const DEBUG = false;

enum class SettingsMode : unsigned int { none, outputPort, channel, outputType };
enum OutputMode { noteBendMod, CCs, NoteGrid };
const std::string OutputModeNames[] =
{
	"NOTE-BEND-MOD",
	"CC 14/15",
	"NOTE GRID"
};

class ofApp : public ofBaseApp{
	private:	
		unsigned int appWidth;
		unsigned int appHeight;
		int penX;
		int penY;
		int lastPenX;
		int lastPenY;
		int startY;
		int startX;
		int strokeWidth;
		ofFbo fbo;
		bool penDown;

		ofxMidiOut midiOut;
		int midiChannel;
		int midiOutput;
		int currentNote;
		OutputMode outputMode;
		UINT8 lastSent[16][128];
		int lastPitchBend;

		bool notificationVisible;
		std::string notificationContent;
		int notificationCounter;
		int notificationDuration;
		SettingsMode settingsMode;

		/*
		ofxNDIsender ndiSender;    // NDI sender
		char senderName[256];      // Sender name
		unsigned int senderWidth;  // Width of the sender output
		unsigned int senderHeight; // Height of the sender output
		*/
	public:
		void setup();
		void update();
		void draw();
		void notify(std::string message);
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void sendControlChange(int channel, int cc, int value);
		void sendPitchBend(int channel, int value);
};
