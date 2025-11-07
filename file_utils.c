#include "compression.h"
#include <dirent.h>

// Create test directory and sample files
void create_test_environment() {
    system("mkdir -p test_files");
    
    // Create sample text file
    FILE* sample = fopen("test_files/sample.txt", "w");
    const char* text = "This is a sample text file for ARMv9 compression testing. "
                      "Huffman coding is particularly effective for text compression "
                      "because it exploits character frequency patterns. "
                      "ARMv9 SVE2 optimizations make this process significantly faster.";
    fwrite(text, 1, strlen(text), sample);
    fclose(sample);
    
    // Create larger binary test file
    FILE* binary = fopen("test_files/large_file.bin", "wb");
    for (int i = 0; i < 100000; i++) {
        unsigned char data = rand() % 256;
        fwrite(&data, 1, 1, binary);
    }
    fclose(binary);
    
    printf("Test environment created successfully!\n");
}

// Benchmark compression performance
void run_performance_benchmark() {
    printf("\n=== ARMv9 Compression Performance Benchmark ===\n");
    
    // Test with different file sizes
    size_t sizes[] = {1024, 10240, 102400, 1048576}; // 1KB to 1MB
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        // Create test file of specific size
        char filename[256];
        snprintf(filename, sizeof(filename), "test_files/benchmark_%zu.bin", sizes[i]);
        
        FILE* test_file = fopen(filename, "wb");
        for (size_t j = 0; j < sizes[i]; j++) {
            unsigned char byte = rand() % 256;
            fwrite(&byte, 1, 1, test_file);
        }
        fclose(test_file);
        
        // Compress and measure performance
        char output_file[256];
        snprintf(output_file, sizeof(output_file), "%s.compressed", filename);
        
        CompressionResult result = compress_file(filename, output_file);
        
        printf("File Size: %zu bytes\n", sizes[i]);
        printf("  Compression Ratio: %.2f%%\n", result.compression_ratio);
        printf("  Processing Time: %.6f seconds\n", result.processing_time);
        printf("  Throughput: %.2f MB/s\n\n", 
               (sizes[i] / (1024.0 * 1024.0)) / result.processing_time);
        
        // Cleanup
        remove(filename);
        remove(output_file);
    }
}
