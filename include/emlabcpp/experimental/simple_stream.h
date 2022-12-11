#include <array>
#include <cstdio>

#pragma once

namespace emlabcpp
{

template < typename WriteCallable >
class simple_stream
{
public:
        simple_stream( WriteCallable wc )
          : writer_( std::forward< WriteCallable >( wc ) )
        {
        }

        operator bool() const
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
