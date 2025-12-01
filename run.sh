#!/bin/bash
# Script para compilar y ejecutar el kernel en Bochs

set -e  # Salir si hay algún error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Kernel SOLID - Compilar y Ejecutar${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Verificar dependencias
echo -e "${YELLOW}Verificando dependencias...${NC}"

if ! command -v nasm &> /dev/null; then
    echo -e "${RED}❌ NASM no está instalado${NC}"
    echo -e "   Instala con: sudo apt-get install nasm"
    exit 1
fi

if ! command -v bochs &> /dev/null; then
    echo -e "${RED}❌ Bochs no está instalado${NC}"
    echo -e "   Instala con: sudo apt-get install bochs bochs-x"
    exit 1
fi

echo -e "${GREEN}✓ NASM encontrado: $(nasm -v)${NC}"
echo -e "${GREEN}✓ Bochs encontrado${NC}"
echo ""

# Limpiar compilación anterior
echo -e "${YELLOW}Limpiando archivos anteriores...${NC}"
make clean > /dev/null 2>&1
echo -e "${GREEN}✓ Limpieza completada${NC}"
echo ""

# Compilar
echo -e "${YELLOW}Compilando kernel...${NC}"
if make; then
    echo -e "${GREEN}✓ Compilación exitosa${NC}"
else
    echo -e "${RED}❌ Error en la compilación${NC}"
    exit 1
fi
echo ""

# Verificar que se creó el kernel
if [ ! -f kernel.bin ]; then
    echo -e "${RED}❌ kernel.bin no fue generado${NC}"
    exit 1
fi

if [ ! -f diskette.img ]; then
    echo -e "${RED}❌ diskette.img no fue generado${NC}"
    exit 1
fi

KERNEL_SIZE=$(stat -f%z kernel.bin 2>/dev/null || stat -c%s kernel.bin 2>/dev/null)
echo -e "${GREEN}✓ kernel.bin creado (${KERNEL_SIZE} bytes)${NC}"
echo ""

# Preguntar modo de ejecución
echo -e "${BLUE}Selecciona modo de ejecución:${NC}"
echo "  1) Normal (sin debugger)"
echo "  2) Debug (con breakpoints automáticos)"
echo "  3) Debug interactivo (línea de comandos)"
echo ""
read -p "Opción [1-3]: " OPTION

case $OPTION in
    1)
        echo -e "${GREEN}Iniciando Bochs en modo normal...${NC}"
        bochs -f bochsrc -q
        ;;
    2)
        echo -e "${GREEN}Iniciando Bochs en modo debug...${NC}"
        bochs -f bochsrc -q -rc bochsdbg
        ;;
    3)
        echo -e "${GREEN}Iniciando Bochs en modo debug interactivo...${NC}"
        echo -e "${YELLOW}Comandos útiles:${NC}"
        echo "  c       - Continue"
        echo "  s       - Step"
        echo "  b ADDR  - Breakpoint"
        echo "  info r  - Ver registros"
        echo "  q       - Quit"
        echo ""
        bochsdbg -f bochsrc -q
        ;;
    *)
        echo -e "${RED}Opción inválida${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Ejecución finalizada${NC}"
echo -e "${BLUE}========================================${NC}"
