#! /usr/bin/env python
# coding: utf-8

"""
Another dummy python module.
"""

def bar(x):
    """
    A dummy function to test doctest tests again.

    >>> bar(1)
    0
    >>> bar(5)
    4
    """
    return x-1

def main():
    """
    A dummy main function.
    """
    print("I'm another dummy main function.")

if __name__ == "__main__":
    main()
