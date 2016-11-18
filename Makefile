OFDVIEWER=./build/bin/ofdviewer
OFDTEST=./build/bin/ofdtest

.PHONY: build run

build:
	mkdir -p build && cd build && cmake .. && make

${OFDVIEWER}: build
${OFDTEST}: build

run: ${OFDVIEWER}
	${OFDVIEWER} ./data/sample0.ofd --v=1

test: ${OFDTEST}
	${OFDTEST} ./data/sample1.pdf --v=1

cscope:
	cscope -Rbq

clean:
	cd build && make clean
