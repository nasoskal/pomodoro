#include "SevSeg.h"
#include "EEPROM.h"

SevSeg sevseg;

int addr = 0; // address at internal EEPROM


const int buzzerPin = A5;
unsigned long buzzerDuration = 12;
unsigned long buzzerDuration_long = buzzerDuration*4;
int buzzerFreq = 2480; 
const int ledPin = 3;

const int mb = A0; // the middle button
volatile boolean mb_state; // the current state of the middle button 

const int sb = A1; // the secondary(side) button
volatile boolean sb_state; // the current state of the middle button 
unsigned long longPress = 400; // The long hold time in millis


volatile char time[5];

volatile boolean isBreakMode; // true if in break mode
volatile boolean isSilent; // true if in silent mode

volatile unsigned long mils; // used to keep track of time passed
volatile unsigned long mils2; // used to keep track of time passed
volatile unsigned long debounceTime; // used for button debounce
volatile boolean actionlock; // used to prevent accidental button hit


unsigned long interval; // 1 sec accuracy countdown

volatile boolean mode;
byte startMinutes;
byte startSeconds;
byte breakMinutes;
byte breakSeconds;

byte startMinutes1;
byte startSeconds1; 
byte breakMinutes1;
byte breakSeconds1;

byte startMinutes2;
byte startSeconds2; 
byte breakMinutes2;
byte breakSeconds2;

// used for set()
byte setMinutes;
byte setSeconds; 
byte setBreakMinutes;
byte setBreakSeconds;
volatile int phase = 1; // 
volatile byte digit[9];








void setup(){
	pinMode(ledPin, OUTPUT);
	pinMode(buzzerPin, OUTPUT);
	pinMode(mb, INPUT);
	pinMode(sb, INPUT);

	isBreakMode = 0;
	isSilent = 0;
	interval = 998; // fine-tune (not reliable)
	mode = 0;

	// load settings from EEPROM
	startMinutes1 = EEPROM.read(addr);
	startSeconds1 = EEPROM.read(addr+1);
	breakMinutes1 = EEPROM.read(addr+2);
	breakSeconds1 = EEPROM.read(addr+3);

	startMinutes2 = EEPROM.read(addr+4);
	startSeconds2 = EEPROM.read(addr+5);
	breakMinutes2 = EEPROM.read(addr+6);
	breakSeconds2 = EEPROM.read(addr+7);

	isSilent = EEPROM.read(addr+8);

	byte numDigits = 4;
	byte digitPins[] = {13, 12, 11, 10};
	byte segmentPins[] = {9, 7, 2, 5, 6, 8, A4, 4};
	bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
	byte hardwareConfig = COMMON_CATHODE; // See README.md for options
	bool updateWithDelays = false; // Default 'false' is Recommended
	bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
	bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

	sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
	updateWithDelays, leadingZeros, disableDecPoint);

	sevseg.setBrightness(10);
	Serial.begin(9600);
}


void loop(){
	idle();
}

void idle(){ // aa 
	sevseg.setChars("--.--");
	beep(1);


	// RESETING THE TIME
	if (!isBreakMode) {
		if (mode == 0) {
			startMinutes = startMinutes1;
			startSeconds = startSeconds1;
			breakMinutes = breakMinutes1;
			breakSeconds = breakSeconds1;
		}
		else {
			startMinutes = startMinutes2;
			startSeconds = startSeconds2;
			breakMinutes = breakMinutes2;
			breakSeconds = breakSeconds2;
		}
	}

	mils = millis();
	while (millis() - mils < 300 || digitalRead(mb) == 0 || digitalRead(sb) == 0) sevseg.refreshDisplay(); // wait

	snprintf(time, 6, "%02d.%02d", startMinutes, startSeconds);
	sevseg.setChars(time);
	while(1){
		sevseg.refreshDisplay();

		debounceTime = millis();
		while (millis() - debounceTime < 100){
			sevseg.refreshDisplay();
		}

		if (digitalRead(mb) == 0) {
			mils2 = 0;
			while (digitalRead(mb) == 0){
				mils = millis();
				while (millis() - mils < 20){
					sevseg.refreshDisplay();
				}
				mils2 += 20;

				if (mils2 > longPress){
					if (isBreakMode) isBreakMode = 0, flash(0), idle(); // allow to reset when in break
					set();
					return;
				}
				
			}
			countdown();
			return;
		}
		else if (digitalRead(sb) == 0) {
			mils2 = 0;
			while (digitalRead(sb) == 0){
				mils = millis();
				while (millis() - mils < 20){
					sevseg.refreshDisplay();
				}
				mils2 += 20;

				if (mils2 > longPress){
					toggleSilent();
					idle();
				}
				
			}
			changeMode();
			return;
		}
	}
}

void countdown(){ // cc
	while(1){
		snprintf(time, 6, "%02d.%02d", startMinutes, startSeconds);
		sevseg.setChars(time);

		mils = millis();
		while(millis() - mils <= interval){
			sevseg.refreshDisplay();
			if (digitalRead(mb) == 0) {
				mils2 = 0;
				while (digitalRead(mb) == 0){
					mils = millis();
					while (millis() - mils < 20){
						sevseg.refreshDisplay();
					}
					mils2 += 20;

					if (mils2 > longPress){
						idle();
					}
					
				}
				pause();
			}
			else if (digitalRead(sb) == 0) {
				mils2 = 0;
				while (digitalRead(sb) == 0){
					mils = millis();
					while (millis() - mils < 20){
						sevseg.refreshDisplay();
					}
					mils2 += 20;

					if (mils2 > longPress){
						toggleSilent();
					}
					
				}
				incr();
			}
		}

		if (!(startMinutes == 0 && startSeconds == 0)) {
			if (startSeconds > 0) startSeconds--;
			else startMinutes--, startSeconds = 59;
		}
		else {
			if (!isBreakMode) isBreakMode = 1, beep(1), takeBreak();
			else isBreakMode = 0, beep(0), finish(), idle();
		}
	}
}

void incr(){ // ii
	if (startMinutes == 59) return;
	startMinutes += 1;
	snprintf(time, 6, "%02d.%02d", startMinutes, startSeconds);
	sevseg.setChars(time);
}

void finish(){
	sevseg.setChars("00.00");

	for(int i=0; i<3; i++){
		mils = millis();
		while(millis() - mils < 400){
			sevseg.refreshDisplay();
		}

		flash(0);
		beep(0);
	}
	return;

}

void takeBreak(){
	if (mode == 0) {
		startMinutes = breakMinutes1;
		startSeconds = breakSeconds1;
	}
	else {
		startMinutes = breakMinutes2;
		startSeconds = breakSeconds2;
	}
	digitalWrite(ledPin, HIGH);
	idle();	
}

void pause(){ // pp
	debounceTime = millis();
	while (millis() - debounceTime < 200){
		sevseg.refreshDisplay();
	}
	while (1){
		if (digitalRead(mb) == 0) {
			mils2 = 0;
			while (digitalRead(mb) == 0){
				mils = millis();
				while (millis() - mils < 20){
					sevseg.refreshDisplay();
				}
				mils2 += 20;

				if (mils2 > longPress){
					idle();
				}
				
			}
			return; // resume countdown
		}
		else if (digitalRead(sb) == 0) {
			mils2 = 0;
			while (digitalRead(sb) == 0){
				mils = millis();
				while (millis() - mils < 20){
					sevseg.refreshDisplay();
				}
				mils2 += 20;

				if (mils2 > longPress){
					toggleSilent();
				}
				
			}
			incr();
		}
		if (digitalRead(mb) == 0) {
			debounceTime = millis();
			while (millis() - debounceTime < 200){
				sevseg.refreshDisplay();
			}
			return;

		}
		sevseg.refreshDisplay();
	}
}

void set(){ // ss
	sevseg.setChars("SET");
	beep(1);
	mils = millis();
	while(millis() - mils < 500) sevseg.refreshDisplay(); // wait

	unsigned long blinkInterval = 380;
	phase = 1;

	if (mode == 0){
		setMinutes = startMinutes1;
		setSeconds = startSeconds1;
		setBreakMinutes = breakMinutes1;
		setBreakSeconds = breakSeconds1;
	}
	else{
		setMinutes = startMinutes2;
		setSeconds = startSeconds2;
		setBreakMinutes = breakMinutes2;
		setBreakSeconds = breakSeconds2;
	}

	digit[0] = -1;
	digit[1] = setMinutes /10;
	digit[2] = setMinutes % 10;
	digit[3] = setSeconds /10;
	digit[4] = setSeconds % 10;
	digit[5] = setBreakMinutes /10;
	digit[6] = setBreakMinutes % 10;
	digit[7] = setBreakSeconds /10;
	digit[8] = setBreakSeconds % 10;



	while(1){
		switch(phase){
			case 1:
				snprintf(time, 6, "% d.%02d", digit[2], setSeconds);
				break;
			case 2:
				snprintf(time, 6, "%d .%02d", digit[1], setSeconds);
				break;
			case 3:
				snprintf(time, 6, "%02d.% d", setMinutes, digit[4]);
				break;
			case 4:
				snprintf(time, 6, "%02d.%d ", setMinutes, digit[3]);
				break;
			case 5:
				snprintf(time, 6, "% d.%02d", digit[6], setBreakSeconds);
				break;
			case 6:
				snprintf(time, 6, "%d .%02d", digit[5], setBreakSeconds);
				break;
			case 7:
				snprintf(time, 6, "%02d.% d", setBreakMinutes, digit[8]);
				break;
			case 8:
				snprintf(time, 6, "%02d.%d ", setBreakMinutes, digit[7]);
				break;

		}
		sevseg.setChars(time);
		mils = millis();
		actionlock = false; // prevents quick button presses
		while(millis() - mils < blinkInterval) sevseg.refreshDisplay(), checkButtons(); // during

		if (phase < 5) snprintf(time, 6, "%02d.%02d", setMinutes, setSeconds);
		else snprintf(time, 6, "%02d.%02d", setBreakMinutes, setBreakSeconds);
		
		sevseg.setChars(time);
		mils = millis();
		actionlock = false; // prevents quick button presses
		while(millis() - mils < blinkInterval) sevseg.refreshDisplay(), checkButtons(); // during blink

	}
}

void checkButtons(){ // used in set
	if (actionlock) return;
	if (digitalRead(mb) == 0) {
		mils2 = 0;
		while (digitalRead(mb) == 0){
			mils = millis();
			while (millis() - mils < 20){
				sevseg.refreshDisplay();
			}
			mils2 += 20;

			if (mils2 > longPress){
				// save settings to memory
				EEPROM.update(addr, startMinutes1);
				EEPROM.update(addr+1, startSeconds1);
				EEPROM.update(addr+2, breakMinutes1);
				EEPROM.update(addr+3, breakSeconds1);

				EEPROM.update(addr+4, startMinutes2);
				EEPROM.update(addr+5, startSeconds2);
				EEPROM.update(addr+6, breakMinutes2);
				EEPROM.update(addr+7, breakSeconds2);

				idle();
			}
			
		}
		if (phase < 5) changePhase(1);
		else changePhase(0);
		actionlock = true;
		
	}
	else if (digitalRead(sb) == 0) {
		mils2 = 0;
		while (digitalRead(sb) == 0){
			mils = millis();
			while (millis() - mils < 20){
				sevseg.refreshDisplay();
			}
			mils2 += 20;

			if (mils2 > longPress){

				// toggle between break time and work time
				if (phase < 5) phase = 5;
				else phase = 1; 
				actionlock = true;

				return;
			}
			
		}
		if (actionlock) return;

		switch(phase){ // nice effect
			case 1: 
			case 5:
				digit[phase] = changeDigit(digit[phase], 1);
				snprintf(time, 6, "%d", digit[phase]);
				break;
			case 2:
			case 6:
				digit[phase] = changeDigit(digit[phase], 0);
				snprintf(time, 6, "% d", digit[phase]);
				break;
			case 3:
			case 7:
				digit[phase] = changeDigit(digit[phase], 1);
				snprintf(time, 6, "%3d", digit[phase]);
				break;
			case 4:
			case 8:
				digit[phase] = changeDigit(digit[phase], 0);
				snprintf(time, 6, "%4d", digit[phase]);
				break;
		}
		sevseg.setChars(time);

		if (phase < 5) {
			setMinutes = digit[1]*10 + digit[2];
			setSeconds = digit[3]*10 + digit[4];
		}
		else {
			setBreakMinutes = digit[5]*10 + digit[6];
			setBreakSeconds = digit[7]*10 + digit[8];
		}


		// changing the presets
		if (mode == 0) {
			startMinutes1 = setMinutes;
			startSeconds1 = setSeconds;
			breakMinutes1 = setBreakMinutes;
			breakSeconds1 = setBreakSeconds;
		}
		else {
			startMinutes2 = setMinutes;
			startSeconds2 = setSeconds;
			breakMinutes2 = setBreakMinutes;
			breakSeconds2 = setBreakSeconds;
		}

		return;
	}
}

void beep(boolean small){ //bb
	if (isSilent) return;
	if (small) tone(buzzerPin, buzzerFreq, buzzerDuration);
	else tone(buzzerPin, buzzerFreq, buzzerDuration_long);
		
}

void flash(boolean smooth){
	if (smooth) {
	}
	else {
		mils2 = millis();
		while(millis() - mils2 < 500){
			sevseg.refreshDisplay();
			digitalWrite(ledPin, HIGH);
		}
		mils2 = millis();
		while(millis() - mils2 < 500){
			sevseg.refreshDisplay();
			digitalWrite(ledPin, LOW);
		}

	}
}

int changeDigit(int digit, boolean upto5){
	int temp;
	if (!upto5) {
		if (digit == 9) temp = 0;
		else temp = digit+1;
		return temp;
	}
	else{
		if (digit >= 5) temp = 0;
		else temp = digit+1;
		return temp;
	}
}

void changeMode(){
	if (mode == 0) {
		startMinutes = startMinutes2;
		startSeconds = startSeconds2;
		breakMinutes = breakMinutes2;
		breakSeconds = breakSeconds2;
	}
	else {
		startMinutes = startMinutes1;
		startSeconds = startSeconds1;
		breakMinutes = breakMinutes1;
		breakSeconds = breakSeconds1;
	}
	mode = !mode;
	sevseg.setChars("chng");
	beep(1);

	mils = millis();
	while(millis() - mils < 500){
		sevseg.refreshDisplay();
	}
	idle();
}

void toggleSilent(){
	isSilent = !isSilent;
	EEPROM.update(addr+8, isSilent); // save setting

	if (!isSilent) sevseg.setChars("on"), beep(1);
	else sevseg.setChars("off");
	mils = millis();
	while (millis() - mils < 500) sevseg.refreshDisplay(); // wait
}

void changePhase(boolean oneToFour){
	if (oneToFour) {
		if (phase >= 4 || phase < 1) phase = 1;
		else phase++;
	}
	else{
		if (phase >= 8 || phase < 5) phase = 5;
		else phase++;
	}
}
