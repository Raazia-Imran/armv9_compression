#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Huffman Tree Node Structure
typedef struct HuffmanNode {
    unsigned char data;
    unsigned frequency;
    struct HuffmanNode *left, *right;
} HuffmanNode;

// Priority Queue Node
typedef struct PriorityQueue {
    int size;
    int capacity;
    HuffmanNode **nodes;
} PriorityQueue;

// Compression Result Structure
typedef struct CompressionResult {
    size_t original_size;
    size_t compressed_size;
    double compression_ratio;
    double processing_time;
} CompressionResult;

// ARMv9 SVE2 optimized function prototypes
void sve2_frequency_count(const unsigned char* data, size_t length, unsigned* freq_table);
void sve2_bit_manipulation(unsigned char* output, const unsigned char* input, size_t length);

// Core compression functions
HuffmanNode* build_huffman_tree(unsigned char* data, size_t size, unsigned* frequencies);
void generate_codes(HuffmanNode* root, char** codes, char* buffer, int depth);
CompressionResult compress_file(const char* input_path, const char* output_path);
CompressionResult decompress_file(const char* input_path, const char* output_path);

// Utility functions
PriorityQueue* create_priority_queue(int capacity);
void insert_node(PriorityQueue* queue, HuffmanNode* node);
HuffmanNode* extract_min(PriorityQueue* queue);

#endif
