#include "RSA.h"
#include<bits/stdc++.h>
using namespace std;
// Fixed RSA keys - DO NOT CHANGE
const uint64_t FIXED_P = 4294967291ULL;
const uint64_t FIXED_Q = 4294967279ULL;
const uint64_t FIXED_N = FIXED_P * FIXED_Q;
const uint64_t FIXED_E = 65537;
const uint64_t FIXED_D = 16909335704520072833ULL; // Precomputed d = e⁻¹ mod φ(n)

// Helper functions
namespace
{
    // Fast exponentiation (modular exponentiation)
    uint64_t mod_exp(uint64_t base, uint64_t exp, uint64_t mod)
    {
        uint64_t result = 1;
        base %= mod;

        while (exp > 0)
        {
            if (exp & 1)
                result = (result * base) % mod;
            exp >>= 1;
            base = (base * base) % mod;
        }
        return result;
    }

    // Serialize keys
vector<uint8_t> serialize_key(uint64_t exp, uint64_t mod)
    {
        vector<uint8_t> key(16, 0);
        for (int i = 0; i < 8; i++)
        {
            key[i] = static_cast<uint8_t>(exp >> (56 - i * 8));
        }
        for (int i = 0; i < 8; i++)
        {
            key[i + 8] = static_cast<uint8_t>(mod >> (56 - i * 8));
        }
        return key;
    }

    // Deserialize keys
pair<uint64_t, uint64_t> deserialize_key(const vector<uint8_t> &key)
    {
        uint64_t exp = 0, mod = 0;
        for (int i = 0; i < 8; i++)
        {
            exp = (exp << 8) | key[i];
        }
        for (int i = 0; i < 8; i++)
        {
            mod = (mod << 8) | key[i + 8];
        }
        return make_pair(exp, mod);
    }
}

// Generate fixed RSA key pair
RSAKeyPair GenerateRSAKeyPair()
{
    RSAKeyPair keyPair;
    keyPair.publicKey = serialize_key(FIXED_E, FIXED_N);
    keyPair.privateKey = serialize_key(FIXED_D, FIXED_N);
    return keyPair;
}

// RSA encryption
vector<uint8_t> RSAEncrypt(const vector<uint8_t> &data, const vector<uint8_t> &publicKey)
{
    // Extract exponent and modulus from public key
    pair<uint64_t, uint64_t> keys = deserialize_key(publicKey);
    uint64_t e = keys.first;
    uint64_t n = keys.second;

    vector<uint8_t> encrypted;
    size_t max_chunk_size = (log2(n) / 8) - 1; // Calculate max bytes per chunk

    for (size_t i = 0; i < data.size(); i += max_chunk_size)
    {
        size_t chunk_size = min(max_chunk_size, data.size() - i);
        uint64_t block = 0;

        // Create block from data
        for (size_t j = 0; j < chunk_size; j++)
        {
            block = (block << 8) | data[i + j];
        }

        // Encrypt the block
        uint64_t encrypted_block = mod_exp(block, e, n);

        // Convert to bytes (big-endian)
        vector<uint8_t> chunk;
        for (int j = 0; j < 8; j++)
        {
            chunk.insert(chunk.begin(), static_cast<uint8_t>(encrypted_block & 0xFF));
            encrypted_block >>= 8;
        }

        encrypted.insert(encrypted.end(), chunk.begin(), chunk.end());
    }
    return encrypted;
}

// RSA decryption
vector<uint8_t> RSADecrypt(const vector<uint8_t> &data, const vector<uint8_t> &privateKey)
{
    pair<uint64_t, uint64_t> keys = deserialize_key(privateKey);
    uint64_t d = keys.first;
    uint64_t n = keys.second;

    vector<uint8_t> decrypted;

    for (size_t i = 0; i < data.size(); i += 8)
    {
        size_t end = min(i + 8, data.size());
        uint64_t block = 0;

        for (size_t j = i; j < end; j++)
        {
            block = (block << 8) | data[j];
        }

        uint64_t decrypted_block = mod_exp(block, d, n);

        // Dynamically extract bytes from decrypted_block
        vector<uint8_t> chunk(7, 0);
        for (int j = 6; j >= 0; j--)
        {
            chunk[j] = static_cast<uint8_t>(decrypted_block & 0xFF);
            decrypted_block >>= 8;
        }
        decrypted.insert(decrypted.end(), chunk.begin(), chunk.end());
        }

    // Trim trailing zeros (if padded during encryption)
    while (!decrypted.empty() && decrypted.back() == 0)
    {
        decrypted.pop_back();
    }

    return decrypted;
}
