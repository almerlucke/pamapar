//
//  string.h
//  pamapar
//
//  Created by Almer Lucke on 13/02/2021.
//

#ifndef pamapar_string_h
#define pamapar_string_h

#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <iomanip>

namespace pamapar {
    // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename ... Args>
    std::string string_format(const std::string& format, Args ... args) {
        int size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
        std::unique_ptr<char[]> buf( new char[ size ] );
        snprintf( buf.get(), size, format.c_str(), args ... );
        return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    }

    std::string unquote(const std::string& s) {
      std::string result;
      std::istringstream ss(s);
      ss >> std::quoted(result);
      return result;
    }
}

#endif /* pamapar_string_h */
