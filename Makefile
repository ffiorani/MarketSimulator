prepare:
	rm -rf ./build
	mkdir -p ./build

dependency_graph:
	cd ./build && cmake .. --graphviz=graph.dot && dot -Tpng graph.dot -o graph.png

generate_documentation:
	cd ./docs && doxygen

prepare_and_build:
	make prepare
	cd ./build && cmake .. && cmake --build .

run_executable:
	cd ./build/app && ./Executable

build_docs:
	cd ./docs && doxygen

run_tests:
	cd ./build/test/LimitOrderBookTests && ./LimitOrderBookTests
	cd ./build/test/MarketPlayerTests && ./MarketPlayerTests
	cd ./build/test/MarketTests && ./MarketTests

build_docs:
	cd ./docs && doxygen
