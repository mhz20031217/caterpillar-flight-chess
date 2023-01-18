build:
	g++ -o game ctpchessV3.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
build_all:
	g++ -o game ctpchessV3.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
	g++ -o launcher launcher.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
	g++ -o winning winning.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
build_winning:
	g++ -o winning winning.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
build_launcher:
	g++ -o launcher launcher.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
run:
	./launcher
run_win:
	./launcher.exe
debug:
	g++ -o game ctpchessV3.cpp -g `pkg-config --cflags --libs gtk+-3.0` -DDEBUG -lao -lmpg123
	g++ -o winning winning.cpp -g `pkg-config --cflags --libs gtk+-3.0` -DDEBUG -lao -lmpg123
	g++ -o launcher launcher.cpp -g `pkg-config --cflags --libs gtk+-3.0` -DDEBUG -lao -lmpg123
build_win:
	g++ -o game.exe ctpchessV3.cpp -O4 -DWINDOWS -lpthread `pkg-config --cflags --libs gtk+-3.0` -lao -lmad
	g++ -o launcher.exe launcher.cpp -O4  -DWINDOWS -lpthread `pkg-config --cflags --libs gtk+-3.0` -lao -lmad
	g++ -o winning.exe winning.cpp -O4 -DWINDOWS -lpthread `pkg-config --cflags --libs gtk+-3.0` -lao -lmad