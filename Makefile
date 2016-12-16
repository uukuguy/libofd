OFDVIEWER=./build/bin/ofdviewer
OFDTEST=./build/bin/ofdtest
PDF2OFD=./build/bin/pdf2ofd

.PHONY: build run 

build:
	mkdir -p build && cd build && cmake .. && make 

${OFDVIEWER}: build
${OFDTEST}: build

run: ${OFDVIEWER}
	${OFDVIEWER} ./data/sample0.ofd --v=1

test: ${OFDTEST}
	${OFDTEST} ./data/sample1.pdf --v=1 

pdf2ofd: ${PDF2OFD}
	rm -fr sample0.ofd sample0
	${PDF2OFD} ./data/sample0.pdf sample0.ofd && unzip -d sample0 sample0.ofd >> /dev/null 
	cat sample0/OFD.xml | xmllint --format -
	cat sample0/Doc_0/Document.xml | xmllint --format -
	cat sample0/Doc_0/Pages/Page_0/Content.xml | xmllint --format -

sample0.ofd: ${PDF2OFD}
	rm -fr sample0.ofd sample0
	${PDF2OFD} ./data/sample0.pdf sample0.ofd && unzip -d sample0 sample0.ofd >> /dev/null 

ofdtest: sample0.ofd ${OFDTEST} 
	${OFDTEST} sample0.ofd

check:
	xmllint --noout --schema ./doc/GBT33190-2016/OFD.xsd ./sample0/OFD.xml
	xmllint --noout --schema ./doc/GBT33190-2016/Document.xsd ./sample0/Doc_0/Document.xml
	xmllint --noout --schema ./doc/GBT33190-2016/Page.xsd ./sample0/Doc_0/Pages/Page_0/Content.xml

cscope:
	cscope -Rbq

clean:
	cd build && make clean