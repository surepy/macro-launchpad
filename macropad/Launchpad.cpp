#include "Launchpad.h"
#include "macropad.h"

void launchpad::Launchpad::Init() {
    unsigned int nPorts = in->getPortCount();
    _DebugString("There are " + std::to_string(nPorts) + " MIDI input sources available.\n");
    std::string portName;
    for (unsigned int i = 0; i < nPorts; i++) {
        try {
            portName = in->getPortName(i);
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }
        _DebugString("  Input Port #" + std::to_string(i) + ": " + portName +  "\n");

        if (portName.find("Launchpad S") != std::string::npos) {
            _DebugString("Using input port " + std::to_string(i) + ".\n");
            in->openPort(i);
            break;
        }
    }

    // Don't ignore sysex, timing, or active sensing messages.
    in->ignoreTypes(false, false, false);

    nPorts = out->getPortCount();
    _DebugString("There are " + std::to_string(nPorts) + " MIDI output sources available.\n");
    for (unsigned int i = 0; i < nPorts; i++) {
        try {
            portName = out->getPortName(i);
        }
        catch (RtMidiError& error) {
            error.printMessage();
        }
        _DebugString("  Output Port #" + std::to_string(i) + ": " + portName + "\n");

        if (portName.find("Launchpad S") != std::string::npos) {
            _DebugString("Using output port " + std::to_string(i) + ".\n");
            out->openPort(i);
            break;
        }
    }


    this->fullLedUpdate();
}

void launchpad::Launchpad::reset()
{
    out->sendMessage(launchpad::commands::reset, sizeof(unsigned char) * 3);
}

void launchpad::Launchpad::low_brightness_test()
{
    out->sendMessage(launchpad::commands::brightness_test_low, sizeof(unsigned char) * 3);
}

void launchpad::Launchpad::medium_brightness_test()
{
    out->sendMessage(launchpad::commands::brightness_test_med, sizeof(unsigned char) * 3);
}

void launchpad::Launchpad::full_brightness_test()
{
    out->sendMessage(launchpad::commands::brightness_test_full, sizeof(unsigned char) * 3);
}

void launchpad::Launchpad::RunDevice()
{
    main_device = new Launchpad();
    main_device->Init();
    main_device->Loop();
}

/// <summary>
/// launchpad input loop...
/// </summary>
void launchpad::Launchpad::Loop() {
    std::vector<unsigned char> message;
    int nBytes, i;
    double stamp;

    while (should_loop && execute_all)
    {
        stamp = in->getMessage(&message);
        nBytes = message.size();

        // no message. skip
        if (nBytes == 0) {
            continue;
        }

        for (i = 0; i < nBytes; i++)
            _DebugString("Byte " + std::to_string(i) + " = " + std::to_string((int)message[i]) + ", ");
        if (nBytes > 0)
            _DebugString("stamp = " + std::to_string(stamp) + "\n");
        
        if (nBytes == 3) {
            // grid button pressed or released.
            if (message[0] == 0x90) {
                // we want to only handle inside grid buttons, not the "page" side buttons.
                if (message[1] % 0x10 <= 0x07) {
                    // pressed
                    if (message[2] == 0x7F) {
                        this->sendMessage(launchpad::commands::led_on(message[1], launchpad::commands::vel_red_full));
                    }
                    // released.
                    else if (message[2] == 0x00) {
                        // TODO: revert to previous state.
                        this->sendMessage(launchpad::commands::led_off(message[1], launchpad::commands::vel_off_off));
                    }
                }
                else {
                    // pressed
                    if (message[2] == 0x7F) {
                        // change page.
                        page = message[1] / 0x10;

                        this->fullLedUpdate();
                    }
                    // released.
                    else if (message[2] == 0x00) {
                    }
                }

            }
            // Automap/Live buttons pressed or released.
            else if (message[0] == 0xB0) {
                // pressed
                if (message[2] == 0x7F) {
                    if (message[1] >= 108)
                        mode = static_cast<launchpad::mode>(message[1]);

                    this->fullLedUpdate();
                }
                // released.
                else if (message[2] == 0x00) {

                }
            }
        }
        message.clear();
    }

    // end of loop. reset
    this->reset();
}

// custom calculated messages go here
void launchpad::Launchpad::sendMessage(unsigned char* message)
{
    try {
        out->sendMessage(message, sizeof(unsigned char) * 3);
    }
    catch (RtMidiError& error) {
        error.printMessage();
        _DebugString(error.getMessage());
    }
    
    delete message;
}

// this will literally update EVERYTHING. do NOT call this functionm unless you ABSOLUTELY NEED TO!
void launchpad::Launchpad::fullLedUpdate()
{
    // reset everything first.
    out->sendMessage(launchpad::commands::reset, sizeof(unsigned char) * 3);

    // set our page indicator
    this->sendMessage(launchpad::commands::led_on(0x10 * page + 0x08, launchpad::commands::vel_yellow_full));

    // set our "mode" indicator
    this->sendMessage(new unsigned char[3]{ 0xB0, (unsigned char)mode, launchpad::commands::vel_yellow_full });

}

void launchpad::Launchpad::TerminateDevice()
{
    execute_all = false;
}

