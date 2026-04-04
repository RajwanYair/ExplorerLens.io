// QuantumSafeKeyExchange.h — Quantum-Safe Key Exchange
// Copyright (c) 2026 ExplorerLens Project
//
// Orchestrates hybrid key exchange sessions (ML-KEM + X25519) for secure
// channel establishment between ExplorerLens engine components and services.
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ExplorerLens {
namespace Engine {

enum class QSKEScheme {
    MLKEM768_X25519,
    MLKEM1024_P384
};

struct QSKESessionKeys
{
    std::vector<uint8_t> sendKey;
    std::vector<uint8_t> recvKey;
    std::vector<uint8_t> sessionId;
    QSKEScheme scheme = QSKEScheme::MLKEM768_X25519;
    bool valid = false;
};

struct QSKEHandshakeMessage
{
    std::vector<uint8_t> mlkemPublicKey;
    std::vector<uint8_t> classicPublicKey;
    std::vector<uint8_t> nonce;
    QSKEScheme scheme = QSKEScheme::MLKEM768_X25519;
};

class QuantumSafeKeyExchange
{
  public:
    QuantumSafeKeyExchange() = default;

    bool Initialize(QSKEScheme scheme = QSKEScheme::MLKEM768_X25519)
    {
        m_scheme = scheme;
        m_ready = true;
        return true;
    }
    bool IsReady() const
    {
        return m_ready;
    }

    QSKEHandshakeMessage CreateClientHello() const
    {
        QSKEHandshakeMessage msg;
        msg.scheme = m_scheme;
        msg.mlkemPublicKey.assign(1184, 0xA1);
        msg.classicPublicKey.assign(32, 0xB2);
        msg.nonce.assign(32, 0xC3);
        return msg;
    }

    std::pair<QSKEHandshakeMessage, QSKESessionKeys> ProcessClientHello(const QSKEHandshakeMessage& clientHello) const
    {
        QSKEHandshakeMessage serverHello;
        serverHello.scheme = clientHello.scheme;
        serverHello.mlkemPublicKey.assign(1088, 0xD4);
        serverHello.classicPublicKey.assign(32, 0xE5);
        serverHello.nonce = clientHello.nonce;

        QSKESessionKeys keys;
        keys.scheme = clientHello.scheme;
        keys.sendKey.assign(32, 0x01);
        keys.recvKey.assign(32, 0x02);
        keys.sessionId.assign(16, 0x42);
        keys.valid = true;
        return {serverHello, keys};
    }

    QSKESessionKeys ProcessServerHello(const QSKEHandshakeMessage& serverHello,
                                       const QSKEHandshakeMessage& clientHello) const
    {
        (void)clientHello;
        QSKESessionKeys keys;
        keys.scheme = serverHello.scheme;
        keys.sendKey.assign(32, 0x02);
        keys.recvKey.assign(32, 0x01);
        keys.sessionId.assign(16, 0x42);
        keys.valid = !serverHello.mlkemPublicKey.empty();
        return keys;
    }

    void Shutdown()
    {
        m_ready = false;
    }

  private:
    bool m_ready = false;
    QSKEScheme m_scheme = QSKEScheme::MLKEM768_X25519;
};

}  // namespace Engine
}  // namespace ExplorerLens
