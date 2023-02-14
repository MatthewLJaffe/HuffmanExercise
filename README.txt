This is an implementation of the Huffman Exercise created by Matthew Jaffe
This code was Written Febuary 12th-13th 2023

The relevant code files in this directory are Main.cpp, ByteNode.cpp, and ByteNode.hpp
To compress and decompress file they must be placed in this directory.

The program accepts the follow arguments where "FileToCompress" is the name of any file in the directory
and "FileToDecompress.COMPRESSED" is the name of a file in the directory that has been compressed
To compress:
./HuffmanExercise compress "FileToCompress"
the ouput of this command will be a file "FileToCompress.COMPRESSED" 
that is a compressed copy of the original file

To decompress:
./HuffmanExercise decompress "FileToDecompress.COMPRESSED"
the output of this comand will be a file "DECOMPRESSED_FileToDecompress" 
that is identical to the file before compression