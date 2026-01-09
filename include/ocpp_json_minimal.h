#ifndef OCPP_JSON_MINIMAL_H
#define OCPP_JSON_MINIMAL_H

/*
 OCPP_JSON_MINIMAL.h
 Minimal RapidJSON setup for OCPP:
 - Document / Value
 - PrettyWriter / StringBuffer
 - Schema validation
 - JSON <-> std::string
 - Exceptions instead of asserts
 - No FileReadStream/FileWriteStream for smaller binary
*/

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wexceptions"
#else
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4061 4127 4365 4464 4996 5027 26451 26819 33010)
#endif

// ================= Feature macros =================
#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_NO_INT64DEFINE
#define RAPIDJSON_NO_DOUBLE
#define RAPIDJSON_NO_POINTER
#define RAPIDJSON_ASSERT_THROWS


#include <stdexcept>
#include <string>

namespace rapidjson {

// Exception for critical parse errors
class parse_exception : public std::logic_error {
public:
    explicit parse_exception(const char* s) : std::logic_error(s) {}
    explicit parse_exception(const std::string& s) : std::logic_error(s) {}
};

// Runtime assertion macro
#ifndef RAPIDJSON_ASSERT
#define RAPIDJSON_ASSERT(x) \
    do { if (!(x)) throw parse_exception(#x); } while(0)
#endif

} // namespace rapidjson

// ================= Required RapidJSON headers =================
#include "rapidjson/document.h"       // Document / Value
#include "rapidjson/writer.h"         // Writer
#include "rapidjson/stringbuffer.h"   // StringBuffer
#include "rapidjson/prettywriter.h"   // PrettyWriter
#include "rapidjson/schema.h"         // Schema
#include "rapidjson/error/en.h"       // Error messages

// ================= Restore warnings =================
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif


#endif /* OCPP_JSON_MINIMAL_H */
