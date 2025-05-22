#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <nzmqt/nzmqt.hpp>

class MidiClientGUI : public QMainWindow
{
    Q_OBJECT

public:
    MidiClientGUI(QWidget *parent = nullptr)
        : QMainWindow(parent), m_context(nzmqt::createDefaultContext(this))
    {
        // Setup GUI
        auto *widget = new QWidget(this);
        auto *layout = new QVBoxLayout(widget);

        m_input = new QLineEdit(this);
        m_input->setPlaceholderText("Enter MIDI note (e.g., D3)");
        layout->addWidget(m_input);

        auto *sendButton = new QPushButton("Send MIDI Note", this);
        layout->addWidget(sendButton);

        m_output = new QTextEdit(this);
        m_output->setReadOnly(true);
        layout->addWidget(m_output);

        setCentralWidget(widget);
        setWindowTitle("MIDI Client GUI");

        // Setup ZeroMQ
        m_pusher = m_context->createSocket(nzmqt::ZMQSocket::TYP_PUSH, m_context);
        m_pusher->connectTo("tcp://benternet.pxl-ea-ict.be:24041");

        m_subscriber = m_context->createSocket(nzmqt::ZMQSocket::TYP_SUB, m_context);
        connect(m_subscriber, &nzmqt::ZMQSocket::messageReceived, this, &MidiClientGUI::onMessageReceived);
        m_subscriber->connectTo("tcp://benternet.pxl-ea-ict.be:24042");
        m_subscriber->subscribeTo("MIDISignal");

        // Connect input
        connect(sendButton, &QPushButton::clicked, this, &MidiClientGUI::onSendClicked);
    }

private slots:
    void onSendClicked()
    {
        QString MIDINote = m_input->text().trimmed();
        if (MIDINote.isEmpty()) return;

        QString msg = "MIDISignal>" + MIDINote + ">APCMini>";
        nzmqt::ZMQMessage message(msg.toUtf8());
        m_pusher->sendMessage(message);

        m_output->append("Sent: " + msg);
        m_input->clear();
    }

    void onMessageReceived(const QList<QByteArray> &messages)
    {
        for (const QByteArray &msg : messages) {
            m_output->append("Received: " + QString::fromUtf8(msg));
        }
    }

private:
    QLineEdit *m_input;
    QTextEdit *m_output;
    nzmqt::ZMQContext *m_context;
    nzmqt::ZMQSocket *m_pusher;
    nzmqt::ZMQSocket *m_subscriber;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MidiClientGUI window;
    window.resize(400, 300);
    window.show();
    return app.exec();
}
