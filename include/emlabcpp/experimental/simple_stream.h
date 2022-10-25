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
          : writer_( std::move( wc ) )
        {
        }

        operator bool()
        {
                return true;  // TODO: this mgiht be bad idea
        }

        simple_stream& operator<<( char value )
        {
                writer_( std::string_view{ &value, 1 } );
                return *this;
        }

        simple_stream& operator<<( short value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%hi", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( unsigned short value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%hu", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( int value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%i", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( unsigned int value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%u", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%li", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( unsigned long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%lu", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( long long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%lli", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( unsigned long long value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%llu", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( float value )
        {
                // TODO: I kinda hate this?
                std::snprintf(
                    buffer_.data(), buffer_.size(), "%f", static_cast< double >( value ) );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( double value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%f", value );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( bool value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%i", value ? 1 : 0 );
                writer_( buffer_.data() );
                return *this;
        }
        simple_stream& operator<<( const void* value )
        {
                std::snprintf( buffer_.data(), buffer_.size(), "%p", value );
                writer_( buffer_.data() );
                return *this;
        }

private:
        WriteCallable writer_;

        // TODO: is 32 enough?
        std::array< char, 32 > buffer_;
};

}  // namespace emlabcpp
