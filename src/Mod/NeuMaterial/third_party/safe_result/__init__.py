# SPDX-FileCopyrightText: 2025 overflowy@riseup.net
# SPDX-FileCopyrightText: 2026 Michael Cummings <mgcummings@yahoo.com>
# SPDX-License-Identifier: MIT

# Adapted from https://github.com/overflowy/safe-result
# Removed async support which is not needed for FreeCAD
#
# MIT License
#
# Copyright (c) 2025 overflowy@riseup.net
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import traceback
from functools import wraps
from typing import (
    Any,
    Callable,
    Generic,
    Literal,
    Never,  # replacement for NoReturn in Python 3.11
    ParamSpec,
    Type,
    TypeGuard,
    TypeVar,
    Union,
    cast,
)

T = TypeVar("T", covariant=True)
E = TypeVar("E", covariant=True, bound=Exception)
U = TypeVar("U")
R = TypeVar("R")
P = ParamSpec("P")
ExcType = TypeVar("ExcType", bound=Exception)


class Ok(Generic[T]):
    """A container for a successful result."""

    __slots__ = ("value",)
    __match_args__ = ("value",)

    def __init__(self, value: T):
        """Initializes Ok with a success value."""
        self.value = value

    @property
    def error(self) -> None:
        """Returns None as Ok represents success."""
        return None

    def is_ok(self) -> Literal[True]:
        """Returns True, indicating this is an Ok result."""
        return True

    def iserr(self) -> Literal[False]:
        """Returns False, indicating this is not an Err result."""
        return False

    def unwrap(self) -> T:
        """
        Returns the contained Ok value.

        Example:
            >>> Ok(42).unwrap()
            42
        """
        return self.value

    def unwrap_or(self, default: object) -> T:
        """
        Returns the contained Ok value.

        Args:
            default: A default value (ignored in Ok).

        Example:
            >>> Ok(42).unwrap_or(0)
            42
        """
        return self.value

    def map(self, func: Callable[[T], R]) -> "Ok[R]":
        """
        Maps an Ok[T] to Ok[R] by applying a function to the contained value.

        Args:
            func: The function to apply to the Ok value.

        Example:
            >>> Ok(5).map(lambda x: x * 2)
            Ok(10)
        """
        return Ok(func(self.value))

    def and_then(self, func: Callable[[T], "Result[R, E]"]) -> "Result[R, E]":
        """
        Calls `func` if the result is Ok, otherwise returns the Err value.
        Useful for chaining operations that might fail.

        Args:
            func: A function that takes the Ok value and returns a Result.

        Example:
            >>> def check_positive(n): return Ok(n) if n > 0 else Err("Not positive")
            >>> Ok(5).and_then(check_positive)
            Ok(5)
            >>> Ok(-1).and_then(check_positive)
            Err('Not positive')
        """
        return func(self.value)

    def flatten(self) -> "Result[T, E]":
        """
        Converts Result[Result[T, E], E] to Result[T, E].

        Example:
            >>> Ok(Ok(42)).flatten()
            Ok(42)
            >>> Ok(Err("nested error")).flatten()
            Err('nested error')
            >>> Ok(42).flatten() # No-op if not nested
            Ok(42)
        """
        result = self
        while ok(result) and isinstance(result.value, (Ok, Err)):
            result = result.value
        return result

    def __hash__(self) -> int:
        return hash((True, self.value))

    def __eq__(self, other: Any) -> bool:
        return ok(other) and self.value==other.value

    def __ne__(self, other: Any) -> bool:
        return not self.__eq__(other)

    def __repr__(self) -> str:
        return f"Ok({self.value!r})"


class Err(Generic[E]):
    """A container for a failed result."""

    __slots__ = ("error",)
    __match_args__ = ("error",)

    def __init__(self, error: E):
        """Initializes Err with an error value (usually an Exception)."""
        self.error = error

    @property
    def value(self) -> None:
        """Returns None as Err represents failure."""
        return None

    def is_ok(self) -> bool:
        """Returns False, indicating this is not an Ok result."""
        return False

    def iserr(self) -> bool:
        """Returns True, indicating this is an Err result."""
        return True

    def unwrap(self) -> Never:
        """
        Raises the contained error.

        Raises:
            E: The contained error.

        Example:
            >>> Err(ValueError("Failure")).unwrap() # Raises ValueError
        """
        raise self.error

    def unwrap_or(self, default: U) -> U:
        """
        Returns the provided default value.

        Args:
            default: The default value to return.

        Example:
            >>> Err(ValueError()).unwrap_or(42)
            42
        """
        return default

    def map(self, _: Callable[[E], R]) -> "Err[E]":
        """
        Returns self, as mapping a function over an Err yields the original Err.

        Args:
            _: The function (ignored).

        Example:
            >>> Err("error").map(lambda x: x + 1)
            Err('error')
        """
        return self

    def and_then(self, func: object) -> "Err[E]":
        """
        Returns self, as chaining an operation after an Err yields the original Err.

        Args:
            func: The function (ignored).

        Example:
            >>> def check(n): return Ok(n)
            >>> Err("init error").and_then(check)
            Err('init error')
        """
        return self

    def flatten(self) -> "Err[E]":
        """
        Returns self, as flattening an Err has no effect.

        Example:
            >>> Err("error").flatten()
            Err('error')
        """
        return self

    def __hash__(self) -> int:
        return hash((False, type(self.error), str(self.error)))

    def __eq__(self, other: Any) -> bool:
        if not err(other):
            return False
        return type(self.error) is type(other.error) and str(self.error)==str(
            other.error
        )

    def __ne__(self, other: Any) -> bool:
        return not self.__eq__(other)

    def __repr__(self) -> str:
        return f"Err({self.error!r})"


Result = Union[Ok[T], Err[E]]
"""
A result type that can be either `Ok` or `Err`.
"""


def safe(func: Callable[P, R]) -> Callable[P, Result[R, Exception]]:
    """
    Decorator that wraps a synchronous function to return a `Result`.
    The decorated function will never raise exceptions.
    Example:
        >>> @safe
        ... def divide(a: int, b: int) -> float:
        ...     return a / b
        ...
        >>> result = divide(10, 0)  # -> `Result[float, Exception]`
    """

    @wraps(func)
    def wrapper(*args: P.args, **kwargs: P.kwargs) -> "Result[R, Exception]":
        try:
            return Ok(func(*args, **kwargs))
        except Exception as e:
            return Err(e)

    return wrapper


def safe_with(
    *exc_types: Type[ExcType],
) -> Callable[[Callable[P, R]], Callable[P, "Result[R, ExcType]"]]:
    """
    Decorator factory that wraps a synchronous function to return a `Result`.
    The decorated function will only catch the specified exception types.
    Example:
        >>> @safe_with(ZeroDivisionError, ValueError)
        ... def divide(a: int, b: int) -> float:
        ...     return a / b
        ...
        >>> result = divide(10, 0)  # -> `Result[float, ZeroDivisionError | ValueError]`
    """

    def decorator(func: Callable[P, R]) -> Callable[P, "Result[R, ExcType]"]:
        @wraps(func)
        def wrapper(*args: P.args, **kwargs: P.kwargs) -> "Result[R, ExcType]":
            try:
                return Ok(func(*args, **kwargs))
            except exc_types as e:
                return Err(cast(ExcType, e))
            # Re-raise other exceptions
            except Exception as e:
                raise e

        return wrapper

    return decorator


def ok(result: Result[T, E]) -> TypeGuard[Ok[T]]:
    """Used for type narrowing from `Result` to `Ok`."""
    return isinstance(result, Ok)


def err(result: Result[Any, E]) -> TypeGuard[Err[E]]:
    """Used for type narrowing from `Result` to `Err`."""
    return isinstance(result, Err)


def err_type(
    result: Result[Any, E], exc_type: Type[ExcType]
) -> TypeGuard[Err[ExcType]]:
    """Check if error of `Result` is of a specific type."""
    return err(result) and isinstance(result.error, exc_type)


def traceback_of(result: Result[Any, Exception]) -> str:
    """Helper function to get the traceback of an error result."""
    if not err(result):
        return ""
    return "".join(
        traceback.format_exception(
            type(result.error), result.error, result.error.__traceback__
        )
    )


__all__ = [
    "Err",
    "err",
    "err_type",
    "Ok",
    "ok",
    "Result",
    "safe",
    "safe_with",
    "traceback_of",
]
