#include "ofApp.h"

/*
Serum Presets to use :
FX / FX Intergalactif CHoo Choo
Misc / MID CalyxTeeBee (BR)



*/



//--------------------------------------------------------------
void ofApp::setup() {
	// inits
	penX = 0;
	penY = 0;
	penDown = false;
	shouldFade = false;
	strokeWidth = 3;
	cursorSize = 150;
	settingsMode = SettingsMode::none;
	outputMode = OutputMode::noteBendMod;

	// init notifications
	notificationDuration = 200;
	notificationVisible = false;
	notify(("> STARTING"));

	// FBO
	if (DEBUG) {
		appWidth = 960;
		appHeight = 540;
	}
	else {
		appWidth = ofGetWidth();
		appHeight = ofGetHeight();
	}
	//
	fbo.allocate(appWidth, appHeight, GL_RGBA);
	fbo.begin();
	ofClear(255, 255, 255, 0);
	fbo.end();

	// MIDI
	//midiOut.listOutPorts();
	midiChannel = 1;
	midiOutput = 0;
	midiOut.openPort(midiOutput);

	// NDI
	/*
	strcpy_s(senderName, 256, "NOISE PROJECT"); // Set the sender name
	ofSetWindowTitle(senderName); // show it on the title bar
	ndiSender.SetReadback();
	ndiSender.SetAsync();
	ndiSender.CreateSender(senderName, appWidth, appHeight);
	*/
	ofSetWindowTitle("NOISE PROJECT");
	ofHideCursor();
}
/*
void ofApp::exit() {
	// The sender must be released 
	// or NDI sender discovery will still find it
	ndiSender.ReleaseSender();
}
*/
//--------------------------------------------------------------
void ofApp::update(){
	if (notificationVisible) {
		if (--notificationCounter < 0) {
			notificationVisible = false;
			settingsMode = SettingsMode::none;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// DRAW ?
	fbo.begin();
	ofEnableSmoothing();
	if (penDown) {
		if (lastPenX != penX || lastPenY != penY) {
			ofSetColor(0, 0, 0, 255);
			ofFill();
			ofSetLineWidth(strokeWidth);
			ofDrawLine(lastPenX, lastPenY, penX, penY);
			ofDrawCircle(penX, penY, floor(strokeWidth / 2 - 1));
			lastPenX = penX;
			lastPenY = penY;
		}
		else {
			ofSetColor(0, 0, 0, 255);
			ofFill();
			ofDrawCircle(penX, penY, floor(strokeWidth / 2));
		}
	}
	// APPLY FADE ?
	if (shouldFade) {
		ofSetColor(255, 255, 255, 3);
		ofFill();
		ofDrawRectangle(0, 0, appWidth, appHeight);
		shouldFade = false;
	}
	fbo.end();
	// SHOW FBO DRAWING
	ofClear(255, 255, 255, 255);
	ofSetColor(255, 255, 255, 255);
	fbo.draw(0, 0);

	// SHOW Mouse Position
	if (penDown)
		ofSetColor(0, 0, 0, 255);
	else
		ofSetColor(200, 200, 200, 255);
	ofSetLineWidth(1);
	ofDrawLine(penX - cursorSize, penY, penX + cursorSize, penY);
	ofDrawLine(penX, penY - cursorSize, penX, penY + cursorSize);

	//ndiSender.SendImage(fbo);

	// notifications
	if (notificationVisible) {
		//ofColor(0, 0, 0, 255);
		ofSetColor(0, 0, 0, 255);
		ofDrawBitmapString(notificationContent, 20, 20);
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_RETURN) {
		// save image as file
		//fbo is the main drawing canvas displayed in the second window   
		int w = fbo.getWidth();
		int h = fbo.getHeight();
		unsigned char* pixels = new unsigned char[w*h * 3]; ;
		ofImage screenGrab;
		screenGrab.allocate(w, h, OF_IMAGE_COLOR);
		screenGrab.setUseTexture(false);


		//copy the pixels from FBO to the pixel array; then set the normal ofImage from those pixels; and use the save method of ofImage  

		fbo.begin();
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, fbo.getWidth(), fbo.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, pixels);
		screenGrab.setFromPixels(pixels, fbo.getWidth(), fbo.getHeight(), OF_IMAGE_COLOR);
		
		screenGrab.save(ofGetTimestampString("IMG %Y-%m-%d %H-%M-%S") + ".png", OF_IMAGE_QUALITY_BEST);
		fbo.end();

		// reset canvas
		//fbo.begin();
		//ofClear(255, 255, 255, 0);
		//fbo.end();

		settingsMode = SettingsMode::none;
		notify(("> IMAGE SAVED"));
	}
	else if (key > 48 && key < 53) { // stroke width selector (numpad 1>4)
		settingsMode = SettingsMode::none;
		strokeWidth = (key - 48) * 3;
		notify(("> STROKE WIDTH : " + std::to_string(strokeWidth)));
		sendControlChange(midiChannel, 100, (key - 49)*42);
	} else if (key == OF_KEY_BACKSPACE) { // BACKSPACE
		notify(("> CANVAS CLEARED"));
		// reset canvasg
		fbo.begin();
		ofClear(255, 255, 255, 0);
		fbo.end();
	} else if (key == OF_KEY_F1) { // F1
		notify("ESC EXIT APP\n\n\
F1  SHOW HELP\n\
F2  SELECT MIDI OUTPUT\n\
F3  SELECT MIDI CHANNEL\n\
F4  SELECT OUTPUT TYPE\n\n\
BACKSPACE   CLEAR SCREEN\n\
ENTER       SAVE IMAGE AND CLEAR SCREEN\n\n\
NUMPAD 1-4  CHANGE STROKE SIZE\n\n\
W,X,C       SEND MIDI VALUES FOR MAPPING");
	} else if (key == OF_KEY_F2) { // F2
		if (settingsMode == SettingsMode::outputPort) {
			midiOut.closePort();
			midiOutput = ++midiOutput % midiOut.getNumOutPorts();
			midiOut.openPort(midiOutput);
			notify(("> CHANGED OUTPUT : ") + midiOut.getOutPortName(midiOutput));
			
		}
		else {
			settingsMode = SettingsMode::outputPort;
			notify(("> CURRENT OUTPUT : ") + midiOut.getOutPortName(midiOutput));
		}
	}
	else if (key == OF_KEY_F3) { // F3
		if (settingsMode == SettingsMode::channel) {
			if (++midiChannel > 16) midiChannel = 1;
			notify("> MIDI CHANNEL : " + std::to_string(midiChannel));
		}
		else {
			settingsMode = SettingsMode::channel;
			notify("> MIDI CHANNEL : " + std::to_string(midiChannel));
		}
	}
	else if (key == OF_KEY_F4) { // F4
		if (settingsMode == SettingsMode::outputType) {
			switch (outputMode)
			{
			case OutputMode::noteBendMod:
				if (currentNote < 0) {
					midiOut.sendNoteOff(midiChannel, currentNote, 100);
					currentNote = -1;
				}
				outputMode = OutputMode::CCs;
				break;
			case OutputMode::CCs:
				if (penDown < 0) {
					midiOut.sendNoteOff(midiChannel, 56, 100);
				}
				outputMode = OutputMode::NoteGrid;
				break;
			case OutputMode::NoteGrid:
				outputMode = OutputMode::noteBendMod;
				if (currentNote < 0) {
					midiOut.sendNoteOff(midiChannel, currentNote, 100);
					currentNote = -1;
				}
				break;
			default:
				break;
			}
		}
		else {
			settingsMode = SettingsMode::outputType;
		}

		notify("> OUTPUT TYPE : " + OutputModeNames[outputMode]);
	}
	else if (key == 'c') { // C
		midiOut.sendControlChange(midiChannel, 100, 1);
		notify("> SENDING CC 100");
	/*switch (outputMode)
	{
	case noteBendMod:
		//midiOut.sendControlChange(midiChannel, 14, 1);
		//notify("> SENDING CC 14");
		break;
	case CCs:
		//midiOut.sendControlChange(midiChannel, 13, 1);
		//notify("> SENDING CC 13");
		break;
		break;
	case NoteGrid:

		break;
	default:
		break;
	}*/
	}
	else if (key == 'w') { // W
	switch (outputMode)
	{
	case OutputMode::noteBendMod:
		midiOut.sendControlChange(midiChannel, 1, 1);
		notify("> SENDING CC 1 (MOD)");
		break;
	case OutputMode::CCs:
		midiOut.sendControlChange(midiChannel, 14, 1);
		notify("> SENDING CC 14");
		break;
		break;
	case OutputMode::NoteGrid:
		break;
	default:
		break;
	}
	}
	else if (key == 'x') { // X
		switch (outputMode)
		{
		case OutputMode::noteBendMod:
			midiOut.sendPitchBend(midiChannel, 8192);
			notify("> SENDING PITCH BEND");
			break;
		case OutputMode::CCs:
			midiOut.sendControlChange(midiChannel, 15, 1);
			notify("> SENDING CC 15");
			break;
			break;
		case OutputMode::NoteGrid:

			break;
		default:
			break;
		}
	}
	else if (key == OF_KEY_UP || key == OF_KEY_DOWN)
	{
		shouldFade = true;
	//} else {
		//ofLog(OF_LOG_NOTICE, "key %d", key);
	}
}

void ofApp::notify(std::string message) {
	notificationContent = message;
	notificationCounter = notificationDuration;
	notificationVisible = true;
}

void ofApp::sendControlChange(int channel, int cc, int value) {
	if (lastSent[channel-1][cc-1] != value) {
		lastSent[channel-1][cc-1] = value;
    midiOut.sendControlChange(channel, cc, value);
  }
}

void ofApp::sendPitchBend(int channel, int value) {
	if (lastPitchBend != value) {
		lastPitchBend = value;
    midiOut.sendPitchBend(channel, value);
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	penX = x;
	penY = y;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	/*fbo.begin();
	//ofLog(OF_LOG_NOTICE, "button %d", button);
	//notify("> BTTON : " + std::to_string(button));
	if (button > 0) {

		ofSetColor(255, 255, 255, 255);
		ofFill();
		ofSetLineWidth(strokeWidth*4);
		ofDrawLine(penX, penY, x, y);
		ofDrawCircle(penX, penY, floor(strokeWidth / 2 - 1)*4);
	}
	else {
		// left button, draw
		ofSetColor(0, 0, 0, 255);
		ofFill();
		ofSetLineWidth(strokeWidth);
		ofDrawLine(penX, penY, x, y);
		ofDrawCircle(penX, penY, floor(strokeWidth / 2 - 1));
	}
	fbo.end();
	*/
	penX = x;
	penY = y;


	switch (outputMode)
	{
	case OutputMode::noteBendMod:
		// send vertical position offset as pitch bend
		sendPitchBend(midiChannel, 8192 + ofClamp((startY - penY) * 8, -8192, 8191));
		// send horizontal position as mod wheel value
		sendControlChange(midiChannel, 1, ofMap(penX, 0, appWidth, 0, 127, true));
		break;

	case OutputMode::CCs:
		// send horizontal position as CC14
		sendControlChange(midiChannel, 14, ofMap(penX, 0, appWidth, 0, 127, true));
		// send horizontal position as CC15
		sendControlChange(midiChannel, 15, ofMap(appHeight - penY, 0, appHeight, 0, 127, true));
		break;
	default:
		break;
	}

	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	//ofLog(OF_LOG_NOTICE, "button %d", button);
	penX = x;
	penY = y;
	startX = x;
	startY = y;
	lastPenX = x;
	lastPenY = y;
	penDown = true;
	/*
	fbo.begin();
	ofFill();
	if (button > 0) {
		ofSetColor(255, 255, 255, 255);
		ofFill();
		ofDrawCircle(penX, penY, floor(strokeWidth / 2)*4);
	}
	else {
		ofSetColor(0, 0, 0, 255);
		ofFill();
		ofDrawCircle(penX, penY, floor(strokeWidth / 2));
	}
	fbo.end();
*/

	switch (outputMode)
	{
	case OutputMode::noteBendMod:
		currentNote = ofMap(y, 0, appHeight, 96, 24);
		sendPitchBend(midiChannel, 8192);
		sendControlChange(midiChannel, 1, ofMap(penX, 0, appWidth, 0, 127, true));
		midiOut.sendNoteOn(midiChannel, currentNote, 100);
		break;

	case OutputMode::CCs:
		midiOut.sendNoteOn(midiChannel, 56, 127);
		sendControlChange(midiChannel, 14, ofMap(penX, 0, appWidth, 0, 127, true));
		sendControlChange(midiChannel, 15, ofMap(appHeight - penY, 0, appHeight, 0, 127, true));
		break;
	default:
		break;
	}

	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	penDown = false;
	switch (outputMode)
	{
	case OutputMode::noteBendMod:
		midiOut.sendNoteOff(midiChannel, currentNote, 100);
		currentNote = -1;
		break;

	case OutputMode::CCs:
		midiOut.sendNoteOff(midiChannel, 56, 100);
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

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
