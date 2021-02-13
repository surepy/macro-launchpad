#include "framework.h"
#include "MidiDevice.h"
#include "RtMidi.h"

namespace midi_device {
	std::vector<MidiDeviceBase*> devices = std::vector<MidiDeviceBase*>();
};