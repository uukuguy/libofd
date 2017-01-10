OFDVIEWER=./build/bin/ofdviewer
OFDTEST=./build/bin/ofdtest
PDF2OFD=./build/bin/pdf2ofd

.PHONY: build run sample0 sample1 sample2

build:
	mkdir -p build && cd build && cmake .. && make 

${PDF2OFD}: build
${OFDVIEWER}: build
${OFDTEST}: build

run: ${OFDVIEWER}
	${OFDVIEWER} ./data/sample0.ofd --v=1

#pdf2ofd: ${PDF2OFD}
	#rm -fr sample0.ofd sample0
	#${PDF2OFD} --v=1 ./data/sample0.pdf sample0.ofd && unzip -d sample0 sample0.ofd >> /dev/null 

sample0.ofd: ${PDF2OFD}
	rm -fr sample0.ofd sample0
	${PDF2OFD} ./data/sample0.pdf sample0.ofd && unzip -d sample0 sample0.ofd >> /dev/null 


ofdtest: sample0.ofd ${OFDTEST} 
	#${OFDTEST} --v=1 ~/Arsenal/Fonts/simhei.ttf
	mkdir -p output/ofdtest
	${OFDTEST} --v=1 ./output/sample0/Doc_0/Res/Font_19.ttf

sample0: ${PDF2OFD}
	mkdir -p output/pdf2ofd
	${PDF2OFD} --v=1 ./data/sample0.pdf ./output/sample0.ofd && rm -fr ./output/sample0 && unzip -d ./output/sample0 ./output/sample0.ofd >> /dev/null 

sample1: ${PDF2OFD}
	mkdir -p output/pdf2ofd
	${PDF2OFD} --v=1 ./data/sample1.pdf ./output/sample1.ofd && rm -fr ./output/sample1 && unzip -d ./output/sample1 ./output/sample1.ofd >> /dev/null 

sample2: ${PDF2OFD}
	mkdir -p output/pdf2ofd
	${PDF2OFD} --v=1 ./data/sample2.pdf ./output/sample2.ofd && rm -fr ./output/sample2 && unzip -d ./output/sample2 ./output/sample2.ofd >> /dev/null 

view0: ${OFDVIEWER} 
	${OFDVIEWER} --v=1 ./output/sample0.ofd

view1: ${OFDVIEWER} 
	${OFDVIEWER} --v=1 ./output/sample1.ofd

view2: ${OFDVIEWER} 
	${OFDVIEWER} --v=1 ./output/sample2.ofd

check:
	cat sample0/OFD.xml | xmllint --format -
	cat sample0/Doc_0/Document.xml | xmllint --format -
	cat sample0/Doc_0/Pages/Page_0/Content.xml | xmllint --format -
	xmllint --noout --schema ./doc/GBT33190-2016/OFD.xsd ./sample0/OFD.xml
	xmllint --noout --schema ./doc/GBT33190-2016/Document.xsd ./sample0/Doc_0/Document.xml
	xmllint --noout --schema ./doc/GBT33190-2016/Page.xsd ./sample0/Doc_0/Pages/Page_0/Content.xml

cscope:
	cscope -Rbq

clean:
	cd build && make clean
