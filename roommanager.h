// 房间生命周期管理器: 管理联机房间从创建到选人的状态流转
#pragma once

#include <QObject>
#include <QJsonObject>
#include <QtQml/qqmlregistration.h>

class NetworkManager;

class RoomManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(NetworkManager* networkManager READ networkManager WRITE setNetworkManager NOTIFY networkManagerChanged)
    Q_PROPERTY(QString role READ role NOTIFY roleChanged)
    QML_ELEMENT

public:
    explicit RoomManager(QObject *parent = nullptr);

    NetworkManager *networkManager() const { return m_nm; }
    void setNetworkManager(NetworkManager *nm);

    QString role() const { return m_role; }

    Q_INVOKABLE void createRoom(const QString &roomName, quint16 port = 12345);
    Q_INVOKABLE void joinRoom(const QString &ip, quint16 port = 12345);
    Q_INVOKABLE void leaveRoom();

signals:
    void networkManagerChanged();
    void roleChanged();
    void goToSelectScreen(bool isHost);

private slots:
    void onClientConnected();
    void onConnectedToHost();
    void onMessageReceived(const QJsonObject &msg);
    void onDisconnected();
    void onErrorOccurred(const QString &error);

private:
    NetworkManager *m_nm = nullptr;
    QString m_role;
    bool m_hostReady = false;
    bool m_clientReady = false;
};
