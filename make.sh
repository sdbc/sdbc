##################################
# Desc: 编译Cmake Files 产生静态库
# Author: liu.weihua<394806487@qq.com>
# Created Time: 06/24/2018
##################################
#!/bin/sh

rm -rf build;
mkdir build;
cd build;
cmake ../;
make;

## libscbase.a
mkdir libscbase;
cp conf/libconf.a libscbase/;
cp crypto/libcrypto.a libscbase/;
cp ds/libds.a libscbase/;
cp pack/libpack.a libscbase/;
cd libscbase/;
ar -x libconf.a;
ar -x libcrypto.a;
ar -x libds.a;
ar -x libpack.a;
ar -r libscbase.a *.o;
mv libscbase.a ../;
cd ../;
rm -rf libscbase;

## libsc.a
mkdir libsc;
cp sccli/libsccli.a libsc/;
cp scsrv/libscsrv.a libsc/;
cp socket/libsocket.a libsc/;
cd libsc/;
ar -x libsccli.a;
ar -x libscsrv.a;
ar -x libsocket.a;
ar -r libsc.a *.o;
mv libsc.a ../;
cd ../;
rm -rf libsc;


