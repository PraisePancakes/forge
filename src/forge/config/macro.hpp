#ifndef FORGE_CONFIG_MACRO_H
#define FORGE_CONFIG_MACRO_H

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define FORGE_STR(arg) #arg
#define FORGE_XSTR(arg) FORGE_STR(arg)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif

#ifndef FORGE_NDEBUG
#define FORGE_ASSERT(condition, message)                                       \
    do {                                                                       \
        if (!(condition)) {                                                    \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__   \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate();                                                  \
        }                                                                      \
    } while (false)
#else
#define FORGE_ASSERT(condition, message) \
    do {                                 \
    } while (false)
#endif