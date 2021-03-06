// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
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
        requires( n <= min )
            [[nodiscard]] bounded_view< iterator, bounded< std::size_t, n, n > > first() const
        {
                return { this->begin(), this->begin() + n };
        }

        template < std::size_t n >
        requires( n <= min ) [[nodiscard]] bounded_view<
            iterator,
            bounded< std::size_t, min - n, max - n > > offset() const
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
