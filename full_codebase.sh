#! /bin/bash
# Borra el archivo viejo primero
rm -f full_codebase.txt

# Define los directorios a buscar (agrega ../protocolo si existe)
SEARCH_PATHS="."
if [ -d "../protocolo" ]; then
    SEARCH_PATHS="$SEARCH_PATHS ../protocolo"
fi

# Genera el nuevo limpio
find $SEARCH_PATHS -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.ino" \) \
-not -path "*/.*" \
-not -path "*/.pio/*" \
-not -path "*/managed_components/*" \
-not -path "*/build/*" \
-not -path "*/Drivers/*" \
-not -path "*/cmake/*" \
-not -name "syscalls.c" \
-not -name "sysmem.c" \
-not -name "system_stm32g4xx.c" \
-not -name "stm32g4xx_hal_msp.c" \
-not -name "stm32g4xx_hal_conf.h" \
| sort \
| while read file; do
    echo -e "\n\n==========================================" >> full_codebase.txt;
    echo "FILE PATH: $file" >> full_codebase.txt;
    echo -e "==========================================\n" >> full_codebase.txt;
    cat "$file" >> full_codebase.txt;
done
