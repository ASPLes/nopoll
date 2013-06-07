# gen nopoll_product_version.nsh
echo -n "!define PRODUCT_VERSION \"" > nopoll_product_version.nsh
value=`cat VERSION`
echo -n $value >> nopoll_product_version.nsh
echo "\"" >> nopoll_product_version.nsh
echo "!define PLATFORM_BITS \"$1\"" >> nopoll_product_version.nsh
echo "InstallDir \"\$PROGRAMFILES$1\noPollW$1\"" >> nopoll_product_version.nsh



