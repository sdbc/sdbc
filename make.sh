#rm */*.o

rm -rf build;
mkdir build;
cd build;
cmake ../sccli/;
make;



#
#cd string
#make
#cd ../conf
#make
#cd ../ds
#make
#cd ../crypto
#make
#cd ../socket
#make
#cd ../sccli
#make
#cd ../scsrv
#make
#
#if [ -d ../pack ]
#then
#cd ../pack
#make
#fi
#
#if [ -d ../sqli ]
#then
#cd ../sqli
#make
#fi
#
#if [ -d ../dau ]
#then
#cd ../dau
#make
#fi
#
#if [ -d ../sdbc ]
#then
#cd ../sdbc
#make
#fi
#
