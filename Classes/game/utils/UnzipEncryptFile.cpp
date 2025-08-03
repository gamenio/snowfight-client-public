#include "UnzipEncryptFile.h"

#include "unzip/unzip.h"


NS_BEGIN

// --------------------- ZipFile ---------------------
// from unzip.cpp
#define UNZ_MAXFILENAMEINZIP 256

struct ZipEntryInfo
{
	unz_file_pos pos;
	uLong uncompressed_size;
};

class ZipFileInternal
{
public:
	unzFile zipFile;

	// std::unordered_map is faster if available on the platform
	typedef std::unordered_map<std::string, struct ZipEntryInfo> FileListContainer;
	FileListContainer fileList;
};

UnzipEncryptFile::UnzipEncryptFile(std::string const& zipFile) : 
	mData(new ZipFileInternal)
{
    std::string fullPath = FileUtils::getInstance()->fullPathForFilename(zipFile);
    if (fullPath.empty())
        CCLOG("UnzipResFile: can not find %s", zipFile.c_str());
    else
        mData->zipFile = unzOpen(fullPath.c_str());

    this->readFileList();
}

UnzipEncryptFile::UnzipEncryptFile(const void* buffer, ssize_t size) :
        mData(new ZipFileInternal)
{
    mData->zipFile = unzOpenBuffer(buffer, size);
    this->readFileList();
}

bool UnzipEncryptFile::readFileList()
{
    bool ret = false;
    do
    {
        CC_BREAK_IF(!mData);
        CC_BREAK_IF(!mData->zipFile);

        // clear existing file list
        mData->fileList.clear();

        // UNZ_MAXFILENAMEINZIP + 1 - it is done so in unzLocateFile
        char szCurrentFileName[UNZ_MAXFILENAMEINZIP + 1];
        unz_file_info64 fileInfo;

        // go through all files and store position information about the required files
        int err = unzGoToFirstFile64(mData->zipFile, &fileInfo,
                                     szCurrentFileName, sizeof(szCurrentFileName) - 1);
        while (err == UNZ_OK)
        {
            unz_file_pos posInfo;
            int posErr = unzGetFilePos(mData->zipFile, &posInfo);
            if (posErr == UNZ_OK)
            {
                std::string currentFileName = szCurrentFileName;
                ZipEntryInfo entry;
                entry.pos = posInfo;
                entry.uncompressed_size = (uLong)fileInfo.uncompressed_size;
                mData->fileList[currentFileName] = entry;
            }
            // next file - also get the information about it
            err = unzGoToNextFile64(mData->zipFile, &fileInfo,
                                    szCurrentFileName, sizeof(szCurrentFileName) - 1);
        }
        ret = true;
    } while (false);

    return ret;
}

bool UnzipEncryptFile::fileExists(std::string const & fileName) const
{
	bool ret = false;
	do
	{
		CC_BREAK_IF(!mData);

		ret = mData->fileList.find(fileName) != mData->fileList.end();
	} while (false);

	return ret;
}

unsigned char* UnzipEncryptFile::getFileData(std::string const& fileName, ssize_t *size, std::string const& password)
{
	unsigned char* buffer = nullptr;
	if (size)
		*size = 0;

	do
	{
		CC_BREAK_IF(!mData->zipFile);
		CC_BREAK_IF(fileName.empty());

		ZipFileInternal::FileListContainer::const_iterator it = mData->fileList.find(fileName);
		CC_BREAK_IF(it == mData->fileList.end());

		ZipEntryInfo fileInfo = it->second;

		int nRet = unzGoToFilePos(mData->zipFile, &fileInfo.pos);
		CC_BREAK_IF(UNZ_OK != nRet);

		nRet = unzOpenCurrentFilePassword(mData->zipFile, password.c_str());
		if (nRet == UNZ_PARAMERROR)
		{
			CCLOG("UnzipResFile: Do you remove the definition of NOUNCRYPT in 'unzip.cpp'?");
			break;
		}
		else
		{
			CC_BREAK_IF(UNZ_OK != nRet);
		}

		buffer = (unsigned char*)malloc(fileInfo.uncompressed_size);
		int CC_UNUSED nSize = unzReadCurrentFile(mData->zipFile, buffer, static_cast<unsigned int>(fileInfo.uncompressed_size));
		CCASSERT(nSize == 0 || nSize == (int)fileInfo.uncompressed_size, "the file size is wrong");

		nRet = unzCloseCurrentFile(mData->zipFile);
		if (nRet != UNZ_OK)
		{
			free(buffer);
			buffer = nullptr;
			CCLOG("UnzipResFile: Can't unzip resource file '%s'. Is the password correct?", fileName.c_str());
			break;
		}

		if (size)
		{
			*size = fileInfo.uncompressed_size;
		}
	} while (0);

	return buffer;
}

UnzipEncryptFile::UnzipEncryptFile() :
	mData(new ZipFileInternal)
{
	mData->zipFile = nullptr;
}

UnzipEncryptFile::~UnzipEncryptFile()
{
	if (mData && mData->zipFile)
	{
		unzClose(mData->zipFile);
	}

	CC_SAFE_DELETE(mData);
}

NS_END
