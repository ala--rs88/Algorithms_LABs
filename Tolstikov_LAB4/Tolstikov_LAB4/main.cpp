//
//  main.cpp
//  Tolstikov_LAB4
//
//  Created by Igor Karpov on 11/13/15.
//  Copyright © 2015 Igor Karpov. All rights reserved.
//

#include <iostream>
#include <fstream>

#define TEST_RUNS_COUNT 3

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
    std::streamsize bytesRead = 0;
    const auto startTime = std::clock();

    long bufferSize = 1024 * 1024 * 1024;
    char* buffer = new char[bufferSize];
    std::ifstream infile (filePath,std::ifstream::binary);
    while(infile.read(buffer, bufferSize))
    {
        bytesRead += infile.gcount();
    }
    delete[] buffer;
    infile.close();
    
    const auto endTime = std::clock();
    
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
    
    std::cout << "Read from disk speed test finised. Test runs count: " << TEST_RUNS_COUNT << std::endl << ". Average read speed: " << averageSpeed << " megabytes/sec" << std::endl << std::endl;
}


double getAccessTimeToDisk(const char *filePath)
{
    int randomReadsCount = 1000;
    int readGroupSize = (1024*1024*1024 / randomReadsCount);
    
    long bufferSize = 1;
    char* buffer = new char[bufferSize];
    
    std::ifstream infile (filePath,std::ifstream::binary);
    
    const auto startTime = std::clock();
    for (int i = 0; i < randomReadsCount; i++)
    {
        int alreadySeeked = i * readGroupSize;
        int seekInCurrentGroup = rand() % (readGroupSize - 1);
        
        infile.seekg(alreadySeeked + seekInCurrentGroup, std::ios::beg);
        infile.read(buffer, bufferSize);
    }
    const auto endTime = std::clock();
    
    delete[] buffer;
    infile.close();
    
    double totalTimeInMillisecs = (double(endTime - startTime) / CLOCKS_PER_SEC) * 1000;
    double accessTime = totalTimeInMillisecs / randomReadsCount;

    return accessTime;
}

void testAccessTimeToDisk(const char *filePath)
{
    std::cout << "Access time to disk test started. File path: " << filePath << std::endl;
    
    double sumSpeed = 0;
    for (int i = 0; i < TEST_RUNS_COUNT; i++)
    {
        double speed = getAccessTimeToDisk(filePath);
        sumSpeed += speed;
        std::cout << speed << std::endl;
    }
    double averageSpeed = sumSpeed / TEST_RUNS_COUNT;
    
    std::cout << "Access time to disk test finised. Test runs count: " << TEST_RUNS_COUNT << std::endl << ". Average access time: " << averageSpeed << " millisec" << std::endl << std::endl;
}

double getAccessToDiskSpeed(const char *filePath)
{
    std::streamsize bytesRead = 0;
    //int randomReadsCount = 1000;
    //int readGroupSize = (1024*1024*1024 / randomReadsCount);
    
    long bufferSize = 1;
    char* buffer = new char[bufferSize];
    
    std::ifstream infile (filePath,std::ifstream::binary);
    
    const auto startTime = std::clock();
    while (bytesRead < 1024)
    {
        int seekPosition = rand() % (1024*1024*1024 - 1000);
        
        infile.seekg(seekPosition, std::ios::beg);
        infile.read(buffer, bufferSize);
        bytesRead += bufferSize;
    }
    const auto endTime = std::clock();
    
    delete[] buffer;
    infile.close();
    
    double totalTimeInSecs = double(endTime - startTime) / CLOCKS_PER_SEC;
    double speed = ((double)bytesRead / (1024*1024)) / totalTimeInSecs;
    
    return speed;
}

void testAccessToDiskSpeed(const char *filePath)
{
    std::cout << "Access to disk speed test started. File path: " << filePath << std::endl;
    
    double sumSpeed = 0;
    for (int i = 0; i < TEST_RUNS_COUNT; i++)
    {
        double speed = getAccessToDiskSpeed(filePath);
        sumSpeed += speed;
        std::cout << speed << std::endl;
    }
    double averageSpeed = sumSpeed / TEST_RUNS_COUNT;
    
    std::cout << "Access to disk speed test finised. Test runs count: " << TEST_RUNS_COUNT << std::endl << ". Average access speed: " << averageSpeed << " megabytes/sec" << std::endl << std::endl;
}


int main(int argc, const char *argv[])
{
    const char *filePath = "/Users/IgorKarpov/Documents/BSU_II/large_file_XXX.txt";
    //generateLargeRandomFile(filePath);
    testReadFromDiskSpeed(filePath);
    std::cout << " --------------------------------  --------------------------------  -------------------------------- " << std::endl;
    testAccessTimeToDisk(filePath);
    std::cout << " --------------------------------  --------------------------------  -------------------------------- " << std::endl;
    testAccessToDiskSpeed(filePath);
    
    return 0;
}
