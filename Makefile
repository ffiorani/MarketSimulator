prepare:
	rm -rf ./build
	mkdir -p ./build && cd ./build && cmake ..

dependency:
	cd ./build && cmake .. --graphviz=graph.dot && dot -Tpng graph.dot -o graph.png
