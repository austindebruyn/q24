

all: npm cli 

npm:
	mkdir -p bin; \
	cd src; \
	node-gyp configure; \
	node-gyp build; \
	cp build/Release/q24.node ../bin

cli: 
	mkdir -p bin
	g++ -o bin/q24sum src/*.cpp

.PHONY: test
test:
	mocha test

clean:
	rm -r bin
	rm -r src/build
	rm -r build
