//
//  main.cpp
//  Tolstikov_LAB4
//
//  Created by Igor Karpov on 11/13/15.
//  Copyright © 2015 Igor Karpov. All rights reserved.
//

#include <iostream>
#include <fstream>

void generateLargeRandomFile(const char *filePath)
{
    std::ofstream outfile (filePath,std::ofstream::binary);

    long bufferSize = 1024;
    char* buffer = new char[bufferSize];
    
    for (int i = 0; i < 1024 * 100; i++)
    {
        for (int j = 0; j < bufferSize; j++)
        {
            buffer[j] = rand() % 256;
        }
        outfile.write(buffer, bufferSize);
    }
    
    delete[] buffer;
    outfile.close();
}

int main(int argc, const char *argv[])
{
    const char *filePath = "large_file.txt";
    std::cout << "File gererating started: " << filePath << std::endl << std::endl;
    generateLargeRandomFile(filePath);
    return 0;
}
