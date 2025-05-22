#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <nzmqt/impl.hpp>
#include <QCoreApplication>
#include <QString>
#include <QThread>
#include <QDateTime>

#ifdef _WIN32
#include <windows.h>
#endif

void simulateKeyPress(const QChar &key)
{
#ifdef _WIN32
    // Map from char to virtual-key code for A-G
    static QMap<QChar, BYTE> keyMap = {
        {'A', 0x41},
        {'B', 0x42},
        {'C', 0x43},
        {'D', 0x44},
        {'E', 0x45},
        {'F', 0x46},
        {'G', 0x47}
    };

    BYTE vk = keyMap.value(key.toUpper(), 0);
    if (vk != 0) {
        keybd_event(vk, 0, 0, 0);           // Key down
        keybd_event(vk, 0, KEYEVENTF_KEYUP, 0); // Key up
        std::cout << "Simulated key press: " << key.toUpper().toLatin1() << std::endl;
    }
#else
    // Implement for other OS if needed
    (void)key;
#endif
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Starting the communication with benternet..." << std::endl;

    try
    {
        // Initialize ZMQ context
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&app);

        // Create PUSH socket
        nzmqt::ZMQSocket *pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        // Create SUB socket
        nzmqt::ZMQSocket *subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        // Subscribe to "MIDISignal"
        subscriber->subscribeTo("MIDISignal");

        // Check connections
        if (!pusher->isConnected() || !subscriber->isConnected()) {
            std::cerr << "NOT CONNECTED !!!" << std::endl;
            return 1;
        }

        // Listener thread with anti-spam logic
        QThread *listenerThread = QThread::create([subscriber, pusher]() {
            QString lastRespondedNote;

            while (true) {
                try {
                    nzmqt::ZMQMessage receivedMessage;

                    if (subscriber->receiveMessage(&receivedMessage)) {
                        QString response = QString::fromUtf8(receivedMessage.toByteArray());
                        std::cout << "Received: " << response.toStdString() << std::endl;

                        QStringList parts = response.split('>');
                        if (parts.size() >= 2) {
                            QString topic = parts[0];
                            QString note = parts[1];  // e.g. "A3", "D3"

                            if (topic == "MIDISignal" && note != lastRespondedNote) {
                                // Simulate the key press for first character of note (A-G)
                                simulateKeyPress(note.left(1).at(0));

                                // Send feedback
                                QString feedback = "MIDISignal>" + note + ">Feedback>";
                                pusher->sendMessage(feedback.toUtf8());
                                std::cout << "Sent: " << feedback.toStdString() << std::endl;

                                lastRespondedNote = note;
                            }
                        }
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error receiving message: " << e.what() << std::endl;
                }
            }
        });

        // Start listener thread
        listenerThread->start();

        // Run Qt event loop
        return app.exec();

    } catch (nzmqt::ZMQException &ex) {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
        return 1;
    }
}
