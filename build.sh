#!/bin/sh
set -e

exit_with_error() {
    echo "Ошибка: $1" >&2
    exit "${2:-1}"
}

if [ -z "$1" ]; then
    exit_with_error "Не указан исходный файл" 1
fi

file=$1

if [ ! -f "$file" ]; then
    exit_with_error "Файл '$file' не найден" 1
fi

current_dir=$(pwd)

out_name=$(grep "Output:" "$file" | awk -F "Output:" '{print $2}' | awk '{print $1}')

if [ -z "$out_name" ]; then
    exit_with_error "В файле нет указания 'Output:'" 1
fi

tmp_folder=$(mktemp -d)

clean() {
    rm -rf "$tmp_folder"
}
trap clean EXIT INT TERM

cp "$file" "$tmp_folder/"
cd "$tmp_folder"

ext=$(echo "$file" | awk -F. '{print $NF}')

if [ "$ext" = "c" ]; then
    gcc "$file" -o "$out_name" || exit_with_error "Сборка C не удалась" 2
elif [ "$ext" = "cpp" ]; then
    g++ "$file" -o "$out_name" || exit_with_error "Сборка C++ не удалась" 2
elif [ "$ext" = "tex" ]; then
    pdflatex -interaction=nonstopmode  "$file" > /dev/null || exit_with_error "Сборка LaTeX не удалась" 2
    mv *.pdf "$out_name" 2>/dev/null || true
else
    exit_with_error "Неизвестное расширение файла '$ext'" 1
fi

if [ -f "$out_name" ]; then
    mv "$out_name" "$current_dir/"
else
    exit_with_error "Целевой файл не был создан" 3
fi
