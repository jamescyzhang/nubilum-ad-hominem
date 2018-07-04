#ifndef UTIL_JSON_HPP
#define UTIL_JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <initializer_list>

namespace json
{
    enum json_parse
    {
        STANDARD, COMMENTS
    };

    class json_value;

    class JSON final
    {
    public:
        enum json_type
        {
            NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
        };

        typedef std::vector<JSON> array;
        typedef std::map<std::string, JSON> object;

        JSON() noexcept;

        JSON(std::nullptr_t) noexcept;

        JSON(double value);

        JSON(int value);

        JSON(bool value);

        JSON(const std::string &value);

        JSON(std::string &&value);

        JSON(const char *value);

        JSON(const array &values);

        JSON(array &&values);

        JSON(const object &values);

        JSON(object &&values);

        template<class T, class=decltype(&T::to_json)>
        JSON(const T &t) : JSON(t.to_json())
        {}

        template<class M, typename std::enable_if<
                std::is_constructible<std::string, decltype(std::declval<M>().begin()->first)>::value
                && std::is_constructible<JSON, decltype(std::declval<M>().begin()->second)>::value,
                int>::type = 0>
        JSON(const M &m) : JSON(object(m.begin(), m.end()))
        {}

        template<class V, typename std::enable_if<
                std::is_constructible<JSON, decltype(*std::declval<V>().begin())>::value,
                int>::type = 0>
        JSON(const V &v) : JSON(array(v.begin(), v.end()))
        {}

        JSON(void *) = delete;

        json_type type() const;

        bool is_null() const
        {
            return type() == NUL;
        }

        bool is_number() const
        {
            return type() == NUMBER;
        }

        bool is_bool() const
        {
            return type() == BOOL;
        }

        bool is_string() const
        {
            return type() == STRING;
        }

        bool is_array() const
        {
            return type() == ARRAY;
        }

        bool is_object() const
        {
            return type() == OBJECT;
        }

        double number_value() const;

        int int_value() const;

        bool bool_value() const;

        const std::string &string_value() const;

        const array &array_items() const;

        const object &object_items() const;

        const JSON &operator[](size_t i) const;

        const JSON &operator[](const std::string &key) const;

        void dump(std::string &out) const;

        std::string dump() const
        {
            std::string out;
            dump(out);
            return out;
        }

        static JSON parse(const std::string &in, std::string &err, json_parse strategy = json_parse::STANDARD);

        static JSON parse(const char *in, std::string &err, json_parse strategy = json_parse::STANDARD)
        {
            if (in)
            {
                return parse(std::string(in), err, strategy);
            }
            else
            {
                err = "null input";
                return nullptr;
            }
        }

        static std::vector<JSON>
        parse_multi(const std::string &in, std::string::size_type &parser_stop_pos, std::string &err,
                    json_parse strategy = json_parse::STANDARD);

        static inline std::vector<JSON>
        parse_multi(const std::string &in, std::string &err, json_parse strategy = json_parse::STANDARD)
        {
            std::string::size_type parser_stop_pos;
            return parse_multi(in, parser_stop_pos, err, strategy);
        }

        bool operator==(const JSON &rhs) const;

        bool operator<(const JSON &rhs) const;

        bool operator!=(const JSON &rhs) const
        {
            return !(*this == rhs);
        }

        bool operator<=(const JSON &rhs) const
        {
            return !(rhs < *this);
        }

        bool operator>(const JSON &rhs) const
        {
            return (rhs < *this);
        }

        bool operator>=(const JSON &rhs) const
        {
            return !(*this < rhs);
        }

        typedef std::initializer_list<std::pair<std::string, json_type>> shape;

        bool has_shape(const shape &types, std::string &err) const;

    private:
        std::shared_ptr<json_value> m_ptr;
    };


    class json_value
    {
    protected:
        friend class JSON;

        friend class json_int;

        friend class json_double;

        virtual JSON::json_type type() const = 0;

        virtual bool equals(const json_value *other) const = 0;

        virtual bool less(const json_value *other) const = 0;

        virtual void dump(std::string &out) const = 0;

        virtual double number_value() const;

        virtual int int_value() const;

        virtual bool bool_value() const;

        virtual const std::string &string_value() const;

        virtual const JSON::array &array_items() const;

        virtual const JSON &operator[](size_t i) const;

        virtual const JSON::object &object_items() const;

        virtual const JSON &operator[](const std::string &key) const;

        virtual ~json_value()
        {}
    };
}

#endif //UTIL_JSON_HPP
