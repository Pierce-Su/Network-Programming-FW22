mkdir build
cd build 
cmake ../
cd CMakeFiles/
cd server.dir/
sed -i 's/-Werror//g' flags.make
cd ..
cd ..
make
