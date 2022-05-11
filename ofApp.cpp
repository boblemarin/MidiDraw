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
	strokeWidth = 3;
	settingsMode = none;
	outputMode = noteBendMod;

	// init notifications
	notificationDuration = 200;
	notificationVisible = false;
	notify(("> STARTING"));

	// FBO
	if (DEBUG) {
		fbo.allocate(960, 540, GL_RGBA);
		//senderWidth = 960;
		//senderHeight = 540;
	}
	else {
		fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
		//senderWidth = ofGetWidth();
		//senderHeight = ofGetHeight();
	}
	//
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
	ndiSender.CreateSender(senderName, senderWidth, senderHeight);
	*/
	ofSetWindowTitle("NOISE PROJECT");
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
			settingsMode = none;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (penDown) {
		ofClear(255, 235, 235, 255);
	}
	else {
		ofClear(255, 255, 255, 255);
	}
	fbo.draw(0, 0);

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
		fbo.begin();
		ofClear(255, 255, 255, 0);
		fbo.end();

		settingsMode = none;
		notify(("> IMAGE SAVED"));
	}
	else if (key > 48 && key < 53) { // stroke width selector (numpad 1>4)
		settingsMode = none;
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
NUMPAD 1-4  CHANGE STROKE SIZE");
	} else if (key == OF_KEY_F2) { // F2
		if (settingsMode == outputPort) {
			midiOut.closePort();
			midiOutput = ++midiOutput % midiOut.getNumOutPorts();
			midiOut.openPort(midiOutput);
			notify(("> CHANGED OUTPUT : ") + midiOut.getOutPortName(midiOutput));
			
		}
		else {
			settingsMode = outputPort;
			notify(("> CURRENT OUTPUT : ") + midiOut.getOutPortName(midiOutput));
		}
	}
	else if (key == OF_KEY_F3) { // F3
		if (settingsMode == channel) {
			if (++midiChannel > 16) midiChannel = 1;
			notify("> MIDI CHANNEL : " + std::to_string(midiChannel));
		}
		else {
			settingsMode = channel;
			notify("> MIDI CHANNEL : " + std::to_string(midiChannel));
		}
	}
	else if (key == OF_KEY_F4) { // F4
		if (settingsMode == outputType) {
			switch (outputMode)
			{
			case noteBendMod:
				if (currentNote < 0) {
					midiOut.sendNoteOff(midiChannel, currentNote, 100);
					currentNote = -1;
				}
				outputMode = CCs;
				break;
			case CCs:
				if (penDown < 0) {
					midiOut.sendNoteOff(midiChannel, 56, 100);
				}
				outputMode = NoteGrid;
				break;
			case NoteGrid:
				outputMode = noteBendMod;
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
			settingsMode = outputType;
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
	case noteBendMod:
		midiOut.sendControlChange(midiChannel, 1, 1);
		notify("> SENDING CC 1 (MOD)");
		break;
	case CCs:
		midiOut.sendControlChange(midiChannel, 14, 1);
		notify("> SENDING CC 14");
		break;
		break;
	case NoteGrid:
		break;
	default:
		break;
	}
	}
	else if (key == 'x') { // X
	switch (outputMode)
	{
	case noteBendMod:
		midiOut.sendPitchBend(midiChannel, 8192);
		notify("> SENDING PITCH BEND");
		break;
	case CCs:
		midiOut.sendControlChange(midiChannel, 15, 1);
		notify("> SENDING CC 15");
		break;
		break;
	case NoteGrid:

		break;
	default:
		break;
	}
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
	fbo.begin();
	//ofLog(OF_LOG_NOTICE, "button %d", button);
	notify("> BTTON : " + std::to_string(button));
	if (button > 0) {
		// right button, try to erase
		/*ofSetColor(0, 0, 0, 0);
		//ofSetLineWidth(40);
		//ofCircle(x, y, 20);
		ofFill();
		ofDrawCircle(x, y, 40);*/
		// left button, draw
		ofSetColor(255, 255, 255, 255);
		ofSetLineWidth(strokeWidth*4);
		ofDrawLine(penX, penY, x, y);
		ofDrawCircle(penX, penY, floor(strokeWidth / 2 - 1)*4);
	}
	else {
		// left button, draw
		ofSetColor(0, 0, 0, 255);
		ofSetLineWidth(strokeWidth);
		ofDrawLine(penX, penY, x, y);
		ofDrawCircle(penX, penY, floor(strokeWidth / 2 - 1));
	}
	fbo.end();
	penX = x;
	penY = y;


	switch (outputMode)
	{
	case noteBendMod:
		// send vertical position offset as pitch bend
		sendPitchBend(midiChannel, 8192 + ofClamp((startY - penY) * 8, -8192, 8191));
		// send horizontal position as mod wheel value
		sendControlChange(midiChannel, 1, ofMap(penX, 0, ofGetWidth(), 0, 127, true));
		break;

	case CCs:
		// send horizontal position as CC14
		sendControlChange(midiChannel, 14, ofMap(penX, 0, ofGetWidth(), 0, 127, true));
		// send horizontal position as CC15
		sendControlChange(midiChannel, 15, ofMap(ofGetHeight() - penY, 0, ofGetHeight(), 0, 127, true));
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
	penDown = true;
	/**/
	fbo.begin();
	ofFill();
	if (button > 0) {
		ofSetColor(255, 255, 255, 255);
		ofDrawCircle(penX, penY, floor(strokeWidth / 2)*4);
	}
	else {
		ofSetColor(0, 0, 0, 255);
		ofDrawCircle(penX, penY, floor(strokeWidth / 2));
	}
	fbo.end();


	switch (outputMode)
	{
	case noteBendMod:
		currentNote = ofMap(y, 0, ofGetHeight(), 96, 24);
		sendPitchBend(midiChannel, 8192);
		sendControlChange(midiChannel, 1, ofMap(penX, 0, ofGetWidth(), 0, 127, true));
		midiOut.sendNoteOn(midiChannel, currentNote, 100);
		break;

	case CCs:
		midiOut.sendNoteOn(midiChannel, 56, 127);
		sendControlChange(midiChannel, 14, ofMap(penX, 0, ofGetWidth(), 0, 127, true));
		sendControlChange(midiChannel, 15, ofMap(ofGetHeight() - penY, 0, ofGetHeight(), 0, 127, true));
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
	case noteBendMod:
		midiOut.sendNoteOff(midiChannel, currentNote, 100);
		currentNote = -1;
		break;

	case CCs:
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
