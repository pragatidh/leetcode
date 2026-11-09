#include <algorithm>
#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

template <typename B, typename F>
[[gnu::always_inline]] constexpr void pbs_for_each_layer(F&& f) noexcept
{
    [&]<size_t... layer> [[gnu::always_inline]] (
        std::index_sequence<layer...>) noexcept
    {
        (f(std::integral_constant<size_t, layer>{}), ...);
    }(std::make_index_sequence<B::kNumLayers>());
}

template <typename B, typename F>
[[gnu::always_inline]] constexpr void pbs_for_each_layer_reversed(
    F&& f) noexcept
{
    pbs_for_each_layer<B>(
        [&] [[gnu::always_inline]] (auto layer) noexcept
        { f(std::integral_constant<size_t, B::kNumLayers - (layer + 1)>{}); });
}

template <std::integral T>
[[nodiscard, gnu::always_inline]] static constexpr T ceil_div(
    T a,
    std::type_identity_t<T> b) noexcept
{
    return (a + (b - 1)) / b;
}

template <u64 value>
using UintForValue = std::conditional_t < value < (1 << 8),
      u8,
      std::conditional_t <
          value<
              (1 << 16),
              u16,
              std::conditional_t<value<(1UL << 32), u32, u64>>>;

template <size_t capacity, typename Word = u64>
struct PyramidBitset
{
    static_assert(capacity > 0);
    using WordType = std::remove_cv_t<Word>;
    static constexpr size_t kCapacity = capacity;
    using ValueType = UintForValue<capacity - 1>;
    static constexpr size_t kWordSize = sizeof(Word) * 8;
    static constexpr u8 kMask = kWordSize - 1;
    static constexpr int kShift = std::bit_width(kMask);
    static constexpr size_t kNumLayers = []
    {
        size_t layers = 1;
        for (size_t n = capacity; n > kWordSize; n = ceil_div(n, kWordSize))
        {
            ++layers;
        }
        return layers;
    }();

private:
    struct Layout
    {
        size_t sizes[kNumLayers];
        size_t offsets[kNumLayers];
        size_t total;
    };

    static constexpr Layout kLayout = []
    {
        Layout layout{};
        for (size_t i = 0, x = capacity; i != kNumLayers; ++i)
        {
            x = ceil_div(x, kWordSize);
            layout.sizes[i] = x;
            layout.offsets[i] = layout.total;
            layout.total += x;
        }
        return layout;
    }();

public:
    static constexpr auto& kLayersSizes = kLayout.sizes;
    static constexpr auto& offsets = kLayout.offsets;
    static constexpr size_t kTotalWords = kLayout.total;
    Word* words;
};

template <size_t capacity, typename Word>
[[gnu::always_inline]] constexpr void pbs_add(
    const PyramidBitset<capacity, Word>& bits,
    typename PyramidBitset<capacity, Word>::ValueType v) noexcept
{
    using B = PyramidBitset<capacity, Word>;
    pbs_for_each_layer<B>(
        [&]<size_t layer> [[gnu::always_inline]] (
            std::integral_constant<size_t, layer>) noexcept
        {
            typename B::ValueType wi = v >> B::kShift;
            bits.words[B::offsets[layer] + wi] |= Word{1} << (v & B::kMask);
            v = wi;
        });
}

struct EmptyType
{
};

inline static constexpr EmptyType kEmpty{};

struct FullType
{
};

inline static constexpr FullType kFull{};

template <bool value, size_t capacity, typename Word>
[[gnu::always_inline]] constexpr void pbs_initialize(
    const PyramidBitset<capacity, Word>& bits,
    size_t end = capacity) noexcept
{
    using B = PyramidBitset<capacity, Word>;
    assert(end <= capacity);
    if (end == 0)
    {
        bits.words[B::offsets[B::kNumLayers - 1]] = 0;
        return;
    }
    constexpr auto m = value ? ~Word{} : Word{};
    pbs_for_each_layer<B>(
        [&]<size_t layer> [[gnu::always_inline]] (
            std::integral_constant<size_t, layer>) noexcept
        {
            const auto count = end;
            end = ceil_div<size_t>(end, B::kWordSize);
            std::fill_n(bits.words + B::offsets[layer], end, m);
            if constexpr (value)
            {
                bits.words[B::offsets[layer] + end - 1] &=
                    std::numeric_limits<Word>::max() >>
                    ((size_t{0} - count) & B::kMask);
            }
        });
}

template <size_t capacity, typename Word>
[[gnu::always_inline]] constexpr void pbs_initialize(
    const PyramidBitset<capacity, Word>& bits,
    FullType,
    size_t end = capacity) noexcept
{
    pbs_initialize<true>(bits, end);
}

template <size_t capacity, typename Word>
[[gnu::always_inline]] constexpr void pbs_initialize(
    const PyramidBitset<capacity, Word>& bits,
    EmptyType,
    size_t end = capacity) noexcept
{
    pbs_initialize<false>(bits, end);
}

// Buffer holds kTotalWords; construction preserves its contents.
template <size_t capacity, typename Word>
[[nodiscard, gnu::always_inline]] constexpr PyramidBitset<capacity, Word>
pbs_make(Word* buffer) noexcept
{
    return {buffer};
}

// x must be nonzero.
template <std::unsigned_integral T>
    requires(sizeof(T) <= sizeof(u64))
[[nodiscard, gnu::always_inline]] constexpr u8 ctz(T x) noexcept
{
    if constexpr (sizeof(T) <= sizeof(u32))
    {
        return __builtin_ctz(x) & 31;
    }
    else
    {
        return __builtin_ctzll(x) & 63;
    }
}

// Inclusive successor; returns capacity when absent.
// One top-down pass takes O(log_word_size(capacity)).
template <size_t capacity, typename Word>
[[nodiscard, gnu::always_inline]] constexpr UintForValue<capacity>
pbs_next_inclusive(
    const PyramidBitset<capacity, Word>& bits,
    typename PyramidBitset<capacity, Word>::ValueType position) noexcept
{
    using B = PyramidBitset<capacity, Word>;
    using Index = UintForValue<capacity>;
    constexpr Index index_mask = std::numeric_limits<Index>::max();
    assert(position < capacity);
    typename B::WordType first_word = bits.words[position >> B::kShift];
    first_word >>= position & B::kMask;
    if (first_word != 0)
    {
        Index result = (Index{position} + ctz(first_word)) & index_mask;
        if (result > capacity) result = capacity;
        return result;
    }
    Index result = capacity;
    pbs_for_each_layer_reversed<B>(
        [&]<size_t layer> [[gnu::always_inline]] (
            std::integral_constant<size_t, layer>) noexcept
        {
            constexpr Index offset = B::offsets[layer];
            constexpr Index layer_size = B::kLayersSizes[layer];
            if constexpr (layer + 1 < B::kNumLayers)
            {
                if (result < layer_size)
                {
                    const Word word = bits.words[offset + result];
                    u8 first = B::kWordSize;
                    if (word != 0) first = ctz(word);
                    result = ((result << B::kShift) + first) & index_mask;
                }
                else
                {
                    result = capacity;
                }
            }

            // A neighbour on the query's path is closer than the fallback.
            const Index query = position >> (layer * B::kShift);
            const Index wi = query >> B::kShift;
            const u8 bi = query & B::kMask;
            typename B::WordType candidates = bits.words[offset + wi];
            // Separate shifts avoid shifting by the word width.
            constexpr u8 skip = layer != 0;
            candidates >>= bi;
            candidates >>= skip;
            u8 first = B::kWordSize;
            if (candidates != 0) first = ctz(candidates);
            const Index candidate = (query + skip + first) & index_mask;
            const Index mask = (Index{0} - Index{candidates != 0}) & index_mask;
            result ^= (result ^ candidate) & mask;
        });
    if (result > capacity) result = capacity;
    return result;
}

template <size_t capacity, typename Word>
[[gnu::always_inline]] constexpr void pbs_remove(
    const PyramidBitset<capacity, Word>& bits,
    typename PyramidBitset<capacity, Word>::ValueType v) noexcept
{
    using B = PyramidBitset<capacity, Word>;
    bool prev_empty = true;
    pbs_for_each_layer<B>(
        [&]<size_t layer> [[gnu::always_inline]] (
            std::integral_constant<size_t, layer>) noexcept
        {
            typename B::ValueType wi = v >> B::kShift;
            auto& w = bits.words[B::offsets[layer] + wi];
            w &= ~(Word{prev_empty} << (v & B::kMask));
            v = wi;
            prev_empty = !w;
        });
}
#ifndef LC_LOCAL_BUILD
auto init = []()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);
    return 'c';
}();
#endif

class Solution
{
    static constexpr u32 kCapacity = 100'000;

    inline static u64 free_words[PyramidBitset<kCapacity>::kTotalWords];

    class BusyHeap
    {
        struct Entry
        {
            u32 time, server;
        };
        alignas(64) inline static Entry data[kCapacity + 1];

    public:
        u32 size = 0;

        [[nodiscard, gnu::always_inline]] u32 time() const noexcept
        {
            return data[1].time;
        }

        [[gnu::always_inline]] void push(u32 time, u32 server) noexcept
        {
            const Entry value{time, server};
            u32 i = ++size;
            while (value.time < data[i >> 1].time)
            {
                data[i] = data[i >> 1];
                i >>= 1;
            }
            data[i] = value;
        }

        [[nodiscard, gnu::always_inline]] u32 pop() noexcept
        {
            const u32 server = data[1].server;
            const Entry value = data[size--];
            u32 i = 1;
            for (u32 child = 2; child <= size; child = i << 1)
            {
                child +=
                    child < size && data[child + 1].time < data[child].time;
                if (value.time <= data[child].time) break;
                data[i] = data[child];
                i = child;
            }
            data[i] = value;
            return server;
        }
    };

    alignas(64) inline static u32 cnt[kCapacity];

public:
    template <u32 max_k>
    [[gnu::no_sanitize_address, gnu::flatten]] static void
    impl(u32 k, std::vector<u32>& arrivals, std::vector<u32>& loads)
    {
        const u32 num_requests = arrivals.size() & ~u32{};
        u32* arrival_data = arrivals.data();
        if (k == 1)
        {
            arrival_data[0] = 0;
            arrivals.resize(1);
            return;
        }
        if (num_requests <= k)
        {
            for (u32 i = 0; i != num_requests; ++i) arrival_data[i] = i;
            return;
        }

        using Index = typename PyramidBitset<max_k>::ValueType;
        constexpr u32 index_mask = std::numeric_limits<Index>::max();
        const auto is_free = pbs_make<max_k>(free_words);
        BusyHeap busy;
        u32 start = k - 1;
        const u32 horizon = arrival_data[num_requests - 1];
        const u32* load_data = loads.data();
        pbs_initialize(is_free, kEmpty, k);

        u32 max_cnt = 1;
        u32 num_max_cnt = k;
        u32 num_free = 0;

        for (u32 i = 0; i != k; ++i)
        {
            cnt[i] = 1;
            const u32 finish = arrival_data[i] + load_data[i];
            if (finish <= arrival_data[k])
            {
                pbs_add(is_free, i & index_mask);
                ++num_free;
            }
            else if (finish <= horizon)
            {
                busy.push(finish, i);
            }
        }

        for (u32 request_index = k; request_index != num_requests;
             ++request_index)
        {
            ++start;
            start &= -u32{start != k};
            u32 arrival = arrival_data[request_index];

            while (busy.size != 0 && busy.time() <= arrival)
            {
                pbs_add(is_free, busy.pop() & index_mask);
                ++num_free;
            }

            if (num_free == 0)
            {
                if (busy.size == 0) break;
                continue;
            }

            u32 srv = pbs_next_inclusive(is_free, start & index_mask);
            if (srv >= k) srv = pbs_next_inclusive(is_free, 0u);
            pbs_remove(is_free, srv & index_mask);
            --num_free;
            u32 new_cnt = ++cnt[srv];
            num_max_cnt &= -u32{new_cnt <= max_cnt};
            max_cnt = std::max(new_cnt, max_cnt);
            num_max_cnt += new_cnt == max_cnt;

            u32 next_time_available = arrival + load_data[request_index];
            if (next_time_available <= horizon)
            {
                busy.push(next_time_available, srv);
            }
        }

        arrivals.clear();
        arrivals.resize(num_max_cnt + 1);
        u32* out_data = arrivals.data();
        u32 out = 0;
        for (u32 i = 0; i != k; ++i)
        {
            out_data[out] = i;
            out += cnt[i] == max_cnt;
        }
        arrivals.pop_back();
    }

    std::vector<int>
    busiestServers(u32 k, std::vector<int>& arrival, std::vector<int>& load)
    {
        auto& arrivals = reinterpret_cast<std::vector<u32>&>(arrival);
        auto& loads = reinterpret_cast<std::vector<u32>&>(load);
        if (k <= 64)
        {
            impl<64>(k, arrivals, loads);
        }
        else if (k <= 4096)
        {
            impl<4096>(k, arrivals, loads);
        }
        else
        {
            impl<kCapacity>(k, arrivals, loads);
        }
        return std::move(arrival);
    }
};