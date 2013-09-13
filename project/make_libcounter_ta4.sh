#!/bin/sh

# удалить папку назначения со всем содержимым
rm -rf "../bin"

# добавление пути для поиска библиотек
export LIBRARY_PATH=/usr/lib/i386-linux-gnu:$LIBRARY_PATH

# создать папку назначения бинарников, если её нет
if [ ! -d "../bin" ] 
then
    mkdir "../bin"
else
    echo "Создаю папку libcounter_ta4/bin/"
fi

# собрать библиотеку
make

if [ -e "../src/libcounter_ta4.h" ] 
then
    cp ../src/libcounter_ta4.h ../bin/libcounter_ta4.h
else
    echo "Файл libcounter_ta4/src/libcounter_ta4.h - отсутствует"
fi
    

