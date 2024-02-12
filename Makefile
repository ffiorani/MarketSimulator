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
