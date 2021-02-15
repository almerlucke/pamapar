//
//  main.cpp
//  pamapar
//
//  Created by Almer Lucke on 29/01/2021.
//

#include <iostream>
#include <vector>
#include <memory>
#include "pamapar.h"
#include "helpers/string/string.h"

class json_string_pattern: public pamapar::concatenation {
public:
    json_string_pattern() {
        auto hex_digit = pamapar::alternation::make_shared(pamapar::pattern_vector{
            pamapar::character_group::make_shared(pamapar::character_group::in_range('a', 'z')),
            pamapar::character_group::make_shared(pamapar::character_group::in_range('A', 'Z')),
            pamapar::character_group::make_shared(pamapar::character_group::in_range('0', '9'))
        });
        
        auto hex_pattern = pamapar::concatenation::make_shared(pamapar::pattern_vector{
            pamapar::terminal_string::make_shared("u"),
            pamapar::repetition::make_shared(hex_digit, 4, 4)
        });
        
        auto escape_sequence = pamapar::concatenation::make_shared(pamapar::pattern_vector{
            pamapar::terminal_string::make_shared("\\"),
            pamapar::alternation::make_shared(pamapar::pattern_vector{
                pamapar::character_group::make_shared(pamapar::character_group::member_of("\"\\/bfnrt")),
                hex_pattern
            })
        });
        
        auto normal_code_point = pamapar::character_group::make_shared([](pamapar::rune r) {
            return iscntrl(r) || r == '\\' || r == '"';
        }, true);
        
        this->patterns = pamapar::pattern_vector{
            pamapar::terminal_string::make_shared("\""),
            pamapar::repetition::make_shared(pamapar::alternation::make_shared(pamapar::pattern_vector{
                normal_code_point, escape_sequence
            })),
            pamapar::terminal_string::make_shared("\"")
        };
    }
    
    static pamapar::pattern_ptr make_shared() {
        return pamapar::pattern_ptr(new json_string_pattern());
    }
    
    void transform(pamapar::match_result& m, pamapar::reader& r, pamapar::context& ctx) override {
        if (!m.match) {
            if (m.partial_match) {
                auto str = m.string_from_reader(r);
                std::cout << "partial: " << str << std::endl;
                m.err = pamapar::error("not a valid string");
                ctx.push_error(m);
            }
            
            return;
        }
        
        auto str = pamapar::unquote(m.string_from_reader(r));
        
        m.value = std::shared_ptr<pamapar::match_result_value>(new pamapar::string_value(str));
    }
};

int main(int argc, const char * argv[]) {
    pamapar::context context;
    auto reader = pamapar::reader("\"dit is een test man\"");
    
    auto json_string = json_string_pattern::make_shared();
    
    auto result = json_string->match(reader, context);
    
    if (result.match) {
        auto str = std::dynamic_pointer_cast<pamapar::string_value>(result.value)->value;
            
        std::cout << str << std::endl;
    } else {
        std::cout << result.err.reason << std::endl;
    }
    
    return 0;
}
