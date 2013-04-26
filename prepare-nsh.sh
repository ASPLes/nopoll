# gen nopoll_product_version.nsh
echo -n "!define PRODUCT_VERSION \"" > nopoll_product_version.nsh
value=`cat VERSION`
echo -n $value >> nopoll_product_version.nsh
echo -n "\"" >> nopoll_product_version.nsh




