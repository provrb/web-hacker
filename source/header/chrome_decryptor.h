#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <iostream>
#include <ostream>
#include <fstream>

namespace GC 
{
	static bool DecryptorInitialized = false;

	static std::string MasterKey = "null";
	static std::string LocalStateFilePath = "null";
}

class GCDecryptor
{
private:
	std::string DecryptMasterKey(std::string& EncryptedMasterKey);
public:

	GCDecryptor();
	explicit GCDecryptor(std::string LocalStatePath);

	bool   GetMasterKey();
	static std::string Decrypt(std::vector<unsigned char> CipherText);
};