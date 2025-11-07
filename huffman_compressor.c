#include "compression.h"
#include <sys/time.h>
#include <arm_neon.h>

// Performance measurement
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

// Create a new Huffman node
HuffmanNode* create_node(unsigned char data, unsigned frequency) {
    HuffmanNode* node = (HuffmanNode*)malloc(sizeof(HuffmanNode));
    node->data = data;
    node->frequency = frequency;
    node->left = node->right = NULL;
    return node;
}

// Create priority queue
PriorityQueue* create_priority_queue(int capacity) {
    PriorityQueue* queue = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    queue->size = 0;
    queue->capacity = capacity;
    queue->nodes = (HuffmanNode**)malloc(capacity * sizeof(HuffmanNode*));
    return queue;
}

// Insert node into priority queue
void insert_node(PriorityQueue* queue, HuffmanNode* node) {
    int i = queue->size++;
    while (i && node->frequency < queue->nodes[(i - 1) / 2]->frequency) {
        queue->nodes[i] = queue->nodes[(i - 1) / 2];
        i = (i - 1) / 2;
    }
    queue->nodes[i] = node;
}

// Extract minimum frequency node
HuffmanNode* extract_min(PriorityQueue* queue) {
    HuffmanNode* min = queue->nodes[0];
    queue->nodes[0] = queue->nodes[--queue->size];
    
    int i = 0;
    while (2 * i + 1 < queue->size) {
        int child = 2 * i + 1;
        if (child + 1 < queue->size && 
            queue->nodes[child + 1]->frequency < queue->nodes[child]->frequency) {
            child++;
        }
        if (queue->nodes[i]->frequency <= queue->nodes[child]->frequency) break;
        
        HuffmanNode* temp = queue->nodes[i];
        queue->nodes[i] = queue->nodes[child];
        queue->nodes[child] = temp;
        i = child;
    }
    return min;
}

// Build Huffman tree using ARMv9 optimized frequency counting
HuffmanNode* build_huffman_tree(unsigned char* data, size_t size, unsigned* frequencies) {
    // Use ARMv9 SVE2 for parallel frequency counting
    sve2_frequency_count(data, size, frequencies);
    
    // Build priority queue
    PriorityQueue* queue = create_priority_queue(256);
    
    for (int i = 0; i < 256; i++) {
        if (frequencies[i] > 0) {
            insert_node(queue, create_node((unsigned char)i, frequencies[i]));
        }
    }
    
    // Build Huffman tree
    while (queue->size > 1) {
        HuffmanNode* left = extract_min(queue);
        HuffmanNode* right = extract_min(queue);
        
        HuffmanNode* parent = create_node(0, left->frequency + right->frequency);
        parent->left = left;
        parent->right = right;
        
        insert_node(queue, parent);
    }
    
    HuffmanNode* root = extract_min(queue);
    free(queue->nodes);
    free(queue);
    return root;
}

// Generate Huffman codes
void generate_codes(HuffmanNode* root, char** codes, char* buffer, int depth) {
    if (!root) return;
    
    if (!root->left && !root->right) {
        buffer[depth] = '\0';
        codes[root->data] = strdup(buffer);
        return;
    }
    
    buffer[depth] = '0';
    generate_codes(root->left, codes, buffer, depth + 1);
    
    buffer[depth] = '1';
    generate_codes(root->right, codes, buffer, depth + 1);
}

// Main compression function
CompressionResult compress_file(const char* input_path, const char* output_path) {
    double start_time = get_time();
    CompressionResult result = {0};
    
    // Read input file
    FILE* input = fopen(input_path, "rb");
    if (!input) {
        printf("Error: Cannot open input file %s\n", input_path);
        return result;
    }
    
    fseek(input, 0, SEEK_END);
    size_t file_size = ftell(input);
    fseek(input, 0, SEEK_SET);
    
    unsigned char* file_data = (unsigned char*)malloc(file_size);
    fread(file_data, 1, file_size, input);
    fclose(input);
    
    // Build Huffman tree with ARMv9 optimizations
    unsigned frequencies[256] = {0};
    HuffmanNode* root = build_huffman_tree(file_data, file_size, frequencies);
    
    // Generate Huffman codes
    char* codes[256] = {0};
    char buffer[256];
    generate_codes(root, codes, buffer, 0);
    
    // Compress data
    FILE* output = fopen(output_path, "wb");
    unsigned char current_byte = 0;
    int bit_count = 0;
    
    for (size_t i = 0; i < file_size; i++) {
        char* code = codes[file_data[i]];
        for (int j = 0; code[j]; j++) {
            current_byte = (current_byte << 1) | (code[j] - '0');
            bit_count++;
            
            if (bit_count == 8) {
                fwrite(&current_byte, 1, 1, output);
                current_byte = 0;
                bit_count = 0;
            }
        }
    }
    
    // Write remaining bits
    if (bit_count > 0) {
        current_byte <<= (8 - bit_count);
        fwrite(&current_byte, 1, 1, output);
    }
    
    fclose(output);
    
    // Calculate results
    result.original_size = file_size;
    result.compressed_size = ftell(output);
    result.compression_ratio = (1.0 - (double)result.compressed_size / file_size) * 100.0;
    result.processing_time = get_time() - start_time;
    
    // Cleanup
    free(file_data);
    for (int i = 0; i < 256; i++) {
        if (codes[i]) free(codes[i]);
    }
    
    return result;
}

// Main function with performance demonstration
int main() {
    printf("=== ARMv9 Huffman Compression Demonstrator ===\n");
    printf("Demonstrating ARMv9 optimizations for data compression\n\n");
    
    const char* test_files[] = {
        "test_files/sample.txt",
        "test_files/large_file.bin", 
        NULL
    };
    
    for (int i = 0; test_files[i]; i++) {
        char output_path[256];
        snprintf(output_path, sizeof(output_path), "%s.compressed", test_files[i]);
        
        printf("Compressing: %s\n", test_files[i]);
        CompressionResult result = compress_file(test_files[i], output_path);
        
        printf("  Original Size: %zu bytes\n", result.original_size);
        printf("  Compressed Size: %zu bytes\n", result.compressed_size);
        printf("  Compression Ratio: %.2f%%\n", result.compression_ratio);
        printf("  Processing Time: %.6f seconds\n", result.processing_time);
        printf("  Throughput: %.2f MB/s\n\n", 
               (result.original_size / (1024.0 * 1024.0)) / result.processing_time);
    }
    
    printf("=== ARMv9 Compression Demo Complete ===\n");
    return 0;
}
