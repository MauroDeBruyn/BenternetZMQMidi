/*#include <iostream>
#include "RtMidi.h"

int main() {
    try {
        RtMidiOut midiout;

        unsigned int nPorts = midiout.getPortCount();
        std::cout << "Found " << nPorts << " MIDI output ports:\n";

        for (unsigned int i = 0; i < nPorts; i++) {
            std::string portName = midiout.getPortName(i);
            std::cout << "  Port #" << i << ": " << portName << '\n';
        }

        // Send a test MIDI message (middle C)
        if (nPorts > 0) {
            midiout.openPort(0);

            std::vector<unsigned char> message;
            message.push_back(0x90); // Note On
            message.push_back(60);   // Middle C
            message.push_back(90);   // Velocity

            midiout.sendMessage(&message);
        }
    }
    catch (RtMidiError &error) {
        error.printMessage();
        return 1;
    }

    return 0;
}
*/
