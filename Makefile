# ARMv9 Huffman Compression Build Configuration
CC = aarch64-linux-gnu-gcc
CFLAGS = -O3 -march=armv8-a+sve2 -mfpu=auto
TARGET = armv9_compressor
SOURCES = huffman_compressor.c file_utils.c sve2_optimizations.s
QEMU = qemu-aarch64

$(TARGET): $(SOURCES)
	@echo "Building ARMv9 Huffman Compressor..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
	@echo "Build complete. Run with: make run"

run: $(TARGET)
	@echo "Executing ARMv9 Compression Demonstrator..."
	$(QEMU) ./$(TARGET)

setup:
	@echo "Setting up test environment..."
	$(QEMU) ./$(TARGET) setup

benchmark: $(TARGET)
	@echo "Running performance benchmarks..."
	$(QEMU) ./$(TARGET) benchmark

clean:
	rm -f $(TARGET)
	rm -rf test_files
	@echo "Clean complete"

debug: $(SOURCES)
	$(CC) $(CFLAGS) -g -o $(TARGET) $(SOURCES)

.PHONY: run setup benchmark clean debug
