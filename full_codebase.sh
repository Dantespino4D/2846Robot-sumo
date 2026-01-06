#! /bin/bash
# Borra el archivo viejo primero
rm -f full_codebase.txt

# Genera el nuevo limpio
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.ino" \) \
-not -path "*/.*" \
-not -path "*/.pio/*" \
-not -path "*/managed_components/*" \
-not -path "*/build/*" \
| sort \
| while read file; do
    echo -e "\n\n==========================================" >> full_codebase.txt;
    echo "FILE PATH: $file" >> full_codebase.txt;
    echo -e "==========================================\n" >> full_codebase.txt;
    cat "$file" >> full_codebase.txt;
done
