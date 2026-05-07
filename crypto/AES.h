#pragma once
#include<bits/stdc++.h>
using namespace std;
void PadMessage(vector<uint8_t> &message);
void RemovePadding(vector<uint8_t> &data);

vector<uint8_t> AESEncrypt(const vector<uint8_t> &data, const vector<uint8_t> &key);
vector<uint8_t> AESDecrypt(const vector<uint8_t> &data, const vector<uint8_t> &key);
vector<uint8_t> GenerateAESKey();