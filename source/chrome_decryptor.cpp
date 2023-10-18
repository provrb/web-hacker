#include "header/chrome_decryptor.h"

#include "../external/json.hpp"
#include "../external/base64.h"

#include <openssl/ssl.h>
#include <openssl/aes.h>
#include <openssl/evp.h> 

#include "header/common.h"

#ifdef _WIN32
#include <windows.h>
#include <ShlObj_core.h>
#endif

using JSON = nlohmann::json;
namespace fs = std::filesystem;

GCDecryptor::GCDecryptor()
{
	if (GC::LocalStateFilePath == "null" || GC::MasterKey == "null" || GC::DecryptorInitialized) {
		std::cerr << "ERROR: Do Not Use This Constructor Unless Local State Path is Inputted First" << std::endl;
		throw std::runtime_error("Bad Constructor.");
	}
}

GCDecryptor::GCDecryptor(std::string LocalStatePath)
{
	if (!fs::exists(LocalStatePath)) {
		std::cerr << "ERROR: Local State Path Invalid." << std::endl;
		throw std::runtime_error("Invalid Local State Path.");
	}

	GC::LocalStateFilePath = LocalStatePath;

	if (!this->GetMasterKey()) {
		std::cerr << "ERROR: Failed Getting Chrome Master Key." << std::endl;
		throw std::runtime_error("Failed Getting Master Key");
	}

	GC::DecryptorInitialized = true;
}

std::string GCDecryptor::Decrypt(std::vector<unsigned char> cipherText) 
{
	std::string signature(cipherText.begin(), cipherText.begin() + 3);
	if (signature == "v10" || signature == "v11") {
		cipherText.erase(cipherText.begin(), cipherText.begin() + 3);
	}

	std::cout << "Attempting to Decrypt" << std::endl;

	std::string key = GC::MasterKey;
	if (!GC::DecryptorInitialized) {
		std::cerr << "ERROR: Google Chrome Decryptor not Initialized." << std::endl;
		return "";
	}

	unsigned char tag[AES_BLOCK_SIZE];
	unsigned char nonce[NONCE_SIZE];

	std::copy(cipherText.begin(), cipherText.begin() + NONCE_SIZE, nonce);
	std::copy(cipherText.end() - TAG_SIZE, cipherText.end(), tag);

	std::vector<unsigned char> plaintext;
	plaintext.resize(cipherText.size(), '\0');

	int actual_size, final_size = 0;

	EVP_CIPHER_CTX* d_ctx = EVP_CIPHER_CTX_new();
	int init = EVP_DecryptInit(d_ctx, EVP_aes_256_gcm(), reinterpret_cast<const unsigned char*>(key.c_str()), nonce);
	if (init != EVP_SUCCESS) {
		std::cerr << "ERROR: Failed to Initialize EVP Decrypt." << std::endl;
		return "";
	}

	int update = EVP_DecryptUpdate(d_ctx, &plaintext[0], &actual_size, &cipherText[NONCE_SIZE], cipherText.size() - sizeof(tag) - sizeof(nonce));
	if (update != EVP_SUCCESS) {
		std::cerr << "ERROR: Failed to Update EVP Decrypt With Decryption Info." << std::endl;
		return "";
	}

	int ctrl = EVP_CIPHER_CTX_ctrl(d_ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag);
	if (ctrl != EVP_SUCCESS) {
		std::cerr << "ERROR: Failed Setting Decryption Tag." << std::endl;
		return "";
	}

	if (EVP_DecryptFinal(d_ctx, &plaintext[actual_size], &final_size) != EVP_SUCCESS)
		std::cerr << "ERROR: Error Decrypting Cipher Text From AES 256 GCM." << std::endl;

	EVP_CIPHER_CTX_free(d_ctx);
	plaintext.resize(actual_size + final_size, '\0');

	return std::string(plaintext.begin(), plaintext.end());
}

bool GCDecryptor::GetMasterKey()
{
	if (!fs::exists(GC::LocalStateFilePath)) {
		std::cerr << "ERROR: Local State Path Invalid (Google Chrome)." << std::endl;
		throw std::runtime_error("Local State Path Invalid.");
	}

	std::ifstream file;
	std::string content;
	
	file.open(GC::LocalStateFilePath);

	content = std::string((std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>()));

	file.close();

	std::string localStateData = content;
	JSON        localStateJson = JSON::parse(localStateData);

	nlohmann::detail::iter_impl<JSON> os_crypt;
	nlohmann::detail::iter_impl<JSON> encrypted_key;

	std::string base64_encrypted_key = "";

	if ((os_crypt = localStateJson.find("os_crypt")) != localStateJson.end())
	{
		if ((encrypted_key = os_crypt.value().find("encrypted_key")) != os_crypt.value().end())
		{
			base64_encrypted_key = encrypted_key.value();
		}
	}
	
	GC::MasterKey = DecryptMasterKey(base64_encrypted_key);
	if (GC::MasterKey != "null")
		return true;

	return false;
}

std::string GCDecryptor::DecryptMasterKey(std::string& EncryptedMasterKey)
{
	// Docode the key from base64
	std::vector<unsigned char> binaryKey;
	std::string base64Key = EncryptedMasterKey;
	std::string b64Decoded = base64_decode(base64Key);

	// Delete "DPAPI" from first 5 letters of password
	b64Decoded.erase(b64Decoded.begin(), b64Decoded.begin() + 5);

	// Crypt unprotect key to get the encryption key that google uses
	DATA_BLOB output;
	DATA_BLOB input = {};
	input.pbData = reinterpret_cast<BYTE*>(b64Decoded.data());
	input.cbData = static_cast<DWORD>(b64Decoded.size());

	BOOL cryptProcess = CryptUnprotectData(&input, NULL, NULL, NULL, NULL, 0, &output);

	if (!cryptProcess)
	{
		std::cerr << "ERROR: Failed to Use CryptUnprotectData. Error Status: " << GetLastError() << std::endl;
		return "null";
	}

	// Return string of the output data of output data size
	return std::string(reinterpret_cast<const char*>(output.pbData), output.cbData);
}
