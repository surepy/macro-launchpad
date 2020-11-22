#pragma once
#include "RtMidi.h"

// this namespace organization does not make any sense.
namespace launchpad {
    enum class mode {
        session = 108,
        user1,
        user2,
        mixer
    };

    namespace config {


        class ButtonBase {
            unsigned char color;
        public:
            ButtonBase() : color(0x0C) {}
            virtual ~ButtonBase() {}
            virtual void execute() = 0;
            inline void set_color(unsigned char col) { color = col; };
            inline unsigned char get_color() { return color; };
        };

        class ButtonSimpleMacro : public ButtonBase {
            int keycode;
        public:
            ButtonSimpleMacro() : keycode(-1) {}
            ButtonSimpleMacro(int keycode) : keycode(keycode) {}
            void execute();
        };

        class ButtonComplexMacro : public ButtonBase {
        public:
            void execute();
        };
    }

    typedef std::array<std::array<launchpad::config::ButtonBase*, 8>, 8> launchpad_grid;

    class Launchpad {
        // lol temp
        inline static bool execute_all = true;
        
        // TODO: multiple device support and think of an actual working execution flow which makes sense 
        // what the FUCK is this shit
        inline static Launchpad* main_device;

        bool should_loop;
        void Loop();

        launchpad::config::ButtonBase* get_button(unsigned char num);

        // https://www.music.mcgill.ca/~gary/rtmidi/
        RtMidiIn* in;
        RtMidiOut* out;

        mode mode = mode::session;
        unsigned int page = 0;

        std::vector<launchpad_grid*> pages;

    public:
        Launchpad() : should_loop(true) {
            try {
                in = new RtMidiIn();
                out = new RtMidiOut();
            }
            catch (RtMidiError& error) {
                _DebugString(error.getMessage());
                exit(EXIT_FAILURE);
            }
        };

        void Init();
        void sendMessage(unsigned char* message);
        void fullLedUpdate();
        void setup_pages();

        // launchpad defined.
        void reset();
        //void select_grid_mapping_mode();
        void low_brightness_test();
        void medium_brightness_test();
        void full_brightness_test();


        static void RunDevice();
        static void TerminateDevice();

        // testing purposes thing proof of consept 1 device thing
        // please fix later
        inline static Launchpad* GetDevice() { return main_device; }
    };


    // look at the Launchpad Programmer’s Reference.
    namespace commands {

        enum led_brightness {
            off = 0,
            low,
            medium,
            high
        };

        // pre-caluclated values.
        constexpr unsigned char vel_off_off = 0x0C;
        constexpr unsigned char vel_red_low = 0x0D;
        constexpr unsigned char vel_red_full = 0x0F;

        constexpr unsigned char vel_yellow_full = 0x3E;

        constexpr unsigned char vel_red_full_flashing = 0x0B;

        inline unsigned char calculate_velocity(int green, int red) {
            return (0x10 * green) + red + 0x0C;
        }

        inline unsigned char calculate_velocity(int green, int red, unsigned char flags) {
            return (0x10 * green) + red + flags;
        }

        inline unsigned char* controller_change(unsigned char controller, unsigned char data) {
            return new unsigned char[3] { 0xB0, controller, data};
        }

        // note: don't use these two functions for setting the LEDS for the automap/ Live control LEDs, they are 0xB0.
        inline unsigned char* led_off(unsigned char key, unsigned char velocity) {
            return new unsigned char[3] { 0x80, key, velocity};
        }

        inline unsigned char* led_on(unsigned char key, unsigned char velocity) {
            return new unsigned char[3] { 0x90, key, velocity};
        }

        inline unsigned char calculate_grid(unsigned char row, unsigned char column) {
            return (0x10 * row) + column;
        }

        inline void calculate_xy_fom_keycode(unsigned char keycode, int &x, int &y) {
            x = keycode / 0x10;
            y = keycode % 0x10;
        }

        constexpr unsigned char reset[3] = { 0xB0, 0x00, 0x00 };
        constexpr unsigned char brightness_test_low[3] = { 0xB0, 0x00, 0x7D };
        constexpr unsigned char brightness_test_med[3] = { 0xB0, 0x00, 0x7E };
        constexpr unsigned char brightness_test_full[3] = { 0xB0, 0x00, 0x7F };
    }
};