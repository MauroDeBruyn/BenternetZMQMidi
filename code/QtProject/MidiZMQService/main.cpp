#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <nzmqt/impl.hpp>
#include <QCoreApplication>
#include <QString>
#include <QThread>
#include <QDateTime>
#include <QMap>

#ifdef _WIN32
#include <windows.h>
#endif

void simulateKeyPress(const QChar &key)
{
#ifdef _WIN32
    static QMap<QChar, BYTE> keyMap = {
        {'A', 0x41}, {'B', 0x42}, {'C', 0x43},
        {'D', 0x44}, {'E', 0x45}, {'F', 0x46}, {'G', 0x47}
    };

    BYTE vk = keyMap.value(key.toUpper(), 0);
    if (vk != 0) {
        keybd_event(vk, 0, 0, 0);               // Key down
        keybd_event(vk, 0, KEYEVENTF_KEYUP, 0); // Key up
        std::cout << "Simulated key press: " << key.toUpper().toLatin1() << std::endl;
    }
#else
    (void)key;
    std::cout << "[!] Key simulation not supported on this OS." << std::endl;
#endif
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Starting the communication with benternet..." << std::endl;

    try {
        // Create ZMQ context
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&app);

        // Create PUSH socket
        nzmqt::ZMQSocket *pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        // Create SUB socket
        nzmqt::ZMQSocket *subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        // Subscribe to both topics
        subscriber->subscribeTo("MIDISignal");
        subscriber->subscribeTo("visualStatus");

        // Check connection
        if (!pusher->isConnected() || !subscriber->isConnected()) {
            std::cerr << "NOT CONNECTED !!!" << std::endl;
            return 1;
        }

        // Map notes (chars) to visual names
        QMap<QChar, QString> visualMap = {
            {'A', "Visual #1"},
            {'B', "Visual #2"},
            {'C', "Visual #3"},
            {'D', "Visual #4"},
            {'E', "Visual #5"},
            {'F', "Visual #6"},
            {'G', "Visual #7"}
        };

        // Launch message handling thread
        QThread *listenerThread = QThread::create([subscriber, pusher, visualMap]() mutable {
            QString lastRespondedNote;

            while (true) {
                try {
                    nzmqt::ZMQMessage receivedMessage;

                    if (subscriber->receiveMessage(&receivedMessage)) {
                        QString message = QString::fromUtf8(receivedMessage.toByteArray()).trimmed();
                        std::cout << "Raw message: [" << message.toStdString() << "]" << std::endl;

                        QStringList parts = message.split('>');
                        if (parts.size() >= 3) {
                            QString topic = parts[0].trimmed();
                            QString payload = parts[1].trimmed();
                            QString sender = parts[2].trimmed();

                            if (topic == "MIDISignal" && !payload.isEmpty() && payload != lastRespondedNote) {
                                QChar noteChar = payload.left(1).at(0);
                                simulateKeyPress(noteChar);

                                QString feedback = "MIDISignal>" + payload + ">Feedback>";
                                pusher->sendMessage(feedback.toUtf8());
                                std::cout << "Sent feedback: " << feedback.toStdString() << std::endl;

                                lastRespondedNote = payload;
                            }
                            else if (topic == "visualStatus" && payload == "currentVisual") {
                                if (!lastRespondedNote.isEmpty()) {
                                    QChar noteChar = lastRespondedNote.left(1).at(0);
                                    QString visualName = visualMap.value(noteChar.toUpper(), "Unknown Visual");

                                    QString response = QString("visualStatus>%1 (%2)>Service>")
                                                           .arg(lastRespondedNote, visualName);
                                    pusher->sendMessage(response.toUtf8());
                                    std::cout << "Sent visualStatus response: " << response.toStdString() << std::endl;
                                } else {
                                    std::cout << "No MIDI note received yet; nothing to respond with." << std::endl;
                                }
                            }
                        } else {
                            std::cout << "Malformed message: " << message.toStdString() << std::endl;
                        }
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error in listener loop: " << e.what() << std::endl;
                }
            }
        });

        listenerThread->start();
        return app.exec();

    } catch (nzmqt::ZMQException &ex) {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
        return 1;
    }
}
