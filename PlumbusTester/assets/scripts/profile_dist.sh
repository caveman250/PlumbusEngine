cd ../
cmake --build ../bin/Dist -j8
perf record --call-graph dwarf ../bin/Dist/PlumbusTester/PlumbusTester
hotspot /home/ouchqt/checkouts/plumbusengine/PlumbusTester/perf.data