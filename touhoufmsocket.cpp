#include "touhoufmsocket.h"
#include <QJsonDocument>


TouhouFMSocket::TouhouFMSocket(QString authToken, QObject *parent) : QObject(parent)
{
    // Create the underlying websocket
    m_wsInfo = new QWebSocket(QString(),QWebSocketProtocol::VersionLatest, this);

    // Connect the essential signals
    connect(m_wsInfo,SIGNAL(connected()),SLOT(handleConnected()));
    connect(m_wsInfo,SIGNAL(textMessageReceived(QString)),SLOT(handleMessage(QString)));

    // Copy over our auth token
    m_sAuth = authToken;

    startTimer(1000);
}

void TouhouFMSocket::setAuthToken(QString authToken)
{
    m_sAuth = authToken;

    if(m_sAuth.isEmpty() && m_wsInfo->isValid())login();
}

void TouhouFMSocket::login()
{
    QVariantMap msg;

    if(m_sAuth.isEmpty())
    {

        msg.insert("type","get-token");
        msg.insert("name","TouHou.FM Radio App");
        msg.insert("description","The official desktop radio player.");

        m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg).toJson());
    }
    else
    {
        msg.insert("type","auth-login");
        msg.insert("token",m_sAuth);

        m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg).toJson());
    }

}

void TouhouFMSocket::skipSong()
{
    QVariantMap msg;

    msg.insert("type","skip");

    m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg).toJson());
}

bool TouhouFMSocket::open()
{
    // Open the websocket connection to the websocket address
    m_wsInfo->open(QUrl("ws://en.touhou.fm/wsapp/"));

    return true;
}

void TouhouFMSocket::handleMessage(QString info)
{
    // Get our message into QVariantMap form
    QVariantMap msg = QJsonDocument::fromJson(info.toUtf8()).toVariant().toMap();

    QString type = msg.value("type").toString();

    if(!type.compare("song-info"))
    {
        // We handle our song info message, it contains a map of
        // tag updates
        QVariantMap tags = msg.value("tags").toMap();
        foreach(QString key, tags.keys())
        {
            // Each tag is sent out as metaDataChanged
            if(key == "rating")
                emit newGlobalRating(tags.value(key).toFloat()+3.0);
            else
                emit metaDataChanged(key,tags.value(key));
        }
    }
    else if(!type.compare("progress"))
    {
        // We handle our progress message, it contains a "progress" value
        // and "time" value.
        emit newProgress(msg.value("progress").toFloat());
        emit newProgress(msg.value("progress").toInt());
        emit newTime(msg.value("time").toString());
    }
    else if(!type.compare("user-rating"))
    {
        emit newUserRating(msg.value("rating").toInt()+3);
    }
    else if(!type.compare("what-options"))
    {
        m_slWhats = msg.value("options").toStringList();
    }
    else if(!type.compare("client-token"))
    {
        m_sAuth = msg.value("token").toString();

        QVariantMap msg2;
        msg2["type"] = "get-perms";
        msg2["token"] = m_sAuth;
        QStringList perms;
        perms << "user.rating.submit";
        msg2["perms"] = perms;

        m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg2).toJson());

        emit newApprovedAuthToken(m_sAuth);
    }
    else if(!type.compare("auth-url"))
    {
        emit grantingRequired(QUrl(msg["url"].toString()));
    }
    else if(!type.compare("auth-success"))
    {
        emit loggedIn();
    }
    else if(!type.compare("auth-failure"))
    {
        m_sAuth = QString();

        login();
    }
    else if(!type.compare("notification"))
    {
        QVariantMap notif = msg.value("notification").toMap();
        emit newNotification(notif.value("type").toString(),notif.value("text").toString());
    }
}

void TouhouFMSocket::handleConnected()
{
    qDebug() << "Sending request message";
    QVariantMap msg;

    msg.insert("type","request-info");

    m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg).toJson());

    if(!m_sAuth.isEmpty())login();

    emit connected();
}

void TouhouFMSocket::timerEvent(QTimerEvent *ev)
{
    Q_UNUSED(ev)

    if(m_wsInfo->state() == QAbstractSocket::UnconnectedState)
    {
        m_wsInfo->open(QUrl("ws://en.touhou.fm/wsapp/"));
    }
}

void TouhouFMSocket::rateSong(int rating)
{
    qDebug() << "Sending rating";
    QVariantMap msg;

    msg.insert("type","user-rating");
    msg.insert("rating",rating-3);

    m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg).toJson());
}

void TouhouFMSocket::sendReport(QString what, QString details)
{
    QVariantMap msg;

    msg.insert("type","report");
    msg.insert("what",what);
    msg.insert("detail",details);

    m_wsInfo->sendTextMessage(QJsonDocument::fromVariant(msg).toJson());
}
