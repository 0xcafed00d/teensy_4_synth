#include <Arduino.h>
#include <Audio.h>
#include <MIDI.h>

#include <midi_UsbTransport.h>

static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;

UsbTransport sUsbTransport;
MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, MIDI);

AudioSynthWaveform waveform1;
AudioEffectFreeverb effect;
AudioOutputI2S i2s1;
AudioConnection patchCord0(waveform1, effect);
AudioConnection patchCord1(effect, 0, i2s1, 0);
AudioConnection patchCord2(effect, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;

static bool playing = false;
static byte playingNote = false;
static float bend = 0;

float midiNoteToFreq(int midi_note) {
	int a = 440;  // frequency of A (440Hz)
	return (a / 32) * powf(2, ((float(midi_note) - 9) / 12));
}

void onNoteOff(midi::Channel channel, byte note, byte velocity) {
	if (playing && playingNote == note) {
		waveform1.amplitude(0.0);
		playing = false;
		playingNote = 0;
	}
}

void onNoteOn(midi::Channel channel, byte note, byte velocity) {
	if (!playing && playingNote != note) {
		waveform1.amplitude(0.6);
		waveform1.frequency(midiNoteToFreq(note) + bend);
		playing = true;
		playingNote = note;
	}
}

void onPitchBend(midi::Channel channel, int freq) {
	bend = ((freq - 0x2000) / 500.0f);
}

void onControlChange(midi::Channel channel, byte controller, byte value) {
	// if (channel == 0) {
	switch (controller) {
		case 7:
			effect.roomsize(float(value) / float(0x7f));
			break;
		case 0xa:
			waveform1.begin(value & 3);
			break;
	}
	//}
}

void setup() {
	AudioMemory(10);
	MIDI.begin();
	sgtl5000_1.enable();
	sgtl5000_1.volume(0.25);
	waveform1.begin(WAVEFORM_SAWTOOTH);
	effect.roomsize(0.2f);
	delay(1000);

	MIDI.setHandleNoteOn(onNoteOn);
	MIDI.setHandleNoteOff(onNoteOff);
	MIDI.setHandlePitchBend(onPitchBend);
	MIDI.setHandleControlChange(onControlChange);
}

void loop() {
	MIDI.read();
	if (playing) {
		float freq = midiNoteToFreq(playingNote) + bend;
		waveform1.frequency(freq);
	}
	/*	Serial.print("Beep #");
	    Serial.println(count);
	    count = count + 1;
	    waveform1.frequency(440);
	    waveform1.amplitude(0.9);
	    delay(250);
	    waveform1.amplitude(0);
	    delay(1750); */
}
