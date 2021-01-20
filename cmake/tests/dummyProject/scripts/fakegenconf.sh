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
# This script aims at faking genconf in the dummyProject
shift 6
plugin_name=$(echo $6 | sed -r -e 's/^.*lib(.*)\.so$/\1/g')
package_name=$4
mkdir -p genConfDir/${package_name}
echo "hey" > genConfDir/${package_name}/${plugin_name}.confdb
echo "hey" > genConfDir/${package_name}/${plugin_name}.confdb2_part
echo "hey" > genConfDir/${package_name}/__init__.py
echo "hey" > genConfDir/${package_name}/${plugin_name}Conf.py
