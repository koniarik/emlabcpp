#include "emlabcpp/bounded.h"
#include "emlabcpp/view.h"

#pragma once

namespace emlabcpp
{
template < typename Iterator, bounded_derived SizeType >
class bounded_view : public view< Iterator >
{
public:
        using size_type = SizeType;
        using iterator  = Iterator;

        static constexpr std::size_t min = size_type::min_val;
        static constexpr std::size_t max = size_type::max_val;

private:
        bounded_view( iterator beg, iterator end )
          : view< Iterator >( beg, end )
        {
        }

public:
        template < typename, bounded_derived >
        friend class bounded_view;

        template < bounded_derived OtherSize >
        requires( OtherSize::min_val >= min && OtherSize::max_val <= max )
            bounded_view( const bounded_view< Iterator, OtherSize >& other )
          : bounded_view( other.begin(), other.end() )
        {
        }

        template < typename Container >
        requires(
            range_container< Container >&& static_sized< Container >&&
                                           std::tuple_size_v< Container > <= max &&
            std::tuple_size_v< Container > >= min ) bounded_view( Container& cont )
          : bounded_view( cont.begin(), cont.end() )
        {
        }

        static std::optional< bounded_view > make( view< Iterator > v )
        {
                if ( v.size() < min ) {
                        return {};
                }
                if ( v.size() > max ) {
                        return {};
                }
                return { bounded_view( v.begin(), v.end() ) };
        }

        template < std::size_t n >
        requires( n <= min ) bounded_view< iterator, bounded< std::size_t, n, n > > first()
        const
        {
                return { this->begin(), this->begin() + n };
        }

        template < std::size_t n >
        requires( n <= min )
            bounded_view< iterator, bounded< std::size_t, min - n, max - n > > offset()
        const
        {
                return { this->begin() + n, this->end() };
        }

        template < typename OffsetSizeType >
        std::optional< bounded_view< iterator, OffsetSizeType > > opt_offset( std::size_t offset )
        {
                auto new_beg = this->begin() + offset;
                if ( new_beg + OffsetSizeType::min_val > this->end() ) {
                        return {};
                }
                auto new_end = new_beg + OffsetSizeType::max_val;
                if ( new_end <= this->end() ) {
                        return { bounded_view< iterator, OffsetSizeType >{ new_beg, new_end } };
                }
                return { bounded_view< iterator, OffsetSizeType >{ new_beg, this->end() } };
        }
};

}  // namespace emlabcpp
