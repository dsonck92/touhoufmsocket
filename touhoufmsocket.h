#ifndef TOUHOUFMSOCKET_H
#define TOUHOUFMSOCKET_H

#include <QObject>
#include <QWebSocket>

//! TouhouFM Socket class.
//! This class handles any protocol details for TouHouFM. It automatically handles
//! authentication. In order to use authentication, you need to respond to
//! #newApprovedAuthToken and #grantingRequired
class TouhouFMSocket : public QObject
{
    Q_OBJECT

    QWebSocket *m_wsInfo; /**< Information channel */

    QString m_sAuth; /**< Authentication token */

    QStringList m_perms; /**< Claimed permissions */

    QStringList m_slWhats; /**< What's wrong options */


public:
    /** Constructor
     * Constructs a new TouHouFMSocket object used for interacting with TouHou.FM.
     * @param authToken the authtoken to be sent upon #login
     * @param parent the parent object this object belongs to
     */
    explicit TouhouFMSocket(QString authToken = QString(), QObject *parent = 0);

    /** Opens the connection
     */
    bool open();

    /** Update the authtoken used for #login
     * @param authToken new authtoken to be used for #login
     */
    void setAuthToken(QString authToken);

    /** Logs in.
     * Attempts to login with the current login token, if no login token is available
     * it will request a new token and possibly emit #newApprovedAuthToken or
     * #grantingRequired
     */
    void login();

    /** Retrieve current permissions.
     */
    QStringList permissions();

    QStringList getWhats() { return m_slWhats; }

signals:
    /** A new approved authorization token is available.
     * Clients are supposed to store this to present this token on a second run
     */
    void newApprovedAuthToken(QString token);
    /** A new granting url is created.
     * Clients are supposed to show this URL and allow the user to login to authorize
     * the permissions requested to the new token.
     */
    void grantingRequired(QUrl grantUrl);

    /** A user rating is updated.
     * This is emitted whenever the current personal rating is updated.
     */
    void newUserRating(int rating);
    /** A global rating is updated.
     * This is emitted whenever the current global rating is updated.
     */
    void newGlobalRating(qreal rating);

    /** The current metadata is changed.
     * This is emitted for each tag each time an update occurs.
     */
    void metaDataChanged(QString key, QVariant value);

    /** The socket is connected.
     * This is emitted after the socket is succesfully connected to an TouHou.FM server.
     */
    void connected();
    void loggedIn();

    void newProgress(qreal progress);
    void newProgress(int progress);
    void newTime(QString time);

    void newNotification(QString type, QString text);

public slots:
    /** Send a song skip request.
     * This slot sends a song skip request to TouHou.FM.
     */
    void skipSong();
    /** Send a user rating update.
     * This slot sends a song rating update to TouHou.FM. Upon success, the server will
     * return the new data through #newUserRating and #newGlobalRating
     */
    void rateSong(int rating);

    void sendReport(QString what,QString details);

private slots:
    void handleMessage(QString info);
    void handleConnected();

private:
    void timerEvent(QTimerEvent *ev);

};

#endif // TOUHOUFMSOCKET_H
