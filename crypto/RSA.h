#pragma once
#include<bits/stdc++.h>
using namespace std;
struct RSAKeyPair
{
    vector<uint8_t> publicKey;
    vector<uint8_t> privateKey;
};

RSAKeyPair GenerateRSAKeyPair();
vector<uint8_t> RSAEncrypt(const vector<uint8_t> &data, const vector<uint8_t> &publicKey);
vector<uint8_t> RSADecrypt(const vector<uint8_t> &data, const vector<uint8_t> &privateKey);