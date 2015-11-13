#include <iostream>
#include <random>
#include <ctime>
#include <map>
#include <vector>

using namespace std;


//__declspec(align(16))
// __restrict



namespace {
    
    struct CacheLineDescriptor
    {
        CacheLineDescriptor()
        {
            
        }
        
        size_t tag = 0;
        size_t accessTime = 0;
        bool isInUse = false;
    };
    
    int cacheSize;
    int cacheLineSize;
    int cacheChannelsCount;
    int offsetBinaryRepresentationLength;
    int blockIndexBinaryRepresentationLength;
    int cacheBlocksCount = 0;
    int lineRewriteOption = 0;
    
    size_t cacheMissesCount = 0;
    size_t memoryAccessesCount = 0;
    map<size_t, vector<CacheLineDescriptor>> cahceBlocksMap; // in <BLOCK_INDEX, BLOCK> format, where BLOCK is vector of CACHE_LINES' descriptors
    
    int GetBinaryRepresentationLength(size_t n)
    {
        int representationLength = 0;
        while (n >>= 1)
        {
            representationLength++;
        }
        return representationLength;
    }
    
    void ResetAndInitCacheStatistics(int rewriteOption)
    {
        memoryAccessesCount = 0;
        cacheLineSize = 64;
        cacheSize = 65536;
        cacheChannelsCount = 8;
        cacheBlocksCount = cacheSize / (cacheLineSize * cacheChannelsCount);
        offsetBinaryRepresentationLength = GetBinaryRepresentationLength(cacheLineSize);
        blockIndexBinaryRepresentationLength = GetBinaryRepresentationLength(cacheBlocksCount);
        lineRewriteOption = rewriteOption;
        
        cahceBlocksMap.clear();
        
        for (int i = 0; i < cacheBlocksCount; ++i)
        {
            vector<CacheLineDescriptor> cacheCell(cacheChannelsCount);
            cahceBlocksMap.insert(pair<size_t, vector<CacheLineDescriptor>>(i, cacheCell));
        }
        cacheMissesCount = 0;
    }
    
    int FindCacheLineIndexInBlockByTag(size_t tag, vector<CacheLineDescriptor>& block) // returns -1 if not found
    {
        int index = -1;
        for (int i = 0; i < cacheChannelsCount; ++i)
        {
            if (block[i].tag == tag)
            {
                index = i;
                break;
            }
        }
        return index;
    }
    
    int FindEmptyCacheLineIndexInBlock(vector<CacheLineDescriptor>& block) // returns -1 if not found, index of first empty line otherwise
    {
        int index = -1;
        for (int i = 0; i < cacheChannelsCount; ++i)
        {
            if (!block[i].isInUse)
            {
                index = i;
                break;
            }
        }
        return index;
    }
    
    int FindIndexOfCacheLineWithMinAccessTime(vector<CacheLineDescriptor>& block)
    {
        int index = -1;
        size_t currentMinAccessTime = size_t(-1);
        for (int i = 0; i < cacheChannelsCount; ++i)
        {
            if (block[i].accessTime < currentMinAccessTime)
            {
                index = i;
                currentMinAccessTime = block[i].accessTime;
            }
        }
        return index;
    }
    
    // rewriteOption:
    //     0 -- rewrite oldest
    //     1 -- rewrite random
    int FindCacheLineIndexForNextWrite(vector<CacheLineDescriptor>& block, int rewriteOption)
    {
        int index = FindEmptyCacheLineIndexInBlock(block);
        if (index < 0) // no empty lines found, which line to rewrite?
        {
            if (rewriteOption == 0) // rewrite oldest
            {
                index = FindIndexOfCacheLineWithMinAccessTime(block);
            }
            else if (rewriteOption == 1) // rewrite random
            {
                index = rand() % 8;
            }
            
        }
        return index;
    }
    
    void RegisterMemoryAccess(size_t address)
    {
        memoryAccessesCount++;
        
        // Calculate BLOCK_INDEX and OFFSET basing on requested memory address
        size_t indexAndOffset = (address) & (~(~0 << (blockIndexBinaryRepresentationLength + offsetBinaryRepresentationLength)));
        size_t offset = indexAndOffset & (~(~0 << offsetBinaryRepresentationLength));
        size_t index = (indexAndOffset ^ offset) >> offsetBinaryRepresentationLength;
        size_t tag = address ^ indexAndOffset;
        
        // Get corresponding cache block for requested address
        vector<CacheLineDescriptor>& cahceBlock = cahceBlocksMap.at(index);
        
        int cachedLineIndex = FindCacheLineIndexInBlockByTag(tag, cahceBlock);
        if (cachedLineIndex >= 0) // hit: data is in cache
        {
            cahceBlock[cachedLineIndex].accessTime = memoryAccessesCount;
        }
        else // miss: data is not in cache
        {
            cacheMissesCount++;
            int lineToBeReplaced = FindCacheLineIndexForNextWrite(cahceBlock, lineRewriteOption);
            cahceBlock[lineToBeReplaced].tag = tag;
            cahceBlock[lineToBeReplaced].accessTime = memoryAccessesCount;
            cahceBlock[lineToBeReplaced].isInUse = true;
        }
    }
    
    void MultSimple(const float* __restrict a, const float* __restrict b, float* __restrict c, int n)
    {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                RegisterMemoryAccess((size_t)&c[i * n + j]);
                c[i * n + j] = 0.f;
                for (int k = 0; k < n; ++k) {
                    RegisterMemoryAccess((size_t)&a[i * n + k]);
                    RegisterMemoryAccess((size_t)&b[k * n + j]);
                    RegisterMemoryAccess((size_t)&c[i * n + j]);
                    RegisterMemoryAccess((size_t)&c[i * n + j]);
                    c[i * n + j] += a[i * n + k] * b[k * n + j];
                }
            }
        }
    }
    
    void FillRandom(float* a, int n)
    {
        std::default_random_engine eng;
        std::uniform_real_distribution<float> dist;
        
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                a[i * n + j] = dist(eng);
            }
        }
    }
}

void RunTest(int n, int lineRewriteOption)
{
    ResetAndInitCacheStatistics(lineRewriteOption);
    
    float* a = new float[n * n];
    float* b = new float[n * n];
    float* c = new float[n * n];
    
    FillRandom(a, n);
    FillRandom(b, n);
    
    {
        const auto startTime = std::clock();
        MultSimple(a, b, c, n);
        const auto endTime = std::clock();
        
        cout << "n: " << n << endl;
        cout << "rewrite option: " << (lineRewriteOption == 0 ? "oldest" : "random") << endl;
        cout << ((double)cacheMissesCount/memoryAccessesCount)*100 << "%" << endl;
        std::cerr << "timeSimple: " << double(endTime - startTime) / CLOCKS_PER_SEC << '\n' << endl;
    }
    
    delete[] a;
    delete[] b;
    delete[] c;
}

int main(int argc, char* argv[])
{
    RunTest(1023, 0);
    RunTest(1023, 1);
    RunTest(1024, 0);
    RunTest(1024, 1);
    RunTest(1025, 0);
    RunTest(1025, 1);
    
    cout << endl << "---------------------" << endl << endl;
    
    RunTest(1039, 0);
    RunTest(1039, 1);
    RunTest(1040, 0);
    RunTest(1040, 1);
    RunTest(1041, 0);
    RunTest(1041, 1);
    
    cout << endl << "---------------------" << endl << endl;
    
    RunTest(1050, 0);
    RunTest(1050, 1);
    RunTest(1100, 0);
    RunTest(1100, 1);
}

