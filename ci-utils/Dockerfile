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

# IMPORTANT: when an update is needed, run
#
# docker build -t gitlab-registry.cern.ch/gaudi/format-checker .
# docker push gitlab-registry.cern.ch/gaudi/format-checker

FROM debian:unstable

RUN apt-get update && apt-get dist-upgrade -y \
    && apt-get install -y clang-format-8 python3-pip git \
    && pip3 install yapf==0.24.0 \
    && rm -rf /var/lib/apt/lists/*
