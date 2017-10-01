/*
 * Copyright (c) 2017, Uber Technologies, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef UBER_JAEGER_TAG_H
#define UBER_JAEGER_TAG_H

#include <string>

#include <boost/variant/variant.hpp>

#include "uber/jaeger/thrift-gen/jaeger_types.h"

namespace uber {
namespace jaeger {

class Tag {
  public:
    using ValueType = boost::variant<std::string,
                                     double,
                                     bool,
                                     int64_t>;

    template <typename ValueArg>
    Tag(const std::string& key, ValueArg&& value)
        : _key(key)
        , _value(std::forward<ValueArg>(value))
    {
    }

    bool operator==(const Tag& rhs) const
    {
        return _key == rhs._key && _value == rhs._value;
    }

    const std::string& key() const { return _key; }

    const ValueType& value() const { return _value; }

    thrift::Tag thrift() const
    {
        thrift::Tag tag;
        tag.__set_key(_key);
        ThriftVisitor visitor(tag);
        _value.apply_visitor(visitor);
        return tag;
    }

  private:
    class ThriftVisitor {
      public:
        using result_type = void;

        explicit ThriftVisitor(thrift::Tag& tag)
            : _tag(tag)
        {
        }

        void operator()(const std::string& value) const
        {
            _tag.__set_vType(thrift::TagType::STRING);
            _tag.__set_vStr(value);
        }

        void operator()(double value) const
        {
            _tag.__set_vType(thrift::TagType::DOUBLE);
            _tag.__set_vDouble(value);
        }

        void operator()(bool value) const
        {
            _tag.__set_vType(thrift::TagType::BOOL);
            _tag.__set_vBool(value);
        }

        void operator()(int64_t value) const
        {
            _tag.__set_vType(thrift::TagType::LONG);
            _tag.__set_vLong(value);
        }

      private:
        thrift::Tag& _tag;
    };

    std::string _key;
    ValueType _value;
};

}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TAG_H
