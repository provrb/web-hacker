#include "header/firefox_decryptor.h"

namespace NSS = _NSS;

/*
  Wrapper for LoadLibraryA().
  Loads library 'lib' and appends it to the loaded dll vector.
  If unloadOnErr is set to true, unload all dlls in loadedDlls vector on an error 
  If the function was unable to load the dll, throw an std::exception error.

  @param lib: Name of the library to load
  @param unloadOnErr: A bool decided whether or not to unload all librarys in loadedDlls on error
  @return auto to the LOADED dll. This is because the return value can either be void* or HMODULE depending on the OS
*/
void* FFDecrypt::NSSLoadLibrary(std::string lib, bool unloadOnErr=true)
{
#ifdef _WIN32
	if (lib.find(".dll") == std::string::npos)
		lib = lib + ".dll";

	HMODULE dll = NULL;
	dll = LoadLibraryA(lib.c_str());
#elif __linux__
	if (lib.find(".so") == std::string::npos)
		lib = lib + ".so";
	
	void* dll = nullptr;
	dll = dlopen(lib, RTLD_LAZY); // Success
#endif
	if (!dll)
	{
		if (unloadOnErr) 
			this->NSSUnload(true);
		
		std::cerr << "ERROR: Error Loading Library." << "\n";
#ifdef _WIN32
		std::cerr << "- Error Code: " << GetLastError() << "\n";
#elif __linux__
		std::cerr << "- Error Code: " << dlerror() << "\n";
#endif
		throw std::runtime_error("Error Loading Library");
	}
	else
	{
		std::cout << "Loaded Library : (" << lib << "}" << std::endl;

		this->loadedDlls.push_back(reinterpret_cast<void*>(dll));
		return reinterpret_cast<void*>(dll);
	}
}

/*
  Prepare nss3.dll by loading all its dependencies
  Fills in function pointers by getting the address of each functions from NSS3.dll

  @return auto to loaded nss3 lib. Auto because it depends on OS
*/
void* FFDecrypt::NSSPrepare()
{
	if (NSS::prepared)
	{
		std::cerr << "WARNING: NSS Already Prepared. Aborting NSSPrepare()." << std::endl;
		return nullptr;
	}

	// Paths for librarys needed to be loaded
	std::string msvcpPath = "";
	std::string mozgluePath = "";
	std::string nss3Path = "";

	// librarys needed to load for NSS
	// Load nss and its dependencies
#ifdef _WIN32
	std::string mozillaPath = Mozilla::GetMozillaPath();
	
	msvcpPath = mozillaPath + "msvcp140.dll";
	mozgluePath = mozillaPath + "mozglue.dll";
	nss3Path = mozillaPath + "nss3.dll";

	void* libMSCVP = this->NSSLoadLibrary(msvcpPath, true);
#elif __linux__
	mozgluePath = "libmozgtk.so";
	nss3Path    = "libnss3.so";
#endif
	 
	// 
	void* libMozGlue = this->NSSLoadLibrary(mozgluePath, true);
	void* libNSS3    = this->NSSLoadLibrary(nss3Path, true);

	using mem = memory;
	
	/*
	   Get all functions from nss3.dll
	   
	   Do this is 4 steps:
	   1. What is the name of the function we want from nss3?
	   2. Create a typedef for the function pointer. E.g : typedef return_value (*function_name)(params...) - this is a function pointer
	   3. Get the process address for that function, we now have a pointer to a function
	   4. Now cast the function address pointer to our type defined function pointer.
			- We cast it to basically inform the compiler about the return values..
			- and parameters
	*/
	NSS::functions::NSS_Init				= mem::Get<NSS::types::NSS_Init>(libNSS3, "NSS_Init");
	NSS::functions::PK11SDR_Decrypt		    = mem::Get<NSS::types::PK11SDR_Decrypt>(libNSS3, "PK11SDR_Decrypt");
	NSS::functions::PK11_GetInternalKeySlot = mem::Get<NSS::types::PK11_GetInternalKeySlot>(libNSS3, "PK11_GetInternalKeySlot");
	NSS::functions::PK11_Authenticate       = mem::Get<NSS::types::PK11_Authenticate>(libNSS3, "PK11_Authenticate");
	NSS::functions::NSS_Shutdown			= mem::Get<NSS::types::NSS_Shutdown>(libNSS3, "NSS_Shutdown");
	NSS::functions::PK11_FreeSlot			= mem::Get<NSS::types::PK11_FreeSlot>(libNSS3, "PK11_FreeSlot");
	
	NSS::prepared = true;
	return reinterpret_cast<void*>(libNSS3);
}

/*
   Run NSSPrepare() and Mozillas NSS_Init functions
   Get Internal Key slot and auithroize it using NSS functions

   Takes in a Firefox pointer class to get the browsers User Data Default profile path-
   to use with NSS_Init

   @param browser: A pointer to a Mozilla class object
   @return SECStatus indicating whether the operation was succesfully or not.
*/
NSS::types::SECStatus FFDecrypt::NSSLoad(Mozilla* Browser)
{
	if (NSS::loaded)
	{
		std::cerr << "NSS already prepared and loaded. Not running NSSPrepare()." << std::endl;
		return NSS::types::SECFailure;
	}

	void* libNSS = this->NSSPrepare();
	
	NSS::types::SECStatus nssInit = NSS::functions::NSS_Init(Browser->BrowserInfo().pProfileDefault.c_str());

	if (nssInit != NSS::types::SECSuccess) {
		std::cerr << "ERROR: Failed to initialize NSS. Status Code: " << nssInit << std::endl;
		throw browser_error("Error Initializing NSS", browser_error::ERR_UNKNOWN);
	}
	
	NSS::types::PK11SlotInfo* keySlot = NSS::functions::PK11_GetInternalKeySlot();

	if (!keySlot) {
		std::cerr << "ERROR: NSS Internal Key Slot Invalid. (FFDecrypt::NSSLoad())." << std::endl;
		throw browser_error("NSS Internal Key Slot Invalid", browser_error::ERR_UNKNOWN);
	}

	NSS::internalKeySlot = keySlot;
	NSS::types::SECStatus authenticate = NSS::functions::PK11_Authenticate(keySlot, PR_TRUE, 0);

	if (authenticate != NSS::types::SECStatus::SECSuccess) {
		std::cerr << "ERROR: Error Authenticating NSS Key Slot" << std::endl;
		throw browser_error("Failed to Authenticate NSS key slot using PK11", browser_error::ERR_UNKNOWN);
	}

	std::cout << "NSS Authenticated" << std::endl;
	NSS::loaded = true;

	return NSS::types::SECSuccess;
}

/*
   Takes in one std::string parameter 'cipherText' of the encrypted string such as:
   encryptedUsername and encryptedPassword.
   Firstly decrypts from base64 to a binary string, then uses-
   Mozilla NSS native : PK11SDR_Decrypt to decrypt the byte string.
   Finally null terminates the string to be readable.

   @param cipherText: The encrypted text to decrypt
   @return The decrypted result as a std::string type.
*/
std::string FFDecrypt::NSSDecrypt(std::string cipherText)
{
	if (!NSS::loaded || !NSS::prepared)
		return "NSS Not Loaded";

	BYTE byteData[8096];
	unsigned long byteLen = 8096;

	memset(byteData, 0, byteLen);
	
	std::string base64_decoded = base64_decode(cipherText);

	// Copy base64_decoded std::string into byteData of BYTE's
	if (base64_decoded.size() <= byteLen && !base64_decoded.empty()) {
		memcpy(byteData, base64_decoded.c_str(), base64_decoded.size());
		byteLen = base64_decoded.size();
	}
	else
		return "Error Converting From BASE64";

	NSS::types::SECItem input = {};
	input.data = reinterpret_cast<unsigned char*>(byteData);
	input.len  = byteLen;

	NSS::types::SECItem output = {};
	output.data = 0;
	output.len  = 0;

	NSS::types::SECStatus decrypt = NSS::functions::PK11SDR_Decrypt(&input, &output, NULL);
	if (decrypt != NSS::types::SECSuccess)
		return "NSS Error Decryption. Make Sure Item Was Encrypted With The Current key4.db.";
	
	// Null terminate string
	memcpy(byteData, output.data, output.len);
	byteData[output.len] = 0;
	std::string result = reinterpret_cast<char*>(byteData);

	return result;
}

/*
  Unload all librarys which were loaded for or by NSS.
  Iterates through loadedDlls vector and uses FreeLibrary() on each
  Finally, clears the vector of HMODULEs to prepare for more.
  
  If dbgPrint is true, print after the process how many librarys were unloaded
  @param dbgPrint: Whether or not to print how many librarys were unloaded.
*/
void FFDecrypt::NSSUnload(bool dbgPrint = true)
{
	if (NSS::prepared && NSS::loaded)
	{
		/* Free the internal key slot and shutdown nss */
		if (NSS::internalKeySlot)
			NSS::functions::PK11_FreeSlot(NSS::internalKeySlot);

		NSS::functions::NSS_Shutdown();

		/* nullptr all function pointers */
		NSS::functions::NSS_Init = nullptr;
		NSS::functions::PK11SDR_Decrypt = nullptr;
		NSS::functions::PK11_Authenticate = nullptr;
		NSS::functions::PK11_GetInternalKeySlot = nullptr;
		NSS::functions::PK11_FreeSlot = nullptr;

		/* Set bool values for future */
		NSS::loaded = false;
		NSS::prepared = false;

		int unloadedCount = 0;

		/*
		   Free all previously loaded libraries
		*/
		for (auto& lib : this->loadedDlls) // Auto is used because loadedDlls will either be void* or HMODULE depending on OS
		{
			bool freedLibrary = false;
#ifdef _WIN32
			int res = FreeLibrary(reinterpret_cast<HMODULE>(lib)); // Non-zero if success
			if (res != 0) freedLibrary = true;
#elif __linux__
			int res = dlclose(lib); // Zero if closed
			if (res == 0) freedLibrary = true;
#endif
			if (freedLibrary)
			{
				std::cout << "Unloaded Library: " << lib << std::endl;
				unloadedCount++;
			}
			else
				std::cerr << "WARNING: Failed to Unload A Library." << "\n";
		}

		this->loadedDlls.clear();

		if (dbgPrint)
			std::cout << "Unloaded " << unloadedCount << " libraries." << std::endl;

	}
}

