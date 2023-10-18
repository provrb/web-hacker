#pragma once

#ifdef _WIN32
#include <windows.h>
#include <ShlObj_core.h>
#include <bcrypt.h>
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")

#elif __linux__
#include <dlfcn.h>
#endif

#include <string>
#include <exception>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <locale>
#include <typeindex>

#include "../external/sqlite3.h"
#include "header/common.h"
#include "header/memory.hpp"


