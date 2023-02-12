#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include<ctime>
#include <queue>

std::map<char, int> frequencyTable;
std::string bitsTable[256];
uint16_t treeSize = 0;

// A Huffman tree node
struct MinHeapNode {

    // One of the input characters
    char data;

    // Frequency of the character
    unsigned freq;

    // Left and right child
    MinHeapNode* left, * right;

    MinHeapNode(char data, unsigned freq)

    {
        left = right = NULL;
        this->data = data;
        this->freq = freq;
    }
};

// For comparison of
// two heap nodes (needed in min heap)
struct compare {

    bool operator()(MinHeapNode* l, MinHeapNode* r)

    {
        return (l->freq > r->freq);
    }
};

// Prints huffman codes from
// the root of Huffman Tree.
void indexCodes(struct MinHeapNode* root, std::string str)
{

    if (!root)
        return;
    treeSize++;
    if (root->data != '$')
    {
        bitsTable[static_cast<unsigned char>(root->data)] = str;
        std::cout << static_cast<unsigned char>(root->data) << " " << str << std::endl;
    }
    indexCodes(root->left, str + "0");
    indexCodes(root->right, str + "1");
}

// The main function that builds a Huffman Tree and
// print codes by traversing the built Huffman Tree
MinHeapNode* HuffmanCodes()
{
    struct MinHeapNode* left, * right, * top;

    // Create a min heap & inserts all characters of data[]
    std::priority_queue<MinHeapNode*, std::vector<MinHeapNode*>, compare> minHeap;
    for (auto& pair : frequencyTable)
        minHeap.push(new MinHeapNode(pair.first, pair.second));

    // Iterate while size of heap doesn't become 1
    while (minHeap.size() != 1) {

        // Extract the two minimum
        // freq items from min heap
        left = minHeap.top();
        minHeap.pop();

        right = minHeap.top();
        minHeap.pop();

        // Create a new internal node with
        // frequency equal to the sum of the
        // two nodes frequencies. Make the
        // two extracted node as left and right children
        // of this new node. Add this node
        // to the min heap '$' is a special value
        // for internal nodes, not used
        top = new MinHeapNode('$', left->freq + right->freq);

        top->left = left;
        top->right = right;

        minHeap.push(top);
    }

    // Print Huffman codes using
    // the Huffman tree built above
    indexCodes(minHeap.top(), "");
    return minHeap.top();
}

void readBinaryFile(std::string const& filename)
{
	size_t bufferSize = 8192;
	char* buffer = new char[bufferSize];
	std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);
	while (input)
	{
		// Try to read next chunk of data
		input.read(buffer, bufferSize);
		// Get the number of bytes actually read
		size_t count = input.gcount();
		// If nothing has been read, break
		if (!count)
			break;
		for (size_t i = 0; i < count; i++)
		{
			frequencyTable[static_cast<unsigned char>(buffer[i])]++;
		}
	}
	delete[] buffer;
}

void writeCompressedFile(std::string const& filename)
{
    size_t bufferSize = 8192;
    char* inputBuffer = new char[bufferSize];
    char* outputBuffer = new char[bufferSize];
    std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);
    std::ofstream output(filename + "_COMPRESSED", std::ios::app | std::ios::binary);
    char bitBuffer = 0;
    int bitPos = 0;
    int outputBuffIdx = 0;
    
    while (input)
    {
        // Try to read next chunk of data
        input.read(inputBuffer, bufferSize);
        // Get the number of bytes actually read
        size_t count = input.gcount();
        // If nothing has been read, break
        if (!count)
            break;
        for (size_t i = 0; i < count; i++)
        {
            std::string bits = bitsTable[static_cast<unsigned char>(inputBuffer[i])];
            for (size_t i = 0; i < bits.size(); i++)
            {
                if (bits[i] == '1')
                    bitBuffer |= (1 << bitPos);
                bitPos++;
                if (bitPos == 8) 
                {
                    outputBuffer[outputBuffIdx] = bitBuffer;
                    bitBuffer = 0;
                    bitPos = 0;
                    outputBuffIdx++;
                    if (outputBuffIdx == bufferSize)
                    {
                        outputBuffIdx = 0;
                        output.write(outputBuffer, bufferSize);
                    }
                }
            }
        }
    }
    //flush bit and byte buffers
    output.write(outputBuffer, outputBuffIdx);
    if (bitPos != 0)
    {
        char remainingBits[1] = { bitBuffer };
        output.write(remainingBits, 1);
    }
    delete[] inputBuffer;
    delete[] outputBuffer;
}

void saveTree(MinHeapNode* root, std::ofstream& outputFile, std::string const& filename)
{
    if (!root) return;

    if (!outputFile.is_open())
    {
        outputFile.open(filename + "_COMPRESSED", std::ios::out | std::ios::binary);
        outputFile << treeSize;
    }
    saveTree(root->left, outputFile, filename);
    saveTree(root->right, outputFile, filename);
    outputFile << root->data;
}

int main(int argc, char* argv[])
{
	clock_t timeReq = clock();
    std::string fileName = "HarryPotter.txt";
	readBinaryFile(fileName);
    MinHeapNode* root = HuffmanCodes();
    std::ofstream outputFile;
    saveTree(root, outputFile, fileName);
    outputFile.flush();
    writeCompressedFile(fileName);
	timeReq = clock() - timeReq;
	std::cout << (float) timeReq / CLOCKS_PER_SEC << " seconds";
	return 0;
}


