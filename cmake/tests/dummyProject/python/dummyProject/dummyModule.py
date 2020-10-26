#! /usr/bin/env python
# coding: utf-8

"""
A dummy python module.
"""

def foo(x):
    """
    A dummy function to test doctest tests.

    >>> foo(1)
    2
    >>> foo(5)
    6
    """
    return x+1

def main():
    """
    A dummy main function.
    """
    print("I'm a dummy main function.")

if __name__ == "__main__":
    main()
