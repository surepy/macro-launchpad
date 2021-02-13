#pragma once

namespace midi_device {
	class MidiDeviceBase {
	protected:
		// https://www.music.mcgill.ca/~gary/rtmidi/
		RtMidiIn* in;
		RtMidiOut* out;
	};

	extern std::vector<MidiDeviceBase*> devices;
}