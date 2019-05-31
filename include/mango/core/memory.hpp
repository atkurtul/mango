/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2017 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <memory>
#include <limits>
#include <algorithm>
#include "configure.hpp"
#include "object.hpp"

namespace mango {
namespace detail {

    // -----------------------------------------------------------------------
    // Memory
    // -----------------------------------------------------------------------

    template <typename P>
    struct Memory
    {
        P* address;
        size_t size;

        Memory()
            : address(nullptr)
            , size(0)
        {
        }

        Memory(P* address, size_t size)
            : address(address)
            , size(size)
        {
        }

        template <typename T>
        Memory(const Memory<T>& memory)
            : address(memory.address)
            , size(memory.size)
        {
        }

        operator P* () const
        {
            return address;
        }

        template <typename T>
        T* cast() const
        {
            return reinterpret_cast<T*>(address);
        }

        Memory slice(size_t slice_offset, size_t slice_size = 0) const
        {
            Memory memory(address + slice_offset, size - slice_offset);
            if (slice_size)
            {
                memory.size = std::min(memory.size, slice_size);
            }
            return memory;
        }
    };

} // namespace detail

    // -----------------------------------------------------------------------
    // memory
    // -----------------------------------------------------------------------

    using Memory = detail::Memory<u8>;
    using ConstMemory = detail::Memory<const u8>;

    class SharedMemory
    {
    private:
        Memory m_memory;
        std::shared_ptr<u8> m_ptr;

    public:
        SharedMemory(size_t size);
        SharedMemory(u8* address, size_t size);

        operator Memory () const
        {
            return m_memory;
        }
    };

    class VirtualMemory : private NonCopyable
    {
    protected:
        ConstMemory m_memory;

    public:
        VirtualMemory() = default;
        virtual ~VirtualMemory() {}

        const ConstMemory* operator -> () const
        {
            return &m_memory;
        }

        operator ConstMemory () const
        {
            return m_memory;
        }
    };

    // -----------------------------------------------------------------------
    // aligned malloc / free
    // -----------------------------------------------------------------------

    // NOTE: The alignment has to be a power-of-two and at least sizeof(void*)

    void* aligned_malloc(size_t size, size_t alignment = MANGO_DEFAULT_ALIGNMENT);
    void aligned_free(void* aligned);

    // -----------------------------------------------------------------------
    // aligned (std) memory allocator
    // -----------------------------------------------------------------------

    template <typename T, size_t ALIGNMENT>
    class AlignedAllocator
    {
    public:
        typedef T               value_type;
        typedef T*              pointer;
        typedef T&              reference;
        typedef const T*        const_pointer;
        typedef const T&        const_reference;
        typedef std::size_t     size_type;
        typedef std::ptrdiff_t  difference_type;

        AlignedAllocator()
        {
        }

        AlignedAllocator(const AlignedAllocator& allocator)
        {
            MANGO_UNREFERENCED_PARAMETER(allocator);
        }

        template <class U>
        AlignedAllocator(const AlignedAllocator<U, ALIGNMENT>& allocator)
        {
            MANGO_UNREFERENCED_PARAMETER(allocator);
        }

        ~AlignedAllocator()
        {
        }

        pointer address(reference x) const
        {
            return &x;
        }

        const_pointer address(const_reference x) const
        {
            return &x;
        }

        pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
        {
            MANGO_UNREFERENCED_PARAMETER(hint);
            void* s = aligned_malloc(n * sizeof(T), ALIGNMENT);
            return reinterpret_cast<pointer>(s);
        }

        void deallocate(pointer p, size_type n)
        {
            MANGO_UNREFERENCED_PARAMETER(n);
            aligned_free(p);
        }

        void construct(pointer p, const_reference value)
        {
            new(p) T(value);
        }

        void destroy(pointer p)
        {
            p->~T();
        }

        template <class U, class... Args>
        void construct(U* p, Args&&... args)
        {
            new(p) T(args...);
        }

        template <class U>
        void destroy (U* p)
        {
            p->~T();
        }

        size_type max_size() const
        {
            return std::numeric_limits<size_type>::max() / sizeof(T);
        }

        template <typename U>
        struct rebind
        {
            typedef AlignedAllocator<U, ALIGNMENT> other;
        };

        bool operator == (AlignedAllocator const& allocator)
        {
            MANGO_UNREFERENCED_PARAMETER(allocator);
            return true;
        }

        bool operator != (AlignedAllocator const& allocator)
        {
            return !operator == (allocator);
        }
    };

} // namespace mango
