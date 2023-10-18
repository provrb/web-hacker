#pragma once

#include <filesystem>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <ShlObj_core.h>
#endif

#include <iostream>
#include <fstream>
#include <ostream>


namespace utils
{	
	namespace fs = std::filesystem;	

	/**
	 * @brief Get the path of a special folder on the system.
	 *
	 * @param CSIDL The CSIDL identifier for the folder to retrieve.
	 * @return The path of the special folder as a string.
	 * @throws std::exception if there is an error in obtaining the folder path.
	 */
	std::string GetFolder(int CSIDL)
	{
		char path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL, NULL, 0, path)))
			return std::string(path);

		std::cerr << "ERROR: Failed Getting Folder Using CSIDL. CSIDL #: " << CSIDL << std::endl;
		throw std::exception("Error Getting Folder Path With CSIDL " + CSIDL);
	}

	/**
	 * @brief Copy a file to a specified path.
	 *
	 * @param input The path to the source file.
	 * @param outputPath The path to the destination where the file should be copied.
	 */
	void CopyFileToPath(std::string input, std::string outputPath)
	{
		if (CopyFileA(input.c_str(), outputPath.c_str(), false) == false)
			std::cout << "Error Copying File: " << input << " Error: " << GetLastError() << std::endl;
	}

	/**
	 * @brief Rename an existing file in a folder.
	 *
	 * @param folder The folder containing the file.
	 * @param currentName The current name of the file.
	 * @param newName The new name to rename the file to.
	 * @return True if the file is successfully renamed; otherwise, false.
	 */
	bool RenameFile(std::string folder, std::string currentName, std::string newName)
	{
		fs::path currentFolderWithFilename = folder + "\\" + currentName;
		fs::path newFolderWithFilename     = folder + "\\" + newName;

		if (fs::exists(currentFolderWithFilename))
		{
			if (!fs::exists(newFolderWithFilename)) {
				try {
					fs::rename(currentFolderWithFilename, newFolderWithFilename);
					
					if (fs::exists(newFolderWithFilename))
						return true;
				}
				catch (fs::filesystem_error& err) {
					std::cout << "ERROR: Couldn't Rename File. Description:\n-\t " << err.what() << "\n";
				}
			}
			else
				std::cout << newFolderWithFilename << " Already Exists.";
		}
		else
			std::cout << currentFolderWithFilename << " Doesn't Exist.";

		return false;
	}

	/**
	 * @brief Rename an existing folder.
	 *
	 * @param pOld The current path to the folder to rename.
	 * @param pNew The new path to change the old folder to.
	 * @param dbgPrint Whether or not to print the process to the console (optional).
	 * @return True if the folder is successfully renamed; otherwise, false.
	 */
	bool RenameFolder(std::string pOld, std::string pNew, bool dbgPrint=false)
	{
		if (dbgPrint)
			std::cout << "Renaming: \"" << pOld << "\" to \"" << pNew << std::endl;

		try {
			fs::rename(pOld, pNew);
			if (fs::exists(pNew))
				return true;
		}
		catch (fs::filesystem_error& err)
		{
			std::cout << "ERROR: Failed To Rename Folder: " << err.what() << std::endl;;
		}

		return false;
	}

	/**
	 * @brief Read the contents of a file into a string variable.
	 *
	 * @param filename The name of the file to read.
	 * @param errIfEmpty Whether to throw an exception if the file is empty.
	 * @return A string containing the file's contents.
	 * @throws std::runtime_error if there is an error in opening or reading the file.
	 */
	std::string ReadFileIntoString(const std::string& filename, bool errIfEmpty) {
		std::ifstream file;
		std::string content;

		file.open(filename);
		if (file.is_open())
		{
			std::cout << "File: " << filename << " Opened: " << file.is_open() << std::endl;

			content = std::string((std::istreambuf_iterator<char>(file)),
				(std::istreambuf_iterator<char>()));

			file.close();
			std::cout << "\t- Finished with file. Closed" << std::endl;
		}
		else {
			std::cerr << "ERROR: Couldnt Open File: " << filename << std::endl;
			throw std::runtime_error("Couldn't Open File.");
		}


		if (content.empty() && errIfEmpty) {
			std::cerr << "ERROR: Cannot Read Empty File: " << filename << std::endl;
			throw std::runtime_error("Error Reading or File is Empty.");
		}


		return content;
	}

	/**
	 * @brief Renames an existing file to have a ".sqlite" extension.
	 *
	 * @param filePath The path to the folder where the file is located.
	 * @param fileName The name of the file (without the extension).
	 * @return True if the file is successfully renamed; otherwise, false.
	 */
	bool ToSqliteFile(std::string filePath, std::string fileName)
	{
		if (fs::exists(filePath + "\\" + fileName))
		{
			std::string sqlFilename = fileName + ".sqlite";
			
			if (utils::RenameFile(filePath, fileName, sqlFilename))
				return true;
			
		}
		else
			return false;
	}

	/**
	 * @brief Renames an existing file to have a ".json" extension.
	 *
	 * @param filePath The path to the folder where the file is located.
	 * @param fileName The name of the file (without the extension).
	 * @return True if the file is successfully renamed; otherwise, false.
	 */
	bool ToJsonFile(std::string filePath, std::string fileName)
	{
		if (fs::exists(filePath + "\\" + fileName))
		{
			std::string sqlFilename = fileName + ".json";
			
			if (utils::RenameFile(filePath, fileName, sqlFilename))
				return false;
		}
		else
			return false;
	}

	/**
	 * @brief Creates a new file at the specified path if it does not exist.
	 *
	 * @param fileNameWithPath The full path to the file to be created.
	 * @return True if the file is successfully created or already exists; otherwise, false.
	 */
	bool CreateFileFromPath(std::string fileNameWithPath)
	{
		if (!fs::exists(fileNameWithPath))
		{
			std::fstream createFile;
			createFile.open(fileNameWithPath, std::ios::out);
			createFile.close();

			if (fs::exists(fileNameWithPath))
				return true;
		}
		return false;
	}

	/*
	 * @brief Converts an existing file to a different format or creates a new file if it doesnt exist.
	 *
	 * @param folder The folder where the file should be located.
	 * @param filename The name of the file (without an extension).
	 * @param conversionMode The mode of conversion (1 for JSON, 2 for SQLITE).
	 * @param newFileExtension The extension to be added when creating a new file.
	 * @return True if the conversion or creation is successful; otherwise, false.
	 */
	bool ConvertOrCreateFile(std::string folder, std::string filename, int conversionMode, std::string newFileExtension)
	{
		bool success = false;

		// File exists without any extension
		// Convert it accordingly
		if (utils::fs::exists(folder + "/" + filename))
		{
			if (conversionMode == 1) // JSON
			{
				success = utils::ToJsonFile(folder, filename);
			}
			else if (conversionMode == 2) // SQLITE
			{
				success = utils::ToSqliteFile(folder, filename);
			}
		}

		// File doesnt exist with or without the extension
		// Create a new one.
		else if (!utils::fs::exists(folder + "\\" + filename)
			&& !utils::fs::exists(folder + "\\" + filename + newFileExtension)) 
		{
				success = utils::CreateFileFromPath(folder + "\\" + filename + newFileExtension);
		}

		if (!success)
			std::cerr << "ERROR: Failed To Convert or Create File: " << folder + "/" + filename << std::endl;

		return success;
	}

	int IsSqliteColumnNull(sqlite3_stmt* stmt, int colNum) {
		return sqlite3_column_type(stmt, colNum) == SQLITE_NULL;
	}

	std::string GetSqliteTextColumnIfNotNull(sqlite3_stmt* stmt, int colNum) {
		if (IsSqliteColumnNull(stmt, colNum)) // Column is null, nothing in it
			return "null";
		else // Column isn't null
			return reinterpret_cast<const char*>(sqlite3_column_text(stmt, colNum));
	}
}

