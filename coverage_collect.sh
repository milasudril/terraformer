mkdir __targets_gcov/.coverage 2> /dev/null || true
cd __targets_gcov/.coverage
gcovr -v -p --html-details coverage.html --exclude '..*\.test.cpp' --root ../../
