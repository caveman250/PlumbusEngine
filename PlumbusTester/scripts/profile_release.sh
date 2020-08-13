cd ../
cmake --build ../bin/Release -j8
perf record --call-graph dwarf ../bin/Release/PlumbusTester/PlumbusTester
hotspot /home/ouchqt/checkouts/plumbusengine/PlumbusTester/perf.data