OFDVIEWER=./build/bin/ofdviewer

.PHONY: build run

build:
	mkdir -p build && cd build && cmake .. && make

${OFDVIEWER}: build

run: ${OFDVIEWER}
	${OFDVIEWER} ./data/sample0.ofd --v=1

test: ${OFDVIEWER}
	${OFDVIEWER} ./data/sample1.ofd --v=1

cscope:
	cscope -Rbq

clean:
	cd build && make clean
