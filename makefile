build:
	g++ -o game ctpchessV3.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
	g++ -o launcher launcher.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
	g++ -o winning winning.cpp -g `pkg-config --cflags --libs gtk+-3.0` -lao -lmpg123
run:
	./launcher
run_win:
	./launcher.exe
debug:
	g++ -o game ctpchessV3.cpp -g `pkg-config --cflags --libs gtk+-3.0` -DDEBUG -lao -lmpg123
	g++ -o winning winning.cpp -g `pkg-config --cflags --libs gtk+-3.0` -DDEBUG -lao -lmpg123
	g++ -o launcher launcher.cpp -g `pkg-config --cflags --libs gtk+-3.0` -DDEBUG -lao -lmpg123
build_win:
	g++ -o game.exe ctpchessV3.cpp -g -DWINDOWS `pkg-config --cflags --libs gtk+-3.0` -lwinmm
	g++ -o launcher.exe launcher.cpp -g  -DWINDOWS `pkg-config --cflags --libs gtk+-3.0` -lwinmm
	g++ -o winning.exe winning.cpp -g -DWINDOWS `pkg-config --cflags --libs gtk+-3.0` -lwinmm