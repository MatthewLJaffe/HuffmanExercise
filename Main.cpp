#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <ctime>
#include "ByteNode.hpp"

std::map<char, int> frequencyTable;
std::string bitsTable[256];
size_t fileSize = 0;
size_t decompressedSize = 0;
//size of tree in bytes when written to compresseion file
uint16_t treeSize = 0;
const std::string compressedExtension = ".COMPRESSED";

/// <summary>
/// Reads the original file and indexes each byte in frequency table
/// </summary>
/// <param name="filename">name of original file</param>
void readBinaryFile(std::string const& filename)
{
	size_t bufferSize = 8192;
	char* buffer = new char[bufferSize];
	std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);
	while (input)
	{
		input.read(buffer, bufferSize);
		size_t count = input.gcount();
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

/// <summary>
/// Writes data to compressed file
/// </summary>
/// <param name="filename">name of original file</param>
/// <param name="outputFile">compressed file</param>
void writeCompressedFile(std::string const& filename, std::ofstream& outputFile)
{
    size_t bufferSize = 8192;
    char* inputBuffer = new char[bufferSize];
    char* outputBuffer = new char[bufferSize];
    std::ifstream input(filename.c_str(), std::ios::in | std::ios::binary);
    std::string compressedFileName;
    char bitBuffer = 0;
    int bitPos = 0;
    int outputBuffIdx = 0;
    
    while (input)
    {
        input.read(inputBuffer, bufferSize);
        size_t count = input.gcount();
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
                        outputFile.write(outputBuffer, bufferSize);
                    }
                }
            }
        }
    }
    //flush bit and byte buffers
    outputFile.write(outputBuffer, outputBuffIdx);
    if (bitPos != 0)
    {
        char remainingBits[1] = { bitBuffer };
        outputFile.write(remainingBits, 1);
    }
    delete[] inputBuffer;
    delete[] outputBuffer;
}


/// <summary>
/// retrieves the next byte of the original file using the supplied Byte Tree and compressed file
/// </summary>
/// <param name="iPos">position in inputBuffer</param>
/// <param name="inputBuffer">buffer to read from</param>
/// <param name="inputBufferSize">size of buffer to read from</param>
/// <param name="root">root of Byte Tree</param>
/// <param name="input">compressed file</param>
/// <param name="inputEmpty">true if nothing is left to read false otherwise</param>
/// <returns></returns>
char getDecompressedByte(size_t& iPos, char* inputBuffer, size_t& inputBufferSize, ByteNode* root, std::ifstream& input, bool& inputEmpty)
{
    static int currBytePos = 0;
    ByteNode* currNode = root;
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
                    return currNode->byte;
            }
        }
    }
    return currNode->byte;
}

/// <summary>
/// uses the supplied Byte Tree to recreate the original file from the compressed file
/// </summary>
/// <param name="input">compressed file</param>
/// <param name="filename">name of compressed file</param>
/// <param name="root">root of Byte Tree</param>
void createDecompressedFile(std::ifstream& input, std::string const& filename, ByteNode* root)
{
    size_t outputBytes = 0;
    size_t bufferSize = 8192;
    char* inputBuffer = new char[bufferSize];
    char* outputBuffer = new char[bufferSize];
    std::string originalFileName = filename.substr(0, filename.find(compressedExtension));
    std::ofstream output(("DECOMPRESSED_" + originalFileName).c_str(), std::ios::out | std::ios::binary);
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
    delete[] inputBuffer;
    delete[] outputBuffer;
}

/// <summary>
/// frees Byte Tree
/// </summary>
/// <param name="root">root of byte tree</param>
void freeTree(ByteNode* root)
{
    if (root == nullptr) return;
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

int main(int argc, char* argv[])
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    clock_t timeReq = clock();
    if (argc != 3)
    {
        std::cout << "Invocation: HuffmanExercise compress filename OR HuffmanExercise decompress filename" << compressedExtension << std::endl;
        return 0;
    }
    std::string commandType = argv[1];
    std::string fileName = argv[2];
    if (commandType == "compress")
    {
        std::ifstream fileToCompress(fileName.c_str());
        if (!fileToCompress.good())
        {
            std::cout << "File " << fileName << " not found" << std::endl;
            return 0;
        }
        readBinaryFile(fileName);
        ByteNode* compressTree = CreateByteTree(frequencyTable);
        indexCodes(bitsTable, treeSize, compressTree, "");
        std::string compressedFileName;
        size_t periodPos = fileName.find('.');
        compressedFileName = fileName + compressedExtension;
        std::ofstream output(compressedFileName.c_str(), std::ios::out | std::ios::binary);
        //insert size of tree and uncompressed fileSize into compressed file
        output << treeSize;
        output << '\n';
        output << fileSize;
        output << '\n';
        saveTree(compressTree, output, fileName, treeSize, fileSize);
        output.flush();
        writeCompressedFile(fileName, output);
        freeTree(compressTree);
    }
    else if (commandType == "decompress")
    {
        if (fileName.find(compressedExtension) == std::string::npos)
        {
            std::cout << "To decompress file must have " << compressedExtension << " extension" << std::endl;
            return 0;
        }
        std::ifstream fileToCompress(fileName.c_str());
        if (!fileToCompress.good())
        {
            std::cout << "File " << fileName << " not found" << std::endl;
            return 0;
        }
        std::ifstream input(fileName.c_str(), std::ios::in | std::ios::binary);
        ByteNode* decompressTree = buildTreeFromCompressed(input, decompressedSize);
        createDecompressedFile(input, fileName, decompressTree);
        freeTree(decompressTree);
    }
    timeReq = clock() - timeReq;
    std::cout << (float)timeReq / CLOCKS_PER_SEC << " seconds";
	return 0;
}