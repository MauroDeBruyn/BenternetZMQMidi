//Working code using user input to send currentVisual
/*
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <nzmqt/nzmqt.hpp>
#include "RtMidi.h"
#include <QMetaObject>
#include <QStringList>

class MidiClientGUI : public QMainWindow
{
    Q_OBJECT

public:
    MidiClientGUI(QWidget *parent = nullptr)
        : QMainWindow(parent), m_context(nzmqt::createDefaultContext(this))
    {
        // GUI setup
        auto *widget = new QWidget(this);
        auto *layout = new QVBoxLayout(widget);

        m_input = new QLineEdit(this);
        m_input->setPlaceholderText("Enter MIDI note manually (e.g., D3)");
        layout->addWidget(m_input);

        auto *sendButton = new QPushButton("Send MIDI Note", this);
        layout->addWidget(sendButton);

        m_serviceInput = new QLineEdit(this);
        m_serviceInput->setPlaceholderText("Enter currentVisual to receive the current running visual");
        layout->addWidget(m_serviceInput);

        auto *serviceButton = new QPushButton("Send Service Command", this);
        layout->addWidget(serviceButton);

        m_output = new QTextEdit(this);
        m_output->setReadOnly(true);
        layout->addWidget(m_output);

        setCentralWidget(widget);
        setWindowTitle("MIDI & Service Client GUI");

        connect(sendButton, &QPushButton::clicked, this, &MidiClientGUI::onSendClicked);
        connect(serviceButton, &QPushButton::clicked, this, &MidiClientGUI::onServiceSendClicked);

        // ZeroMQ setup
        m_pusher = m_context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, m_context);
        m_pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        m_subscriber = m_context->createSocket(nzmqt::ZMQSocket::TYP_SUB, m_context);
        connect(m_subscriber, &nzmqt::ZMQSocket::messageReceived, this, &MidiClientGUI::onMessageReceived);
        m_subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");
        m_subscriber->subscribeTo("MIDISignal");
        m_subscriber->subscribeTo("visualStatus");

        m_context->start();

        // MIDI Setup
        try {
            m_midiIn = new RtMidiIn();

            if (m_midiIn->getPortCount() == 0) {
                m_output->append("No MIDI ports available.");
            } else {
                m_midiIn->openPort(0);
                m_midiIn->setCallback(&MidiClientGUI::staticMidiCallback, this);
                m_midiIn->ignoreTypes(false, false, false);
                m_output->append("MIDI port 0 opened.");
            }
        } catch (RtMidiError &e) {
            m_output->append(QString("MIDI error: %1").arg(e.getMessage().c_str()));
        }
    }

    ~MidiClientGUI() override {
        delete m_midiIn;
    }

    static void staticMidiCallback(double, std::vector<unsigned char> *message, void *userData) {
        auto *self = static_cast<MidiClientGUI *>(userData);
        if (!message || message->empty()) return;

        QString hexMsg;
        for (unsigned char byte : *message)
            hexMsg += QString::asprintf("%02X ", byte);

        QMetaObject::invokeMethod(self, "handleMidiMessage", Qt::QueuedConnection,
                                  Q_ARG(QString, hexMsg.trimmed()));
    }

public slots:
    void handleMidiMessage(const QString &msg) {
        m_output->append("[MIDI In] " + msg);

        QStringList byteStrings = msg.split(' ');
        if (byteStrings.size() < 3) return; // Not a full MIDI message

        bool ok1, ok2, ok3;
        int status = byteStrings[0].toInt(&ok1, 16);
        int note = byteStrings[1].toInt(&ok2, 16);
        int velocity = byteStrings[2].toInt(&ok3, 16);

        if (!ok1 || !ok2 || !ok3) return;

        // Note On with velocity > 0
        if ((status & 0xF0) == 0x90 && velocity > 0) {
            QString noteName = midiNoteNumberToName(note);
            QString zmqMsg = "MIDISignal>" + noteName + ">APCMini>";
            m_pusher->sendMessage(zmqMsg.toUtf8());
            m_output->append("Sent (Auto MIDI): " + zmqMsg);
        }
    }

private slots:
    void onSendClicked() {
        QString MIDINote = m_input->text().trimmed();
        if (MIDINote.isEmpty()) return;

        QString msg = "MIDISignal>" + MIDINote + ">APCMini>";
        m_pusher->sendMessage(msg.toUtf8());
        m_output->append("Sent (MIDI): " + msg);
        m_input->clear();
    }

    void onServiceSendClicked() {
        QString command = m_serviceInput->text().trimmed();
        if (command.isEmpty()) return;

        QString msg = "visualStatus>" + command + ">User>";
        m_pusher->sendMessage(msg.toUtf8());
        m_output->append("Sent (Current visual): " + msg);
        m_serviceInput->clear();
    }

    void onMessageReceived(const QList<QByteArray> &messages) {
        for (const QByteArray &msg : messages) {
            QString decoded = QString::fromUtf8(msg);
            if (decoded.endsWith(">APCMini>") || decoded.endsWith(">User>"))
                continue;
            m_output->append("[Received] " + decoded);
        }
    }

private:
    QString midiNoteNumberToName(int noteNumber) {
        static const QStringList noteNames = {
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };
        int octave = (noteNumber / 12) - 1;
        QString name = noteNames[noteNumber % 12];
        return name + QString::number(octave);
    }

    QLineEdit *m_input;
    QLineEdit *m_serviceInput;
    QTextEdit *m_output;
    nzmqt::ZMQContext *m_context;
    nzmqt::ZMQSocket *m_pusher;
    nzmqt::ZMQSocket *m_subscriber;
    RtMidiIn *m_midiIn = nullptr;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MidiClientGUI window;
    window.resize(400, 400);
    window.show();
    return app.exec();
}
*/

//Code for automatic currentVisual status with button
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <nzmqt/nzmqt.hpp>
#include "RtMidi.h"
#include <QMetaObject>
#include <QStringList>

class MidiClientGUI : public QMainWindow
{
    Q_OBJECT

public:
    MidiClientGUI(QWidget *parent = nullptr)
        : QMainWindow(parent), m_context(nzmqt::createDefaultContext(this))
    {
        // GUI setup
        auto *widget = new QWidget(this);
        auto *layout = new QVBoxLayout(widget);

        m_input = new QLineEdit(this);
        m_input->setPlaceholderText("Enter MIDI note manually (e.g., D3)");
        layout->addWidget(m_input);

        auto *sendButton = new QPushButton("Send MIDI Note", this);
        layout->addWidget(sendButton);

        auto *visualButton = new QPushButton("Request Current Visual", this);
        layout->addWidget(visualButton);

        m_output = new QTextEdit(this);
        m_output->setReadOnly(true);
        layout->addWidget(m_output);

        setCentralWidget(widget);
        setWindowTitle("MIDI & Service Client GUI");

        connect(sendButton, &QPushButton::clicked, this, &MidiClientGUI::onSendClicked);
        connect(visualButton, &QPushButton::clicked, this, &MidiClientGUI::onRequestCurrentVisualClicked);

        // ZeroMQ setup
        m_pusher = m_context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, m_context);
        m_pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        m_subscriber = m_context->createSocket(nzmqt::ZMQSocket::TYP_SUB, m_context);
        connect(m_subscriber, &nzmqt::ZMQSocket::messageReceived, this, &MidiClientGUI::onMessageReceived);
        m_subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");
        m_subscriber->subscribeTo("MIDISignal");
        m_subscriber->subscribeTo("visualStatus");

        m_context->start();

        // MIDI Setup
        try {
            m_midiIn = new RtMidiIn();

            if (m_midiIn->getPortCount() == 0) {
                m_output->append("No MIDI ports available.");
            } else {
                m_midiIn->openPort(0);
                m_midiIn->setCallback(&MidiClientGUI::staticMidiCallback, this);
                m_midiIn->ignoreTypes(false, false, false);
                m_output->append("MIDI port 0 opened.");
            }
        } catch (RtMidiError &e) {
            m_output->append(QString("MIDI error: %1").arg(e.getMessage().c_str()));
        }
    }

    ~MidiClientGUI() override {
        delete m_midiIn;
    }

    static void staticMidiCallback(double, std::vector<unsigned char> *message, void *userData) {
        auto *self = static_cast<MidiClientGUI *>(userData);
        if (!message || message->empty()) return;

        QString hexMsg;
        for (unsigned char byte : *message)
            hexMsg += QString::asprintf("%02X ", byte);

        QMetaObject::invokeMethod(self, "handleMidiMessage", Qt::QueuedConnection,
                                  Q_ARG(QString, hexMsg.trimmed()));
    }

public slots:
    void handleMidiMessage(const QString &msg) {
        m_output->append("[MIDI In] " + msg);

        QStringList byteStrings = msg.split(' ');
        if (byteStrings.size() < 3) return;

        bool ok1, ok2, ok3;
        int status = byteStrings[0].toInt(&ok1, 16);
        int note = byteStrings[1].toInt(&ok2, 16);
        int velocity = byteStrings[2].toInt(&ok3, 16);

        if (!ok1 || !ok2 || !ok3) return;

        if ((status & 0xF0) == 0x90 && velocity > 0) {
            QString noteName = midiNoteNumberToName(note);
            QString zmqMsg = "MIDISignal>" + noteName + ">APCMini>";
            m_pusher->sendMessage(zmqMsg.toUtf8());
            m_output->append("Sent (Auto MIDI): " + zmqMsg);
        }
    }

private slots:
    void onSendClicked() {
        QString MIDINote = m_input->text().trimmed();
        if (MIDINote.isEmpty()) return;

        QString msg = "MIDISignal>" + MIDINote + ">APCMini>";
        m_pusher->sendMessage(msg.toUtf8());
        m_output->append("Sent (MIDI): " + msg);
        m_input->clear();
    }

    void onRequestCurrentVisualClicked() {
        QString msg = "visualStatus>currentVisual>User>";
        m_pusher->sendMessage(msg.toUtf8());
        m_output->append("Sent (Request Current Visual): " + msg);
    }

    void onMessageReceived(const QList<QByteArray> &messages) {
        for (const QByteArray &msg : messages) {
            QString decoded = QString::fromUtf8(msg);
            if (decoded.endsWith(">APCMini>") || decoded.endsWith(">User>"))
                continue;
            m_output->append("[Received] " + decoded);
        }
    }

private:
    QString midiNoteNumberToName(int noteNumber) {
        static const QStringList noteNames = {
            "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
        };
        int octave = (noteNumber / 12) - 1;
        QString name = noteNames[noteNumber % 12];
        return name + QString::number(octave);
    }

    QLineEdit *m_input;
    QTextEdit *m_output;
    nzmqt::ZMQContext *m_context;
    nzmqt::ZMQSocket *m_pusher;
    nzmqt::ZMQSocket *m_subscriber;
    RtMidiIn *m_midiIn = nullptr;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MidiClientGUI window;
    window.resize(400, 400);
    window.show();
    return app.exec();
}
