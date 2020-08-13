cd ../
cmake --build ../bin/Debug -j8
perf record --call-graph dwarf ../bin/Debug/PlumbusTester/PlumbusTester
hotspot /home/ouchqt/checkouts/plumbusengine/PlumbusTester/perf.data