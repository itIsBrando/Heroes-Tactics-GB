<<<<<<< HEAD
lcc -Wa-l -Wl-m -Wl-j -Wm-ys -o main.gb ${PWD}/src/*.c ${PWD}/src/data/*.c
=======
REM lcc -Wa-l -Wl-m -Wl-j -Wm-ys -o main.gb src/main.c src/menu.c src/map.c src/ai.c src/hud.c src/oam.c src/cursor.c src/units.c src/diamond.c src/path.c src/game.c src/data/sprite.c src/data/mapdata.c

lcc -Wa-l -Wl-m -Wl-j -Wm-yc -o main.gb src/*.c src/data/*.c
>>>>>>> 85b17dc... Basic CGB support.
