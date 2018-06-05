/*******************************<GINKGO LICENSE>******************************
Copyright 2017-2018

Karlsruhe Institute of Technology
Universitat Jaume I
University of Tennessee

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************<GINKGO LICENSE>*******************************/

#ifndef GKO_CORE_BASE_RANGE_ACCESSORS_HPP_
#define GKO_CORE_BASE_RANGE_ACCESSORS_HPP_


#include "core/base/dim.hpp"
#include "core/base/range.hpp"
#include "core/synthesizer/utils.hpp"


#include <array>


namespace gko {
namespace accessor {


// TODO: this accessor should be completely replaced by the strided accessor
//      (we can still keep a type alias called row_major for simplicity)
/**
 * A row_major accessor is a bridge between a range and the row-major memory
 * layout.
 *
 * You should never try to explicitly create an instance of this accessor.
 * Instead, supply it as a template parameter to a range, and pass the
 * constructor parameters for this class to the range (it will forward it to
 * this class).
 *
 * @warning The current implementation is incomplete, and only allows for
 *          2-dimensional ranges.
 *
 * @tparam ValueType  type of values this accessor returns
 * @tparam Dimensionality  number of dimensions of this accessor (has to be 2)
 */
template <typename ValueType, size_type Dimensionality>
class row_major {
public:
    friend class range<row_major>;

    static_assert(Dimensionality == 2,
                  "This accessor is only implemented for matrices");

    /**
     * Type of values returned by the accessor.
     */
    using value_type = ValueType;

    /**
     * Type of underlying data storage.
     */
    using data_type = value_type *;

    /**
     * Number of dimensions of the accessor.
     */
    static constexpr size_type dimensionality = 2;

protected:
    /**
     * Creates a row_major accessor.
     *
     * @param data  pointer to the block of memory containing the data
     * @param num_row  number of rows of the accessor
     * @param num_cols  number of columns of the accessor
     * @param stride  distance (in elements) between starting positions of
     *                consecutive rows (i.e. `data + i * stride` points to the
     *                `i`-th row)
     */
    GKO_ATTRIBUTES constexpr explicit row_major(data_type data,
                                                size_type num_rows,
                                                size_type num_cols,
                                                size_type stride)
        : data{data}, lengths{num_rows, num_cols}, stride{stride}
    {}

public:
    /**
     * Returns the data element at position (row, col)
     *
     * @param row  row index
     * @param col  column index
     *
     * @return data element at (row, col)
     */
    GKO_ATTRIBUTES constexpr value_type &operator()(size_type row,
                                                    size_type col) const
    {
        return GKO_ASSERT(row < lengths[0]), GKO_ASSERT(col < lengths[1]),
               data[row * stride + col];
    }

    /**
     * Returns the sub-range spanning the range (rows, cols)
     *
     * @param rows  row span
     * @param cols  column span
     *
     * @return sub-range spanning the range (rows, cols)
     */
    GKO_ATTRIBUTES constexpr range<row_major> operator()(const span &rows,
                                                         const span &cols) const
    {
        return GKO_ASSERT(rows.is_valid()), GKO_ASSERT(cols.is_valid()),
               GKO_ASSERT(rows <= span{lengths[0]}),
               GKO_ASSERT(cols <= span{lengths[1]}),
               range<row_major>(data + rows.begin * stride + cols.begin,
                                rows.end - rows.begin, cols.end - cols.begin,
                                stride);
    }

    /**
     * Returns the length in dimension `dimension`.
     *
     * @param dimension  a dimension index
     *
     * @return length in dimension `dimension`
     */
    GKO_ATTRIBUTES constexpr size_type length(size_type dimension) const
    {
        return dimension < 2 ? lengths[dimension] : 1;
    }

    /**
     * Copies data from another accessor
     *
     * @tparam OtherAccessor  type of the other accessor
     *
     * @param other  other accessor
     */
    template <typename OtherAccessor>
    GKO_ATTRIBUTES void copy_from(const OtherAccessor &other) const
    {
        for (size_type i = 0; i < lengths[0]; ++i) {
            for (size_type j = 0; j < lengths[1]; ++j) {
                (*this)(i, j) = other(i, j);
            }
        }
    }

    /**
     * Reference to the underlying data.
     */
    const data_type data;

    /**
     * An array of dimension sizes.
     */
    const std::array<const size_type, dimensionality> lengths;

    /**
     * Distance between consecutive rows.
     */
    const size_type stride;
};


// TODO: computing the indexes of a non strided accessor can be optimized for
// special cases. For example, assume a general 3D accessor which maps the
// index (i1, i2, i3) to i1 * stride1 + i2 * stride2 + i3 * stride3. If the
// second stride is known to be `zero`, the formula can be simplified to
// i1 * stride1 + i3 * stride3. Similarly, if the third stride is known to be a
// unit stride, then the formula can be further simplified to i1 * stride1 + i3.
enum class stride_type { zero, unit, nontrivial };


// TODO: this class should implement a general strided accessor which is a
// generalization of vector accessors, vector with increment accessors, row
// major and column major accessors.
//
// For instance, a (column) vector accessor can be obtained as strided<T, unit>.
// A row vector accessor (where valid values are (0, 0), (0, 1), ... (0, k)) can
// be obtained as strided<T, zero, unit>. A vector accessor with increment can
// be obtained as strided<T, nontrivial>.
//
// A row-major matrix accessor is obtained as strided<T, nontrivial, unit>.
// A column-major matrix accessor is obitained as strided<T, unit, nontrivial>.
//
// Note that this class is just a reference of what has to be done. The code
// compiles, but it is not verified if it can be instantiated with different
// template parameters, or if it works correctly.
template <typename ValueType, stride_type... StrideTypes>
class strided {
public:
    friend class range<strided>;

    using value_type = ValueType;

    using data_type = value_type *;

    static constexpr size_type dimensionality = sizeof...(StrideTypes);

    static constexpr size_type num_nontrivial_strides =
        get_num_nontrivial_strides(StrideTypes...);

protected:
    template <typename... Strides>
    GKO_ATTRIBUTES constexpr explicit strided(data_type data,
                                              dim<dimensionality> lengths,
                                              const Strides &... strides)
        : data{data}, lengths{lengths}, strides{strides...}
    {}

public:
    template <typename... DimensionTypes>
    GKO_ATTRIBUTES auto operator()(const DimensionTypes &... dimensions)
        -> void /* TODO: compute what should be the return type */
    {
        // TODO: this should return a value, or a subrange depending on the
        // passed in input parameters. It should do it efficiently, by taking
        // into account the trivial and non-trivial strides, and avoiding
        // unnecessary computations for strides that are known to be trivial.
        //
        // The implementation of this will require a lot of template
        // merafunctions that handle compile-time sequences of types / values
        // and create different functionality depending on those sequences.
        // Metafunctions that are not particularly related to accessors and
        // could be used in other contexts should go into the core/synthesizer/
        // directory.
    }

    // TODO: implement other accessor functions - see row_major accessor for
    // details

private:
    static constexpr size_type get_num_nontrivial_strides() { return 0u; }

    // gets the number of nontrivial stride types among all parameters passed to
    // the function
    template <typename... Rest>
    static constexpr size_type get_num_nontrivial_strides(
        const stride_type &first, const Rest &... rest)
    {
        return get_num_nontrivial_strides(rest...) +
               (first == stride_type::nontrivial);
    }


    const data_type data;
    const gko::dim<dimensionality> lengths;
    const gko::dim<num_nontrivial_strides> strides;
};


}  // namespace accessor
}  // namespace gko


#endif  // GKO_CORE_BASE_RANGE_ACCESSORS_HPP_
