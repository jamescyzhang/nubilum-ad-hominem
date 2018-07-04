#include "JSON.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <limits>

namespace json
{
    static const int max_depth = 200;

    using std::string;
    using std::vector;
    using std::map;
    using std::make_shared;
    using std::initializer_list;
    using std::move;

    struct null_struct
    {
        bool operator==(null_struct) const
        {
            return true;
        }

        bool operator<(null_struct) const
        {
            return false;
        }
    };

    static void dump(null_struct, string &out)
    {
        out += "null";
    }

    static void dump(double value, string &out)
    {
        if (std::isfinite(value))
        {
            char buf[32];
            snprintf(buf, sizeof buf, "%.17g", value);
            out += buf;
        }
        else
        {
            out += "null";
        }
    }

    static void dump(int value, string &out)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "%d", value);
        out += buf;
    }

    static void dump(bool value, string &out)
    {
        out += value ? "true" : "false";
    }

    static void dump(const string &value, string &out)
    {
        out += '"';
        for (size_t i = 0; i < value.length(); i++)
        {
            const char ch = value[i];
            if (ch == '\\')
            {
                out += "\\\\";
            }
            else if (ch == '"')
            {
                out += "\\\"";
            }
            else if (ch == '\b')
            {
                out += "\\b";
            }
            else if (ch == '\f')
            {
                out += "\\f";
            }
            else if (ch == '\n')
            {
                out += "\\n";
            }
            else if (ch == '\r')
            {
                out += "\\r";
            }
            else if (ch == '\t')
            {
                out += "\\t";
            }
            else if (static_cast<uint8_t>(ch) <= 0x1f)
            {
                char buf[8];
                snprintf(buf, sizeof buf, "\\u%04x", ch);
                out += buf;
            }
            else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(value[i + 1]) == 0x80
                     && static_cast<uint8_t>(value[i + 2]) == 0xa8)
            {
                out += "\\u2028";
                i += 2;
            }
            else if (static_cast<uint8_t>(ch) == 0xe2 && static_cast<uint8_t>(value[i + 1]) == 0x80
                     && static_cast<uint8_t>(value[i + 2]) == 0xa9)
            {
                out += "\\u2029";
                i += 2;
            }
            else
            {
                out += ch;
            }
        }
        out += '"';
    }

    static void dump(const JSON::array &values, string &out)
    {
        bool first = true;
        out += "[";
        for (const auto &value : values)
        {
            if (!first)
                out += ", ";
            value.dump(out);
            first = false;
        }
        out += "]";
    }

    static void dump(const JSON::object &values, string &out)
    {
        bool first = true;
        out += "{";
        for (const auto &kv : values)
        {
            if (!first)
                out += ", ";
            dump(kv.first, out);
            out += ": ";
            kv.second.dump(out);
            first = false;
        }
        out += "}";
    }

    void JSON::dump(std::string &out) const
    {
        m_ptr->dump(out);
    }

    template<JSON::json_type tag, typename T>
    class value : public json_value
    {
    protected:
        explicit value(const T &value) : m_value(value)
        {}

        explicit value(T &&value) : m_value(move(value))
        {}

        JSON::json_type type() const override
        {
            return tag;
        }

        bool equals(const json_value *other) const override
        {
            return m_value == static_cast<const value<tag, T> *>(other)->m_value;
        }

        bool less(const json_value *other) const override
        {
            return m_value < static_cast<const value<tag, T> *>(other)->m_value;
        }

        const T m_value;

        void dump(string &out) const override
        {
            json::dump(m_value, out);
        }
    };

    class json_double final : public value<JSON::NUMBER, double>
    {
        double number_value() const override
        {
            return m_value;
        }

        int int_value() const override
        {
            return static_cast<int>(m_value);
        }

        bool equals(const json_value *other) const override
        {
            return m_value == other->number_value();
        }

        bool less(const json_value *other) const override
        {
            return m_value < other->number_value();
        }

    public:
        explicit json_double(double val) : value(val)
        {}
    };

    class json_int final : public value<JSON::NUMBER, int>
    {
        double number_value() const override
        {
            return m_value;
        }

        int int_value() const override
        {
            return m_value;
        }

        bool equals(const json_value *other) const override
        {
            return m_value == other->number_value();
        }

        bool less(const json_value *other) const override
        {
            return m_value < other->number_value();
        }

    public:
        explicit json_int(int val) : value(val)
        {}
    };


    class json_bool final : public value<JSON::BOOL, bool>
    {
        bool bool_value() const override
        {
            return m_value;
        }

    public:
        explicit json_bool(bool val) : value(val)
        {}
    };

    class json_string final : public value<JSON::STRING, string>
    {
        const string &string_value() const override
        {
            return m_value;
        }

    public:
        explicit json_string(const string &val) : value(val)
        {}

        explicit json_string(string &&val) : value(move(val))
        {}
    };

    class json_array final : public value<JSON::ARRAY, JSON::array>
    {
        const JSON::array &array_items() const override
        {
            return m_value;
        }

        const JSON &operator[](size_t i) const override;

    public:
        explicit json_array(const JSON::array &val) : value(val)
        {}

        explicit json_array(JSON::array &&val) : value(move(val))
        {}
    };

    class json_object final : public value<JSON::OBJECT, JSON::object>
    {
        const JSON::object &object_items() const override
        {
            return m_value;
        }

        const JSON &operator[](const string &key) const override;

    public:
        explicit json_object(const JSON::object &val) : value(val)
        {}

        explicit json_object(JSON::object &&val) : value(move(val))
        {}
    };


    class json_null final : public value<JSON::NUL, null_struct>
    {
    public:
        json_null() : value({})
        {}
    };

    struct statics
    {
        const std::shared_ptr<json_value> null = make_shared<json_null>();
        const std::shared_ptr<json_value> t = make_shared<json_bool>(true);
        const std::shared_ptr<json_value> f = make_shared<json_bool>(false);
        const string empty_string;
        const vector<JSON> empty_vector;
        const map<string, JSON> empty_map;

        statics()
        {}
    };

    static const statics &get_statics()
    {
        static const statics s{};
        return s;
    }

    static const JSON &static_null()
    {
        static const JSON null_json;
        return null_json;
    }

    JSON::JSON() noexcept : m_ptr(statics().null)
    {}

    JSON::JSON(std::nullptr_t) noexcept : m_ptr(statics().null)
    {}

    JSON::JSON(double value) : m_ptr(make_shared<json_double>(value))
    {}

    JSON::JSON(int value) : m_ptr(make_shared<json_int>(value))
    {}

    JSON::JSON(bool value) : m_ptr(value ? statics().t : statics().f)
    {}

    JSON::JSON(const string &value) : m_ptr(make_shared<json_string>(value))
    {}

    JSON::JSON(string &&value) : m_ptr(make_shared<json_string>(move(value)))
    {}

    JSON::JSON(const char *value) : m_ptr(make_shared<json_string>(value))
    {}

    JSON::JSON(const JSON::array &values) : m_ptr(make_shared<json_array>(values))
    {}

    JSON::JSON(JSON::array &&values) : m_ptr(make_shared<json_array>(move(values)))
    {}

    JSON::JSON(const JSON::object &values) : m_ptr(make_shared<json_object>(values))
    {}

    JSON::JSON(JSON::object &&values) : m_ptr(make_shared<json_object>(move(values)))
    {}

    JSON::json_type JSON::type() const
    {
        return m_ptr->type();
    }

    double JSON::number_value() const
    {
        return m_ptr->number_value();
    }

    int JSON::int_value() const
    {
        return m_ptr->int_value();
    }

    bool JSON::bool_value() const
    {
        return m_ptr->bool_value();
    }

    const string &JSON::string_value() const
    {
        return m_ptr->string_value();
    }

    const vector<JSON> &JSON::array_items() const
    {
        return m_ptr->array_items();
    }

    const map<string, JSON> &JSON::object_items() const
    {
        return m_ptr->object_items();
    }

    const JSON &JSON::operator[](size_t i) const
    {
        return (*m_ptr)[i];
    }

    const JSON &JSON::operator[](const string &key) const
    {
        return (*m_ptr)[key];
    }

    double json_value::number_value() const
    {
        return 0;
    }

    int json_value::int_value() const
    {
        return 0;
    }

    bool json_value::bool_value() const
    {
        return false;
    }

    const string &json_value::string_value() const
    {
        return get_statics().empty_string;
    }

    const vector<JSON> &json_value::array_items() const
    {
        return get_statics().empty_vector;
    }

    const map<string, JSON> &json_value::object_items() const
    {
        return get_statics().empty_map;
    }

    const JSON &json_value::operator[](size_t) const
    {
        return static_null();
    }

    const JSON &json_value::operator[](const string &) const
    {
        return static_null();
    }

    const JSON &json_object::operator[](const string &key) const
    {
        auto iter = m_value.find(key);
        return (iter == m_value.end()) ? static_null() : iter->second;
    }

    const JSON &json_array::operator[](size_t i) const
    {
        if (i >= m_value.size()) return static_null();
        else return m_value[i];
    }

    bool JSON::operator==(const JSON &other) const
    {
        if (m_ptr == other.m_ptr)
            return true;
        if (m_ptr->type() != other.m_ptr->type())
            return false;

        return m_ptr->equals(other.m_ptr.get());
    }

    bool JSON::operator<(const JSON &other) const
    {
        if (m_ptr == other.m_ptr)
            return false;
        if (m_ptr->type() != other.m_ptr->type())
            return m_ptr->type() < other.m_ptr->type();

        return m_ptr->less(other.m_ptr.get());
    }


    static inline string esc(char c)
    {
        char buf[12];
        if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f)
        {
            snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
        }
        else
        {
            snprintf(buf, sizeof buf, "(%d)", c);
        }
        return string(buf);
    }

    static inline bool in_range(long x, long lower, long upper)
    {
        return (x >= lower && x <= upper);
    }

    namespace
    {
        struct json_parser final
        {
            const string &str;
            size_t i;
            string &err;
            bool failed;
            const json_parse strategy;

            JSON fail(string &&msg)
            {
                return fail(move(msg), JSON());
            }

            template<typename T>
            T fail(string &&msg, const T err_ret)
            {
                if (!failed)
                    err = std::move(msg);
                failed = true;
                return err_ret;
            }

            void consume_whitespace()
            {
                while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t')
                    i++;
            }

            bool consume_comment()
            {
                bool comment_found = false;
                if (str[i] == '/')
                {
                    i++;
                    if (i == str.size())
                        return fail("unexpected end of input after start of comment", false);
                    if (str[i] == '/')
                    {
                        i++;
                        // advance until next line, or end of input
                        while (i < str.size() && str[i] != '\n')
                        {
                            i++;
                        }
                        comment_found = true;
                    }
                    else if (str[i] == '*')
                    {
                        i++;
                        if (i > str.size() - 2)
                            return fail("unexpected end of input inside multi-line comment", false);
                        // advance until closing tokens
                        while (!(str[i] == '*' && str[i + 1] == '/'))
                        {
                            i++;
                            if (i > str.size() - 2)
                                return fail("unexpected end of input inside multi-line comment", false);
                        }
                        i += 2;
                        comment_found = true;
                    }
                    else
                        return fail("malformed comment", false);
                }
                return comment_found;
            }

            void consume_garbage()
            {
                consume_whitespace();
                if (strategy == json_parse::COMMENTS)
                {
                    bool comment_found = false;
                    do
                    {
                        comment_found = consume_comment();
                        if (failed) return;
                        consume_whitespace();
                    } while (comment_found);
                }
            }

            char get_next_token()
            {
                consume_garbage();
                if (failed) return (char) 0;
                if (i == str.size())
                    return fail("unexpected end of input", (char) 0);

                return str[i++];
            }

            void encode_utf8(long pt, string &out)
            {
                if (pt < 0)
                    return;

                if (pt < 0x80)
                {
                    out += static_cast<char>(pt);
                }
                else if (pt < 0x800)
                {
                    out += static_cast<char>((pt >> 6) | 0xC0);
                    out += static_cast<char>((pt & 0x3F) | 0x80);
                }
                else if (pt < 0x10000)
                {
                    out += static_cast<char>((pt >> 12) | 0xE0);
                    out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
                    out += static_cast<char>((pt & 0x3F) | 0x80);
                }
                else
                {
                    out += static_cast<char>((pt >> 18) | 0xF0);
                    out += static_cast<char>(((pt >> 12) & 0x3F) | 0x80);
                    out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
                    out += static_cast<char>((pt & 0x3F) | 0x80);
                }
            }

            string parse_string()
            {
                string out;
                long last_escaped_codepoint = -1;
                while (true)
                {
                    if (i == str.size())
                        return fail("unexpected end of input in string", "");

                    char ch = str[i++];

                    if (ch == '"')
                    {
                        encode_utf8(last_escaped_codepoint, out);
                        return out;
                    }

                    if (in_range(ch, 0, 0x1f))
                        return fail("unescaped " + esc(ch) + " in string", "");

                    if (ch != '\\')
                    {
                        encode_utf8(last_escaped_codepoint, out);
                        last_escaped_codepoint = -1;
                        out += ch;
                        continue;
                    }

                    if (i == str.size())
                        return fail("unexpected end of input in string", "");

                    ch = str[i++];

                    if (ch == 'u')
                    {
                        string esc = str.substr(i, 4);

                        if (esc.length() < 4)
                        {
                            return fail("bad \\u escape: " + esc, "");
                        }
                        for (size_t j = 0; j < 4; j++)
                        {
                            if (!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F')
                                && !in_range(esc[j], '0', '9'))
                                return fail("bad \\u escape: " + esc, "");
                        }

                        long codepoint = strtol(esc.data(), nullptr, 16);

                        if (in_range(last_escaped_codepoint, 0xD800, 0xDBFF)
                            && in_range(codepoint, 0xDC00, 0xDFFF))
                        {
                            encode_utf8((((last_escaped_codepoint - 0xD800) << 10) | (codepoint - 0xDC00)) + 0x10000,
                                        out);
                            last_escaped_codepoint = -1;
                        }
                        else
                        {
                            encode_utf8(last_escaped_codepoint, out);
                            last_escaped_codepoint = codepoint;
                        }

                        i += 4;
                        continue;
                    }

                    encode_utf8(last_escaped_codepoint, out);
                    last_escaped_codepoint = -1;

                    if (ch == 'b')
                    {
                        out += '\b';
                    }
                    else if (ch == 'f')
                    {
                        out += '\f';
                    }
                    else if (ch == 'n')
                    {
                        out += '\n';
                    }
                    else if (ch == 'r')
                    {
                        out += '\r';
                    }
                    else if (ch == 't')
                    {
                        out += '\t';
                    }
                    else if (ch == '"' || ch == '\\' || ch == '/')
                    {
                        out += ch;
                    }
                    else
                    {
                        return fail("invalid escape character " + esc(ch), "");
                    }
                }
            }

            JSON parse_number()
            {
                size_t start_pos = i;

                if (str[i] == '-')
                    i++;

                // Integer part
                if (str[i] == '0')
                {
                    i++;
                    if (in_range(str[i], '0', '9'))
                        return fail("leading 0s not permitted in numbers");
                }
                else if (in_range(str[i], '1', '9'))
                {
                    i++;
                    while (in_range(str[i], '0', '9'))
                        i++;
                }
                else
                {
                    return fail("invalid " + esc(str[i]) + " in number");
                }

                if (str[i] != '.' && str[i] != 'e' && str[i] != 'E'
                    && (i - start_pos) <= static_cast<size_t>(std::numeric_limits<int>::digits10))
                {
                    return std::atoi(str.c_str() + start_pos);
                }

                if (str[i] == '.')
                {
                    i++;
                    if (!in_range(str[i], '0', '9'))
                        return fail("at least one digit required in fractional part");

                    while (in_range(str[i], '0', '9'))
                        i++;
                }

                if (str[i] == 'e' || str[i] == 'E')
                {
                    i++;

                    if (str[i] == '+' || str[i] == '-')
                        i++;

                    if (!in_range(str[i], '0', '9'))
                        return fail("at least one digit required in exponent");

                    while (in_range(str[i], '0', '9'))
                        i++;
                }
                return std::strtod(str.c_str() + start_pos, nullptr);
            }

            JSON expect(const string &expected, JSON res)
            {
                assert(i != 0);
                i--;
                if (str.compare(i, expected.length(), expected) == 0)
                {
                    i += expected.length();
                    return res;
                }
                else
                {
                    return fail("parse error: expected " + expected + ", got " + str.substr(i, expected.length()));
                }
            }

            JSON parse_json(int depth)
            {
                if (depth > max_depth)
                {
                    return fail("exceeded maximum nesting depth");
                }

                char ch = get_next_token();
                if (failed)
                    return JSON();

                if (ch == '-' || (ch >= '0' && ch <= '9'))
                {
                    i--;
                    return parse_number();
                }

                if (ch == 't')
                    return expect("true", true);

                if (ch == 'f')
                    return expect("false", false);

                if (ch == 'n')
                    return expect("null", JSON());

                if (ch == '"')
                    return parse_string();

                if (ch == '{')
                {
                    map<string, JSON> data;
                    ch = get_next_token();
                    if (ch == '}')
                        return data;

                    while (1)
                    {
                        if (ch != '"')
                            return fail("expected '\"' in object, got " + esc(ch));

                        string key = parse_string();
                        if (failed)
                            return JSON();

                        ch = get_next_token();
                        if (ch != ':')
                            return fail("expected ':' in object, got " + esc(ch));

                        data[std::move(key)] = parse_json(depth + 1);
                        if (failed)
                            return JSON();

                        ch = get_next_token();
                        if (ch == '}')
                            break;
                        if (ch != ',')
                            return fail("expected ',' in object, got " + esc(ch));

                        ch = get_next_token();
                    }
                    return data;
                }

                if (ch == '[')
                {
                    vector<JSON> data;
                    ch = get_next_token();
                    if (ch == ']')
                        return data;

                    while (1)
                    {
                        i--;
                        data.push_back(parse_json(depth + 1));
                        if (failed)
                            return JSON();

                        ch = get_next_token();
                        if (ch == ']')
                            break;
                        if (ch != ',')
                            return fail("expected ',' in list, got " + esc(ch));

                        ch = get_next_token();
                        (void) ch;
                    }
                    return data;
                }

                return fail("expected value, got " + esc(ch));
            }
        };
    }

    JSON JSON::parse(const std::string &in, std::string &err, json::json_parse strategy)
    {
        json_parser parser{in, 0, err, false, strategy};
        JSON result = parser.parse_json(0);
        parser.consume_garbage();
        if (parser.failed)
            return JSON();
        if (parser.i != in.size())
            return parser.fail("unexpected trailing " + esc(in[parser.i]));
        return result;
    }

    vector<JSON>
    JSON::parse_multi(const string &in, std::string::size_type &parser_stop_pos, string &err, json_parse strategy)
    {
        json_parser parser{in, 0, err, false, strategy};
        parser_stop_pos = 0;
        vector<JSON> json_vec;
        while (parser.i != in.size() && !parser.failed)
        {
            json_vec.push_back(parser.parse_json(0));
            if (parser.failed)
                break;

            parser.consume_garbage();
            if (parser.failed)
                break;
            parser_stop_pos = parser.i;
        }
        return json_vec;
    }

    bool JSON::has_shape(const shape &types, string &err) const
    {
        if (!is_object())
        {
            err = "expected JSON object, got " + dump();
            return false;
        }

        for (auto &item : types)
        {
            if ((*this)[item.first].type() != item.second)
            {
                err = "bad type for " + item.first + " in " + dump();
                return false;
            }
        }
        return true;
    }
}
