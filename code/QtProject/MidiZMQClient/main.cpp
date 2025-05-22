#include <iostream>
#include <thread>
#include <atomic>
#include <nzmqt/nzmqt.hpp>
#include <nzmqt/impl.hpp>
#include <QCoreApplication>
#include <QString>
#include <QObject>

// Helper class: leest user input op aparte thread en signaleert naar Qt
class InputReader : public QObject
{
    Q_OBJECT
public:
    explicit InputReader(QObject *parent = nullptr) : QObject(parent), m_stop(false) {}

    void start()
    {
        m_thread = std::thread([this]() {
            while (!m_stop.load()) {
                std::cout << "Enter MIDI note (e.g. D3): " << std::flush;
                std::string line;
                if (!std::getline(std::cin, line)) {
                    emit inputReceived(QString());
                    break;
                }
                emit inputReceived(QString::fromStdString(line));
            }
        });
    }

    void stop()
    {
        m_stop.store(true);
        if (m_thread.joinable())
            m_thread.join();
    }

signals:
    void inputReceived(const QString &input);

private:
    std::thread m_thread;
    std::atomic<bool> m_stop;
};

// Main client class: verstuurt MIDI input en ontvangt response via ZeroMQ
class MidiClient : public QObject
{
    Q_OBJECT

public:
    MidiClient(nzmqt::ZMQSocket *pusher, nzmqt::ZMQSocket *subscriber, QObject *parent = nullptr)
        : QObject(parent), m_pusher(pusher), m_subscriber(subscriber)
    {
        connect(m_subscriber, &nzmqt::ZMQSocket::messageReceived,
                this, &MidiClient::onMessageReceived);
    }

public slots:
    void onUserInput(const QString &MIDINote)
    {
        if (MIDINote.isEmpty()) {
            std::cout << "Exiting application..." << std::endl;
            QCoreApplication::quit();
            return;
        }

        // Format: "MIDISignal>{input}>APCMini>"
        QString msg = "MIDISignal>" + MIDINote + ">APCMini>";
        nzmqt::ZMQMessage message(msg.toUtf8());
        m_pusher->sendMessage(message);

        std::cout << "Verzonden: " << msg.toStdString() << std::endl;
    }

private slots:
    void onMessageReceived(const QList<QByteArray> &messages)
    {
        for (const QByteArray &msgData : messages) {
            QString response = QString::fromUtf8(msgData);
            std::cout << "Ontvangen: " << response.toStdString() << std::endl;
        }
    }

private:
    nzmqt::ZMQSocket *m_pusher;
    nzmqt::ZMQSocket *m_subscriber;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Start communicatie met benternet..." << std::endl;

    try {
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext(&app);

        nzmqt::ZMQSocket *pusher = context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, context);
        pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        nzmqt::ZMQSocket *subscriber = context->createSocket(nzmqt::ZMQSocket::TYP_SUB, context);
        subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");
        subscriber->subscribeTo("MIDISignal");

        MidiClient client(pusher, subscriber);
        InputReader inputReader;

        QObject::connect(&inputReader, &InputReader::inputReceived,
                         &client, &MidiClient::onUserInput,
                         Qt::QueuedConnection);

        inputReader.start();

        int ret = app.exec();
        inputReader.stop();
        return ret;
    }
    catch (nzmqt::ZMQException &ex) {
        std::cerr << "Exception gevangen: " << ex.what() << std::endl;
        return 1;
    }
}

#include "main.moc"
