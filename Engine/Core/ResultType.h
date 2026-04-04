// ResultType.h — Rust-inspired Result<T, E> type for error propagation
// Copyright (c) 2026 ExplorerLens Project
//
// Provides Result<T, E> with Ok/Err factories, Map/MapErr/AndThen chaining,
// and TRY_RESULT macro for early-return on error. Zero-overhead when successful.
//
#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include "StructuredErrorDomain.h"

namespace ExplorerLens {
namespace Engine {

/// Tag types for explicit construction
struct OkTag
{};
struct ErrTag
{};

/// Rust-inspired Result<T, E> — holds either a success value or an error.
/// Default error type is StructuredError.
/// No exceptions are thrown; callers must check IsOk()/IsErr() before accessing.
template <typename T, typename E = StructuredError>
class Result
{
  public:
    /// Construct a successful result
    Result(OkTag, const T& value) : m_hasValue(true)
    {
        new (&m_storage.value) T(value);
    }

    Result(OkTag, T&& value) : m_hasValue(true)
    {
        new (&m_storage.value) T(std::move(value));
    }

    /// Construct an error result
    Result(ErrTag, const E& error) : m_hasValue(false)
    {
        new (&m_storage.error) E(error);
    }

    Result(ErrTag, E&& error) : m_hasValue(false)
    {
        new (&m_storage.error) E(std::move(error));
    }

    /// Copy constructor
    Result(const Result& other) : m_hasValue(other.m_hasValue)
    {
        if (m_hasValue)
            new (&m_storage.value) T(other.m_storage.value);
        else
            new (&m_storage.error) E(other.m_storage.error);
    }

    /// Move constructor
    Result(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<E>)
        : m_hasValue(other.m_hasValue)
    {
        if (m_hasValue)
            new (&m_storage.value) T(std::move(other.m_storage.value));
        else
            new (&m_storage.error) E(std::move(other.m_storage.error));
    }

    /// Copy assignment
    Result& operator=(const Result& other)
    {
        if (this != &other) {
            Destroy();
            m_hasValue = other.m_hasValue;
            if (m_hasValue)
                new (&m_storage.value) T(other.m_storage.value);
            else
                new (&m_storage.error) E(other.m_storage.error);
        }
        return *this;
    }

    /// Move assignment
    Result& operator=(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>
                                               && std::is_nothrow_move_constructible_v<E>)
    {
        if (this != &other) {
            Destroy();
            m_hasValue = other.m_hasValue;
            if (m_hasValue)
                new (&m_storage.value) T(std::move(other.m_storage.value));
            else
                new (&m_storage.error) E(std::move(other.m_storage.error));
        }
        return *this;
    }

    ~Result()
    {
        Destroy();
    }

    /// True if the result holds a success value
    bool IsOk() const noexcept
    {
        return m_hasValue;
    }

    /// True if the result holds an error
    bool IsErr() const noexcept
    {
        return !m_hasValue;
    }

    /// Access the success value (undefined behavior if IsErr())
    const T& Value() const& noexcept
    {
        return m_storage.value;
    }
    T& Value() & noexcept
    {
        return m_storage.value;
    }
    T&& Value() && noexcept
    {
        return std::move(m_storage.value);
    }

    /// Access the error (undefined behavior if IsOk())
    const E& Error() const& noexcept
    {
        return m_storage.error;
    }
    E& Error() & noexcept
    {
        return m_storage.error;
    }
    E&& Error() && noexcept
    {
        return std::move(m_storage.error);
    }

    /// Return the value if Ok, or the provided default if Err
    T ValueOr(const T& defaultValue) const&
    {
        return m_hasValue ? m_storage.value : defaultValue;
    }

    T ValueOr(T&& defaultValue) &&
    {
        return m_hasValue ? std::move(m_storage.value) : std::move(defaultValue);
    }

    /// Map: transform the success value with a function, pass error through
    template <typename Fn>
    auto Map(Fn&& fn) const -> Result<decltype(fn(std::declval<const T&>())), E>
    {
        using U = decltype(fn(std::declval<const T&>()));
        if (m_hasValue) {
            return Result<U, E>(OkTag{}, fn(m_storage.value));
        }
        return Result<U, E>(ErrTag{}, m_storage.error);
    }

    /// MapErr: transform the error with a function, pass value through
    template <typename Fn>
    auto MapErr(Fn&& fn) const -> Result<T, decltype(fn(std::declval<const E&>()))>
    {
        using F = decltype(fn(std::declval<const E&>()));
        if (m_hasValue) {
            return Result<T, F>(OkTag{}, m_storage.value);
        }
        return Result<T, F>(ErrTag{}, fn(m_storage.error));
    }

    /// AndThen: chain another Result-returning operation (flatMap)
    template <typename Fn>
    auto AndThen(Fn&& fn) const -> decltype(fn(std::declval<const T&>()))
    {
        using ResultType = decltype(fn(std::declval<const T&>()));
        if (m_hasValue) {
            return fn(m_storage.value);
        }
        return ResultType(ErrTag{}, m_storage.error);
    }

  private:
    void Destroy()
    {
        if (m_hasValue)
            m_storage.value.~T();
        else
            m_storage.error.~E();
    }

    union Storage
    {
        T value;
        E error;
        Storage() {}   // NOLINT: union left uninitialized deliberately
        ~Storage() {}  // Destruction handled by Result
    } m_storage;

    bool m_hasValue = false;
};

/// Specialization of Result for void success type (result is just ok or error)
template <typename E>
class Result<void, E>
{
  public:
    /// Construct a successful void result
    explicit Result(OkTag) : m_hasValue(true) {}

    /// Construct an error result
    Result(ErrTag, const E& error) : m_hasValue(false), m_error(error) {}

    Result(ErrTag, E&& error) : m_hasValue(false), m_error(std::move(error)) {}

    bool IsOk() const noexcept
    {
        return m_hasValue;
    }
    bool IsErr() const noexcept
    {
        return !m_hasValue;
    }

    const E& Error() const& noexcept
    {
        return m_error;
    }
    E& Error() & noexcept
    {
        return m_error;
    }

  private:
    bool m_hasValue = false;
    E m_error{};
};

/// Factory: create a successful Result
template <typename T>
Result<std::decay_t<T>> Ok(T&& value)
{
    return Result<std::decay_t<T>>(OkTag{}, std::forward<T>(value));
}

/// Factory: create a successful void Result
inline Result<void> OkVoid()
{
    return Result<void>(OkTag{});
}

/// Factory: create an error Result
template <typename T, typename E>
Result<T, E> Err(E&& error)
{
    return Result<T, E>(ErrTag{}, std::forward<E>(error));
}

/// Factory: create an error Result with StructuredError defaults
template <typename T>
Result<T> MakeError(StructuredErrorDomain domain, ErrorSeverity severity, HRESULT code, const std::string& message,
                    ErrorSourceLocation location = {})
{
    return Result<T>(ErrTag{}, StructuredError(domain, severity, code, message, location));
}

/// Macro: early-return on error, propagating the error Result
/// Usage: TRY_RESULT(auto val, SomeFunction());
/// Expands to: if the result is Err, return the error immediately
#define TRY_RESULT(varDecl, expr)                                                                          \
    auto _tryResult_##__LINE__ = (expr);                                                                   \
    if (_tryResult_##__LINE__.IsErr())                                                                     \
        return decltype(expr)(::ExplorerLens::Engine::ErrTag{}, std::move(_tryResult_##__LINE__.Error())); \
    varDecl = std::move(_tryResult_##__LINE__.Value())

}  // namespace Engine
}  // namespace ExplorerLens
