cmake_minimum_required(VERSION 3.10)
project(JaskiniaProjekt CXX)

# Standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Wymagana biblioteka dla wątków (pthread)
find_package(Threads REQUIRED)

# --- DEFINICJE PLIKÓW WYKONYWALNYCH ---

# 1. Zarządca (Main)
add_executable(main main.cpp common.h)

# 2. Kasjer
add_executable(kasjer kasjer.cpp common.h)

# 3. Zwiedzający (używa wątków)
add_executable(zwiedzajacy zwiedzajacy.cpp common.h)
target_link_libraries(zwiedzajacy PRIVATE Threads::Threads)

# --- PRZYGOTOWANIE NA PRZYSZŁOŚĆ ---
# Kiedy stworzysz te pliki, po prostu odkomentuj poniższe linie:

# 4. Przewodnik
 add_executable(przewodnik przewodnik.cpp common.h)
 target_link_libraries(przewodnik PRIVATE Threads::Threads)

# 5. Strażnik
 add_executable(straznik straznik.cpp common.h)

# --- OPCJONALNE: Kopiowanie binarek do folderu głównego ---
# Dzięki temu main zawsze znajdzie ./kasjer i ./zwiedzajacy
set_target_properties(main kasjer zwiedzajacy PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
