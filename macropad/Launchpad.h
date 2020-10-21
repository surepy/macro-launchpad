#pragma once
#include "RtMidi.h"

namespace launchpad {
    enum class mode {
        session = 108,
        user1,
        user2,
        mixer
    };

    class Launchpad {
        bool bShouldLoop;
        void Loop();

        // https://www.music.mcgill.ca/~gary/rtmidi/
        RtMidiIn* in;
        RtMidiOut* out;

        void sendMessage(unsigned char* message);

        void fullLedUpdate();

        mode mode = mode::session;
        unsigned int page = 0;

    public:
        Launchpad() : bShouldLoop(true) {
            try {
                in = new RtMidiIn();
                out = new RtMidiOut();
            }
            catch (RtMidiError& error) {
                error.printMessage();
                exit(EXIT_FAILURE);
            }
        };

        void Init();
        static void InitDevice();
        static void Terminate();
    };


    // look at the Launchpad Programmer’s Reference.
    namespace commands {
        constexpr unsigned char vel_off_off = 0x0C;
        constexpr unsigned char vel_red_low = 0x0D;
        constexpr unsigned char vel_red_full = 0x0F;

        constexpr unsigned char vel_yellow_full = 0x3E;

        constexpr unsigned char vel_red_full_flashing = 0x0B;

        inline unsigned char calculate_velocity(unsigned char green, unsigned char red) {
            return (0x10 * green) + red + 0x0C;
        }
        inline unsigned char calculate_velocity(unsigned char green, unsigned char red, unsigned char flags) {
            return (0x10 * green) + red + flags;
        }

        inline unsigned char* controller_change(unsigned char controller, unsigned char data) {
            return new unsigned char[3] { 0xB0, controller, data};
        }

        inline unsigned char* led_off(unsigned char key, unsigned char velocity) {
            return new unsigned char[3] { 0x80, key, velocity};
        }

        inline unsigned char* led_on(unsigned char key, unsigned char velocity) {
            return new unsigned char[3] { 0x90, key, velocity};
        }

        constexpr unsigned char reset[3] = { 0xB0, 0x00, 0x00 };
        constexpr unsigned char brightness_test_low[3] = { 0xB0, 0x00, 0x7D };
        constexpr unsigned char brightness_test_med[3] = { 0xB0, 0x00, 0x7E };
        constexpr unsigned char brightness_test_full[3] = { 0xB0, 0x00, 0x7F };
    }

    
};