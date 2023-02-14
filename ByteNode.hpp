#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <map>
#include <stack>


struct ByteNode
{

    char byte;
    bool isInternal = false;
    unsigned frequency = 0;

    ByteNode* left;
    ByteNode *right;

    ByteNode(char byte, unsigned freqency, bool isInternal);
};


struct compare
{
    inline bool operator()(ByteNode* l, ByteNode* r) { return (l->frequency > r->frequency); }
};

/// <summary>
/// Stores binary bit sequences for the position of each node in the Byte Tree 
/// </summary>
/// <param name="root">root of the Byte Tree</param>
/// <param name="bitsTable">array where all bits sequences are indexed</param>
/// <param name="bitSequence">recursively built to create bitSequence for a leaf node</param>
void indexCodes(std::string bitsTable[], uint16_t& treeSize, ByteNode* root, std::string bitSequence);

/// <summary>
///  builds tree for computing optimal bit sequences for each byte using provided table of byte frequencies
/// </summary>
/// <param name="frequencyTable">map containing the frequency of each byte</param>
ByteNode* CreateByteTree(std::map<char, int> frequencyTable);

/// <summary>
/// Stores relevant data for rebuilding the Byte Tree as well as how big the file is into the compressed file
/// </summary>
/// <param name="root"> root of the Byte Tree</param>
/// <param name="outputFile"> Compressed file</param>
/// <param name="fileName"> Name of the original file</param>
/// <param name="treeSize"> number of bytes dedicated to storing tree data in the compressed file</param>
/// <param name="fileSize"> size of file in bytes</param>
void saveTree(ByteNode* root, std::ofstream& outputFile, std::string const& fileName, uint16_t treeSize, size_t fileSize);

/// <summary>
/// returns the root of the Byte Tree based on the supplied input file
/// </summary>
/// <param name="input">compressed file</param>
/// <param name="decompressedSize">retrieves how big the file should be when decompressed</param>
ByteNode* buildTreeFromCompressed(std::ifstream& input, size_t& decompressedSize);