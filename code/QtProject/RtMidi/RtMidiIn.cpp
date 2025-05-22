#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "RtMidi.h"

int main() {
    try {
        std::cout << "Initializing MIDI...\n";
        RtMidiIn midiin;

        unsigned int nPorts = midiin.getPortCount();
        if (nPorts == 0) {
            std::cout << "No MIDI input ports available.\n";
            return 1;
        }

        std::cout << "Available MIDI input ports:\n";
        for (unsigned int i = 0; i < nPorts; ++i) {
            std::cout << "  Port " << i << ": " << midiin.getPortName(i) << "\n";
        }

        std::cout << "Opening port 0...\n";
        try {
            midiin.openPort(0); // You can change the index if needed
        } catch (RtMidiError& e) {
            std::cerr << "Failed to open MIDI port: ";
            e.printMessage();
            return 1;
        }

        midiin.ignoreTypes(false, false, false);
        std::cout << "Listening for MIDI input... Press Ctrl+C to stop.\n";

        std::vector<unsigned char> message;

        while (true) {
            midiin.getMessage(&message);
            if (!message.empty()) {
                std::cout << "Received MIDI message: ";
                for (auto byte : message) {
                    std::cout << std::hex << std::uppercase << (int)byte << " ";
                }
                std::cout << std::dec << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    } catch (RtMidiError& e) {
        std::cerr << "RtMidi error: ";
        e.printMessage();
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Standard exception: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}
