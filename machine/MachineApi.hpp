#ifdef _WIN32
    #ifdef MACHINE_DLL
        #define MACHINE_API __declspec(dllexport)
    #else
        #define MACHINE_API __declspec(dllimport)
    #endif
#else
    #define MACHINE_API
#endif
