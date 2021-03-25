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
                new (&left_) left_item(std::move(item));
        }

        either(const left_item &item) noexcept : id_(item::LEFT) { new (&left_) left_item(item); }

        template <typename U = RH, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either(RH &&item) noexcept : id_(item::RIGHT) {
                new (&right_) right_item(std::move(item));
        }

        template <typename U = RH, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either(const RH &item) noexcept : id_(item::RIGHT) {
                new (&right_) right_item(item);
        }

        either(const either &other) noexcept : id_(other.id_) {
                if (id_ == item::LEFT) {
                        new (&left_) left_item(other.left_);
                } else {
                        new (&right_) right_item(other.right_);
                }
        }

        either(either &&other) noexcept : id_(other.id_) {
                if (id_ == item::LEFT) {
                        new (&left_) left_item(std::move(other.left_));
                } else {
                        new (&right_) right_item(std::move(other.right_));
                }
        }

        either &operator=(const either &other) {
                if (this == &other) {
                        return *this;
                }
                if (other.id_ == item::LEFT) {
                        *this = other.left_;
                } else {
                        *this = other.right_;
                }
                return *this;
        }

        either &operator=(either &&other) noexcept {
                if (other.id_ == item::LEFT) {
                        *this = std::move(other.left_);
                } else {
                        *this = std::move(other.right_);
                }
                return *this;
        }

        either &operator=(const left_item &other) {
                destruct();
                id_ = item::LEFT;
                new (&left_) left_item(other);
                return *this;
        }
        either &operator=(left_item &&other) {
                destruct();
                id_ = item::LEFT;
                new (&left_) left_item(std::move(other));
                return *this;
        }

        template <typename U = right_item, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either &operator=(const right_item &other) {
                destruct();
                id_ = item::RIGHT;
                new (&right_) right_item(other);
                return *this;
        }

        template <typename U = right_item, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either &operator=(RH &&other) {
                destruct();
                id_ = item::RIGHT;
                new (&right_) right_item(std::move(other));
                return *this;
        }

        [[nodiscard]] constexpr bool is_left() const { return id_ == item::LEFT; }

        template <typename LeftFunction>
        auto convert_left(LeftFunction &&left_f) & {
                using return_either = either<decltype(left_f(left_)), right_item>;

                if (id_ == item::LEFT) {
                        return return_either{left_f(left_)};
                }

                return return_either{right_};
        }

        template <typename LeftFunction>
        auto convert_left(LeftFunction &&left_f) && {
                using return_either = either<decltype(left_f(std::move(left_))), right_item>;

                if (id_ == item::LEFT) {
                        return return_either{left_f(std::move(left_))};
                }

                return return_either{std::move(right_)};
        }

        template <typename RightFunction>
        auto convert_right(RightFunction &&right_f) & {
                using return_either = either<left_item, decltype(right_f(right_))>;

                if (id_ == item::LEFT) {
                        return return_either{left_};
                }

                return return_either{right_f(right_)};
        }

        template <typename RightFunction>
        auto convert_right(RightFunction &&right_f) && {
                using return_either = either<left_item, decltype(right_f(std::move(right_)))>;

                if (id_ == item::LEFT) {
                        return return_either{std::move(left_)};
                }

                return return_either{right_f(std::move(right_))};
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) & {
                if (id_ == item::LEFT) {
                        left_f(left_);
                } else {
                        right_f(right_);
                }
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) const & {
                if (id_ == item::LEFT) {
                        left_f(left_);
                } else {
                        right_f(right_);
                }
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) && {
                if (id_ == item::LEFT) {
                        left_f(std::move(left_));
                } else {
                        right_f(std::move(right_));
                }
        }

        template <typename U = left_item, typename K = right_item>
        std::enable_if_t<std::is_same_v<U, K>, left_item> join() && {
                if (id_ == item::LEFT) {
                        return std::move(left_);
                }

                return std::move(right_);
        }
        template <typename U = left_item, typename K = right_item>
        std::enable_if_t<std::is_same_v<U, K>, LH> join() & {
                if (id_ == item::LEFT) {
                        return left_;
                }

                return right_;
        }

        template <typename T>
        either<left_item, T> construct_right() & {
                if (id_ == item::LEFT) {
                        return {left_};
                }
                return {T{right_}};
        }

        template <typename T>
        either<left_item, T> construct_right() && {
                if (id_ == item::LEFT) {
                        return {std::move(left_)};
                }
                return {T{std::move(right_)}};
        }

        template <typename UnaryFunction>
        auto bind_left(UnaryFunction &&left_f) & {
                using return_either =
                    either<typename decltype(left_f(left_))::left_item, right_item>;

                if (id_ == item::LEFT) {
                        return left_f(left_).template construct_right<right_item>();
                }

                return return_either{right_};
        }
        template <typename UnaryFunction>
        auto bind_left(UnaryFunction &&left_f) && {
                using return_either =
                    either<typename decltype(left_f(std::move(left_)))::left_item, right_item>;

                if (id_ == item::LEFT) {
                        return left_f(std::move(left_)).template construct_right<right_item>();
                }

                return return_either{std::move(right_)};
        }

        template <typename T>
        either<T, right_item> construct_left() & {
                if (id_ != item::LEFT) {
                        return {right_};
                }
                return {T{left_}};
        }

        template <typename T>
        either<T, right_item> construct_left() && {
                if (id_ != item::LEFT) {
                        return {std::move(right_)};
                }
                return {T{std::move(left_)}};
        }

        template <typename UnaryFunction>
        auto bind_right(UnaryFunction &&right_f) & {
                using return_either =
                    either<left_item, typename decltype(right_f(right_))::right_item>;

                if (id_ == item::RIGHT) {
                        return right_f(right_).template construct_left<left_item>();
                }

                return return_either{left_};
        }
        template <typename UnaryFunction>
        auto bind_right(UnaryFunction &&right_f) && {
                using return_either =
                    either<left_item, typename decltype(right_f(std::move(right_)))::right_item>;

                if (id_ == item::RIGHT) {
                        return right_f(std::move(right_)).template construct_left<left_item>();
                }

                return return_either{std::move(left_)};
        }

        ~either() { destruct(); }

      private:
        void destruct() {
                if (id_ == item::LEFT) {
                        left_.~left_item();
                } else {
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
assemble_optionals(std::optional<Ts> &&...opt) {
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
template <typename FirstE, typename... Eithers>
inline auto assemble_left_collect_right(FirstE &&first, Eithers &&...others) {
        static_assert(are_same_v<typename std::decay_t<Eithers>::right_item...>,
                      "Right items of Eithers have to be same for collection!");

        using right_type               = typename std::decay_t<FirstE>::right_item;
        constexpr std::size_t either_n = 1 + sizeof...(Eithers);

        static_vector<right_type, either_n> collection{};

        auto convert = [&](auto either) {
                using either_t  = decltype(either);
                using left_type = typename std::remove_reference_t<either_t>::left_item;

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
#pragma GCC diagnostic pop
} // namespace emlabcpp
