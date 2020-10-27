#! /usr/bin/env python
# coding: utf-8
#####################################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
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
    return x - 1


def main():
    """
    A dummy main function.
    """
    print("I'm another dummy main function.")


if __name__ == "__main__":
    main()
