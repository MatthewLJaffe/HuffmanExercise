#include "ByteNode.hpp"

ByteNode::ByteNode(char byte, unsigned frequency, bool isInternal)
{
    left = nullptr;
    right = nullptr;
    this->byte = byte;
    this->frequency = frequency;
    this->isInternal = isInternal;
}


void indexCodes(std::string bitsTable[], uint16_t& treeSize, ByteNode* root, std::string str)
{
    if (!root)
        return;
    if (root->isInternal)
        treeSize += 3;
    else
    {
        treeSize++;
        bitsTable[static_cast<unsigned char>(root->byte)] = str;
    }
    indexCodes(bitsTable, treeSize, root->left, str + "0");
    indexCodes(bitsTable, treeSize, root->right, str + "1");
}


ByteNode* CreateByteTree(std::map<char, int> frequencyTable)
{
    struct ByteNode* left, * right, * top;

    std::priority_queue<ByteNode*, std::vector<ByteNode*>, compare> minHeap;
    for (auto& pair : frequencyTable)
        minHeap.push(new ByteNode(pair.first, pair.second, false));
    if (minHeap.size() == 0)
        return new ByteNode('x', 0, true);
    while (minHeap.size() != 1) {

        left = minHeap.top();
        minHeap.pop();
        right = minHeap.top();
        minHeap.pop();
        top = new ByteNode('x', left->frequency + right->frequency, true);

        top->left = left;
        top->right = right;

        minHeap.push(top);
    }
    return minHeap.top();
}

void saveTree(ByteNode* root, std::ofstream& outputFile, std::string const& filename, uint16_t treeSize, size_t fileSize)
{
    if (!root) return;
    saveTree(root->left, outputFile, filename, treeSize, fileSize);
    saveTree(root->right, outputFile, filename, treeSize, fileSize);
    //use INT for internal symbol
    if (root->isInternal)
    {
        outputFile << 'I';
        outputFile << 'N';
        outputFile << 'T';
    }
    else
        outputFile << root->byte;
}

ByteNode* buildTreeFromCompressed(std::ifstream& input, size_t& decompressedSize)
{
    std::stack<ByteNode*> treeStack;
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
            ByteNode* internalNode = new ByteNode('x', 0, true);
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
        treeStack.push(new ByteNode(treeBuff[treePos], 0, false));
    }
    delete[] treeBuff;
    if (treeStack.empty())
        return nullptr;
    return treeStack.top();
}