#include "mjpegstreamer.h"

#include <QBuffer>
#include <QImageReader>
#include <QCryptographicHash>

MJPEGStreamer::MJPEGStreamer(QWidget * parent)
{
    setParent(parent);
    setScaledContents(true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    m_state = StreamState::Stopped;
    m_socket = new QTcpSocket(this);
    m_socket->open(QIODevice::ReadWrite);

    connect(m_socket, SIGNAL(connected()), this, SLOT(on_connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(on_disconnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(on_readyRead()));

    /** clear video surface */ {
        QPixmap pmap(width(), height());
        pmap.fill(Qt::black);
        setPixmap(pmap);
    }
}

MJPEGStreamer::~MJPEGStreamer()
{
    m_socket->close();
    delete m_socket;
}

void MJPEGStreamer::start()
{
    m_socket->connectToHost(m_url.host(), m_url.port(80));
}

void MJPEGStreamer::stop()
{
    m_socket->disconnectFromHost();
    m_state = StreamState::Stopped;
}

void MJPEGStreamer::set_url(const QString &url)
{
    m_url = QUrl(url, QUrl::TolerantMode);
}

QString MJPEGStreamer::url()
{
    return m_url.toString();
}

QString MJPEGStreamer::generate_auth_digest(const QString &resp_unauthorized)
{
    int ind_wwwauth_line_first_byte = resp_unauthorized.indexOf("WWW-Authenticate");
    if(ind_wwwauth_line_first_byte > -1) {
        // excluding everything before WWW-Authenticate
        QString temp = resp_unauthorized.mid(ind_wwwauth_line_first_byte);
        int ind_of_crlf = temp.indexOf("\r\n");
        // excluding everything after WWW-Authentic...opaque="" (excluding \r\n too)
        QString digest_line = temp.mid(0, ind_of_crlf);
        // excluding "WWW-Authenticate: Digest "
        QString digest_content = digest_line.mid(25);
        // splitting comma separated sentences as a="b", c="d" into new strings to store them in map_pieces
        QStringList auth_line_pieces = digest_content.split(',');
        QMap <QString, QString> map_pieces;

        for(int i = 0; i < auth_line_pieces.size(); i++) {
            QString piece = auth_line_pieces[i];
            // we need to get rid of double quotes as are not needed in calculations
            piece.remove("\"");
            // we also need to remove useless spaces at the start of each sentence such as ' a="b"' to 'a="b"'
            if(piece.at(0) == ' ') {
                piece.remove(0, 1);
            }
            map_pieces.insert(piece.left(piece.indexOf('=')), piece.mid(piece.indexOf('=') + 1));
        }

        QString ha1 = QString::fromUtf8(QCryptographicHash::hash(QString("%1:%2:%3").arg(
            m_url.userName(), map_pieces["realm"], m_url.password()
        ).toUtf8(),QCryptographicHash::Md5).toHex());

        QString ha2 = QString::fromUtf8(QCryptographicHash::hash(QString("%1:%2").arg(
            "GET", m_url.path() + "?" + m_url.query()
        ).toUtf8(),QCryptographicHash::Md5).toHex());

        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        srand((time_t)ts.tv_nsec);

        QString nc = QString::number(rand()%100000000);
        QString cnonce = QString("%1%2%3").arg(map_pieces["nonce"], "benden selam olsun bolu beyine", QString::number(time(nullptr))).toUtf8().toBase64().toHex();
        QString response = QString::fromUtf8(QCryptographicHash::hash(QString("%1:%2:%3:%4:%5:%6").arg(
            ha1, map_pieces["nonce"], nc, cnonce, map_pieces["qop"], ha2).toUtf8(), QCryptographicHash::Md5).toHex()
        );
        QString digest_header = QString("Digest username=\"%1\", realm=\"%2\", nonce=\"%3\", uri=\"%4\", response=\"%5\", qop=%6, nc=%7, cnonce=\"%8\"").arg(
            m_url.userName(), map_pieces["realm"], map_pieces["nonce"], m_url.path() + "?" + m_url.query(), response, map_pieces["qop"], nc, cnonce
        );
        return QString("GET %1 HTTP/1.1\r\n"
                       "Host: %2\r\n"
                       "Authorization: %3\r\n"
                       "Upgrade-Insecure-Requests: 1\r\n"
                       "Accept: */*\r\n"
                       "Connection: keep-alive\r\n\r\n").arg(m_url.path() + "?" + m_url.query(), m_url.host(), digest_header);
    }
    return "";
}

void MJPEGStreamer::on_connected()
{
    qDebug() << "connected.";
    switch (m_state) {
    case StreamState::Stopped:{
        /**
         * this is the default case when the streamer
         * connects to the server.
         *
         * if basic authentication is sufficient for the server
         * it will return status 200 and the stream will start.
         * (m_state will be set to StreamState::Streaming)
         *
         * if the server challenges
         * us with some extra authentication (401 unauthorized
         * with digest in this case) socket
         * connection will be closed by the server after
         * 401 response. this is where we need to reconnect
         * and ask for authorization for the second time with
         * our authentication digest.
         */
        QString request = QString("GET %1 HTTP/1.1\r\n"
                                  "Host: %2\r\n"
                                  "Authorization: Basic %3\r\n"
                                  "Accept: */*\r\n"
                                  "Upgrade-Insecure-Requests: 1\r\n"
                                  "Connection: keep-alive\r\n\r\n").arg(
            m_url.path() + "?" + m_url.query(), m_url.host(), QString(m_url.userName() + ":" + m_url.password()).toUtf8().toBase64()
        );
        m_socket->write(request.toStdString().c_str());
    }
    break;
    case StreamState::Authorizing:{
        QString buffer_str = QString::fromUtf8(m_buffer);
        QString request_with_digest;
        if(!(request_with_digest = generate_auth_digest(buffer_str)).isEmpty()) {
            m_socket->write(request_with_digest.toUtf8());
        }
    }
    break;
        default:
        break;
    }
}

void MJPEGStreamer::on_disconnected()
{
    qDebug() << "disconnected.";
    /** clear video surface */ {
        QPixmap pmap(width(), height());
        pmap.fill(Qt::black);
        setPixmap(pmap);
    }
    if(m_state == StreamState::Authorizing) {
        /** this means we got rejected by server once,
         * now we will try to reconnect with a stronger
         * authentication challenge.
         */
        start();
    }
    else {
        m_buffer.clear();
    }
}

void MJPEGStreamer::on_readyRead()
{
    m_buffer += m_socket->readAll();    
    QString sbuffer = QString::fromUtf8(m_buffer);

    switch (m_state) {
    case StreamState::Stopped: {
        if(sbuffer.contains("401")) {
            /** we switch to authorization state so we can
             * try one more time with digest authentication.
             * see: on_disconnected()
             */
            m_state = StreamState::Authorizing;
        }
        else if(sbuffer.contains("200")) {
            m_state = StreamState::Streaming;
        }
    }
    break;
    case StreamState::Authorizing:
        if(sbuffer.contains("200")) {
            m_state = StreamState::Streaming;
        }
        else if(sbuffer.contains("401")) {
            /** if we get rejected in Stopped state we try one more time,
             * if we get rejected in Authorizing state, we give up. */
            stop();
        }
    break;
    case StreamState::Streaming: {
        int ind_start_bytes = m_buffer.lastIndexOf("\xff\xd8");
        int ind_end_bytes = m_buffer.lastIndexOf("\xff\xd9");
        if(ind_start_bytes != -1 && ind_end_bytes != -1) {
            if(ind_start_bytes < ind_end_bytes){
                QByteArray image_data = m_buffer.mid(ind_start_bytes, ind_end_bytes + 2);
                m_buffer = m_buffer.mid(ind_end_bytes+2);
                QPixmap pmap;
                if(pmap.loadFromData(image_data, "JPEG")) {
                    setPixmap(pmap.scaled(pmap.size(), Qt::KeepAspectRatio));
                }
                else
                    qDebug() << "error: bad data or JPEG is not supported. currently supported types are: " << QImageReader::supportedImageFormats();
            }
        }
    }
    break;
    default:
    break;
    }
}
