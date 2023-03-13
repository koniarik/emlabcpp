///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include <array>
#include <cstdio>

#pragma once

namespace emlabcpp
{

template < typename WriteCallable >
class simple_stream
{
public:
        explicit simple_stream( WriteCallable wc )
          : writer_( std::forward< WriteCallable >( wc ) )
        {
        }

        explicit operator bool() const
        {
                return true;  // TODO: this mgiht be bad idea
        }

        simple_stream& operator<<( const char value )
        {
                write( std::string_view{ &value, 1 } );
                return *this;
        }

        simple_stream& operator<<( const short value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%hi", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const unsigned short value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%hu", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const int value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%i", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const unsigned int value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%u", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%li", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const unsigned long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%lu", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const long long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%lli", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const unsigned long long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%llu", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const float value )
        {
                // TODO: I kinda hate this?
                std::snprintf(
                    buffer_.data(), buffer_.size(), "%f", static_cast< double >( value ) );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const double value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%f", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const bool value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%i", value ? 1 : 0 );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const void* const value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%p", value );
                write( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( std::string_view sv )
        {
                write( sv );
                return *this;
        }

private:
        void write( std::string_view sv )
        {
                writer_( sv );
        }

        WriteCallable writer_;

        // TODO: is 32 enough?
        std::array< char, 32 > buffer_;
};

}  // namespace emlabcpp
