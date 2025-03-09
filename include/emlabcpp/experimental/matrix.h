///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include "emlabcpp/algorithm.h"
#include "emlabcpp/range.h"

namespace emlabcpp
{

template < std::size_t N, std::size_t M, typename T = float >
class matrix
{
public:
        using value_type                  = T;
        static constexpr std::size_t rows = N;
        static constexpr std::size_t cols = M;
        using row_type                    = std::array< value_type, M >;

        constexpr matrix()
        {
                for ( row_type& row : data_ )
                        for ( value_type& val : row )
                                val = 0.f;
        };

        constexpr matrix( std::array< row_type, N > data )
          : data_( data )
        {
        }

        [[nodiscard]] constexpr auto begin() const
        {
                return data_.begin();
        }

        [[nodiscard]] constexpr auto end() const
        {
                return data_.end();
        }

        constexpr const row_type& operator[]( std::size_t i ) const
        {
                return data_[i];
        }

        constexpr row_type& operator[]( std::size_t i )
        {
                return data_[i];
        }

        constexpr bool operator==( const matrix& other ) const = default;

private:
        std::array< row_type, N > data_;
};

template < typename Matrix >
class transposed_matrix
{
        template < typename Matrix2 >
        class stub
        {
        public:
                stub( std::size_t i, Matrix2& m )
                  : i_( i )
                  , m_( m )
                {
                }

                constexpr const auto& operator[]( std::size_t j ) const
                {
                        return m_[j][i_];
                }

                constexpr auto& operator[]( std::size_t j )
                {
                        return m_[j][i_];
                }

        private:
                std::size_t i_;
                Matrix2&    m_;
        };

public:
        using value_type                  = typename Matrix::value_type;
        static constexpr std::size_t rows = Matrix::cols;
        static constexpr std::size_t cols = Matrix::rows;

        constexpr transposed_matrix( Matrix& m )
          : m_( m )
        {
        }

        constexpr stub< const Matrix > operator[]( std::size_t i ) const
        {
                return { i, m_ };
        }

        constexpr stub< Matrix > operator[]( std::size_t i )
        {
                return { i, m_ };
        }

        operator matrix< rows, cols, value_type >()
        {
                matrix< rows, cols, value_type > res;
                for ( const std::size_t i : range( rows ) )
                        for ( const std::size_t j : range( cols ) )
                                res[i][j] = m_[j][i];
                return res;
        }

        constexpr bool operator==( const transposed_matrix& other ) const = default;

private:
        Matrix& m_;
};

template < std::size_t N, typename T = float >
class identity_matrix
{
        class stub
        {
        public:
                stub( std::size_t i )
                  : i_( i )
                {
                }

                constexpr T operator[]( std::size_t j ) const
                {
                        return i_ == j ? 1 : 0;
                }

        private:
                std::size_t i_;
        };

public:
        using value_type                  = T;
        static constexpr std::size_t rows = N;
        static constexpr std::size_t cols = N;

        constexpr identity_matrix() = default;

        constexpr stub operator[]( std::size_t i ) const
        {
                return { i };
        }

        operator matrix< rows, cols, value_type >()
        {
                matrix< rows, cols, value_type > res;
                for ( const std::size_t i : range( rows ) )
                        for ( const std::size_t j : range( cols ) )
                                res[i][j] = i == j ? 1 : 0;
                return res;
        }
};

template < typename M >
concept matrix_like = requires( M m, std::size_t i, std::size_t j ) {
        { M::rows } -> std::convertible_to< std::size_t >;
        { M::cols } -> std::convertible_to< std::size_t >;
        { m[i][j] } -> std::convertible_to< typename M::value_type >;
};

template < matrix_like Matrix, std::size_t I, std::size_t J >
class rowcol_submatrix
{
        template < typename Matrix2 >
        class stub
        {
        public:
                stub( std::size_t i, Matrix2& m )
                  : i_( i )
                  , m_( m )
                {
                }

                constexpr const auto& operator[]( std::size_t j ) const
                {
                        return m_[i_][j < J ? j : j + 1];
                }

                constexpr auto& operator[]( std::size_t j )
                {
                        return m_[i_][j < J ? j : j + 1];
                }

        private:
                std::size_t i_;
                Matrix2&    m_;
        };

public:
        using value_type                  = typename Matrix::value_type;
        static constexpr std::size_t rows = Matrix::rows - 1;
        static constexpr std::size_t cols = Matrix::cols - 1;

        constexpr rowcol_submatrix( Matrix& m )
          : m_( m )
        {
        }

        constexpr stub< const Matrix > operator[]( std::size_t i ) const
        {
                return { i < I ? i : i + 1, m_ };
        }

        constexpr stub< Matrix > operator[]( std::size_t i )
        {
                return { i < I ? i : i + 1, m_ };
        }

private:
        Matrix& m_;
};

#ifdef EMLABCPP_USE_OSTREAM
template < matrix_like Matrix >
std::ostream& operator<<( std::ostream& os, const Matrix& m )
{
        for ( const std::size_t i : range( Matrix::rows ) ) {
                for ( const std::size_t j : range( Matrix::cols ) )
                        os << m[i][j] << '\t';
                os << '\n';
        }
        return os;
}
#endif

template < matrix_like LH, matrix_like RH >
requires( LH::rows == RH::rows && LH::cols == RH::cols )
constexpr auto operator==( const LH& lh, const RH& rh )
{
        for ( const std::size_t i : range( LH::rows ) ) {
                for ( const std::size_t j : range( LH::cols ) )
                        if ( lh[i][j] != rh[i][j] )
                                return false;
        }
        return true;
}

template < matrix_like LH, matrix_like RH, typename T = typename LH::value_type >
requires( LH::cols == RH::rows )
constexpr matrix< LH::rows, RH::cols, T > operator*( const LH& lh, const RH& rh )
{
        matrix< LH::rows, RH::cols, T > res;

        for ( const std::size_t i : range( LH::rows ) ) {
                for ( const std::size_t j : range( RH::cols ) ) {
                        T v{};
                        for ( const std::size_t k : range( LH::cols ) )
                                v += lh[i][k] * rh[k][j];
                        res[i][j] = v;
                }
        }

        return res;
}

template < matrix_like LH >
constexpr matrix< LH::rows, LH::cols, typename LH::value_type >
operator*( const LH& lh, const typename LH::value_type& val )
{
        auto res = lh;
        for ( const std::size_t i : range( LH::rows ) )
                for ( const std::size_t j : range( LH::cols ) )
                        res[i][j] *= val;

        return res;
}

template < matrix_like LH >
constexpr matrix< LH::rows, LH::cols, typename LH::value_type >
operator*( const typename LH::value_type& val, const LH& lh )
{
        return lh * val;
}

template < matrix_like LH, matrix_like RH, typename T = typename LH::value_type >
requires( LH::cols == RH::cols && LH::rows == RH::rows )
constexpr matrix< LH::rows, LH::cols, T > operator+( const LH& lh, const RH& rh )
{
        matrix< LH::rows, LH::cols, T > res{};
        for ( const std::size_t i : range( LH::rows ) )
                for ( const std::size_t j : range( LH::cols ) )
                        res[i][j] = lh[i][j] + rh[i][j];
        return res;
}

template < matrix_like LH, matrix_like RH, typename T = typename LH::value_type >
requires( LH::cols == RH::cols && LH::rows == RH::rows )
constexpr matrix< LH::rows, LH::cols, T > operator-( const LH& lh, const RH& rh )
{
        matrix< LH::rows, LH::cols, T > res{};
        for ( const std::size_t i : range( LH::rows ) )
                for ( const std::size_t j : range( LH::cols ) )
                        res[i][j] = lh[i][j] - rh[i][j];
        return res;
}

template < matrix_like M >
constexpr transposed_matrix< M > transpose( M& m )
{
        return { m };
}

template < matrix_like M >
constexpr matrix< M::cols, M::rows, typename M::value_type > transpose( M&& m )
{
        return transpose( m );
};

template < matrix_like M >
requires( M::rows == 2 && M::cols == 2 )
constexpr auto determinant( const M& m )
{
        return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

template < matrix_like M >
requires( M::rows > 2 && M::cols == M::rows )
constexpr auto determinant( const M& m )
{
        // TODO: tests!
        constexpr std::size_t N   = M::rows;
        float                 res = 0.f;
        for_each_index< N >( [&]< std::size_t i > {
                const rowcol_submatrix< const M, i, 0 > submatrix{ m };
                res += ( i % 2 == 0 ? 1 : -1 ) * m[i][0] * determinant( submatrix );
        } );
        return res;
}

template < matrix_like M >
requires( M::rows == 1 && M::cols == 1 )
constexpr matrix< M::rows, M::cols, typename M::value_type > inverse( const M& m )
{
        matrix< M::rows, M::cols, typename M::value_type > res;
        res[0][0] = 1.f / m[0][0];
        return res;
}

template < matrix_like M >
requires( M::rows == 2 && M::cols == 2 )
constexpr matrix< M::rows, M::cols, typename M::value_type > inverse( const M& m )
{
        auto v = 1.f / determinant( m );

        matrix< M::rows, M::cols, typename M::value_type > res;
        res[0] = { m[1][1], -m[0][1] };
        res[1] = { -m[1][0], m[0][0] };
        return v * res;
}

}  // namespace emlabcpp
