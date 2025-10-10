# Makefile - compile all .c in src/ into a single executable ./build/program
CC ?= gcc
CFLAGS ?= -std=c11 -O2 -g -Wall -Wextra -I./src

SRCDIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/program

SRCS := $(wildcard $(SRCDIR)/*.c)

.PHONY: all clean rebuild dirs show

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(SRCS)
	@echo "CC -> $(TARGET)"
	@$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	@echo "Cleaning $(BUILD_DIR)"
	@rm -rf $(BUILD_DIR)/*

rebuild: clean all

show:
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "SRCS = $(SRCS)"
	@echo "TARGET = $(TARGET)"
