#include "emlabcpp/static_vector.h"
#include "emlabcpp/types.h"

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#pragma once

namespace emlabcpp {

/// Either is heterogenous structure that holds one of the two types specified.
/// This is stored as union, so the memory requirement of either is always the size of the bigger
/// element + constant for identifying which on is contained.
///
/// The intervall value can't be directly accessed, unless both sides are same, in which case
/// join() / returns the internal value. (Which is safe if both are same) In case they are
/// different, you can / only access the value by either convert_left/right, bind_left/right or
/// match, which calls / appropiate callable based on whenever left or right item is present.
///
/// This prevents user of the code from errors in accessing proper side of the either.
template <typename LH, typename RH>
class either {
        union {
                LH left_;
                RH right_;
        };

        enum class item : uint8_t { LEFT = 0, RIGHT = 1 };

        item id_;

      public:
        using left_item  = LH;
        using right_item = RH;

        either(left_item &&item) noexcept : id_(item::LEFT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(std::move(item));
        }

        either(const left_item &item) noexcept : id_(item::LEFT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(item);
        }

        template <typename U = RH, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either(RH &&item) noexcept : id_(item::RIGHT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(std::move(item));
        }

        template <typename U = RH, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either(const RH &item) noexcept : id_(item::RIGHT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(item);
        }

        either(const either &other) noexcept : id_(other.id_) {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&left_) left_item(other.left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&right_) right_item(other.right_);
                }
        }

        either(either &&other) noexcept : id_(other.id_) {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&left_) left_item(std::move(other.left_));
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&right_) right_item(std::move(other.right_));
                }
        }

        either &operator=(const either &other) {
                if (other.id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = other.left_;
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = other.right_;
                }
                return *this;
        }

        either &operator=(either &&other) noexcept {
                if (other.id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = std::move(other.left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = std::move(other.right_);
                }
                return *this;
        }

        either &operator=(const left_item &other) {
                destruct();
                id_ = item::LEFT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(other);
                return *this;
        }
        either &operator=(left_item &&other) {
                destruct();
                id_ = item::LEFT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(std::move(other));
                return *this;
        }

        template <typename U = right_item, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either &operator=(const right_item &other) {
                destruct();
                id_ = item::RIGHT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(other);
                return *this;
        }

        template <typename U = right_item, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either &operator=(RH &&other) {
                destruct();
                id_ = item::RIGHT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(std::move(other));
                return *this;
        }

        [[nodiscard]] constexpr bool is_left() const { return id_ == item::LEFT; }

        template <typename LeftFunction>
        auto convert_left(LeftFunction &&left_f) & {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<decltype(left_f(left_)), right_item>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{left_f(left_)};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_};
        }

        template <typename LeftFunction>
        auto convert_left(LeftFunction &&left_f) && {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<decltype(left_f(std::move(left_))), right_item>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{left_f(std::move(left_))};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{std::move(right_)};
        }

        template <typename RightFunction>
        auto convert_right(RightFunction &&right_f) & {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<left_item, decltype(right_f(right_))>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{left_};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_f(right_)};
        }

        template <typename RightFunction>
        auto convert_right(RightFunction &&right_f) && {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<left_item, decltype(right_f(std::move(right_)))>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{std::move(left_)};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_f(std::move(right_))};
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) & {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_f(left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_f(right_);
                }
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) const & {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_f(left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_f(right_);
                }
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) && {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_f(std::move(left_));
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_f(std::move(right_));
                }
        }

        template <typename U = left_item, typename K = right_item>
        std::enable_if_t<std::is_same_v<U, K>, left_item> join() && {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return std::move(left_);
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return std::move(right_);
        }
        template <typename U = left_item, typename K = right_item>
        std::enable_if_t<std::is_same_v<U, K>, LH> join() & {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return left_;
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return right_;
        }

        template <typename UnaryFunction>
        auto bind_left(UnaryFunction &&left_f) & {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = decltype(left_f(left_));

                static_assert(std::is_same_v<typename return_either::right_item, right_item>,
                              "In bind left, the right_types has to be same!");

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return left_f(left_);
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_};
        }
        template <typename UnaryFunction>
        auto bind_left(UnaryFunction &&left_f) && {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = decltype(left_f(std::move(left_)));

                static_assert(std::is_same_v<typename return_either::right_item, right_item>,
                              "In bind left, the right_types has to be same!");

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return left_f(std::move(left_));
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{std::move(right_)};
        }

        ~either() { destruct(); }

      private:
        void destruct() {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_.~left_item();
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_.~right_item();
                }
        }
};

/// Marks empty assembly
struct empty_assembly_tag {};

/// Function gets a set of various std::optionals and either returns all their values assembled as
/// tuple or 'empty_assembly_tag' implicating that some of the optionals was empty.
template <typename... Ts>
inline either<std::tuple<Ts...>, empty_assembly_tag>
assemble_optionals(std::optional<Ts> &&... opt) {
        if ((... && opt)) {
                return std::make_tuple<Ts...>(std::forward<Ts>(*opt)...);
        }

        return empty_assembly_tag{};
}

/// Function expects eithers of any left type, but same right type.
/// These are acceessed and either tuple of _all_ left items is returned or vector of any of right
/// items.
///
/// This returns appropiated either< std::tuple< LeftItems... >, static_vector< Righitems, N
/// >>. This is handy for uses cases when you have expected values of multiple functions on the
/// left, / and their errors on the right. It either returns all values or the errors that happend.

template <typename FirstE, typename... Eithers>
inline auto assemble_left_collect_right(FirstE &&first, Eithers &&... others) {
        static_assert(are_same_v<typename Eithers::right_item...>,
                      "Right items of Eithers have to be same for collection!");

        using right_type        = typename FirstE::right_item;
        constexpr std::size_t N = 1 + sizeof...(Eithers);

        static_vector<right_type, N> collection{};

        auto convert = [&](auto either) {
                using eitherT   = decltype(either);
                using left_type = typename std::remove_reference_t<eitherT>::left_item;

                return std::move(either)
                    .convert_left([&](auto item) { //
                            return std::make_optional(std::move(item));
                    })
                    .convert_right([&](auto item) {
                            collection.emplace_back(std::move(item));
                            return std::optional<left_type>();
                    })
                    .join();
        };

        return assemble_optionals(convert(std::forward<FirstE>(first)),
                                  convert(std::forward<Eithers>(others))...)
            .convert_right([&](empty_assembly_tag) { //
                    return collection;
            });
}
} // namespace emlabcpp
