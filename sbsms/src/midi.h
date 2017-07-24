#ifndef MIDI_H
#define MIDI_H

#include "audioeffectx.h"

enum {
  PITCHBEND_MAX = 16384,
  PITCHBEND_NULL = 8192
};

const double NOTE_UP_SCALAR = 1.059463094359295264561825294946;
const long EVENTS_QUEUE_MAX = 32768;

enum {
	midiNote,
	midiPitchbend,
	midiNotesOff,
	midiChannelAfterTouch,
  midiPolyphonicAfterTouch,
	midiControl,
	midiNull
};

struct BlockEvents {
	int eventStatus;	// the event status MIDI byte
	int byte1;	// the first MIDI data byte
	int byte2;	// the second MIDI data byte
	long delta;	// the delta offset (the sample position in the current block where the event occurs)
	int channel;	// the MIDI channel
};

void processMIDIEvents(VstEvents *events, BlockEvents *blockEvents, int *numBlockEvents);

#endif
