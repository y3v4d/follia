.PHONY: cmake compile

cmake:
	cmake -B build
	cp build/compile_commands.json .

compile:
	make -C build

examples/%:
	./build/$@

tests/%:
	./build/$@
