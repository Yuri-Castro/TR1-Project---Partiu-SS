CC := g++
RM := rm -f
MKDIR := mkdir -p

BUILDDIR := build
SRCDIR := src
TARGET := bin

all: 
	@echo "Comando não encontrado. Digite 'make build' se quiser fazer o build do projeto ou 'make run' \nse quiser executá-lo depois do build ou 'make clean' para apagar os arquivos gerados pelo build"

build: $(TARGET)/main

$(TARGET)/main: $(BUILDDIR)/main.o
	$(CC) $(BUILDDIR)/main.o -o $(TARGET)/main
    
$(BUILDDIR)/main.o: $(SRCDIR)/main.cpp
	$(MKDIR) $(BUILDDIR)
	$(CC) -c $(SRCDIR)/main.cpp -o $(BUILDDIR)/main.o

clean: 
	$(RM) $(TARGET)/main
	$(RM) -r $(BUILDDIR)

run:
	./$(TARGET)/main
