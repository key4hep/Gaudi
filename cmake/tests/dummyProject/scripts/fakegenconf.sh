# This script aims at faking genconf in the dummyProject
shift 6
plugin_name=$(echo $6 | sed -r -e 's/^.*lib(.*)\.so$/\1/g')
package_name=$4
mkdir -p genConf/${package_name}
echo "hey" > genConf/${package_name}/${plugin_name}.confdb
echo "hey" > genConf/${package_name}/${plugin_name}.confdb2_part
echo "hey" > genConf/${package_name}/__init__.py
echo "hey" > genConf/${package_name}/${plugin_name}Conf.py
