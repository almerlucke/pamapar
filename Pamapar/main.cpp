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

/*
 func jsonStringTransform(m *MatchResult, r *Reader) error {
     if !m.Match {
         if m.PartialMatch {
             m.Error = errors.New("not a valid string")
             r.PushError(m)
         }
         return nil
     }

     value, err := strconv.Unquote(r.StringFromResult(m))
     if err != nil {
         return err
     }

     m.Result = value

     return nil
 }
 */

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
    }
};

//pamapar::concatenation json_string_pattern() {
//
//}

/*
 func jsonStringPattern() Pattern {

     normalCodePoint := NewCharacterGroup(func(r rune) bool {
         return unicode.IsControl(r) || r == '\\' || r == '"'
     }, true, nil)

     return NewConcatenation(
         []Pattern{
             NewTerminalString(`"`, nil),
             NewAny(NewAlternation([]Pattern{normalCodePoint, escapeSequence}, nil), nil),
             NewTerminalString(`"`, nil),
         },
         jsonStringTransform,
     )
 }
 */

int main(int argc, const char * argv[]) {
    pamapar::context context;
    auto reader = pamapar::reader("121288z");
    
    auto character_member_group = new pamapar::character_group(pamapar::character_group::member_of("128"));
    auto character_member_group_ptr = std::shared_ptr<pamapar::pattern>(character_member_group);
    
    auto repeat = pamapar::repetition(character_member_group_ptr, 3, 4);
    
    auto result = repeat.match(reader, context);
    
    if (result.match) {
        auto matches = std::dynamic_pointer_cast<pamapar::matches_value>(result.value)->value;
        
        for (auto it = matches.begin(); it != matches.end(); ++it) {
            auto str = std::dynamic_pointer_cast<pamapar::string_value>((*it).value)->value;
            
            std::cout << str << std::endl;
        }
    } else {
        std::cout << result.err.reason << std::endl;
    }
    
    return 0;
}
