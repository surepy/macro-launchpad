#include <windows.h>
#include <array>
#include <sstream>
#include <iomanip>
#include "framework.h"
#include "Launchpad.h"
#include "macropad.h"
#include "Config.h"


// r

bool midi_device::launchpad::execute_all = true;


void midi_device::launchpad::Launchpad::Init() {
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
            try {
                in->openPort(i);
            }
            catch (RtMidiError& error) {
                _DebugString("Failed to use input port.\n");
            }
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
            try {
                out->openPort(i);
            }
            catch (RtMidiError& error) {
                _DebugString("Failed to use output port.\n");
            }
            break;
        }
    }

    this->setup_pages_test();
    this->fullLedUpdate();

    midi_device::devices.push_back(this);
}

void midi_device::launchpad::Launchpad::reset()
{
    // don't do anything.
    if (!out->isPortOpen())
        return;

    out->sendMessage(launchpad::commands::reset, sizeof(unsigned char) * 3);
}

void midi_device::launchpad::Launchpad::low_brightness_test()
{
    // don't do anything.
    if (!out->isPortOpen())
        return;

    out->sendMessage(launchpad::commands::brightness_test_low, sizeof(unsigned char) * 3);
}

void midi_device::launchpad::Launchpad::medium_brightness_test()
{
    // don't do anything.
    if (!out->isPortOpen())
        return;

    out->sendMessage(launchpad::commands::brightness_test_med, sizeof(unsigned char) * 3);
}

void midi_device::launchpad::Launchpad::full_brightness_test()
{
    // don't do anything.
    if (!out->isPortOpen())
        return;

    out->sendMessage(launchpad::commands::brightness_test_full, sizeof(unsigned char) * 3);
}

void midi_device::launchpad::Launchpad::RunDevice()
{
    main_device = new Launchpad();
    main_device->Init();
    main_device->Loop();
}

midi_device::launchpad::message_type midi_device::launchpad::input::message_type() {
    midi_device::launchpad::message_type type = static_cast<midi_device::launchpad::message_type>(message.at(0) + message.at(2));
    
    // this shouldn't happen...
    if (type > message_type::automap_live_pressed || type < message_type::grid_depressed) {
        return message_type::invalid;
    }
    
    if (message.at(1) % 0x10 == 0x08) {
        if (type == message_type::grid_depressed) {
            return message_type::grid_page_change_depressed;
        }
        else if (type == message_type::grid_pressed) {
            return message_type::grid_page_change_pressed;
        }
    }

    return static_cast<midi_device::launchpad::message_type>(message.at(0) + message.at(2));
}

unsigned char midi_device::launchpad::input::keycode() {
    return message.at(1);
}


/// <summary>
/// launchpad input loop...
/// </summary>
void midi_device::launchpad::Launchpad::Loop() {
    std::vector<unsigned char> message;
    int nBytes, i;
    double stamp;
    launchpad::config::ButtonBase* button;

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

        if (nBytes != 3) {
            continue;
        }

        midi_device::launchpad::input input = midi_device::launchpad::input(message);

        switch (input.message_type()) {
        case message_type::grid_pressed: {
            this->sendMessage(launchpad::commands::led_on(input.keycode(), launchpad::commands::vel_red_full));
            break;
        }
        case message_type::grid_depressed: {
            button = get_button(input.keycode());

            if (button == nullptr) {
                this->sendMessage(launchpad::commands::led_off(input.keycode(), launchpad::commands::vel_off_off));
            }
            else {
                button->execute();
                this->sendMessage(launchpad::commands::led_on(input.keycode(), button->get_color()));
            }
            break;
        }
        case message_type::grid_page_change_pressed: {
            // change page.
            page = message[1] / 0x10;

            // update all buttons
            this->fullLedUpdate();
            break;
        }
        case message_type::automap_live_pressed: {
            if (message[1] >= 108) {
                mode = static_cast<launchpad::mode>(message[1]);
            }
            this->fullLedUpdate();
            break;
        }
        case message_type::automap_live_depressed: {
            break;
        }
        }

        button = nullptr;
        message.clear();
    }

    // end of loop. reset
    this->reset();
}

midi_device::launchpad::config::ButtonBase* midi_device::launchpad::Launchpad::get_button(unsigned char key)
{
    if (page >= pages.size()) {
        return nullptr;
    }

    if (pages.at(page) == nullptr) {
        return nullptr;
    }

    int x, y;
    commands::calculate_xy_fom_keycode(key, x, y);

    return pages.at(page)->at(x).at(y);
}

// custom calculated messages go here
void midi_device::launchpad::Launchpad::sendMessage(unsigned char* message)
{
    if (!out->isPortOpen()) {  
        goto cleanup;
    }
    
    try {
        out->sendMessage(message, sizeof(unsigned char) * 3);
    }
    catch (RtMidiError& error) {
        error.printMessage();
        _DebugString(error.getMessage());
    }
    
    cleanup: 
        delete[] message;
}

// this will literally update EVERYTHING. do NOT call this functionm unless you ABSOLUTELY NEED TO!
void midi_device::launchpad::Launchpad::fullLedUpdate()
{
    // don't do anything.
    if (!out->isPortOpen())
        return;

    // reset everything first.
    out->sendMessage(launchpad::commands::reset, sizeof(unsigned char) * 3);

    // set our page indicator
    this->sendMessage(launchpad::commands::led_on(0x10 * page + 0x08, launchpad::commands::vel_yellow_full));

    // set our "mode" indicator
    this->sendMessage(new unsigned char[3]{ 0xB0, (unsigned char)mode, launchpad::commands::vel_yellow_full });

    // update every LEDs.
    if ((size_t)page < pages.size()) {

        // row
        for (size_t row = 0; row < pages.at(page)->size(); ++row) {
            // column
            for (size_t col = 0; col < pages.at(page)->at(row).size(); ++col) {
                if (pages.at(page) == nullptr) {
                    continue;
                }

                config::ButtonBase* button = pages.at(page)->at(row).at(col);

                if (button == nullptr) {
                    continue;
                }

                this->sendMessage(launchpad::commands::led_on(commands::calculate_grid(row, col), button->get_color()));
            }
        
        }
    }
}

void midi_device::launchpad::Launchpad::setup_pages_test()
{
    launchpad_grid* page = new launchpad_grid{ nullptr };

    config::ButtonBase* button = new config::ButtonSimpleKeycodeTest(0x41);

    button->set_color(launchpad::commands::calculate_velocity(commands::led_brightness::high, commands::led_brightness::high));

    page->at(7)[7] = button;

    button = new config::ButtonComplexMacro([]() { _DebugString("lol\n"); });

    button->set_color(launchpad::commands::calculate_velocity(commands::led_brightness::low, commands::led_brightness::high));
    page->at(7)[6] = button;


    https://onlineunicodetools.com/convert-unicode-to-hex use UCS-2-BE
    wchar_t* ste = new wchar_t[] { 
            0xd14c, // (korean) te
            0xc2a4, // s
            0xd2b8, // t
            0x0021, // !
            0x0 // null terminator
    };
    std::wstring test = std::wstring(ste);
    button = new config::ButtonStringMacro(test);
    button->set_color(launchpad::commands::calculate_velocity(commands::led_brightness::low, commands::led_brightness::low));
    page->at(7)[5] = button;

    // mute
    button = new config::ButtonSimpleKeycodeTest(VK_F13);
    button->set_color(launchpad::commands::vel_yellow_full);
    page->at(7)[0] = button;

    // deafen
    button = new config::ButtonSimpleKeycodeTest(VK_F14);
    button->set_color(launchpad::commands::vel_red_low);
    page->at(7)[1] = button;

    button = new config::ButtonSimpleKeycodeTest('a');
    button->set_color(launchpad::commands::calculate_velocity(commands::led_brightness::high, commands::led_brightness::high));
    page->at(6)[4] = button;

    pages.push_back(page);
}

void midi_device::launchpad::Launchpad::load_config_buttons_test() {
    try {
        /*if (::config::config_file.at("devices").contains("Launchpad_S")) {
            return;
        }*/

        // why
        if (!::config::config_file.at("devices").at("Launchpad_S").is_object()) {
            return;
        }

        nlohmann::json& config = ::config::config_file.at("devices").at("Launchpad_S");
        pages.clear();
        // FIXME: hard limit of 8 pages by buttons but this should be handled better.
        pages.resize(8);

        for (auto& [page, buttons] : config.at("session").items()) {
            int index = std::stoi(page);
            launchpad_grid* page_buttons = new launchpad_grid{ nullptr };

            if (!buttons.is_array()) {
                _DebugString("lol you're fucked\n");
            }

            for (auto& button : buttons) {
                std::string type = button.at("type");
                int position_x = button.at("position").at(0);
                int position_y = button.at("position").at(1);
                unsigned char color = commands::calculate_velocity(1, 2);

                config::ButtonBase* new_button;

                if (type == "key_test") {
                    if (!button.at("data").is_number()) {
                        continue;
                    }

                    new_button = new config::ButtonSimpleKeycodeTest(button.at("data"));

                }
                else if (type == "key_string") {
                    if (!button.at("data").is_string()) {
                        continue;
                    }

                    new_button = new config::ButtonStringMacro(string_to_wstring(button.at("data")));
                }
                else {
                    new_button = new config::ButtonSimpleKeycodeTest('b');
                }

                new_button->set_color(color);

                page_buttons->at(position_x).at(position_y) = new_button;
            }
            
            pages.at(index) = page_buttons;
        }
    }
    catch (std::invalid_argument& e) {
        _DebugString("invalid args!\n");
    }
    catch (nlohmann::json::type_error& e) {
        _DebugString("type error!\n");
    }
    catch (nlohmann::json::out_of_range &e) {
        _DebugString("range error!\n");
    }

}


void midi_device::launchpad::Launchpad::TerminateDevice()
{
    execute_all = false;
}

void midi_device::launchpad::config::ButtonSimpleKeycodeTest::execute()
{
    if (keycode == -1) {
        return;
    }

    INPUT input;

    input.type = INPUT_KEYBOARD;
    input.ki.time = 0;
    input.ki.wScan = NULL;
    input.ki.dwExtraInfo = NULL;

    input.ki.wVk = keycode;
    input.ki.dwFlags = 0;

    // send
    SendInput(1, &input, sizeof(INPUT));

    // wait
    Sleep(100);

    // release
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));

}

void midi_device::launchpad::config::ButtonComplexMacro::execute()
{
    this->func();
}

void midi_device::launchpad::config::ButtonStringMacro::execute()
{
    for (const wchar_t a : string) {
        INPUT input;

        input.type = INPUT_KEYBOARD;
        input.ki.time = 0;
        input.ki.wScan = a;
        input.ki.dwExtraInfo = NULL;

        input.ki.wVk = NULL;
        input.ki.dwFlags = KEYEVENTF_UNICODE;

        // send
        SendInput(1, &input, sizeof(INPUT));

        // wait
        Sleep(1);

        // release
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        Sleep(1);
    }
}

std::wstring midi_device::launchpad::config::ButtonStringMacro::to_wstring()
{
    return L"midi_device::launchpad::config::ButtonStringMacro : color=" + std::to_wstring(this->get_color()) + L" str=\"" + this->string + L"\"";
}


std::wstring midi_device::launchpad::config::ButtonBase::to_wstring()
{
    return L"midi_device::launchpad::config::ButtonBase : empty button";
}

std::wstring midi_device::launchpad::config::ButtonComplexMacro::to_wstring()
{
    std::wstringstream buffer;
    buffer << std::hex << &this->func;

    return L"midi_device::launchpad::config::ButtonComplexMacro : color=" + std::to_wstring(this->get_color()) + L" func_ptr=" + buffer.str();
}

std::wstring midi_device::launchpad::config::ButtonSimpleKeycodeTest::to_wstring()
{
    return L"midi_device::launchpad::config::ButtonSimpleKeycodeTest : color=" + std::to_wstring(this->get_color()) + L" keycode=" + std::to_wstring(this->keycode);
}