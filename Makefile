

all: npm cli

npm:
	cd src; \
	node-gyp configure; \
	node-gyp build; \
	cp build/Release/q24.node ../bin

cli: 
	g++ -o bin/q24sum src/*.cpp

clean:
	rm -r bin/*
	rm -r src/build
