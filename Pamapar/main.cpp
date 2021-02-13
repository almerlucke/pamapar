//
//  main.cpp
//  pamapar
//
//  Created by Almer Lucke on 29/01/2021.
//

#include <iostream>
#include <vector>
#include "pamapar.h"

int main(int argc, const char * argv[]) {
    pamapar::context context;
    auto reader = pamapar::reader("11z");
    
    auto character_member_group = pamapar::character_group(pamapar::character_group::member_of("128"));
    
    auto repeat = pamapar::repetition(character_member_group, 3, 4);
    
    auto result = repeat.match(reader, context);
    
    if (result.match) {
        auto matches = dynamic_cast<pamapar::matches_value*>(result.value)->value;
        
        for (auto it = matches.begin(); it != matches.end(); ++it) {
            std::cout <<  dynamic_cast<pamapar::string_value*>((*it).value)->value << std::endl;
        }
    } else {
        std::cout << result.err.reason << std::endl;
    }
    
    return 0;
}
