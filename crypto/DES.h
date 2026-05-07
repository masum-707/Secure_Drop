#pragma once
#include<bits/stdc++.h>
using namespace std;
vector<uint8_t> DESEncrypt(const vector<uint8_t> &data, const vector<uint8_t> &key);
vector<uint8_t> DESDecrypt(const vector<uint8_t> &data, const vector<uint8_t> &key);
vector<uint8_t> GenerateDESKey();