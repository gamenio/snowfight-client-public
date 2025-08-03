#ifndef __UNZIP_ENCRYPT_FILE_H__
#define __UNZIP_ENCRYPT_FILE_H__

#include "common/Common.h"

USING_NS_CC;

NS_BEGIN

class ZipFileInternal;

class UnzipEncryptFile
{
public:
	UnzipEncryptFile(std::string const& zipFile);
	UnzipEncryptFile(const void* buffer, ssize_t size);
	~UnzipEncryptFile();

	bool fileExists(std::string const& fileName) const;
	unsigned char* getFileData(std::string const& fileName, ssize_t* size, std::string const& password);

private:
	UnzipEncryptFile();

	bool readFileList();

	ZipFileInternal* mData;
};

NS_END

#endif // __UNZIP_ENCRYPT_FILE_H__