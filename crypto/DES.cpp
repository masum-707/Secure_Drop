#include "DES.h"
#include<bits/stdc++.h>

using namespace std;

// Initial Permutation Table
const int IP[] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7};

// Final Permutation Table
const int FP[] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9, 49, 17, 57, 25};

// Expansion Table
const int E[] = {
    32, 1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1};

// S-boxes
const int S[8][4][16] = {
    {{14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
     {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
     {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
     {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}},
    {{15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
     {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
     {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
     {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}},
    {{10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
     {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
     {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
     {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}},
    {{7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
     {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
     {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
     {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}},
    {{2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
     {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
     {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
     {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}},
    {{12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
     {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
     {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
     {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}},
    {{4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
     {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
     {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
     {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}},
    {{13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
     {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
     {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
     {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}}};

// Permutation P
const int P[] = {
    16, 7, 20, 21,
    29, 12, 28, 17,
    1, 15, 23, 26,
    5, 18, 31, 10,
    2, 8, 24, 14,
    32, 27, 3, 9,
    19, 13, 30, 6,
    22, 11, 4, 25};

// Key Schedule Tables
const int PC1[] = {
    57, 49, 41, 33, 25, 17, 9,
    1, 58, 50, 42, 34, 26, 18,
    10, 2, 59, 51, 43, 35, 27,
    19, 11, 3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
    7, 62, 54, 46, 38, 30, 22,
    14, 6, 61, 53, 45, 37, 29,
    21, 13, 5, 28, 20, 12, 4};

const int PC2[] = {
    14, 17, 11, 24, 1, 5,
    3, 28, 15, 6, 21, 10,
    23, 19, 12, 4, 26, 8,
    16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32};

const int shiftSchedule[16] = {1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1};

// Helper to convert bytes to binary string
string bytesToBin(const vector<uint8_t> &bytes)
{
    string bin;
    for (uint8_t b : bytes)
    {
        bin += bitset<8>(b).to_string();
    }
    return bin;
}

// Helper to convert binary string to bytes
vector<uint8_t> binToBytes(const string &bin)
{
    vector<uint8_t> bytes;
    for (size_t i = 0; i < bin.size(); i += 8)
    {
        string byteStr = bin.substr(i, 8);
        bytes.push_back(static_cast<uint8_t>(bitset<8>(byteStr).to_ulong()));
    }
    return bytes;
}

// Helper to apply permutation
string permute(string input, const int *table, int n)
{
    string output = "";
    for (int i = 0; i < n; i++)
    {
        output += input[table[i] - 1];
    }
    return output;
}

// Left circular shift
string leftShift(string key, int shifts)
{
    return key.substr(shifts) + key.substr(0, shifts);
}

// Generate 16 round keys
vector<string> generateRoundKeys(string key)
{
    // Apply PC-1 (64-bit -> 56-bit)
    string permKey = permute(key, PC1, 56);

    // Split into halves
    string left = permKey.substr(0, 28);
    string right = permKey.substr(28, 28);

    vector<string> roundKeys;
    for (int i = 0; i < 16; i++)
    {
        // Apply shifts
        left = leftShift(left, shiftSchedule[i]);
        right = leftShift(right, shiftSchedule[i]);

        // Combine and apply PC-2 (56-bit -> 48-bit)
        string combined = left + right;
        string roundKey = permute(combined, PC2, 48);
        roundKeys.push_back(roundKey);
    }
    return roundKeys;
}

// XOR operation
string XOR(string a, string b)
{
    string ans = "";
    for (int i = 0; i < a.size(); i++)
    {
        ans += (a[i] == b[i]) ? '0' : '1';
    }
    return ans;
}

// DES Encryption
string DES(string plaintext, vector<string> roundKeys)
{
    plaintext = permute(plaintext, IP, 64);

    string left = plaintext.substr(0, 32);
    string right = plaintext.substr(32, 32);

    for (int round = 0; round < 16; round++)
    {
        string rightExpanded = permute(right, E, 48);

        // Use round-specific key
        string xored = XOR(rightExpanded, roundKeys[round]);

        string sboxResult = "";
        for (int i = 0; i < 8; i++)
        {
            int row = 2 * (xored[i * 6] - '0') + (xored[i * 6 + 5] - '0');
            int col = 8 * (xored[i * 6 + 1] - '0') +
                      4 * (xored[i * 6 + 2] - '0') +
                      2 * (xored[i * 6 + 3] - '0') +
                      (xored[i * 6 + 4] - '0');
            int val = S[i][row][col];
            bitset<4> bits(val);
            sboxResult += bits.to_string();
        }

        sboxResult = permute(sboxResult, P, 32);
        string newRight = XOR(left, sboxResult);

        // Update for next round
        left = right;
        right = newRight;
    }

    // Final swap and FP
    string combined = right + left;
    return permute(combined, FP, 64);
}

// DES Decryption (uses reversed round keys)
string DES_Decrypt(string ciphertext, vector<string> roundKeys)
{
    // Reverse the round keys for decryption
    reverse(roundKeys.begin(), roundKeys.end());
    return DES(ciphertext, roundKeys);
}

vector<uint8_t> DESEncrypt(const vector<uint8_t> &data, const vector<uint8_t> &key)
{
    // Extract first 8 bytes of key
    vector<uint8_t> key8(8, 0);
    for (int i = 0; i < 8 && i < key.size(); i++)
    {
        key8[i] = key[i];
    }

    // Pad data using PKCS#7
    vector<uint8_t> padded = data;
    uint8_t pad_value = 8 - (data.size() % 8);
    if (pad_value == 0)
        pad_value = 8;
    for (int i = 0; i < pad_value; i++)
    {
        padded.push_back(pad_value);
    }

    // Generate round keys
    string binKey = bytesToBin(key8);
    vector<string> roundKeys = generateRoundKeys(binKey);

    vector<uint8_t> cipher;
    for (int i = 0; i < padded.size(); i += 8)
    {
        vector<uint8_t> block(padded.begin() + i, padded.begin() + i + 8);
        string binBlock = bytesToBin(block);
        string cipherBin = DES(binBlock, roundKeys);
        vector<uint8_t> cipherBlock = binToBytes(cipherBin);
        cipher.insert(cipher.end(), cipherBlock.begin(), cipherBlock.end());
    }

    return cipher;
}

vector<uint8_t> DESDecrypt(const vector<uint8_t> &data, const vector<uint8_t> &key)
{
    // Data must be multiple of 8 bytes
    if (data.size() % 8 != 0)
    {
        return vector<uint8_t>();
    }

    // Extract first 8 bytes of key
    vector<uint8_t> key8(8, 0);
    for (int i = 0; i < 8 && i < key.size(); i++)
    {
        key8[i] = key[i];
    }

    string binKey = bytesToBin(key8);
    vector<string> roundKeys = generateRoundKeys(binKey);

    vector<uint8_t> plain;
    for (int i = 0; i < data.size(); i += 8)
    {
        vector<uint8_t> block(data.begin() + i, data.begin() + i + 8);
        string binBlock = bytesToBin(block);
        string plainBin = DES_Decrypt(binBlock, roundKeys);
        vector<uint8_t> plainBlock = binToBytes(plainBin);
        plain.insert(plain.end(), plainBlock.begin(), plainBlock.end());
    }

    // Remove PKCS#7 padding
    if (plain.empty())
        return plain;
    uint8_t pad_value = plain.back();
    if (pad_value > 0 && pad_value <= 8)
    {
        if (plain.size() >= pad_value)
        {
            bool valid = true;
            for (int i = 0; i < pad_value; i++)
            {
                if (plain[plain.size() - 1 - i] != pad_value)
                {
                    valid = false;
                    break;
                }
            }
            if (valid)
            {
                plain.resize(plain.size() - pad_value);
            }
        }
    }

    return plain;
}

vector<uint8_t> GenerateDESKey()
{
    return {0x13, 0x34, 0x57, 0x79, 0x9B, 0xBC, 0xDF, 0xF1}; // Example 8-byte DES key
}
