//
//  pamapar.h
//  Pamapar
//
//  Created by Almer Lucke on 10/02/2021.
//

#ifndef pamapar_h
#define pamapar_h

#include "helpers/utf8/utf8.h"
#include "helpers/string/string.h"

#include <iostream>
#include <vector>
#include <memory>

namespace pamapar {
    typedef char32_t rune;

    class reader {
    private:
        std::u32string buffer;
        int64_t buf_pos;
        std::vector<int64_t> buf_pos_stack;
        std::vector<int64_t> lines;
        int64_t line_pos;
        std::vector<int64_t> line_pos_stack;
        
    public:
        struct position {
            int64_t absolute_char_pos;
            int64_t relative_char_pos;
            int64_t line_pos;
            
            position(int64_t absolute_char_pos = 0, int64_t relative_char_pos = 0, int64_t line_pos = 0) {
                this->absolute_char_pos = absolute_char_pos;
                this->relative_char_pos = relative_char_pos;
                this->line_pos = line_pos;
            }
        };
        
        reader(std::string utf8_buffer): buffer(utf8::utf8to32(utf8_buffer)) {
            int64_t end_index = buffer.length();
            
            for (int64_t index = 0; index < end_index;) {
                rune r = buffer[index];
                index++;
                
                if (r == '\r') {
                    if (index < end_index && buffer[index] == '\n') {
                        index++;
                    }
                    
                    lines.push_back(index);
                } else if (r == '\n') {
                    lines.push_back(index);
                }
            }
            
            buf_pos = 0;
            line_pos = 0;
            buf_pos_stack.push_back(buf_pos);
            line_pos_stack.push_back(line_pos);
        }
        
        int64_t relative_position() {
            if (line_pos == 0) {
                return buf_pos;
            }
            
            return buf_pos - lines[line_pos - 1];
        }
        
        position current_position() {
            return position(buf_pos, relative_position(), line_pos);
        }
        
        void push_state() {
            buf_pos_stack.push_back(buf_pos);
            line_pos_stack.push_back(line_pos);
        }
        
        void restore_state() {
            buf_pos = buf_pos_stack[buf_pos_stack.size() - 1];
            line_pos = line_pos_stack[line_pos_stack.size() - 1];
            pop_state();
        }
        
        void pop_state() {
            buf_pos_stack.pop_back();
            line_pos_stack.pop_back();
        }
        
        std::string string() {
            int64_t prev_pos = buf_pos_stack[buf_pos_stack.size() - 1];
            
            return utf8::utf32to8(buffer.substr(prev_pos, buf_pos - prev_pos));
        }
        
        std::string string_from(int64_t begin, int64_t end) {
            return utf8::utf32to8(buffer.substr(begin, end - begin));
        }
        
        bool finished() {
            return buf_pos >= buffer.length();
        }
        
        rune peek() {
            if (buf_pos < buffer.length()) {
                return buffer[buf_pos];
            }
            
            return EOF;
        }
        
        rune read() {
            if (buf_pos < buffer.length()) {
                rune c = buffer[buf_pos];
                buf_pos++;
                
                if (buf_pos < buffer.length() && line_pos < lines.size()) {
                    if (buf_pos >= lines[line_pos]) {
                        line_pos++;
                    }
                }
                
                return c;
            }
            
            return EOF;
        }
    };

    struct error {
        std::string reason = "";
        int64_t type = 0;
        
        error(std::string reason = "", int64_t type = 0) {
            this->reason = reason;
            this->type = type;
        }
    };

    struct match_result_value {
        match_result_value() {}
        virtual ~match_result_value() {}
    };

    struct string_value: match_result_value {
        std::string value;
        
        string_value(std::string value): value(value) {}
    };

    struct match_result {
        bool match = false;
        bool partial_match = false;
        reader::position begin_pos = reader::position();
        reader::position end_pos = reader::position();
        std::shared_ptr<match_result_value> value;
        error err = error();
        
        match_result(bool match = false,
                     bool partial_match = false,
                     reader::position begin_pos = reader::position(),
                     reader::position end_pos = reader::position(),
                     std::shared_ptr<match_result_value> value = std::shared_ptr<match_result_value>(nullptr),
                     error err = error()): match(match), partial_match(partial_match), begin_pos(begin_pos), end_pos(end_pos), value(value), err(err) {
        }
        
        std::string string_from_reader(reader& r) {
            int64_t begin = begin_pos.absolute_char_pos;
            int64_t end = end_pos.absolute_char_pos;
            
            return r.string_from(begin, end);
        }
    };

    struct matches_value: match_result_value {
        std::vector<match_result> value;
        
        matches_value(std::vector<match_result> value): value(value) {}
    };

    class context {
        
    public:
        // Keep track of match result errors, use transform to push errors, can be
        // used later to track errors back
        std::vector<match_result> error_stack;
        
        ~context() {
        }
        
        void push_error(match_result result) {
            error_stack.push_back(result);
        }
    };

    class pattern {
    public:
        virtual ~pattern() {
            // empty stub
        }
        
        // Must be implemented by subclasses
        virtual match_result match(reader& r, context& ctx) = 0;
        
        // Can be implemented by subclasses
        virtual void transform(match_result& m, reader& r, context& ctx) {
            // empty stub, you would typically transform the value here
        }
    };

    typedef std::shared_ptr<pattern> pattern_ptr;
    typedef std::vector<pattern_ptr> pattern_vector;

    class terminal_string: public pattern {
    private:
        std::u32string u32_str;
        
    public:
        
        terminal_string(std::string utf8_str): u32_str(utf8::utf8to32(utf8_str)) {}
        
        static pattern_ptr make_shared(std::string utf8_str) {
            return pattern_ptr(new terminal_string(utf8_str));
        }
        
        match_result match(reader& r, context& ctx) override {
            match_result result(false, false, r.current_position());
            
            r.push_state();
            
            for (auto it = u32_str.begin(); it != u32_str.end(); ++it) {
                rune c = r.read();
                
                if (*it != c) {
                    result.end_pos = r.current_position();
                    transform(result, r, ctx);
                    r.restore_state();
                    return result;
                }
            }
            
            result.match = true;
            result.end_pos = r.current_position();
            result.value = std::shared_ptr<match_result_value>(new string_value(r.string()));
            
            transform(result, r, ctx);
            
            r.pop_state();
            
            return result;
        }
    };

    class character_group: public pattern {
    private:
        std::function<bool(rune)> is_member;
        bool inverted = false;
    
    public:
        character_group(std::function<bool(rune)> is_member,
                        bool inverted = false):is_member(is_member), inverted(inverted) {}
        
        static pattern_ptr make_shared(std::function<bool(rune)> is_member, bool inverted = false) {
            return pattern_ptr(new character_group(is_member, inverted));
        }
        
        match_result match(reader &r, context& ctx) override {
            match_result result(false, false, r.current_position());
            
            r.push_state();
            
            rune c = r.read();
            
            result.end_pos = r.current_position();
            
            bool member = is_member(c);
            
            if (inverted) {
                member = !member;
            }
            
            if (EOF == c || !member) {
                transform(result, r, ctx);
                r.restore_state();
                return result;
            }
            
            result.match = true;
            result.value = std::shared_ptr<match_result_value>(new string_value(r.string()));
            
            transform(result, r, ctx);
            r.pop_state();
            
            return result;
        }
        
        static std::function<bool(rune)> in_range(rune low, rune high) {
            return [low, high](rune r) {
                return r >= low && r <= high;
            };
        }
        
        static std::function<bool(rune)> member_of(std::string utf8_str) {
            std::u32string member_str = utf8::utf8to32(utf8_str);

            return [member_str](rune r) {
                return member_str.find(r) != std::u32string::npos;
            };
        }
    };

    class alternation: public pattern {
    private:
        pattern_vector patterns;
        
    public:
        alternation(pattern_vector patterns): patterns(patterns) {}
        
        static pattern_ptr make_shared(pattern_vector patterns) {
            return pattern_ptr(new alternation(patterns));
        }
        
        match_result match(reader &r, context& ctx) override {
            reader::position begin_pos = r.current_position();
            
            for (auto it = patterns.begin(); it != patterns.end(); ++it) {
                auto pattern = *it;
                auto result = pattern->match(r, ctx);
                
                if (result.match) {
                    transform(result, r, ctx);
                    return result;
                }
            }
            
            match_result result = match_result(false, false, begin_pos, begin_pos);
            
            transform(result, r, ctx);
            
            return result;
        }
    };

    class concatenation: public pattern {
    protected:
        pattern_vector patterns;
        
    public:
        concatenation() {}
        concatenation(pattern_vector patterns): patterns(patterns) {}
        
        static pattern_ptr make_shared(pattern_vector patterns) {
            return pattern_ptr(new concatenation(patterns));
        }
        
        match_result match(reader &r, context& ctx) override {
            match_result result(false, false, r.current_position());
            std::vector<match_result> matches;
            bool partial_match = false;
            
            r.push_state();
            
            for (auto it = patterns.begin(); it != patterns.end(); ++it) {
                auto pattern = *it;
                auto sub_result = pattern->match(r, ctx);
                
                if (!sub_result.match) {
                    result.partial_match = partial_match;
                    result.end_pos = r.current_position();
                    transform(result, r, ctx);
                    
                    r.restore_state();
                    return result;
                }
                
                partial_match = true;
                
                matches.push_back(sub_result);
            }
            
            result.match = true;
            result.end_pos = r.current_position();
            result.value = std::shared_ptr<match_result_value>(new matches_value(matches));
            
            transform(result, r, ctx);
            
            r.pop_state();
            
            return result;
        }
    };

    class repetition: public pattern {
    private:
        int min = 0;
        int max = 0;
        pattern_ptr p;
    
    public:
        repetition(pattern_ptr p, int min = 0, int max = 0): p(p), min(min), max(max) {}
        
        static pattern_ptr make_shared(pattern_ptr p, int min = 0, int max = 0) {
            return pattern_ptr(new repetition(p, min, max));
        }
        
        match_result match(reader &r, context &ctx) override {
            match_result result(false, false, r.current_position());
            std::vector<match_result> matches;
            
            r.push_state();
            
            while (true) {
                if (r.finished()) {
                    break;
                }
                
                auto sub_result = p->match(r, ctx);
                
                if (sub_result.match) {
                    matches.push_back(sub_result);
                    
                    if (max != 0 && matches.size() == max) {
                        break;
                    }
                } else {
                    break;
                }
            }
            
            result.end_pos = r.current_position();
            
            if (matches.size() < min) {
                result.err = error(string_format("expected minimum of %d repetitions", min));
                transform(result, r, ctx);
                r.restore_state();
                return result;
            }
            
            result.match = true;
            result.value = std::shared_ptr<match_result_value>(new matches_value(matches));
            
            transform(result, r, ctx);
            
            r.pop_state();
            
            return result;
        }
    };

    class optional: public repetition {
        optional(pattern_ptr p): repetition(p, 0, 1) {}
        
        static pattern_ptr make_shared(pattern_ptr p) {
            return pattern_ptr(new optional(p));
        }
    };

    class one: public repetition {
        one(pattern_ptr p): repetition(p, 1, 1) {}
        
        static pattern_ptr make_shared(pattern_ptr p) {
            return pattern_ptr(new one(p));
        }
    };

    class exception: public pattern {
    private:
        pattern_ptr must_match;
        pattern_ptr except;
    
    public:
        exception(pattern_ptr must_match, pattern_ptr except): must_match(must_match), except(except) {}
        
        static pattern_ptr make_shared(pattern_ptr must_match, pattern_ptr except) {
            return pattern_ptr(new exception(must_match, except));
        }
        
        match_result match(reader &r, context &ctx) override {
            r.push_state();
            
            auto result = except->match(r, ctx);
            
            if (result.match) {
                result.match = false;
                
                transform(result, r, ctx);
                
                r.restore_state();
                
                return result;
            }
            
            r.pop_state();
            
            result = must_match->match(r, ctx);
            
            transform(result, r, ctx);
            
            return result;
        }
    };

    class eof: public pattern {
    public:
        static pattern_ptr make_shared() {
            return pattern_ptr(new eof());
        }
        
        match_result match(reader &r, context &ctx) override {
            match_result result = match_result(r.finished());
           
            transform(result, r, ctx);
            
            return result;
        }
    };
}

#endif /* pamapar_h */
