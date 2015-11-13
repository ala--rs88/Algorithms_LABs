//
//  main.cpp
//  Tolstikov_LAB4
//
//  Created by Igor Karpov on 11/13/15.
//  Copyright © 2015 Igor Karpov. All rights reserved.
//

#include <iostream>
#include <fstream>

#define TEST_RUNS_COUNT 10

void generateLargeRandomFile(const char *filePath)
{
    std::cout << "Large random file gererating started. File path: " << filePath << std::endl;
    const auto startTime = std::clock();
    
    std::ofstream outfile (filePath,std::ofstream::binary);

    long bufferSize = 1024;
    char* buffer = new char[bufferSize];
    
    for (int i = 0; i < 1024 * 1024; i++)
    {
        for (int j = 0; j < bufferSize; j++)
        {
            buffer[j] = rand() % 256;
        }
        outfile.write(buffer, bufferSize);
    }
    
    delete[] buffer;
    outfile.close();
    
    const auto endTime = std::clock();
    std::cout << "Large random file gererating finised. Took " << double(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl << std::endl;
}

double getReadFromDiskSpeed(const char *filePath)
{
    std::ifstream infile (filePath,std::ifstream::binary);
    
    long bufferSize = 1024 * 1024 * 1024;
    char* buffer = new char[bufferSize];
    std::streamsize bytesRead = 0;
    
    const auto startTime = std::clock();
    
    while(infile.read(buffer, bufferSize))
    {
        bytesRead += infile.gcount();
    }
    
    const auto endTime = std::clock();
    
    delete[] buffer;
    infile.close();
    
    double totalTimeInSecs = double(endTime - startTime) / CLOCKS_PER_SEC;
    double speed = (bytesRead / (1024*1024)) / totalTimeInSecs;
    
    return speed;
}

void testReadFromDiskSpeed(const char *filePath)
{
    std::cout << "Read from disk speed test started. File path: " << filePath << std::endl;
    
    double sumSpeed = 0;
    for (int i = 0; i < TEST_RUNS_COUNT; i++)
    {
        double speed = getReadFromDiskSpeed(filePath);
        sumSpeed += speed;
        std::cout << speed << std::endl;
    }
    double averageSpeed = sumSpeed / TEST_RUNS_COUNT;
    
    std::cout << "Read from disk speed test finised. Test runs count: " << TEST_RUNS_COUNT << ". Average read speed: " << averageSpeed << " megabytes/sec" << std::endl << std::endl;
}

int main(int argc, const char *argv[])
{
    const char *filePath = "large_file.txt";
    generateLargeRandomFile(filePath);
    testReadFromDiskSpeed(filePath);
    
    return 0;
}
