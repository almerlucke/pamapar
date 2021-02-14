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
        
    }
};

//pamapar::concatenation json_string_pattern() {
//
//}

/*
 func jsonStringPattern() Pattern {
     hexDigit := NewAlternation(
         []Pattern{
             NewCharacterRange('a', 'z', false, nil),
             NewCharacterRange('A', 'Z', false, nil),
             NewCharacterRange('0', '9', false, nil),
         },
         nil,
     )

     hexPattern := NewConcatenation(
         []Pattern{
             NewTerminalString("u", nil),
             NewRepetition(hexDigit, 4, 4, nil),
         },
         nil,
     )

     escapeSequence := NewConcatenation(
         []Pattern{
             NewTerminalString(`\`, nil),
             NewAlternation(
                 []Pattern{
                     NewCharacterEnum(`"\/bfnrt`, false, nil),
                     hexPattern,
                 },
                 nil,
             ),
         },
         nil,
     )

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
    
    auto character_member_group = pamapar::character_group(pamapar::character_group::member_of("128"));
    
    auto repeat = pamapar::repetition(character_member_group, 3, 4);
    
    auto result = repeat.match(reader, context);
    
    if (result.match) {
        auto matches = std::dynamic_pointer_cast<pamapar::matches_value>(result.value).get()->value;
        
        for (auto it = matches.begin(); it != matches.end(); ++it) {
            auto str = std::dynamic_pointer_cast<pamapar::string_value>((*it).value).get()->value;
            
            std::cout << str << std::endl;
        }
    } else {
        std::cout << result.err.reason << std::endl;
    }
    
    return 0;
}
