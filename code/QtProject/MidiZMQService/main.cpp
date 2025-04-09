#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <nzmqt/impl.hpp>
#include <QCoreApplication>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QDateTime>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Starting the communication with benternet..." << std::endl;

    try
    {
        //Initialize ZMQ context
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&app);

        //Create PUSH socket
        nzmqt::ZMQSocket *pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        //Create SUB socket
        nzmqt::ZMQSocket *subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");

        //Subscribe to the topic "MIDISignal"
        subscriber->subscribeTo("MIDISignal");

        //Check connections
        if (!pusher->isConnected() || !subscriber->isConnected())
        {
            std::cerr << "NOT CONNECTED !!!" << std::endl;
            return 1;
        }

        //Listener thread: Waits for a message before sending a response
        QThread *listenerThread = QThread::create([subscriber, pusher]() {
            while (true) {
                try {
                    nzmqt::ZMQMessage receivedMessage;

                    //Blocking wait for a message before proceeding
                    while (!subscriber->receiveMessage(&receivedMessage)) {
                        QThread::msleep(10); //Prevent CPU overuse
                    }

                    //Process received message
                    QString response = QString::fromUtf8(receivedMessage.toByteArray());
                    std::cout << "Received: " << response.toStdString() << std::endl;

                    //Create MIDI input message
                    QString MIDIFeedback;
                    MIDIFeedback = "D3";
                    QString messageFeedback = "MIDISignal>" + MIDIFeedback + ">Feedback>";


                    //Send the response
                    nzmqt::ZMQMessage message(messageFeedback.toUtf8());
                    pusher->sendMessage(message);
                    std::cout << "Sent: " << messageFeedback.toStdString() << std::endl;

                } catch (const std::exception &e) {
                    std::cerr << "Error receiving message: " << e.what() << std::endl;
                }
            }
        });

        //Start the listener thread
        listenerThread->start();

        //Start the event loop
        return app.exec();

    } catch (nzmqt::ZMQException &ex) {
        std::cerr << "Caught an exception: " << ex.what() << std::endl;
        return 1;
    }
}
