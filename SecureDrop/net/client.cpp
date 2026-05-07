#include "client.h"
#include "../crypto/AES.h"
#include "../crypto/DES.h"
#include "../crypto/RSA.h"
#include <winsock2.h>
#include<bits/stdc++.h>
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
static vector<uint8_t> serverPublicKey;

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

void net::runClient()
{
    // Initialize Winsock
    WSADATA c_data;
    if (WSAStartup(MAKEWORD(2, 2), &c_data) != 0)
    {
        cerr << "WSAStartup failed" << endl;
        return;
    }

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return;
    }

    // Define server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(clientSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        cerr << "Connection failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    cout << "Connected to server!" << endl;

    // Key exchange
    // Receive server public key
    serverPublicKey.resize(16);
    if (!receiveAll(clientSocket, reinterpret_cast<char *>(serverPublicKey.data()), 16))
    {
        cerr << "Failed to receive public key" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    // Generate session keys
    aesKey = GenerateAESKey();
    desKey = GenerateDESKey();
   
    // Send encrypted session keys
    vector<uint8_t> encAesKey = RSAEncrypt(aesKey, serverPublicKey);
    vector<uint8_t> encDesKey = RSAEncrypt(desKey, serverPublicKey);
   
    // Send encrypted session keys with size prefix
    auto sendKey = [&](const vector<uint8_t> &key) -> bool
    {
        // Send key size
        uint32_t keySize = htonl(key.size());
        if (!sendAll(clientSocket, reinterpret_cast<const char *>(&keySize), 4))
        {
            cerr << "Failed to send key size" << endl;
            return false;
        }

        // Send key data
        if (!sendAll(clientSocket, reinterpret_cast<const char *>(key.data()), key.size()))
        {
            cerr << "Failed to send key data" << endl;
            return false;
        }
        return true;
    };

    if (!sendKey(encAesKey) || !sendKey(encDesKey))
    {
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    // Main communication loop
    string input;
    while (true)
    {
        cout << "> ";
        getline(cin, input);

        if (input == "exit")
        {
            break;
        }
        // ... in the mode switch handling block ...
        else if (input == "switch aes" || input == "switch des" || input == "switch both")
        {
            // Determine requested mode
            EncryptionMode requestedMode;
            if (input == "switch aes")
                requestedMode = AES_RSA;
            else if (input == "switch des")
                requestedMode = DES_RSA;
            else
                requestedMode = AES_DES_RSA;

            // Send command
            sendEncryptedMessage(clientSocket, input);

            // Receive and decrypt response
            string response = receiveDecryptedMessage(clientSocket);

            if (response == "MODE_OK")
            {
                currentMode = requestedMode; // Update mode after confirmation
                cout << "Mode changed to " << input.substr(7) << " successfully" << endl;
            }
            else
            {
                cout << "Mode change failed: " << response << endl;
            }
            continue;
        }
        else if (input.substr(0, 5) == "file " && is_file_path(input.substr(5)))
        {
            string filePath = input.substr(5);
            ifstream file(filePath, ios::binary | ios::ate);
            if (!file)
            {
                cerr << "File not found: " << filePath << endl;
                continue;
            }

            streamsize size = file.tellg();
            file.seekg(0, ios::beg);

            vector<char> buffer(size);
            if (!file.read(buffer.data(), size))
            {
                cerr << "File read error: " << filePath << endl;
                continue;
            }

            // Prepare file message
            string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
            input = "FILE:" + fileName + ":" + string(buffer.data(), size);
        }

        // Encrypt and send message
        sendEncryptedMessage(clientSocket, input);

        // Receive and decrypt response
        string response = receiveDecryptedMessage(clientSocket);

        if (!response.empty())
        {
            if (response == "switch aes" || response == "switch des" || response == "switch both")
            {
                // Determine requested mode
                EncryptionMode requestedMode;
                if (response == "switch aes")
                    requestedMode = AES_RSA;
                else if (response == "switch des")
                    requestedMode = DES_RSA;
                else
                    requestedMode = AES_DES_RSA;

                // Save current mode for acknowledgment
                EncryptionMode currentModeBeforeSwitch = currentMode;

                // Update mode immediately
                currentMode = requestedMode;

                // Send acknowledgment in PREVIOUS mode
                currentMode = currentModeBeforeSwitch;
                sendEncryptedMessage(clientSocket, "MODE_OK");
                currentMode = requestedMode;

                cout << "Mode changed to " << response.substr(7) << " by server" << endl;
                continue; // Skip further processing
            }

            // Handle file transfer from server
            if (response.substr(0, 5) == "FILE:")
            {
                size_t colonPos = response.find(':', 5);
                if (colonPos != string::npos)
                {
                    string fileName = response.substr(5, colonPos - 5);
                    string fileContent = response.substr(colonPos + 1);

                    // Save to download directory
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
                cout << "Server: " << response << endl;
            }
        }
    }

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
}