#include "server.h"
#include "../crypto/AES.h"
#include "../crypto/DES.h"
#include "../crypto/RSA.h"
#include <winsock2.h>
#include <bits/stdc++.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Encryption modes
enum EncryptionMode
{
    AES_RSA,
    DES_RSA,
    AES_DES_RSA
};
static EncryptionMode currentMode = AES_RSA;

// Session keys
static vector<uint8_t> aesKey;
static vector<uint8_t> desKey;
static RSAKeyPair rsaKeyPair;

static bool is_file_path(const string &rawPath)
{
    string path = rawPath;
    ifstream file(path, ios::binary);
    return file.good(); // true if file opens successfully
}
// Reliable data transmission helpers
static bool receiveAll(SOCKET sock, char *buffer, int length)
{
    int received = 0;
    while (received < length)
    {
        int bytes = recv(sock, buffer + received, length - received, 0);
        if (bytes <= 0)
            return false;
        received += bytes;
    }
    return true;
}

static bool sendAll(SOCKET sock, const char *buffer, int length)
{
    int sent = 0;
    while (sent < length)
    {
        int bytes = send(sock, buffer + sent, length - sent, 0);
        if (bytes <= 0)
            return false;
        sent += bytes;
    }
    return true;
}

// Encrypt data based on current mode
static vector<uint8_t> encryptData(const vector<uint8_t> &data)
{
    vector<uint8_t> result = data;
    switch (currentMode)
    {
    case DES_RSA:
        result = DESEncrypt(result, desKey);
        break;
    case AES_RSA:
        result = AESEncrypt(result, aesKey);
        break;
    case AES_DES_RSA:
        result = DESEncrypt(result, desKey);
        result = AESEncrypt(result, aesKey);
        break;
    }
    return result;
}

// Decrypt data based on current mode
static vector<uint8_t> decryptData(const vector<uint8_t> &data)
{
    vector<uint8_t> result = data;
    switch (currentMode)
    {
    case DES_RSA:
        result = DESDecrypt(result, desKey);
        break;
    case AES_RSA:
        result = AESDecrypt(result, aesKey);
        break;
    case AES_DES_RSA:
        result = AESDecrypt(result, aesKey);
        result = DESDecrypt(result, desKey);
        break;
    }
    return result;
}

// Send encrypted message
static void sendEncryptedMessage(SOCKET socket, const string &message)
{
    vector<uint8_t> plaintext(message.begin(), message.end());
    vector<uint8_t> encrypted = encryptData(plaintext);

    // Send message size
    uint32_t size = htonl(encrypted.size());
    if (!sendAll(socket, reinterpret_cast<const char *>(&size), 4))
    {
        cerr << "Failed to send message size" << endl;
        return;
    }

    // Send encrypted data
    if (!sendAll(socket, reinterpret_cast<const char *>(encrypted.data()), encrypted.size()))
    {
        cerr << "Failed to send encrypted data" << endl;
    }
}

// Receive and decrypt message
static string receiveDecryptedMessage(SOCKET socket)
{
    // Receive message size
    uint32_t size;
    if (!receiveAll(socket, reinterpret_cast<char *>(&size), 4))
    {
        cerr << "Failed to receive message size" << endl;
        return "";
    }
    size = ntohl(size);

    // Receive encrypted data
    vector<uint8_t> encrypted(size);
    if (!receiveAll(socket, reinterpret_cast<char *>(encrypted.data()), size))
    {
        cerr << "Failed to receive encrypted data" << endl;
        return "";
    }

    // Decrypt data
    vector<uint8_t> decrypted = decryptData(encrypted);
    return string(decrypted.begin(), decrypted.end());
}

void net::runServer()
{
    // Initialize Winsock
    WSADATA s_data;
    if (WSAStartup(MAKEWORD(2, 2), &s_data) != 0)
    {
        cerr << "WSAStartup failed" << endl;
        return;
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return;
    }

    // Address info
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(serverSocket, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Listen
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Listen failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    cout << "[Server] Listening on port 8080..." << endl;

    // Accept connection
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET)
    {
        cerr << "Accept failed" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    cout << "[Server] Client connected!" << endl;

    // Key exchange
    rsaKeyPair = GenerateRSAKeyPair();
    if (!sendAll(clientSocket, reinterpret_cast<const char *>(rsaKeyPair.publicKey.data()), rsaKeyPair.publicKey.size()))
    {
        cerr << "Failed to send public key" << endl;
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Receive session keys
    auto receiveKey = [&]() -> vector<uint8_t>
    {
        // Receive key size
        uint32_t keySize;
        if (!receiveAll(clientSocket, reinterpret_cast<char *>(&keySize), 4))
        {
            cerr << "Failed to receive key size" << endl;
            return {};
        }
        keySize = ntohl(keySize);

        // Receive key data
        vector<uint8_t> key(keySize);
        if (!receiveAll(clientSocket, reinterpret_cast<char *>(key.data()), keySize))
        {
            cerr << "Failed to receive key data" << endl;
            return {};
        }
        return key;
    };

    vector<uint8_t> encAesKey = receiveKey();
    vector<uint8_t> encDesKey = receiveKey();

    if (encAesKey.empty() || encDesKey.empty())
    {
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    // Decrypt session keys
    aesKey = RSADecrypt(encAesKey, rsaKeyPair.privateKey);
    desKey = RSADecrypt(encDesKey, rsaKeyPair.privateKey);

    // Ensure keys are correct size
    if (aesKey.size() != 16)
    {
        aesKey = GenerateAESKey(); // Fallback to default
    }
    if (desKey.size() != 8)
    {
        desKey = GenerateDESKey(); // Fallback to default
    }


    // Main communication loop
    while (true)
    {
        string message = receiveDecryptedMessage(clientSocket);
        if (message.empty())
        {
            break;
        }
        string response;
        bool promptUser = true;

        // Handle mode change requests
        if (message == "switch aes" || message == "switch des" || message == "switch both")
        {
            // Save current mode for response encryption
            EncryptionMode originalMode = currentMode;

            // Determine new mode
            EncryptionMode newMode;
            if (message == "switch aes")
                newMode = AES_RSA;
            else if (message == "switch des")
                newMode = DES_RSA;
            else // "switch both"
                newMode = AES_DES_RSA;

            // Send response using ORIGINAL mode
            currentMode = originalMode;
            sendEncryptedMessage(clientSocket, "MODE_OK");

            // Update mode AFTER sending response
            currentMode = newMode;
            cout << "[Server] Mode changed to ";
            if (newMode == AES_RSA)
            {
                cout << "AES_RSA";
            }
            else if (newMode == DES_RSA)
            {
                cout << "DES_RSA";
            }
            else
            {
                cout << "AES_DES_RSA";
            }
            cout << endl;

            promptUser = false;
            continue;
        }

        else if (message == "exit")
        {
            break;
        }
        // Handle file transfer
        else if (message.substr(0, 5) == "FILE:")
        {
            size_t pos1 = message.find(':', 5);
            if (pos1 != string::npos)
            {
                string fileName = message.substr(5, pos1 - 5);
                string fileContent = message.substr(pos1 + 1);
                string fullPath = string("SecureDrop_Downloads\\") + fileName;
                ofstream file(fullPath, ios::binary);
                if (file)
                {
                    file.write(fileContent.data(), fileContent.size());
                    file.close();

                    // Get absolute path
                    char absPath[MAX_PATH];
                    GetFullPathNameA(fullPath.c_str(), MAX_PATH, absPath, NULL);
                    cout << "File received: " << absPath << " (" << fileContent.size() << " bytes)" << endl;
                }
                else
                {
                    cerr << "Failed to save file: " << fullPath << endl;
                }
            }
        }
        else
        {
            cout << "Client: " << message << endl;
            promptUser = true;
        }

        if (promptUser)
        {
            cout << "> ";
            getline(cin, response);

            if (response == "switch aes" || response == "switch des" || response == "switch both")
            {
                // Send switch command
                sendEncryptedMessage(clientSocket, response);

                // Wait for acknowledgment
                string ack = receiveDecryptedMessage(clientSocket);
                if (ack == "MODE_OK")
                {
                    // Update mode after acknowledgment
                    if (response == "switch aes")
                        currentMode = AES_RSA;
                    else if (response == "switch des")
                        currentMode = DES_RSA;
                    else
                        currentMode = AES_DES_RSA;

                    cout << "Mode changed to " << response.substr(7) << " successfully" << endl;
                }
                else
                {
                    cout << "Mode change failed: " << ack << endl;
                }
                continue; // Skip normal response sending
            }

            // Handle file sending from server
            if (response.substr(0, 5) == "file " && is_file_path(response.substr(5)))
            {

                string filePath = response.substr(5);

                ifstream file(filePath, ios::binary | ios::ate);
                if (!file)
                {
                    cerr << "File not found: " << filePath << endl;
                    continue;
                }

                streamsize size = file.tellg();
                file.seekg(0, ios::beg);

                vector<char> buffer(size);
                if (file.read(buffer.data(), size))
                {
                    string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
                    response = "FILE:" + fileName + ":" + string(buffer.data(), size);
                    cout << "Sending file: " << fileName << " (" << size << " bytes)" << endl;
                }
                else
                {
                    cerr << "File read error: " << filePath << endl;
                    continue;
                }
            }
        }

        // Encrypt and send response
        sendEncryptedMessage(clientSocket, response);
    }

    // Cleanup
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}