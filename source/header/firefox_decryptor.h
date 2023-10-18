#pragma once

#include <vector>
#include <string>
#include "memory.hpp"
#include "browser.h"

/* 
    PK11 Booleans
*/
#define PR_TRUE  TRUE
#define PR_FALSE FALSE

/*
  Types, enums, structs and functions defined in nss3.dll which are
  Used to interpret functions and results used by Firefox's-
  security services.

  Functions are retrieved by getting the memory address of the original function from the dll
  and put into a typedef function pointer to be used whenever.
*/
namespace _NSS
{
    namespace types
    {
        // Enum of SECItemType, a member in struct SECItem.
        typedef enum SECItemType 
        {
            siBuffer = 0,
            siClearDataBuffer = 1,
            siCipherDataBuffer,
            siDERCertBuffer,
            siEncodedCertBuffer,
            siDERNameBuffer,
            siEncodedNameBuffer,
            siAsciiNameString,
            siAsciiString,
            siDEROID,
            siUnsignedInteger,
            siUTCTime,
            siGeneralizedTime
        };

        // Parameters for PK11SDR_Decrypt
        typedef struct SECItem 
        {
            SECItemType    type;
            unsigned char* data;
            unsigned int   len;
        };

        // Return status of NSS functions
        typedef enum SECStatus 
        {
            SECSuccess    = 0,
            SECFailure    = -1,
            SECWouldBlock = -2,
        };
        
        // Return values and parameter type alias'
        typedef int    PRBOOL;
        typedef struct PK11SlotInfo;
        typedef struct PK11SymKey;
        
        /*
            Function pointer definitions used in nss3.dll
            Typedef _returnvalue_ (*_functionname_)(_params...._);
         */
        typedef SECStatus     (*NSS_Init)(const char* config);        
        typedef void          (*NSS_Shutdown)(void);
        typedef SECStatus     (*PK11SDR_Decrypt)(SECItem* data, SECItem* dataOut, void* cx);
        typedef PK11SlotInfo* (*PK11_GetInternalKeySlot)(void);
        typedef SECStatus     (*PK11_Authenticate)(PK11SlotInfo* slot, PRBOOL loadCerts, void* wincx);
        typedef void          (*PK11_FreeSlot)(PK11SlotInfo* slot);
    }

    /*  Current internal key slot. Saved to be free'd during unloading phase  */
    static types::PK11SlotInfo* internalKeySlot = nullptr;

    static bool prepared = false;
    static bool loaded   = false;

    /* ------------------- [ INFO BLOCK ] -----------------------
        Usable functions that have been filled by 
        the desired function address by
        using GetProcAddress and LoadLibrary().
        ---------------------------------------------------------- */ 
    namespace functions
    {

        /*
            Initializes Mozillas Security Service.
            @param config: Firefox default profile folder.
            @return SECStatus indicating failure or success.
         */
        static types::NSS_Init NSS_Init;

        /*
            Decrypts a SECItem passed into as param 'data'.
            Fills SECItem 'dataOut' with fianl processed data
            @param data: data in to decrypt.
            @param dataOut: where to place the process data.
            @param cx: Context for UI. Can be NULL
            @return SECStatus indicating failure or success.
        */
        static types::PK11SDR_Decrypt PK11SDR_Decrypt;

        /*
            Retrieves an internal key slot
            @return PK11SlotInfo ptr struct representing the info pulled
        */
        static types::PK11_GetInternalKeySlot PK11_GetInternalKeySlot;

        /*
            Authenticate InternalKeySlot of PK11SlotInfo ptr struct- the first param: 'slot'.
            If loadCerts is true, load certificates during authentication,
            A pointer to a window can also be included in the 'wincx' parameter
            as a window context for the user to interacte with. Can also be NULL

            @param slot: Key slot struct to authenticate
            @param loadCerts: whether to load certicates during authentication
            @param winCx: Window context for UI. Can be NULL.
            @return SECStatus indicating failure or success.
        */
        static types::PK11_Authenticate PK11_Authenticate;

        /*
           Showdown and free any resources allocated by NSS
        */
        static types::NSS_Shutdown NSS_Shutdown;

        /* 
           Free an internal key slot.

           @param KeySlot: the PK11 keyslot to be free'd
        */
        static types::PK11_FreeSlot PK11_FreeSlot;
    }
}

/*
  @class FFDecrypt
  @brief Firefox Decryption Class that uses NSS functions to decrypt.

  A firefox decryption class which uses Mozillas encryption and decryption-
  functions to decrypt usernames and passwords.
  
  Dynamically loads nss3.dll and its dependencies found in program files/
  Mozilla Firefox path and saves function pointers which can be used whenever.
  
  Finally, unloads any nss3 dependencies or previously loaded-
  librarys which were loaded by NSSLoadLibrary()
*/
class FFDecrypt
{
private:
    /*
      Prepare nss3.dll by loading all its dependencies
      Fills in function pointers by getting the address of each functions from NSS3.dll

      @return auto to loaded nss3.dll.
    */
    void* NSSPrepare();
    /*
      Wrapper for LoadLibraryA().
      Loads library 'lib' and appends it to the loaded dll vector.
      If unloadOnErr is set to true, unload all dlls in loadedDlls vector on an error
      If the function was unable to load the dll, throw an std::exception error.

      @param lib: Name of the library to load
      @param unloadOnErr: A bool decided whether or not to unload all librarys in loadedDlls on error
      @return auto to the LOADED dll. This is because the return value can either be void* or HMODULE depending on the OS
    */
    void* NSSLoadLibrary(std::string lib, bool unloadOnErr);

public:
    std::vector<void*> loadedDlls;

    /*
      Unload all librarys which were loaded for or by NSS.
      Iterates through loadedDlls vector and uses FreeLibrary() on each
      Finally, clears the vector of HMODULEs to prepare for more.

      @param dbgPrint: Whether or not to print how many librarys were unloaded
    */
    void NSSUnload(bool dbgPrint);

    /*
       Run NSSPrepare() and Mozillas NSS_Init functions
       Get Internal Key slot and auithroize it using NSS functions

       Takes in a Firefox pointer class to get the browsers User Data Default profile path-
       to use with NSS_Init

       @param browser: A pointer to a Firefox class object
       @return SECStatus indicating whether the operation was succesfully or not.
    */
    _NSS::types::SECStatus NSSLoad(Mozilla* Browser);
    
    /*
       Takes in one std::string parameter 'cipherText' of the encrypted string such as:
       encryptedUsername and encryptedPassword.
       Firstly decrypts from base64 to a binary string, then uses-
       Mozilla NSS native : PK11SDR_Decrypt to decrypt the byte string.
       Finally null terminates the string to be readable.

       @param cipherText: The encrypted text to decrypt
       @return The decrypted result as a std::string type.
    */
    std::string NSSDecrypt(std::string cipherText);
};


