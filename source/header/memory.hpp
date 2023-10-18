#ifndef _MEMORY_HPP
#define _MEMORY_HPP

#ifdef __linux__
#include <dlfcn.h>
#elif _WIN32
#include <Windows.h>
#endif

#include <string>

/*
   A class to deal with memory related topics such as
   calling functions from dlls and getting the return value
   of functions from dlls
*/
class memory
{
public:
	/*
	    Calls a function within a dynamically loaded library using the provided function name.
		Library must be loaded then the HMODULE to that library should be passed as first parameter.

	    This function is designed to call a function within a dynamically loaded library (DLL)
	    using the specified function name. It retrieves the function's address using GetProcAddress
	    and then invokes the function with the provided arguments.
	   
	    @tparam Function: The type of the function pointer to call.
	    @tparam Args Variadic: Rpresents the types of variadic arguments.

	    @param lib: The handle to the loaded library (HMODULE).
	    @param functionName: The name of the function to call within the library.
	    @param args: Arguments to be passed to the called function.
	    
		@return A void pointer containing the result of the called function.
	*/
	template<typename Function, typename ...Args>
	static auto Call(HMODULE lib, std::string functionName, Args&&... args) noexcept
	{
#ifdef _WIN32
		return reinterpret_cast<Function>(GetProcAddress(lib, functionName.c_str()))(args...);
#elif __linux__
		return reinterpert_cast<Function>(dlsym(lib, functionName.c_str()))(args...);
#endif
	}
	

	/*
	    Retrieves a function pointer from a dynamically loaded library.
	   
	    This function is designed to retrieve a function pointer from a dynamically loaded library (DLL)
	    using the specified function name. It uses GetProcAddress to obtain the function's address.
		
	    @tparam Function: The type of the function pointer to retrieve.
	    @param lib: The handle to the loaded library (HMODULE).
	    @param functionName: The name of the function to retrieve from the library.
	    @return A void pointer containing the function pointer retrieved from the library.
	*/
	template<typename Function>	
	static auto Get(void* lib, std::string functionName) noexcept
	{
#ifdef _WIN32
		return reinterpret_cast<Function>(GetProcAddress(reinterpret_cast<HMODULE>(lib), functionName.c_str()));
#elif __linux__
		return reinterpert_cast<Function>(dlsym(lib, functionName.c_str()));
#endif
	}
};

#endif

