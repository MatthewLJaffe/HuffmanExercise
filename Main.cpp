#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <ctime>
#include <queue>
#include <stack>

std::map<char, int> frequencyTable;
std::string bitsTable[256];
size_t fileSize = 0;
size_t decompressedSize = 0;
//size of tree in bytes when written to compresseion file
uint16_t treeSize = 0;

// A Huffman tree node
struct MinHeapNode 
{

    // One of the input characters
    char data;
    bool isInternal = false;
    // Frequency of the character
    unsigned freq;

    // Left and right child
    MinHeapNode* left, * right;

    MinHeapNode(char data, unsigned freq, bool isInternal)

    {
        left = right = NULL;
        this->data = data;
        this->freq = freq;
        this->isInternal = isInternal;
    }
};

// For comparison of
// two heap nodes (needed in min heap)
struct compare 
{

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
    if (root->isInternal)
        treeSize += 3;
    else
    {
        treeSize++;
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
        minHeap.push(new MinHeapNode(pair.first, pair.second, false));
    if (minHeap.size() == 0)
        return new MinHeapNode('x', 0, true);
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
        top = new MinHeapNode('x', left->freq + right->freq, true);

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
            fileSize++;
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
        outputFile << '\n';
        outputFile << fileSize;
        outputFile << '\n';
    }
    saveTree(root->left, outputFile, filename);
    saveTree(root->right, outputFile, filename);
    //use INT for internal symbol
    if (root->isInternal)
    {
        outputFile << 'I';
        outputFile << 'N';
        outputFile << 'T';
    }
    else
        outputFile << root->data;
}

MinHeapNode* buildTreeFromCompressed(std::ifstream& input)
{
    //build tree
    std::stack<MinHeapNode*> treeStack;
    uint16_t inputTreeSize = 0;
    input >> inputTreeSize;
    char c = input.get();
    input >> decompressedSize;
    c = input.get();
    char* treeBuff = new char[inputTreeSize];
    input.read(treeBuff, static_cast<std::streamsize>(inputTreeSize));
    for (uint16_t treePos = 0; treePos < inputTreeSize; treePos++)
    {
        if (treeBuff[treePos] == 'I' && treeBuff[treePos + 1] == 'N' && treeBuff[treePos + 2] == 'T')
        {
            MinHeapNode* internalNode = new MinHeapNode('x', 0, true);
            if (!treeStack.empty())
            {
                internalNode->right = treeStack.top();
                treeStack.pop();
            }
            if (!treeStack.empty())
            {
                internalNode->left = treeStack.top();
                treeStack.pop();
            }
            treeStack.push(internalNode);
            treePos += 2;
            continue;
        }
        treeStack.push(new MinHeapNode(treeBuff[treePos], 0, false));
    }
    delete[] treeBuff;
    if (treeStack.empty())
        return nullptr;
    return treeStack.top();
}

char getDecompressedByte(size_t& iPos, char* inputBuffer, size_t& inputBufferSize, MinHeapNode* root, std::ifstream& input, bool& inputEmpty)
{
    static int currBytePos = 0;
    MinHeapNode* currNode = root;
    char outputByte = 0;
    while (currNode->isInternal)
    {
        //go right
        char inputChar = inputBuffer[iPos];
        bool goRight = inputChar & (1 << currBytePos);
        if (goRight)
        {
            currNode = currNode->right;
        }
        else
        {
            currNode = currNode->left;
        }
        currBytePos++;
        if (currBytePos == 8)
        {
            currBytePos = 0;
            iPos++;
            if (iPos == inputBufferSize)
            {
                iPos = 0;
                input.read(inputBuffer, inputBufferSize);
                inputBufferSize = input.gcount();
                inputEmpty = inputBufferSize == 0;
                if (inputEmpty)
                    return currNode->data;
            }
        }
    }
    return currNode->data;
}

void createDecompressed(std::ifstream& input, std::string const& filename, MinHeapNode* root)
{
    size_t outputBytes = 0;
    size_t bufferSize = 8192;
    char* inputBuffer = new char[bufferSize];
    char* outputBuffer = new char[bufferSize];
    std::ofstream output(("DECOMPRESSED_" + filename).c_str(), std::ios::out | std::ios::binary);
    if (root == nullptr) return;
    input.read(inputBuffer, bufferSize);
    size_t bytesRead = input.gcount();
    bool inputEmpty = false;
    size_t iPos = 0;
    while (!inputEmpty)
    {
        size_t oPos = 0;
        for (; oPos < bufferSize; oPos++) 
        {
            if (inputEmpty)
            {
                std::cout << "No more input" << std::endl;
                break;
            }
            char outputByte = getDecompressedByte(iPos, inputBuffer, bytesRead, root, input, inputEmpty);
            outputBuffer[oPos] = outputByte;
            outputBytes++;
            if (outputBytes == decompressedSize)
            {
                std::cout << "Output bytes at decompressed size" << std::endl;
                oPos++;
                inputEmpty = true;
                break;
            }
        }
        output.write(outputBuffer, oPos);
    }
}

//TODO make sure to free all memory
int main(int argc, char* argv[])
{
	clock_t timeReq = clock();
    std::string fileName = "FIEAPortfolio.zip";
	readBinaryFile(fileName);
    std::cout << "read file" << std::endl;
    MinHeapNode* compressTree = HuffmanCodes();
    std::ofstream outputFile;
    saveTree(compressTree, outputFile, fileName);
    std::cout << "Tree saved" << std::endl;
    outputFile.flush();
    writeCompressedFile(fileName);
    std::cout << "Compressed File Written" << std::endl;
    std::ifstream input((fileName + "_COMPRESSED").c_str(), std::ios::in | std::ios::binary);
    MinHeapNode* decompressTree = buildTreeFromCompressed(input);
    createDecompressed(input, fileName, decompressTree);
    std::cout << "Decompressed File Written" << std::endl;
	timeReq = clock() - timeReq;
	std::cout << (float) timeReq / CLOCKS_PER_SEC << " seconds";
	return 0;
}


