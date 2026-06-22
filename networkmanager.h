// Module
// File: networkmanager.h   Version: 0.1.0   License: AGPLv3
// Created:  wang cheng    2026-06-24
// Description:
//     联机网络管理器: TCP 服务器/客户端, UDP 局域网房间发现, JSON 消息收发
#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QJsonObject>
#include <QVariantList>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

class NetworkManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isServer READ isServer NOTIFY isServerChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool isDiscovering READ isDiscovering NOTIFY isDiscoveringChanged)
    Q_PROPERTY(QString localIp READ localIp NOTIFY localIpChanged)
    Q_PROPERTY(QVariantList discoveredRooms READ discoveredRooms NOTIFY discoveredRoomsChanged)

    QML_ELEMENT
public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager() override;

    bool isServer() const { return m_isServer; }
    bool isConnected() const { return m_isConnected; }
    bool isDiscovering() const { return m_isDiscovering; }
    QString localIp() const { return m_localIp; }
    QVariantList discoveredRooms() const { return m_discoveredRooms; }

    Q_INVOKABLE void startServer(quint16 port = 12345);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void connectToHost(const QString &ip, quint16 port = 12345);
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE void startDiscovery(quint16 port = 12346);
    Q_INVOKABLE void stopDiscovery();
    Q_INVOKABLE void announceRoom(const QString &roomName);
    Q_INVOKABLE void sendMessage(const QJsonObject &msg);

signals:
    void isServerChanged();
    void connectedChanged();
    void isDiscoveringChanged();
    void localIpChanged();
    void discoveredRoomsChanged();
    void clientConnected();
    void connectedToHost();
    void disconnected();
    void messageReceived(QJsonObject msg);
    void errorOccurred(QString error);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError err);
    void onUdpReadyRead();
    void onAnnounceTimer();

private:
    void detectLocalIp();

    QTcpServer *m_server = nullptr;
    QTcpSocket *m_socket = nullptr;
    QUdpSocket *m_udpSocket = nullptr;
    QTimer *m_announceTimer = nullptr;

    bool m_isServer = false;
    bool m_isConnected = false;
    bool m_isDiscovering = false;
    QString m_localIp;
    QString m_roomName;
    QVariantList m_discoveredRooms;
    QByteArray m_readBuffer;
    quint16 m_tcpPort = 12345;
    quint16 m_discoveryPort = 12346;
};
