# BrickGame Тетрис

Терминальный тетрис на C11. Игровая логика вынесена в отдельную динамическую библиотеку и построена на конечном автомате; интерфейс на ncurses загружает её во время работы. Поддерживаются Linux и macOS.

## Возможности

- Поле 10x20, все семь тетрамино, превью следующей фигуры
- Перемещение, поворот, ускоренное падение, пауза, перезапуск и выход
- Уничтожение линий со счётом: 1/2/3/4 линии = 100/300/700/1500 очков
- Уровень повышается каждые 600 очков (максимум 10), скорость падения растёт
- Сохранение рекорда в `tetris.score`
- Состояния автомата: `START`, `SPAWN`, `MOVING`, `SHIFTING`, `ATTACHING`, `GAMEOVER`
- Юнит-тесты на Check и HTML-отчёт о покрытии через lcov

## Архитектура

```
brick_game/include/brickgame.h   Публичный API: userInput(), updateCurrentState(), GameInfo_t
brick_game/tetris/               Игровая логика, автомат, работа с матрицами
gui/cli/                         Интерфейс ncurses, обработка ввода, отрисовка
brick_game/tetris/test/          Тесты на Check
```

CLI не линкуется с игровой библиотекой. Она загружается через `dlopen`/`dlsym`: `libtetris.so` на Linux или `libtetris.dylib` на macOS.

## Зависимости

Ubuntu/Debian:

```sh
sudo apt update
sudo apt install build-essential libncurses-dev check libsubunit-dev
# опционально: clang-format lcov valgrind
```

macOS:

```sh
xcode-select --install
brew install check
# опционально: brew install lcov clang-format
```

## Сборка

```sh
make
```

Результат: `brick_game/tetris/lib/libtetris.so` (или `.dylib`) и `./brickgame`.

## Запуск

```sh
./brickgame tetris
```

CLI ищет библиотеку по относительным путям и через rpath относительно исполняемого файла, поэтому `./brickgame tetris` работает из корня репозитория, а установленный бинарник — из любого каталога.

## Управление

| Клавиша | Действие |
| --- | --- |
| `Enter` | Старт / перезапуск |
| `P` | Пауза |
| `Q` | Выход |
| `Left` / `Right` | Движение фигуры |
| `Down` | Ускоренное падение |
| `Space` | Поворот |
| `Up` | Не используется |

## Тесты

```sh
make test          # собрать и запустить тесты Check
make gcov_report   # HTML-покрытие в coverage/html/index.html
make valgrind      # запустить игру под memcheck
make clang         # форматирование через clang-format
```

Тесты покрывают игровую библиотеку: 8 тестов, 100% строк и функций.

## Установка

```sh
make install
make uninstall
```

По умолчанию файлы ставятся в `~/.local`: бинарник — в `~/.local/bin`, библиотека — в `~/.local/lib`. Для установки в другой каталог задайте `PREFIX`:

```sh
make install PREFIX=/tmp/brickgame
sudo make install PREFIX=/usr/local   # системная установка
```

`DESTDIR` можно использовать для упаковки: `make install DESTDIR=/tmp/pkg PREFIX=/usr/local`. Запуск установленной версии: `PREFIX/bin/brickgame tetris`.
