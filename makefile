build:
	g++ ctpchessV3.cpp -o FlightChess -g `pkg-config --cflags --libs gtk+-3.0`
run:
	./FlightChess